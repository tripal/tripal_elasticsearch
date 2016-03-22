<?php

/** elastic_search_blocks form handler **/
function elastic_search_blocks_form($form, &$form_state, $elastic_search_blocks){
	//foreach($_SESSION['elastic_search_blocks'] as $key => $value){
	foreach($elastic_search_blocks as $key => $value){
		$form[$key] = $value;
		$form[$key]['#title'] = $key;
		$form[$key]['#tree'] = TRUE;
		$form[$key]['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Delete|'.$key,
			'#submit' => array('_delete_elastic_block'),
		);
	}

	return $form;
}


/**
 *elastic_search_blocks_form submit callback
*/
function _delete_elastic_block($form, &$form_state){
	$block_name = substr($form_state['values']['op'], 7);
	//unset($_SESSION['elastic_search_blocks'][$block_name]);
	$elastic_search_blocks = variable_get('elastic_search_blocks');
	unset($elastic_search_blocks[$block_name]);
	variable_set('elastic_search_blocks', $elastic_search_blocks);

	$form_state['rebuild'] = TRUE;
}


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

//	$form_state['rebuild'] = TRUE;
	//$form_state['redirect'] = 'elastic_search/search_results';
//dpm($elastic_output);

}






