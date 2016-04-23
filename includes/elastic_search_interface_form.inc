<?php


/** elastic_search_interface form handler **/
function elastic_search_interface_form($form, &$form_state){

    // Build search blocks
    $search_block_elements = db_select('elastic_search', 'e')
                                ->fields('e')                   
                                ->execute();                    
    foreach($search_block_elements as $record){
        // Build search_block_name field only once
        if(empty($form[$record->search_block_name]['#title'])){
            $form[$record->search_block_name] = array(
                '#type' => 'fieldset',
                '#title' => $record->search_block_name,
				'#tree' => TRUE,
            );          

			$form[$record->search_block_name]['submit'] = array(
				'#type' => 'submit',
				'#value' => 'Elasticsearch|'.$record->search_block_name,
				'#weight' => '3',
			);
        }       
        // Add fields to the fieldset
		if(empty($form[$record->table_field]['#title'])){
			$form[$record->search_block_name][$record->table_field] = array(
				'#type' => 'fieldset',
			);
		}
        $form[$record->search_block_name][$record->table_field]['field'] = array(
            '#type' => 'textfield',
            '#title' => $record->table_field,
			'#title_display' => 'after',
			'#size' => '40',
        );      
        $form[$record->search_block_name][$record->table_field]['search'] = array(
            '#type' => 'select',
            '#options' => array(
                'match' => 'match',
                'fuzzy' => 'fuzzy match',
                'match_phrase' => 'phrase match',
            ),
            '#default_value' => 'fuzzy',
        );

    }   



    $form['from_nth_entry']['nth'] = array(
        '#type' => 'textfield',
        '#title' => t('Display 1,000 records from nth entry:'),
        '#size' => 30,
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

	$form['#attributes']['class'][]  = 'elastic_search_interface_form';

	return $form;
}




/**
 *elastic_search_interface_form submit callback
 */
function elastic_search_interface_form_submit($form, &$form_state){
	$search_block_name = substr($form_state['values']['op'], strlen('Elasticsearch|'));

	//--------------------run elastic search----------------------------------
    // Build query statement, just need to replace some of this query string
    // with the input form data
	$body_curl_head = '{';
    $body_head =  '"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
    $body_end = ']}}';
	$body_curl_end = '}';

	$elastic_special_chars = array('+', '-', '=', '&&', '||', '>', 
								'<', '!', '(', ')', '{', '}', '[', 
								']', '^', '"', '~', '*', '?', ':', '\\', '/');


	$elasticsearch_query_elements = array();
	foreach($form_state['values'][$search_block_name] as $field_name=>$search_data){
		if($field_name != 'submit'){
			$table = explode('|', $field_name)[0];
			$field = explode('|', $field_name)[1];
			$keyword = $search_data['field'];
			$searchMethod = $search_data['search'];

			if(!empty($keyword)){
				$keyword = str_replace($elastic_special_chars, '', $keyword);
				$keyword = trim($keyword);
				$elasticsearch_query_elements[$field] = _build_elastic_query($searchMethod, $field, $keyword);
			}
		} 
	}

    $elasticsearch_query_string = implode(',', $elasticsearch_query_elements);
    $body = $body_curl_head . $body_head . $elasticsearch_query_string.$body_end.$body_curl_end;


	// Implement search and obtain search results
    $client = new Elasticsearch\Client();
	$params = array();
    $params['index'] = $table;
    $params['type'] = $table;
    $params['body'] = $body;
    $params['size'] = 1000;
    $search_result = $client->search($params);


    // Use the same searching parameters to obtain hits count
    $client = new Elasticsearch\Client();
    $params = array();
    $params['index'] = $table;
    $params['type'] = $table;
    $params['body'] = $body;
    $search_hits_count = $client->count($params)['count'];



	$search_output_arr = array();
    foreach($search_result['hits']['hits'] as $key=>$value){
    	foreach($form_state['values'][$search_block_name] as $field_name=>$search_data){
        	if($field_name != 'submit'){
            	$field = explode('|', $field_name)[1];
				$search_output_arr[$key][$field] = $value['_source'][$field];
        	}
    	}
    }
	//---------end of running elastic search ---------------------

	// store search results in a cookie
	setcookie('search_output_arr', _elastic_search_results($search_output_arr));
	// store search parameters in a cookie
	setcookie('search_params', implode('|', $params));
	// store $search_block_name in a cookie
	setcookie('search_block_name', $search_block_name);
}




//** implements hook_preprocess_elastic_search_interface_form(). **//
function elastic_search_preprocess_elastic_search_interface_form(&$variables){
    // shorten the form variable name for easier access.
    $form = $variables['form'];


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

    // be sure to print the remaining rendered form items.
    $variables['children'] = drupal_render_children($form);


	drupal_add_css(drupal_get_path('module', 'elastic_search').'/css/elastic_search_interface_form.css');

}


/** _from_nth_entry form element submit callback **/
function _from_nth_entry_submit($form, &$form_state){

	if(!empty($_COOKIE['search_params'])){
		$search_params = explode('|', $_COOKIE['search_params']);
		$params['index'] = $search_params[0];
		$params['type'] = $search_params[1];
		$params['body'] = $search_params[2];
	
		$client = new Elasticsearch\Client();
		$params['from'] = $form_state['values']['nth']; 
	    $params['size'] = 1000;
		dpm($params);
	
	    $search_result = $client->search($params);
	
		$search_block_name = $_COOKIE['search_block_name'];
		$search_output_arr = array();
	    foreach($search_result['hits']['hits'] as $key=>$value){
	    	foreach($form_state['values'][$search_block_name] as $field_name=>$search_data){
	        	if($field_name != 'submit'){
	            	$field = explode('|', $field_name)[1];
					$search_output_arr[$key][$field] = $value['_source'][$field];
	        	}
	    	}
	    }
		//---------end of running elastic search ---------------------
	
		// store search results in a cookie
		setcookie('search_output_arr', _elastic_search_results($search_output_arr));
	
	}
}



/** download_table form submit **/
function _download_table_submit($form, &$form_state){

	if(!empty($_COOKIE['search_params'])){
		$search_params = explode('|', $_COOKIE['search_params']);
		$params['index'] = $search_params[0];
		$params['type'] = $search_params[1];
		$params['body'] = $search_params[2];
	
		$client = new Elasticsearch\Client();
	    $params['size'] = 10000000;
	
	    $search_result = $client->search($params);
	
		$search_block_name = $_COOKIE['search_block_name'];
		$search_output_arr = array();
	    foreach($search_result['hits']['hits'] as $key=>$value){
	    	foreach($form_state['values'][$search_block_name] as $field_name=>$search_data){
	        	if($field_name != 'submit'){
	            	$field = explode('|', $field_name)[1];
					$search_output_arr[$key][$field] = $value['_source'][$field];
	        	}
	    	}
	    }
		//---------end of running elastic search ---------------------

	
		// create a file pointer connected to the output stream
		$destination = file_destination('temporary://'.'/elastic_output.csv', FILE_EXISTS_RENAME);
		$output = fopen($destination, 'w');

    	foreach($search_output_arr as $value){
			fputcsv($output, $value);
    	}

		// strip off 'temporary://'
		$filename = substr($destination, 12); 
		//drupal_goto(drupal_realpath($destination));
		drupal_goto('sites/default/files/tmp/'. $filename);
	}

}



