<?php



function elastic_search_interface_form($form, &$form_state){
	foreach($_SESSION['elastic_search_blocks'] as $key => $value){
		$form[$key] = array(
			'#type' => 'fieldset',
			'#title' => $key,
			'#tree' => TRUE,
		);
		foreach(element_children($value) as $form_field){
			$form[$key][$form_field] = array(
				'#type' => 'textfield',
				'#title' => $form_field,
			);
		}
		

		$form[$key]['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Elasticsearch|'.$key,
			'#submit' => array('_elastic_search_run'),
		);
	}
	return $form;
}




/**
 *elastic_search_interface_form submit callback
 */
function _elastic_search_run($form, &$form_state){
	$block_name = substr($form_state['values']['op'], strlen('Elasticsearch|'));
	// obtain fields from table 'elastic_search_interface'
	$result = db_select('elastic_search_interface', 'e')
				->fields('e', array('table_name'))
				->condition('search_block_name', $block_name, '=')
				->execute()
				->fetchCol(0);
	$tables = explode(',', $result[0]);

	$search_block = $form_state['values'][$block_name];
	array_pop($search_block);
	

	$fields_mix = array();
	foreach($search_block as $key=>$keyword){
		$table_to_field = explode('|', $key);
		$table = $table_to_field[0];
		$field = $table_to_field[1];
		$fields_mix[$table][$field] = $keyword;
	}




    // Build query statement, just need to replace some of this query string
    // with the input form data
    $body_header =  '{"query" : {"bool" : {"must" : [';
    $query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
    $body_end = ']}}}';


	foreach($fields_mix as $table=>$fields){
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
        $params['type'] = $table;
        $params['size'] = 100000;
        $params['body'] = $body_header.$output.$body_end; 
        //===run elasticsearch================  
        //++++++++++++++++++++++++++++++++++++
        $client = new Elasticsearch\Client();
        $search_result = $client->search($params);
//dpm($search_result);
	}

		foreach($search_result['hits']['hits'] as $key=>$value){
			foreach($fields as $field=>$keyword){
				$elastic_output[$table][$key][$field] = $value['_source'][$field];
			}
			$primary_key = get_primary_key($table);
			if(!empty($primary_key)){
				$elastic_output[$table][$key][$primary_key] = $value['_source'][$primary_key];
			} 
		}


	$_SESSION['search_output_table'] = $elastic_output[$table];

//	$form_state['rebuild'] = TRUE;
	$form_state['redirect'] = 'elastic_search/search_results';

}

