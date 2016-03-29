<?php

/** elastic_search_interface form handler **/
function elastic_search_interface_form($form, &$form_state, $elastic_search_interface){
	//foreach($_SESSION['elastic_search_interface'] as $key => $value){
	foreach($elastic_search_interface as $key => $value){
		$form[$key] = array(
			'#type' => 'fieldset',
			'#title' => $key,
			'#tree' => TRUE,
		);
		foreach(element_children($value) as $form_field){
			$form[$key][$form_field] = array(
				'#type' => 'textfield',
				'#title' => $form_field,
				'#prefix' => '<div class="elastic-search-form-item">',
				'#suffix' => '</div>',
			);
			$form[$key]['search_'.$form_field] = array(
				'#type' => 'select',
				'#options' => array(
					'match' => 'match',
					'fuzzy' => 'fuzzy match',
					'match_phrase' => 'phrase match',
					//'sort_ascending' => 'sort by ascending',
					//'sort_descending' => 'sort by descending',
					//'range' => 'range search',
				),
				'#default_value' => 'match',
			);
		}
		

		$form[$key]['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Elasticsearch|'.$key,
			//'#submit' => array('elastic_search_interface_form_submit'),
		);
	}


    $form['from_nth_entry']['nth'] = array(
        '#type' => 'textfield',
        '#title' => t('Display 1,000 records from nth entry:'),
        '#size' => 30,
        '#prefix' => '<div class="from-nth-entry">',
        '#suffix' => '</div>',
		'#ajax' => array(
			'callback' => '_from_nth_entry_ajax',
			'wrapper' => 'from_nth_entry',
		),
        '#attributes' => array(
            'placeholder' => t('Please enter an offset number')
        ),
    );
    $form['from_nth_entry']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Go!'),
        '#submit' => array('_from_nth_entry_submit'),
		'#attributes' => array(
			'id' => array('from-nth-entry-submit'),
		),
    );


	/** download_search_results_form handler **/
    $form['download_table'] = array(
        '#type' => 'submit',
        '#value' => 'Download csv',
		'#submit' => array('_download_table_submit'), 
    );
	$form['download_fasta'] = array(
		'#type' => 'submit',
		'#value' => 'Download fasta',
		'#submit' => array('_download_fasta_submit'),
	);


	$form['#attributes']['class'][]  = 'elastic_search_interface_form';


	return $form;
}




/**
 *elastic_search_interface_form submit callback
 */
function elastic_search_interface_form_submit($form, &$form_state){
	// store $form_state['values'] for later use in _from_nth_entry_submit() callback
	variable_set('form_state_values', $form_state['values']);

	// The name of search block being submitted
	$block_name = substr($form_state['values']['op'], strlen('Elasticsearch|'));
	// get field and corresponding input keywords
	foreach($form_state['values'][$block_name] as $field=>$keyword){
		if($field != 'submit'){
			$table_field_array = explode('|', $field);
			$field_searchMethod_pairs[$table_field_array[1]][$table_field_array[0]] = $keyword;
		}
	}
	


	//--------------------run elastic search----------------------------------
    // Build query statement, just need to replace some of this query string
    // with the input form data
	$body_curl_head = '{';
    $body_head =  '"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
    $body_end = ']}}';
	$body_curl_end = '}';


	$sort_query_element = array();
	$none_sort_query_element = array();
	foreach($field_searchMethod_pairs as $field=>$searchMethod_keyword){
		$key1 = array_keys($searchMethod_keyword)[0];
		$key2 = array_keys($searchMethod_keyword)[1];
		if(strlen($key1 < strlen($key2))){
			$keyword = $searchMethod_keyword[$key1];
			$searchMethod = $searchMethod_keyword[$key2];
			$elasticsearch_query_type = $key1;
		} else {
			$keyword = $searchMethod_keyword[$key2];
			$searchMethod = $searchMethod_keyword[$key1];
			$elasticsearch_query_type = $key2;
		}
		if($searchMethod == 'sort_ascending' or $searchMethod == 'sort_descending') {
            // build query for sort query
            $sort_query_element[$field] = _build_elastic_query($searchMethod, $field, $keyword);
        } elseif(!empty($keyword)){
			// match and fuzzy query are at the same level
			$none_sort_query_element[$field] = _build_elastic_query($searchMethod, $field, $keyword);  
		}	
	}

	$sort_query_string = implode(',', $sort_query_element);
	$sort_query_head = ' "sort":[';
	$sort_query_end = ']';
	$sort_query = $sort_query_head . $sort_query_string . $sort_query_end;	

	$none_sort_query_element = implode(',', $none_sort_query_element);
	

	$body = $body_curl_head . $body_head . $none_sort_query_element.$body_end.',' . $sort_query . $body_curl_end;


	$client = new Elasticsearch\Client();
	$params['index'] = $elasticsearch_query_type;
	$params['type'] = $elasticsearch_query_type;
	$params['body'] = $body;
	$params['size'] = 1000;
    $search_result = $client->search($params);


		//------obtain the hits count--------
		unset($params_count);
		$params_count['index'] = $elasticsearch_query_type;
		$params_count['type'] = $elasticsearch_query_type;
		// $client->count does not support sort query. So sort query has to be removed.
		$body_count = $body_curl_head . $body_head . $none_sort_query_element.$body_end. $body_curl_end;
		$params_count['body'] = $body_count;
		variable_set('search_record_count', $client->count($params_count));



    foreach($search_result['hits']['hits'] as $key=>$value){
        foreach(array_keys($field_searchMethod_pairs) as $field){
        	$elastic_output[$elasticsearch_query_type][$key][$field] = $value['_source'][$field];
        }
    }


	// unset search_output_table session if it have already existed.
	if(!empty($_SESSION['search_output_table'])){
		unset($_SESSION['search_output_table']);
	}
	// display results only when elastic search results is not empty.
	if(!empty($value['_source'])){ //count how many tables involved.
		if(count($elastic_output, 0) == 1){
			$_SESSION['search_output_table'] = $elastic_output[$elasticsearch_query_type];
		}
	}

}




//** implements hook_preprocess_elastic_search_interface_form(). **//
function elastic_search_preprocess_elastic_search_interface_form(&$variables){
    // shorten the form variable name for easier access.
    $form = $variables['form'];


/**
    // create variables for individual elements.
    $variables['organism'] = render($form['search_transcripts']['search_transcripts_all|abbreviation']);
    //$variables['blast_hit_eval'] = render($form['search_transcripts']['search_transcripts_all|hit_best_eval']);
    //$variables['blast_hit_score'] = render($form['search_transcripts']['search_transcripts_all|hit_best_score']);
    $variables['blast_hit_description'] = render($form['search_transcripts']['search_transcripts_all|hit_description']);
    //$variables['seqlen'] = render($form['search_transcripts']['search_transcripts_all|seqlen']);
    $variables['seqtype'] = render($form['search_transcripts']['search_transcripts_all|type']);
    $variables['uniquename'] = render($form['search_transcripts']['search_transcripts_all|uniquename']);
    $variables['search_transcripts_submit'] = render($form['search_transcripts']['submit']);


    // create variables for individual elements
    $variables['search_webpages'] = render($form['website_node_content']['field_data_body|body_value']);
    $variables['search_webpages_submit'] = render($form['website_node_content']['submit']);
*/

	foreach(element_children($form) as $key){
		if(isset($form[$key]['#type'])){
			$type = $form[$key]['#type'];
			if($type != 'hidden' and $type != 'token'){
				$variables['elastic_search_interface'][$key] = render($form[$key]); 
			}
		}
	}

    $search_record_count = variable_get('search_record_count');
    if(!empty(variable_get('search_record_count'))){
        $variables['from_nth_entry_nth'] = render($form['from_nth_entry']['nth']);
        $variables['from_nth_entry_submit'] = render($form['from_nth_entry']['submit']);
    } else {
		$variables['from_nth_entry_nth'] = '';
		$variables['from_nth_entry_nth'] = '';
    }

    // obtain search records count.
    $count_arr = variable_get('search_record_count');
    $variables['search_record_count'] = $count_arr['count'];


	// download search results file
	//$variables['download_table'] = render($form['download_table']);
	//$variables['download_fasta'] = render($form['download_fasta']);

    // be sure to print the remaining rendered form items.
    $variables['children'] = drupal_render_children($form);


	drupal_add_css(drupal_get_path('module', 'elastic_search').'/css/elastic_search_interface_form.css');
	drupal_add_js(drupal_get_path('module', 'elastic_search').'/js/elastic_search.js');

	// add the datepicker library:
	//drupal_add_library('system', 'ui.datepicker');

}




/** _from_nth_entry form element ajax callback **/
function _from_nth_entry_ajax($form, &$form_state){
	variable_set('nth_entry', $form_state['values']['nth']);
	return $form['from_nth_entry']['nth'];
}


/** _from_nth_entry form element submit callback **/
function _from_nth_entry_submit($form, &$form_state){
	$form_state['values'] = variable_get('form_state_values');

	// The name of search block being submitted
	$block_name = substr($form_state['values']['op'], strlen('Elasticsearch|'));
	// get field and corresponding input keywords
	foreach($form_state['values'][$block_name] as $field=>$keyword){
		if($field != 'submit'){
			$table_field_array = explode('|', $field);
			$field_searchMethod_pairs[$table_field_array[1]][$table_field_array[0]] = $keyword;
		}
	}
	


	//--------------------run elastic search----------------------------------
    // Build query statement, just need to replace some of this query string
    // with the input form data
	$body_curl_head = '{';
    $body_head =  '"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
    $body_end = ']}}';
	$body_curl_end = '}';

    $sort_query_element = array();
    $none_sort_query_element = array();
	foreach($field_searchMethod_pairs as $field=>$searchMethod_keyword){
		$key1 = array_keys($searchMethod_keyword)[0];
		$key2 = array_keys($searchMethod_keyword)[1];
		if(strlen($key1 < strlen($key2))){
			$keyword = $searchMethod_keyword[$key1];
			$searchMethod = $searchMethod_keyword[$key2];
			$elasticsearch_query_type = $key1;
		} else {
			$keyword = $searchMethod_keyword[$key2];
			$searchMethod = $searchMethod_keyword[$key1];
			$elasticsearch_query_type = $key2;
		}
		if($searchMethod == 'sort_ascending' or $searchMethod == 'sort_descending') {
            // build query for sort query
            $sort_query_element[$field] = _build_elastic_query($searchMethod, $field, $keyword);
        } elseif(!empty($keyword)){
			// match and fuzzy query are at the same level
			$none_sort_query_element[$field] = _build_elastic_query($searchMethod, $field, $keyword);  
		}	
	}

	$sort_query_string = implode(',', $sort_query_element);
	$sort_query_head = ' "sort":[';
	$sort_query_end = ']';
	$sort_query = $sort_query_head . $sort_query_string . $sort_query_end;	

	$none_sort_query_element = implode(',', $none_sort_query_element);
	

	$body = $body_curl_head . $body_head . $none_sort_query_element.$body_end.',' . $sort_query . $body_curl_end;


	$client = new Elasticsearch\Client();
	$params['index'] = $elasticsearch_query_type;
	$params['type'] = $elasticsearch_query_type;
	$params['body'] = $body;
	$nth = intval(variable_get('nth_entry'));
	$params['from'] = $nth; 
	$params['size'] = 1000;
    $search_result = $client->search($params);


    foreach($search_result['hits']['hits'] as $key=>$value){
        foreach(array_keys($field_searchMethod_pairs) as $field){
        	$elastic_output[$elasticsearch_query_type][$key][$field] = $value['_source'][$field];
        }
    }


	// unset search_output_table session if it have already existed.
	if(!empty($_SESSION['search_output_table'])){
		unset($_SESSION['search_output_table']);
	}
	// display results only when elastic search results is not empty.
	if(!empty($value['_source'])){ //count how many tables involved.
		if(count($elastic_output, 0) == 1){
			$_SESSION['search_output_table'] = $elastic_output[$elasticsearch_query_type];
		}
	}


}



/** download_table form submit **/
function _download_table_submit($form, &$form_state){

	$form_state['values'] = variable_get('form_state_values');

	// The name of search block being submitted
	$block_name = substr($form_state['values']['op'], strlen('Elasticsearch|'));
	// get field and corresponding input keywords
	foreach($form_state['values'][$block_name] as $field=>$keyword){
		if($field != 'submit'){
			$table_field_array = explode('|', $field);
			$field_searchMethod_pairs[$table_field_array[1]][$table_field_array[0]] = $keyword;
		}
	}
	


	//--------------------run elastic search----------------------------------
    // Build query statement, just need to replace some of this query string
    // with the input form data
	$body_curl_head = '{';
    $body_head =  '"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
    $body_end = ']}}';
	$body_curl_end = '}';

    $sort_query_element = array();
    $none_sort_query_element = array();
	foreach($field_searchMethod_pairs as $field=>$searchMethod_keyword){
		$key1 = array_keys($searchMethod_keyword)[0];
		$key2 = array_keys($searchMethod_keyword)[1];
		if(strlen($key1 < strlen($key2))){
			$keyword = $searchMethod_keyword[$key1];
			$searchMethod = $searchMethod_keyword[$key2];
			$elasticsearch_query_type = $key1;
		} else {
			$keyword = $searchMethod_keyword[$key2];
			$searchMethod = $searchMethod_keyword[$key1];
			$elasticsearch_query_type = $key2;
		}
		if($searchMethod == 'sort_ascending' or $searchMethod == 'sort_descending') {
            // build query for sort query
            $sort_query_element[$field] = _build_elastic_query($searchMethod, $field, $keyword);
        } elseif(!empty($keyword)){
			// match and fuzzy query are at the same level
			$none_sort_query_element[$field] = _build_elastic_query($searchMethod, $field, $keyword);  
		}	
	}

	$sort_query_string = implode(',', $sort_query_element);
	$sort_query_head = ' "sort":[';
	$sort_query_end = ']';
	$sort_query = $sort_query_head . $sort_query_string . $sort_query_end;	

	$none_sort_query_element = implode(',', $none_sort_query_element);
	

	$body = $body_curl_head . $body_head . $none_sort_query_element.$body_end.',' . $sort_query . $body_curl_end;


	$client = new Elasticsearch\Client();
	$params['index'] = $elasticsearch_query_type;
	$params['type'] = $elasticsearch_query_type;
	$params['body'] = $body;
	$params['size'] = 10000000;
    $search_result = $client->search($params);




	
	// create a file pointer connected to the output stream
	$destination = file_destination('public://'.'/elastic_output.csv', FILE_EXISTS_RENAME);
	$output = fopen($destination, 'w');

    foreach($search_result['hits']['hits'] as $key=>$value){
		fputcsv($output, $value['_source']);
    }


	// strip off 'public://'
	$filename = substr($destination, 9); 
	//drupal_goto(drupal_realpath($destination));
	drupal_goto('sites/default/files/'. $filename);

	//file_unmanaged_delete($destination);

}



/** **/
function _download_fasta_submit($form, &$form_state){

}
