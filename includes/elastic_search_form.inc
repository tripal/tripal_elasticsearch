<?php




/**
 * Generate the search form.
 */
function elastic_search_form($form, &$form_state) {

    //=====Create a table to display indexed tables and their fields===========
    $rows = array();
    $header = array('Indexed table', 'Indexed fields');
    // You have to create a new $client to get access to the elasticsearch
    $client = new Elasticsearch\Client();
    $mappings = $client->indices()->getMapping();
    $indices = array_keys($mappings);
    foreach($indices as $index){
        $fields = array_keys($mappings[$index]['mappings'][$index]['properties']);
        if(!empty($fields)){
            $rows[] = array($index, implode(', ', $fields));
        //  $variables['elastic_indexing_form'][$index] = $fields;
        }
    }
    $variables['elastic_indexing_form']['table'] = theme('table', array('header' => $header, 'rows' => $rows ));
    $variables['elastic_indexing_form']['table'] .= theme('pager');
    //=========================================================================



	// A form field with type of checkbox so that users can select
	// which tables they want to search
	// $indices is an array consisting of all indexed tables
	$form['elastic_indexed'] = array(
		'#type' => 'fieldset',
		'#title' => t('Select tables for search'),
	);
	$form['elastic_indexed']['indexed_tables'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Select tables for searching'),
		'#description' => t('You can select one single table or join multiple tables for searching.
			However, only relational tables can be joined.'),
		// indices are named by indexed table names
		'#options' => $indices,
		'#ajax' => array(
			'callback' => 'table_dependent_fields_callback',
			'wrapper' => 'fields_replace',
		),
	);

	// Form fieldsets to display indexed fields for searching.
	$form['elastic_search'] = array(
		'#type' => 'fieldset',
		'#title' => t('Select fields for searching'),
		'#prefix' => '<div id="fields_replace">',
		'#suffix' => '</div>',
	);


	// Create an array to store all selected fields
	$all_selected_fields = array();
	if(!empty($form_state['values']['indexed_tables'])){
    	foreach($form_state['values']['indexed_tables'] as $key){
    		// check if $key is an alphanumeric character.
    		if(ctype_alnum($key)){
    			$selected_table = $indices[$key];
    			// get list of fields for the selected table
    			$fields = array_keys($mappings[$selected_table]['mappings'][$selected_table]['properties']);
				// note that this form field is within a for loop.
				// It creates a form field for each table, with table fields as being displayed 
    			$form['elastic_search'][$selected_table] = array(
    				'#type' => 'checkboxes',
    				'#title' => t("$selected_table"),
    				'#options' => $fields,
    				'#ajax' => array(
    					'callback' => 'all_fields_callback',
    					'wrapper' => 'all_fields_replace'
    				),
    			);
    
				// Selected fields are stored as:
				/* $all_selected_fields = array{
							'table_1' => array('field_1', 'field_2', '...'),
							'table_2' => array('field_1', 'field_2', '...'),
					}
				*/
				// $form_state['values'][$selected_table] is an array of keys of
				// selected fields from table $selected_table
    			foreach($form_state['values'][$selected_table] as $key){
    				if(ctype_alnum($key)){
    					$all_selected_fields[$selected_table][] = $fields[$key];
    				}
    				
    			};			
    
    		}

			//======= Useful variable ==============
			//||    							  ||			
			//======================================
    		// Store selected table and fields
    		$form_state['all_selected_fields'] = $all_selected_fields;
    		//========================================================
    
    	}

	}

	




	// Build search box for all selected fields
	$form['all_selected_fields'] = array(
		'#type' => 'fieldset',
		'#title' => t('Enter keywords for searching'),
		'#prefix' => '<div id="all_fields_replace">',
		'#suffix' => '</div>',
	);		


	// for loop through the first level of $all_selected_fields to
	// get the selected tables
	foreach($all_selected_fields as $table){
		// Second level of for loop gets the selected fields for a given selected table
		foreach($table as $field){
				// build search method form field for the corresponding table field

			//	$form['all_selected_fields']['query_'.$field] = array(
			//		'#type' => 'select',
			//		'#title' => 'Query: '.$field,
			//		'#options' => drupal_map_assoc(array(t('query_string'))),
					//'#options' => drupal_map_assoc(array(t('query_string'), t('match'), t('fuzzy'), t('Ascending'))),
			//	);

				// build keyword box for each table field
				$form['all_selected_fields'][$field] = array(
					'#type' => 'textfield',
					'#title' => $field,
				);
		}
	}


	$form['build_search_boxes'] = array(
		'#type' => 'submit',
		'#value' => 'Elasticsearch',
		'#submit' => array('_run_elastic_search'),
	);	
	
	return $form;	
}





/**
 * AJAX callback function to generate all fields for a selected table
 */
function table_dependent_fields_callback($form, &$form_state){
    return $form['elastic_search'];
}


/**
 * AJAX callback function to generate searching box for selected fields
 */
function all_fields_callback($form, &$form_state){
    return $form['all_selected_fields'];
}






//===========================================================================
/**
 * Call the submit callback to get form values and implement elastic search
 */
//===========================================================================
function _run_elastic_search($form, &$form_state){


	// Get all the selected table and selected fields
	// remember that $form_state['all_selected_fields'] is a two level array:
	// first level contains the selected tables
	// second level contains the selected fields
	//================ Useful variable ===============================
	//||															||
	//================================================================
	$_SESSION['all_selected_fields'] = $form_state['all_selected_fields'];
	//===================================================================

	// Get query method and keywords for each field
	foreach($_SESSION['all_selected_fields'] as $table){
		foreach($table as $field){

			//$query_name = 'query_'.$field;	No need for query name.
			//============ Useful variable ====================================
			//||															 ||
			//=================================================================
//			$_SESSION['query'][$query_name] = $form_state['values'][$query_name];
			//=================================================================

			//=========== Useful variable =====================================
			//||															 ||
			//=================================================================
			$_SESSION['keywords'][$field] = $form_state['values'][$field];   
			//=================================================================
		}
	}







	//====== AT THIS STAGE, WE HAVE ALL THE DATA WE NEED TO IMPLEMENT ELASTICSEARCH========
	//======	$_SESSION['all_selected_fields']	========== 	Field name
	//====== 	$_SESSION['query'][$query_name]		========== 	Query method
	//======	$_SESSION['keywords'][$field]		==========	Keywords
	//===+++++++++++++++++++++++++++++++++++++++++++++++===================================
	
	
	
	


	//==================== The start of elasticsearch =========================
	//+++++++++++++++++++++++++++++++++++++++++++++ +++++++++++++++++++++++++++
	
	// Build query statement, just need to replace some of this query string
	// with the input form data
	$body_header =  '{"query" : {"bool" : {"must" : [';
	$query_sentence = '{"query_string" : {"default_field": "_field_", "query": "_keyword_"}},';
	$body_end = ']}}}';
		
	
	//Find common field to join tables 
	if(!empty($_SESSION['all_selected_fields'])){
	
	    $all_selected_tables = array_keys($_SESSION['all_selected_fields']);
	
		// The following two lines are needed for retrieving all fields from selected table later
	    $client = new Elasticsearch\Client();
	    $mappings = $client->indices()->getMapping();
		//======================================================================================

	    switch(count($all_selected_tables)){


			// When only one table is selected    
	    	case 1:
				$table_1 = $all_selected_tables[0];
	            $table_1_fields = array_keys($mappings[$table_1]['mappings'][$table_1]['properties']);
				//=====================================
				// Implement elasticsearch on $table_1
				//+++++++++++++++++++++++++++++++++++++	
		    	$params['index'] = $table_1 ;
				$params['type'] = $table_1;
				$params['size'] = 100000;
	
				// replace the query string with corresponding data (query method, field name, keywords) 
		    	$output = '';
		    	foreach($_SESSION['all_selected_fields'][$table_1] as $field){
		    		$search = array("_field_", "_keyword_");
		    		$replace = array($field, $_SESSION['keywords'][$field]);
					// Don't insert query sentences when keyword is empty
					if(!empty($_SESSION['keywords'][$field])){
		    			$replaced_query_sentence = str_replace($search, $replace, $query_sentence);
		    			$output .= $replaced_query_sentence;
					}
	
				}
				$output = rtrim($output, ',');
	
				$params['body'] = $body_header.$output.$body_end;    
	
				// display 100000 records
				$params['size'] = 100000;
	
	
				//===run elasticsearch================	
				//++++++++++++++++++++++++++++++++++++
				$client = new Elasticsearch\Client();
				$elastic_table_1_results = $client->search($params);
	
	
	
				// remove unselected fields from search results		
				foreach($elastic_table_1_results['hits']['hits'] as $key=>$search_hit){
						foreach($_SESSION['all_selected_fields'][$table_1] as $field){
							$search_output_table_1[$key][$field] = $search_hit['_source'][$field]; 
						}
	//					$search_output_table_1[$key]['score'] = $search_hit['_score'];
				}
	
	
				$_SESSION['search_output_table'] = $search_output_table_1;
	
	
	
	
	
	
									
				break;


	    	case 2:
	    		$table_1 = $all_selected_tables[0];
	    		$table_1_fields = array_keys($mappings[$table_1]['mappings'][$table_1]['properties']);
	    		$table_2 = $all_selected_tables[1];
	    		$table_2_fields = array_keys($mappings[$table_2]['mappings'][$table_2]['properties']);
				
				$common_field = array_intersect($table_1_fields, $table_2_fields);
				// $common_field is an array, but I need it to be a string
				$common_field = implode($common_field);
	
				if(!empty($common_field)){
	
					//=====================================
					// Implement elasticsearch on $table_1
					//+++++++++++++++++++++++++++++++++++++	
			    	$params['index'] = $table_1 ;
					$params['type'] = $table_1;
					$params['size'] = 100000;
	
					// replace the query string with corresponding data (query method, field name, keywords) 
			    	$output = '';
			    	foreach($_SESSION['all_selected_fields'][$table_1] as $field){
			    		$search = array("_field_", "_keyword_");
			    		$replace = array($field, $_SESSION['keywords'][$field]);
						// Don't insert query sentences when keyword is empty
						if(!empty($_SESSION['keywords'][$field])){
			    			$replaced_query_sentence = str_replace($search, $replace, $query_sentence);
			    			$output .= $replaced_query_sentence;
						}
	
					}
					$output = rtrim($output, ',');
	
					$params['body'] = $body_header.$output.$body_end;    
	
					// display 100000 records
					$params['size'] = 100000;
	
	
					//===run elasticsearch================	
					//++++++++++++++++++++++++++++++++++++
					$client = new Elasticsearch\Client();
					$elastic_table_1_results = $client->search($params);
				
	
	
	
	
				
					//=====================================
					// Implement elasticsearch on $table_2
					//+++++++++++++++++++++++++++++++++++++	
			    	$params['index'] = $table_2 ;
					$params['type'] = $table_2;
					$params['size'] = 100000;
	
					// replace the query string with corresponding data (query method, field name, keywords) 
			    	$output = '';
			    	foreach($_SESSION['all_selected_fields'][$table_2] as $field){
			    		$search = array("_field_", "_keyword_");
			    		$replace = array($field, $_SESSION['keywords'][$field]);
						// Don't insert query sentences when keyword is empty
						if(!empty($_SESSION['keywords'][$field])){
			    			$replaced_query_sentence = str_replace($search, $replace, $query_sentence);
			    			$output .= $replaced_query_sentence;
						}
	
					}
					$output = rtrim($output, ',');
	
					$params['body'] = $body_header.$output.$body_end;    
	
	
	
					//===run elasticsearch================	
					//++++++++++++++++++++++++++++++++++++
					$client = new Elasticsearch\Client();
					$elastic_table_2_results = $client->search($params);
	
	
	
					// Add the $common_field to the list of selected fields for each table if it is not selected
					// Add to $table_1
					if(!in_array($common_field, $_SESSION['all_selected_fields'][$table_1])){
						$_SESSION['all_selected_fields'][$table_1][] = $common_field;
					}
					// remove unselected fields from search results
					foreach($elastic_table_1_results['hits']['hits'] as $key=>$search_hit){
							foreach($_SESSION['all_selected_fields'][$table_1] as $field){
								$search_output_table_1[$key][$field] = $search_hit['_source'][$field]; 
							}
					//		$search_output_table_1[$key]['score'] = $search_hit['_score'];
					}
					
	
	
	
	
	
	
	
					// Add to $table_2
					if(!in_array($common_field, $_SESSION['all_selected_fields'][$table_2])){
						$_SESSION['all_selected_fields'][$table_2][] = $common_field;
					}
					// remove unselected fields from search results
					foreach($elastic_table_2_results['hits']['hits'] as $key=>$search_hit){
							foreach($_SESSION['all_selected_fields'][$table_2] as $field){
								$search_output_table_2[$key][$field] = $search_hit['_source'][$field]; 
							}
					//		$search_output_table_2[$key]['score'] = $search_hit['_score'];
					}
		
	

	
					//================= get primary keys for $table_1 and $table_2 ============== 
					$primary_key_table_1 = get_primary_key($table_1);
					$primary_key_table_2 = get_primary_key($table_2);
	
	
	
					//========================= Inner join two tables ===========================
					//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					// The table which is $common_field as primary key is called right table
					// then the other one is called left table
					if($primary_key_table_1 == $common_field){
						$right_table = $search_output_table_1;
						$left_table = $search_output_table_2;
					}else{
						$right_table = $search_output_table_2;
						$left_table = $search_output_table_1;
					}
					
	
					// replace the keys of right table with the common field's value
					foreach($right_table as $key => $value){
						unset($right_table[$key]);
						$new_key = 'key_'.$value[$common_field];
						$right_table[$new_key] = $value;
					}				
	
	
					// merge the right table's elements to the left table
					// use the common field's value from the left table as the key value for the right table
					$inner_join_table = array();
					foreach($left_table as $value){
						$key = 'key_'.$value[$common_field];
						if(isset($right_table[$key])){
							$inner_join_table[] = array_merge($value, $right_table[$key]);
						}
					}				
					//===================== End of inner join two tables ===============================
					
	
					$_SESSION['search_output_table'] = $inner_join_table;
	
				
				}
				else{
					$_SESSION['search_output_table'] = array(
															0 => array('Oops!' => 'These two tables has no common field.'),
															);
				}


				break;
	
		

            case 3:
                $table_1 = $all_selected_tables[0];
                $table_1_fields = array_keys($mappings[$table_1]['mappings'][$table_1]['properties']);
                $table_2 = $all_selected_tables[1];
                $table_2_fields = array_keys($mappings[$table_2]['mappings'][$table_2]['properties']);

                $table_3 = $all_selected_tables[2];
                $table_3_fields = array_keys($mappings[$table_3]['mappings'][$table_3]['properties']);

                // We need to find the connection table, which has common field with both other two tables
                // The connection table is the one that will be elasticsearched twice.
                $table_1_elastic_count = 0;
                $table_2_elastic_count = 0;
                $table_3_elastic_count = 0;



                $common_field_12 = array_intersect($table_1_fields, $table_2_fields);
                // $common_field is an array, but I need it to be a string
                $common_field_12 = implode($common_field_12);

                $common_field_13 = array_intersect($table_1_fields, $table_3_fields);
                // $common_field is an array, but I need it to be a string
                $common_field_13 = implode($common_field_13);

                $common_field_23 = array_intersect($table_2_fields, $table_3_fields);
                // $common_field is an array, but I need it to be a string
                $common_field_23 = implode($common_field_23);


				$empty_cnt = empty($common_field_12) + empty($common_field_13) + empty($common_field_23);
				if($empty_cnt > 1){
						$_SESSION['search_output_table'] = array(
															0 => array('Oops!' => 'At least one table has no common field with any other tables.'),
															);
				}
				else{

	                //================= table_1 vs table_2 ================================================
	                //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 
	
	                if(!empty($common_field_12)){
	
	                    //=====================================
	                    // Implement elasticsearch on $table_1
	                    //+++++++++++++++++++++++++++++++++++++ 
	                    $params['index'] = $table_1 ;
	                    $params['type'] = $table_1;
	                    $params['size'] = 100000;
	
	                    // replace the query string with corresponding data (query method, field name, keywords) 
	                    $output = '';
	                    foreach($_SESSION['all_selected_fields'][$table_1] as $field){
	                        $search = array("_field_", "_keyword_");
	                        $replace = array($field, $_SESSION['keywords'][$field]);
	                        // Don't insert query sentences when keyword is empty
	                        if(!empty($_SESSION['keywords'][$field])){
	                            $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
	                            $output .= $replaced_query_sentence;
	                        }
	
	                    }
	                    $output = rtrim($output, ',');
	
	                    $params['body'] = $body_header.$output.$body_end;
	
	                    // display 100000 records
	                    $params['size'] = 100000;
	
	
	                    //===run elasticsearch================  
	                    //++++++++++++++++++++++++++++++++++++
	                    $client = new Elasticsearch\Client();
	                    $elastic_table_1_results = $client->search($params);
	                    $table_1_elastic_count++;
	
	
		
	                    //=====================================
	                    // Implement elasticsearch on $table_2
	                    //+++++++++++++++++++++++++++++++++++++ 
	                    $params['index'] = $table_2 ;
	                    $params['type'] = $table_2;
	                    $params['size'] = 100000;
	
	                    // replace the query string with corresponding data (query method, field name, keywords) 
	                    $output = '';
	                    foreach($_SESSION['all_selected_fields'][$table_2] as $field){
	                        $search = array("_field_", "_keyword_");
	                        $replace = array($field, $_SESSION['keywords'][$field]);
	                        // Don't insert query sentences when keyword is empty
	                        if(!empty($_SESSION['keywords'][$field])){
	                            $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
	                            $output .= $replaced_query_sentence;
	                        }
	
	                    }
	                    $output = rtrim($output, ',');
	
	                    $params['body'] = $body_header.$output.$body_end;
	
	
	
	                    //===run elasticsearch================  
	                    //++++++++++++++++++++++++++++++++++++
	                    $client = new Elasticsearch\Client();
	                    $elastic_table_2_results = $client->search($params);
	                    $table_2_elastic_count++;
	
	
		
	
	                    // Add the $common_field to the list of selected fields for each table if it is not selected
	                    // Add to $table_1
	                    if(!in_array($common_field_12, $_SESSION['all_selected_fields'][$table_1])){
	                        $_SESSION['all_selected_fields'][$table_1][] = $common_field_12;
	                    }
	                    // remove unselected fields from search results
	                    foreach($elastic_table_1_results['hits']['hits'] as $key=>$search_hit){
	                            foreach($_SESSION['all_selected_fields'][$table_1] as $field){
	                                $search_output_table_1[$key][$field] = $search_hit['_source'][$field];
	                            }
	                    //        $search_output_table_1[$key]['score'] = $search_hit['_score'];
	                    }
	
	
	
	
	
	
	
	
	                    // Add to $table_2
	                    if(!in_array($common_field_12, $_SESSION['all_selected_fields'][$table_2])){
	                        $_SESSION['all_selected_fields'][$table_2][] = $common_field_12;
	                    }
	                    // remove unselected fields from search results
	                    foreach($elastic_table_2_results['hits']['hits'] as $key=>$search_hit){
	                            foreach($_SESSION['all_selected_fields'][$table_2] as $field){
	                                $search_output_table_2[$key][$field] = $search_hit['_source'][$field];
	                            }
	                    //        $search_output_table_2[$key]['score'] = $search_hit['_score'];
	                    }
	
	
	                }
	
	                //================= End of table_1 vs table_2 ============================
	                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	
	
	                //================= table_1 vs table_3 ================================================
	                //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	                if(!empty($common_field_13)){
	
	                    //=====================================
	                    // Implement elasticsearch on $table_1
	                    //+++++++++++++++++++++++++++++++++++++ 
	                    $params['index'] = $table_1 ;
	                    $params['type'] = $table_1;
	                    $params['size'] = 100000;
	
	                    // replace the query string with corresponding data (query method, field name, keywords) 
	                    $output = '';
	                    foreach($_SESSION['all_selected_fields'][$table_1] as $field){
	                        $search = array("_field_", "_keyword_");
	                        $replace = array($field, $_SESSION['keywords'][$field]);
	                        // Don't insert query sentences when keyword is empty
	                        if(!empty($_SESSION['keywords'][$field])){
	                            $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
	                            $output .= $replaced_query_sentence;
	                        }
	
	                    }
	                    $output = rtrim($output, ',');
	
	                    $params['body'] = $body_header.$output.$body_end;
	
	                    // display 100000 records
	                    $params['size'] = 100000;
	
	
	                    //===run elasticsearch================  
	                    //++++++++++++++++++++++++++++++++++++
	                    $client = new Elasticsearch\Client();
	                    $elastic_table_1_results = $client->search($params);
	                    $table_1_elastic_count++;
	
	
	
	
	
	                    //=====================================
	                    // Implement elasticsearch on $table_3
	                    //+++++++++++++++++++++++++++++++++++++ 
	                    $params['index'] = $table_3 ;
	                    $params['type'] = $table_3;
	                    $params['size'] = 100000;
	
	                    // replace the query string with corresponding data (query method, field name, keywords) 
	                    $output = '';
	                    foreach($_SESSION['all_selected_fields'][$table_3] as $field){
	                        $search = array("_field_", "_keyword_");
	                        $replace = array($field, $_SESSION['keywords'][$field]);
	                        // Don't insert query sentences when keyword is empty
	                        if(!empty($_SESSION['keywords'][$field])){
	                            $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
	                            $output .= $replaced_query_sentence;
	                        }
	
	                    }
	                    $output = rtrim($output, ',');
	
	                    $params['body'] = $body_header.$output.$body_end;
	
	
	                    //===run elasticsearch================  
	                    //++++++++++++++++++++++++++++++++++++
	                    $client = new Elasticsearch\Client();
	                    $elastic_table_3_results = $client->search($params);
	                    $table_3_elastic_count++;
	
	
	                    	
	
	                    // Add the $common_field to the list of selected fields for each table if it is not selected
	                    // Add to $table_1
	                    if(!in_array($common_field_13, $_SESSION['all_selected_fields'][$table_1])){
	                        $_SESSION['all_selected_fields'][$table_1][] = $common_field_13;
	                    }
	                    // remove unselected fields from search results
	                    foreach($elastic_table_1_results['hits']['hits'] as $key=>$search_hit){
	                            foreach($_SESSION['all_selected_fields'][$table_1] as $field){
	                                $search_output_table_1[$key][$field] = $search_hit['_source'][$field];
	                            }
	                     //       $search_output_table_1[$key]['score'] = $search_hit['_score'];
	                    }
	
		
	
	                    // Add to $table_3
	                    if(!in_array($common_field_13, $_SESSION['all_selected_fields'][$table_3])){
	                        $_SESSION['all_selected_fields'][$table_3][] = $common_field_13;
	                    }
	                    // remove unselected fields from search results
	                    foreach($elastic_table_3_results['hits']['hits'] as $key=>$search_hit){
	                            foreach($_SESSION['all_selected_fields'][$table_3] as $field){
	                                $search_output_table_3[$key][$field] = $search_hit['_source'][$field];
	                            }
	                    //        $search_output_table_3[$key]['score'] = $search_hit['_score'];
	                    }
	
	
	                }
	
	                //================= End of table_1 vs table_3 ============================
	                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	
	
	               //================= table_2 vs table_3 ================================================
	                //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	                if(!empty($common_field_23)){
	
	                    //=====================================
	                    // Implement elasticsearch on $table_2
	                    //+++++++++++++++++++++++++++++++++++++ 
	                    $params['index'] = $table_2 ;
	                    $params['type'] = $table_2;
	                    $params['size'] = 100000;
	
	                    // replace the query string with corresponding data (query method, field name, keywords) 
	                    $output = '';
	                    foreach($_SESSION['all_selected_fields'][$table_2] as $field){
	                        $search = array("_field_", "_keyword_");
	                        $replace = array($field, $_SESSION['keywords'][$field]);
	                        // Don't insert query sentences when keyword is empty
	                        if(!empty($_SESSION['keywords'][$field])){
	                            $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
	                            $output .= $replaced_query_sentence;
	                        }
	
	                    }
	                    $output = rtrim($output, ',');
	
	                    $params['body'] = $body_header.$output.$body_end;
	
	                    // display 100000 records
	                    $params['size'] = 100000;
	
	
	                    //===run elasticsearch================  
	                    //++++++++++++++++++++++++++++++++++++
	                    $client = new Elasticsearch\Client();
	                    $elastic_table_2_results = $client->search($params);
	                    $table_2_elastic_count++;
	
	
	
	                    //=====================================
	                    // Implement elasticsearch on $table_3
	                    //+++++++++++++++++++++++++++++++++++++ 
	                    $params['index'] = $table_3 ;
	                    $params['type'] = $table_3;
	                    $params['size'] = 100000;
	
	                    // replace the query string with corresponding data (query method, field name, keywords) 
	                    $output = '';
	                    foreach($_SESSION['all_selected_fields'][$table_3] as $field){
	                        $search = array("_field_", "_keyword_");
	                        $replace = array($field, $_SESSION['keywords'][$field]);
	                        // Don't insert query sentences when keyword is empty
	                        if(!empty($_SESSION['keywords'][$field])){
	                            $replaced_query_sentence = str_replace($search, $replace, $query_sentence);
	                            $output .= $replaced_query_sentence;
	                        }
	
	                    }
	                    $output = rtrim($output, ',');
	
	                    $params['body'] = $body_header.$output.$body_end;
	
	
	
	                    //===run elasticsearch================  
	                    //++++++++++++++++++++++++++++++++++++
	                    $client = new Elasticsearch\Client();
	                    $elastic_table_3_results = $client->search($params);
	                    $table_3_elastic_count++;
	
	
	
	
	
	                    // Add the $common_field to the list of selected fields for each table if it is not selected
	                    // Add to $table_2
	                    if(!in_array($common_field_23, $_SESSION['all_selected_fields'][$table_2])){
	                        $_SESSION['all_selected_fields'][$table_2][] = $common_field_23;
	                    }
	                    // remove unselected fields from search results
	                    foreach($elastic_table_2_results['hits']['hits'] as $key=>$search_hit){
	                            foreach($_SESSION['all_selected_fields'][$table_2] as $field){
	                                $search_output_table_2[$key][$field] = $search_hit['_source'][$field];
	                            }
	                      //      $search_output_table_2[$key]['score'] = $search_hit['_score'];
	                    }
	
	
	
	
	
	
	
	
	                    // Add to $table_3
	                    if(!in_array($common_field_23, $_SESSION['all_selected_fields'][$table_3])){
	                        $_SESSION['all_selected_fields'][$table_3][] = $common_field_23;
	                    }
	                    // remove unselected fields from search results
	                    foreach($elastic_table_3_results['hits']['hits'] as $key=>$search_hit){
	                            foreach($_SESSION['all_selected_fields'][$table_3] as $field){
	                                $search_output_table_3[$key][$field] = $search_hit['_source'][$field];
	                            }
	                        //    $search_output_table_3[$key]['score'] = $search_hit['_score'];
	                    }
	
	
	                }
	
	                //================= End of table_2 vs table_3 ============================
	                //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	
	
	
	
	
	
					//============== Get primary keys for $table_1, $table_2 and $table_3 =====
					//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					$primary_key_table_1 = get_primary_key($table_1);
					$primary_key_table_2 = get_primary_key($table_2);
					$primary_key_table_3 = get_primary_key($table_3);
	
	
					
	
	                //=========== Inner join $table_1, $table_2 and $table_3====================
					$inner_join_table = array();
	                // when $table_1 is the connecting table
	                if($table_1_elastic_count == 2){
						// first round table joining
				        if($primary_key_table_1 == $common_field_12){
	        	            $right_table = $search_output_table_1;
	            	        $left_table = $search_output_table_2;
	              		}else{
							$right_table = $search_output_table_2;
	                    	$left_table = $search_output_table_1;
	                	}
	
	
						foreach($right_table as $key => $value){
	                    	unset($right_table[$key]);
	                    	$new_key = 'key_'.$value[$common_field_12];
	                    	$right_table[$new_key] = $value;
	                	}
	
	
						$firt_round_inner_join_table = array();
	                	foreach($left_table as $value){
	                    	$key = 'key_'.$value[$common_field_12];
	                    	if(isset($right_table[$key])){
	                        	$first_round_inner_join_table[] = array_merge($value, $right_table[$key]);
	                    	}
	                	}
	
						
						// second round table joining
						if($primary_key_table_3 == $common_field_13){
							$right_table = $search_output_table_3;
							$left_table = $first_round_inner_join_table;
						}else{
							$right_table = $first_round_inner_join_table;
							$left_table = $search_output_table_3;
						}
	
	
						foreach($right_table as $key => $value){
	                    	unset($right_table[$key]);
	                    	$new_key = 'key_'.$value[$common_field_13];
	                    	$right_table[$new_key] = $value;
	                	}
	
	
	
						$second_round_inner_join_table = array();
	                	foreach($left_table as $value){
	                    	$key = 'key_'.$value[$common_field_13];
	                    	if(isset($right_table[$key])){
	                        	$second_round_inner_join_table[] = array_merge($value, $right_table[$key]);
	                    	}
	                	}
	
	
	
						$_SESSION['search_output_table'] = $second_round_inner_join_table;
	
	
	
	                }
					// when $table_2 is the connecting table
					elseif($table_2_elastic_count == 2){
						// first round table joining
				        if($primary_key_table_2 == $common_field_12){
	        	            $right_table = $search_output_table_2;
	            	        $left_table = $search_output_table_1;
	              		}else{
							$right_table = $search_output_table_1;
	                    	$left_table = $search_output_table_2;
	                	}
	
	
						foreach($right_table as $key => $value){
	                    	unset($right_table[$key]);
	                    	$new_key = 'key_'.$value[$common_field_12];
	                    	$right_table[$new_key] = $value;
	                	}
	
	
						$firt_round_inner_join_table = array();
	                	foreach($left_table as $value){
	                    	$key = 'key_'.$value[$common_field_12];
	                    	if(isset($right_table[$key])){
	                        	$first_round_inner_join_table[] = array_merge($value, $right_table[$key]);
	                    	}
	                	}
	
						
						// second round table joining
						if($primary_key_table_3 == $common_field_23){
							$right_table = $search_output_table_3;
							$left_table = $first_round_inner_join_table;
						}else{
							$right_table = $first_round_inner_join_table;
							$left_table = $search_output_table_3;
						}
	
	
						foreach($right_table as $key => $value){
	                    	unset($right_table[$key]);
	                    	$new_key = 'key_'.$value[$common_field_23];
	                    	$right_table[$new_key] = $value;
	                	}
	
	
	
						$second_round_inner_join_table = array();
	                	foreach($left_table as $value){
	                    	$key = 'key_'.$value[$common_field_13];
	                    	if(isset($right_table[$key])){
	                        	$second_round_inner_join_table[] = array_merge($value, $right_table[$key]);
	                    	}
	                	}
	
	
	
						$_SESSION['search_output_table'] = $second_round_inner_join_table;
	
	
	                }
					// when $table_3 is the connecting table
					else{
						// first round table joining
				        if($primary_key_table_3 == $common_field_13){
	        	            $right_table = $search_output_table_3;
	            	        $left_table = $search_output_table_1;
	              		}else{
							$right_table = $search_output_table_1;
	                    	$left_table = $search_output_table_3;
	                	}
	
	
						foreach($right_table as $key => $value){
	                    	unset($right_table[$key]);
	                    	$new_key = 'key_'.$value[$common_field_13];
	                    	$right_table[$new_key] = $value;
	                	}
	
	
	
						$firt_round_inner_join_table = array();
	                	foreach($left_table as $value){
	                    	$key = 'key_'.$value[$common_field_13];
	                    	if(isset($right_table[$key])){
	                        	$first_round_inner_join_table[] = array_merge($value, $right_table[$key]);
	                    	}
	                	}
	
						
						// second round table joining
						if($primary_key_table_2 == $common_field_23){
							$right_table = $search_output_table_2;
							$left_table = $first_round_inner_join_table;
						}else{
							$right_table = $first_round_inner_join_table;
							$left_table = $search_output_table_2;
						}
	
	
						foreach($right_table as $key => $value){
	                    	unset($right_table[$key]);
	                    	$new_key = 'key_'.$value[$common_field_23];
	                    	$right_table[$new_key] = $value;
	                	}
	
	
	
						$second_round_inner_join_table = array();
	                	foreach($left_table as $value){
	                    	$key = 'key_'.$value[$common_field_23];
	                    	if(isset($right_table[$key])){
	                        	$second_round_inner_join_table[] = array_merge($value, $right_table[$key]);
	                    	}
	                	}
	
	
	
						$_SESSION['search_output_table'] = $second_round_inner_join_table;
	
	                }
	

				}
	
				break;

	    }
	    
	
	}
	
	
	
	//====================== The end of elasticsearch ==========================
	//++++++++++++++++++++++ The end of elasticsearch ++++++++++++++++++++++++++	



	$form_state['redirect'] = 'elastic_search/search_results';

}//========================= The end of form submit function ====================
//++++++++++++++++++++++++++ The end of form submit function ++++++++++++++++++++




