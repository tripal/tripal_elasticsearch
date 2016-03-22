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



//** implements hook_preprocess_elastic_search_interface_form(). **//
function elastic_search_preprocess_elastic_search_interface_form(&$variables){
	// shorten the form variable name for easier access.
	$form = $variables['form'];
dpm($form);

	//------------ modify form for search_transcripts---------------------------------
	// change labels for the form elements.
	$form['search_transcripts']['search_transcripts_all|abbreviation']['#title'] 	= 	t('Organism');
	$form['search_transcripts']['search_transcripts_all|abbreviation']['#type']		= 'select';
	$form['search_transcripts']['search_transcripts_all|abbreviation']['#options']	= array(
		'\"A. saccharum\"' => t('Sugar Maple (Acer saccharum)'),
		'\"C. dentata\"' => t('American Chestnut (Castanea dentat)'),
		'\"C. mollissima\"' => t('Chinese Chestnut (Castanea mollissima)'),
		'\"F. grandifolia\"' => t('American Beech (Fagus grandifolia)'),
		'\"P. serotina\"' => t('Black Cherry (Prunus serotina)'),
		'\"F. americana\"' => t('White Ash (Fraxinus americana)'),
		'\"F. pennsylvanica\"' => t('Green Ash (Fraxinus pennsylvanica)'),
		'\"G. triacanthos\"' => t('Honeylocust (Gleditsia triacanthos)'),
		'\"J. nigra\"' => t('Black Walnut (Juglans nigra)'),
		'\"L. styraciflua\"' => t('Sweetgum (Liquidambar styraciflua)'),
		'\"L. tulipifera\"' => t('Tulip Poplar (Liriodendron tulipifera)'),
		'\"N. sylvatica\"' => t('Blackgum (Nyssa sylvatica)'),
		'\"P. borbonia' => t('Redbay (Persea borbonia)'),
		'\"Q. alba\"' => t('White Oak (Quercus alba)'),
		'\"Q. robur\"' => t('Pedunculate Oak (Quercus robur)'),
		'\"Q. rubra\"' => t('Northern Red Oak (Quercus rubra)'),
	);
	//$form['search_transcripts']['search_transcripts_all|hit_best_eval']['#title'] 	= 	t('Blast hit evalue');
	//$form['search_transcripts']['search_transcripts_all|hit_best_score']['#title'] 	= 	t('Blast hit score');
	$form['search_transcripts']['search_transcripts_all|hit_description']['#title'] = 	t('Blast hit description');
	//$form['search_transcripts']['search_transcripts_all|seqlen']['#title'] 			= 	t('Sequence length');
	$form['search_transcripts']['search_transcripts_all|type']['#title'] 			= 	t('Sequence type');
	$form['search_transcripts']['search_transcripts_all|uniquename']['#title'] 		= 	t('Unique name');
	$form['search_transcripts']['submit']['#value'] = t('Elasticsearch|transcripts');

	// create variables for individual elements.
	$variables['organism'] = render($form['search_transcripts']['search_transcripts_all|abbreviation']);
	//$variables['blast_hit_eval'] = render($form['search_transcripts']['search_transcripts_all|hit_best_eval']);
	//$variables['blast_hit_score'] = render($form['search_transcripts']['search_transcripts_all|hit_best_score']);
	$variables['blast_hit_description'] = render($form['search_transcripts']['search_transcripts_all|hit_description']);
	//$variables['seqlen'] = render($form['search_transcripts']['search_transcripts_all|seqlen']);
	$variables['seqtype'] = render($form['search_transcripts']['search_transcripts_all|type']);
	$variables['uniquename'] = render($form['search_transcripts']['search_transcripts_all|uniquename']);
	$variables['search_transcripts_submit'] = render($form['search_transcripts']['submit']);



	//------------ modify form for website_node_content---------------------------------
	// change labels
	$form['website_node_content']['field_data_body|body_value']['#title'] = t('search webpages');
	$form['website_node_content']['submit']['#value'] = t('Elasticsearch|webpages');

	// create variables for individual elements
	$variables['search_webpages'] = render($form['website_node_content']['field_data_body|body_value']);
	$variables['search_webpages_submit'] = render($form['website_node_content']['submit']);


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

}


