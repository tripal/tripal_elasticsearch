<?php

	
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


				//================= table_1 vs table_2 ================================================
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				$common_field_12 = array_intersect($table_1_fields, $table_2_fields);
				// $common_field is an array, but I need it to be a string
				$common_field_12 = implode($common_field);
	
	
				if(!empty($common_field_12)){
	
					//=====================================
					// Implement elasticsearch on $table_1
					//+++++++++++++++++++++++++++++++++++++	
			    	$params['index'] = $table_1 ;
					$params['type'] = $table_1;
					$params['size'] = 1000000;
	
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
	
					// display 1000000 records
					$params['size'] = 1000000;
	
	
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
					$params['size'] = 1000000;
	
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
	
				
					// Display the elasticsearch results
	//				dpm($elastic_table_1_results);
	//				dpm($elastic_table_2_results);
	
	
	
	
	
	
	
	
	
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
							$search_output_table_1[$key]['score'] = $search_hit['_score'];
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
							$search_output_table_2[$key]['score'] = $search_hit['_score'];
					}
		
//					dpm($search_output_table_1);
//					dpm($search_output_table_2);
	
				}

				//================= End of table_1 vs table_2 ============================
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++






				//================= table_1 vs table_3 ================================================
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				$common_field_13 = array_intersect($table_1_fields, $table_3_fields);
				// $common_field is an array, but I need it to be a string
				$common_field_13 = implode($common_field);
	
	
				if(!empty($common_field_13)){
	
					//=====================================
					// Implement elasticsearch on $table_1
					//+++++++++++++++++++++++++++++++++++++	
			    	$params['index'] = $table_1 ;
					$params['type'] = $table_1;
					$params['size'] = 1000000;
	
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
	
					// display 1000000 records
					$params['size'] = 1000000;
	
	
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
					$params['size'] = 1000000;
	
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
	
				
					// Display the elasticsearch results
	//				dpm($elastic_table_1_results);
	//				dpm($elastic_table_3_results);
	
	
	
	
	
	
	
	
	
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
							$search_output_table_1[$key]['score'] = $search_hit['_score'];
					}
					
	
	
	
	
	
	
	
					// Add to $table_3
					if(!in_array($common_field, $_SESSION['all_selected_fields'][$table_3])){
						$_SESSION['all_selected_fields'][$table_3][] = $common_field;
					}
					// remove unselected fields from search results
					foreach($elastic_table_3_results['hits']['hits'] as $key=>$search_hit){
							foreach($_SESSION['all_selected_fields'][$table_3] as $field){
								$search_output_table_3[$key][$field] = $search_hit['_source'][$field]; 
							}
							$search_output_table_3[$key]['score'] = $search_hit['_score'];
					}
		
//					dpm($search_output_table_1);
//					dpm($search_output_table_3);
	
				}

				//================= End of table_1 vs table_3 ============================
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++







				//================= table_2 vs table_3 ================================================
				//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				$common_field_13 = array_intersect($table_2_fields, $table_3_fields);
				// $common_field is an array, but I need it to be a string
				$common_field_13 = implode($common_field);
	
	
				if(!empty($common_field_13)){
	
					//=====================================
					// Implement elasticsearch on $table_2
					//+++++++++++++++++++++++++++++++++++++	
			    	$params['index'] = $table_2 ;
					$params['type'] = $table_2;
					$params['size'] = 1000000;
	
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
	
					// display 1000000 records
					$params['size'] = 1000000;
	
	
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
					$params['size'] = 1000000;
	
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
	
	
				
					// Display the elasticsearch results
	//				dpm($elastic_table_2_results);
	//				dpm($elastic_table_3_results);
	
	
	
	
	
	
	
	
	
					// Add the $common_field to the list of selected fields for each table if it is not selected
					// Add to $table_2
					if(!in_array($common_field, $_SESSION['all_selected_fields'][$table_2])){
						$_SESSION['all_selected_fields'][$table_2][] = $common_field;
					}
					// remove unselected fields from search results
					foreach($elastic_table_2_results['hits']['hits'] as $key=>$search_hit){
							foreach($_SESSION['all_selected_fields'][$table_2] as $field){
								$search_output_table_2[$key][$field] = $search_hit['_source'][$field]; 
							}
							$search_output_table_2[$key]['score'] = $search_hit['_score'];
					}
					
	
	
	
	
	
	
	
					// Add to $table_3
					if(!in_array($common_field, $_SESSION['all_selected_fields'][$table_3])){
						$_SESSION['all_selected_fields'][$table_3][] = $common_field;
					}
					// remove unselected fields from search results
					foreach($elastic_table_3_results['hits']['hits'] as $key=>$search_hit){
							foreach($_SESSION['all_selected_fields'][$table_3] as $field){
								$search_output_table_3[$key][$field] = $search_hit['_source'][$field]; 
							}
							$search_output_table_3[$key]['score'] = $search_hit['_score'];
					}
		
//					dpm($search_output_table_2);
//					dpm($search_output_table_3);
	
				}

				//================= End of table_2 vs table_3 ============================
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


				

				//=========== Inner join $table_1, $table_2 and $table_3====================
				
				// when $table_1 is the connecting table
				if($table_1_elastic_count == 2){
					foreach($search_output_table_1 as $table_1){
						foreach($search_output_table_2 as $table_2){
							if($table_1[$common_field_12] == $table_2[$common_field_12]){
								$inner_join_table_0 = array_merge($table_1, $table_2);
							}
						}
					}

					foreach($inner_join_table_0 as $table_0){
						foreach($search_output_table_3 as $table_3){
							if($table_0[$common_field_13] == $table_3[$common_field_13]){
								$inner_join_table = array_merge($table_1, $table_3);
							}
						}
					}

				$_SESSION['search_output_table'] = $inner_join_table;

				}elseif($table_2_elastic_count == 2){
					foreach($search_output_table_2 as $table_2){
						foreach($search_output_table_1 as $table_1){
							if($table_2[$common_field_12] == $table_1[$common_field_12]){
								$inner_join_table_0 = array_merge($table_1, $table_2);
							}
						}
					}

					foreach($inner_join_table_0 as $table_0){
						foreach($search_output_table_3 as $table_3){
							if($table_0[$common_field_23] == $table_3[$common_field_23]){
								$inner_join_table = array_merge($table_2, $table_3);
							}
						}
					}

				$_SESSION['search_output_table'] = $inner_join_table;


				}else($table_3_elastic_count == 2){
					foreach($search_output_table_3 as $table_3){
						foreach($search_output_table_1 as $table_1){
							if($table_3[$common_field_13] == $table_1[$common_field_12]){
								$inner_join_table_0 = array_merge($table_1, $table_3);
							}
						}
					}

					foreach($inner_join_table_0 as $table_0){
						foreach($search_output_table_2 as $table_2){
							if($table_0[$common_field_23] == $table_2[$common_field_23]){
								$inner_join_table = array_merge($table_2, $table_3);
							}
						}
					}

				$_SESSION['search_output_table'] = $inner_join_table;



				}








	
				break;
	
	
	
