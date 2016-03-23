<?php

/** elastic_search_interface form handler **/
function elastic_search_interface_form($form, &$form_state, $elastic_search_interface){
	//foreach($_SESSION['elastic_search_blocks'] as $key => $value){
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
				'#prefix' => '<div class="container-inline">',
				'#suffix' => '</div>',
				'#attributes' => array('class'=>array('elastic_search_form_item')),
			);
			$form[$key]['search_'.$form_field] = array(
				'#type' => 'select',
				'#options' => array(
					'match' => 'exact match',
					'sort_ascending' => 'sort by ascending',
					'sort_descending' => 'sort by descending',
					'fuzzy' => 'fuzzy match',
					'match_phrase' => 'phrase search',
					//'range' => 'range search',
				),
				'#default_value' => 'fuzzy match',
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
        '#title' => '',
        '#size' => 30,
        '#prefix' => '<div id="from_nth_entry" class="container-inline">Display 100 records from nth entry:',
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
    );




	$form['#attributes']['class'][]  = 'elastic_search_interface_form';

	// add CSS file
	$form['#attached']['css'][] = array(
		'data' => drupal_get_path('module', 'elastic_search').'/css/elastic_search_interface_form.css',
	);


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
	

dpm($field_searchMethod_pairs);

	//--------------------run elastic search----------------------------------
    // Build query statement, just need to replace some of this query string
    // with the input form data
	$body_curl_head = '{';
    $body_head =  '"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
    $body_end = ']}}';
	$body_curl_end = '}';

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
			$query_level_1[$field] = _build_elastic_query($searchMethod, $field, $keyword);  
		}	
	}

	$sort_query_string = implode(',', $sort_query_element);
	$sort_query_head = ' "sort":[';
	$sort_query_end = ']';
	$sort_query = $sort_query_head . $sort_query_string . $sort_query_end;	

	$query_level_1 = implode(',', $query_level_1);
	

	$body = $body_curl_head . $body_head . $query_level_1.$body_end.',' . $sort_query . $body_curl_end;
	dpm($body);


	$params['index'] = $elasticsearch_query_type;
	$params['type'] = $elasticsearch_query_type;
	$params['body'] = $body;
	$params['size'] = 100;
        $client = new Elasticsearch\Client();
        $search_result = $client->search($params);

dpm($search_result);
dpm($search_result['hits']['hits']);

/**
	foreach($table_field_comb_arr as $table=>$fields){
        // replace the query string with corresponding data (query method, field name, keywords) 
        $output = '';
        foreach($fields as $field=>$keyword){
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
            // Don't insert query sentences when keyword is empty
            if(!empty($keyword)){
                $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
                $output .= $replaced_query_sentence;
            }
    
        }
        $output = rtrim($output, ',');
        $params['index'] = $table;
		//$params['index'] = 'hardwoodgenomics';
        $params['type'] = $table;
        $params['size'] = 100;
		//$params['from']	= 5125630;
        $params['body'] = $body_header.$output.$body_end; 
        //===run elasticsearch================  
        //++++++++++++++++++++++++++++++++++++
        $client = new Elasticsearch\Client();
        $search_result = $client->search($params);

		//------obtain the hits count--------
		$params_count['index'] = $table;
		$params_count['type'] = $table;
		$params_count['body'] = $body_header.$output.$body_end;
		variable_set('search_record_count', $client->count($params_count));

		//elastic search return results with all fields from a document
		//but we only need to display fields we need
		$primary_key = get_primary_key($table);
		foreach($search_result['hits']['hits'] as $key=>$value){
			foreach($table_field_mix as $table_field=>$keyword){
			    $table_field_arr = explode('|', $table_field);
	        	if($table == $table_field_arr[0]){
	        		$field = $table_field_arr[1];
					$elastic_output[$table][$key][$field] = $value['_source'][$field];
					//$elastic_output[$table][$key][$primary_key] = $value['_source'][$primary_key];
				}
			}
		}
	}//------------------end of elastic search-------------------------------------


	// unset search_output_table session if it have already existed.
	if(!empty($_SESSION['search_output_table'])){
		unset($_SESSION['search_output_table']);
	}
	// display results only when elastic search results is not empty.
	if(!empty($value['_source'])){ //count how many tables involved.
		if(count($elastic_output, 0) == 1){
			$_SESSION['search_output_table'] = $elastic_output[$table];
		}
	}
	else{
		$_SESSION['search_output_table'][] = array('Oops!' => 'No records found.');
	}
*/

}



/** function to build elastic query for each field **/
function _build_elastic_query($searchMethod, $field, $keyword){
    $body_header =  '{"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}}';
    $body_end = ']}}}';

	$query_string = '';
	switch($searchMethod){
		case 'query_string':
			$query_string =	'{"query_string" : {"default_field": "_field_", "query": "_keyword_"}}';
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
            $final_query_string = str_replace($search, $replace, $query_string);
            break;
		case 'match':
			$query_string = ' {"match":{"_field_":"_keyword_"}} ';
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
            $final_query_string = str_replace($search, $replace, $query_string);
			break;
		case 'fuzzy':
			$query_string = ' {"fuzzy":{"_field_":"_keyword_"}} ';
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
            $final_query_string = str_replace($search, $replace, $query_string);
			break;
		case 'match_phrase':
			$query_string = ' {"query":{"match_phrase": {"_field_":"_keyword_"} }} ';
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
			$final_query_string = str_replace($search, $replace, $query_string);
			break;
		case 'sort_ascending':
			$query_string = ' {"_field_":{"order":"asc"}} ';
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
            $final_query_string = str_replace($search, $replace, $query_string);
			break;
		case 'sort_descending':
            $query_string = ' {"_field_":{"order":"asc"}}] ';
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
            $final_query_string = str_replace($search, $replace, $query_string);
			break;
		case 'range':
			break;
	}

	return $final_query_string;
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


    $search_record_count = variable_get('search_record_count');
    if(!empty($search_record_count)){
        $variables['from_nth_entry_nth'] = render($form['from_nth_entry']['nth']);
        $variables['from_nth_entry_submit'] = render($form['from_nth_entry']['submit']);
    } else {
        $variables['from_nth_entry'] = NULL;
    }

    // obtain search records count.
    $count_arr = variable_get('search_record_count');
    $variables['search_record_count'] = $count_arr['count'];

    // be sure to print the remaining rendered form items.
    $variables['children'] = drupal_render_children($form);

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
            $table_field_mix[$field] = $keyword;
        }
    }
    // group fields by table in case that there are more than one table involved in this search block
    foreach($table_field_mix as $table_field=>$keyword){
        $table_field_arr = explode('|', $table_field);
        $table = $table_field_arr[0];
        $field = $table_field_arr[1];
        $table_field_comb_arr[$table][$field] = $keyword;
    }

/**
    //--------------------run elastic search----------------------------------
    // Build query statement, just need to replace some of this query string
    // with the input form data
    $body_header =  '{"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
    $body_end = ']}}}';


    foreach($table_field_comb_arr as $table=>$fields){
        // replace the query string with corresponding data (query method, field name, keywords) 
        $output = '';
        foreach($fields as $field=>$keyword){
            $search = array("_field_", "_keyword_");
            $replace = array($field, $keyword);
            // Don't insert query sentences when keyword is empty
            if(!empty($keyword)){
                $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
                $output .= $replaced_query_sentence;
            }

        }
        $output = rtrim($output, ',');
        $params['index'] = $table;
        //$params['index'] = 'hardwoodgenomics';
        $params['type'] = $table;
        $params['size'] = 100;
		$nth_entry = variable_get('nth_entry');
       	$params['from'] = $nth_entry;
        $params['body'] = $body_header.$output.$body_end;
        // store $params for __from_nth_entry_submit() callback
        $_SESSION['params'] = $params;
        //===run elasticsearch================  
        //++++++++++++++++++++++++++++++++++++
        $client = new Elasticsearch\Client();
        $search_result = $client->search($params);

       
        //elastic search return results with all fields from a document
        //but we only need to display fields we need
        $primary_key = get_primary_key($table);
        foreach($search_result['hits']['hits'] as $key=>$value){
            foreach($table_field_mix as $table_field=>$keyword){
                $table_field_arr = explode('|', $table_field);
                if($table == $table_field_arr[0]){
                    $field = $table_field_arr[1];
                    $elastic_output[$table][$key][$field] = $value['_source'][$field];
                    //$elastic_output[$table][$key][$primary_key] = $value['_source'][$primary_key];
                }
            }
        }
    }//------------------end of elastic search-------------------------------------


    // unset search_output_table session if it have already existed.
    if(!empty($_SESSION['search_output_table'])){
        unset($_SESSION['search_output_table']);
    }
    // display results only when elastic search results is not empty.
    if(!empty($value['_source'])){ //count how many tables involved.
        if(count($elastic_output, 0) == 1){
            $_SESSION['search_output_table'] = $elastic_output[$table];
        }
    }
    else{
        $_SESSION['search_output_table'][] = array('Oops!' => 'No records found.');
    }
*/

}


