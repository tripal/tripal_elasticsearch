<?php




/** elastic_search_form handler.**/
function elastic_search_block_build_form($form, &$form_state) {

    // You have to create a new $client to get access to the elasticsearch
    $client = new Elasticsearch\Client();
    $mappings = $client->indices()->getMapping();
    $indices = array_keys($mappings);


	// A form field with type of checkbox so that users can select
	// which tables they want to search
	// $indices is an array consisting of all indexed tables
	$form['search_block'] = array(
		'#type' => 'fieldset',
		'#title' => t('Build search block'),
        '#prefix' => '<div id="search_block">',
        '#suffix' => '</div>',
	);
    // Add search block name
    $form['search_block']['search_block_name'] = array(
        '#type' => 'textfield',
        '#description' => t('Please input a name for the search block you want to add to the search interface.'),
        '#title' => t('Name of elastic search block'),
    );
	
	$form['search_block']['tables'] = array(
		'#type' => 'fieldset',
		'#title' => t('Select tables for search'),
	);
	/*
	if(!isset($form_state['storage']['table_list'])){
		foreach($indices as $table){
			$form_state['storage']['table_list'][$table] = $table;
		}
	}
	*/
	$form['search_block']['tables']['indexed_tables'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Select tables for searching'),
		'#description' => t('You can select one single table or join multiple tables for searching.
			However, only relational tables can be joined.'),
		// indices are named by indexed table names
		'#options' => elastic_search_get_table_list_options(),
		'#ajax' => array(
			'callback' => 'search_block_ajax_callback',
			'wrapper' => 'search_block',
		),
	);

	//-----------------------------------------------------------------

	$form['search_block']['fields'] = array(
		'#type' => 'fieldset',
		'#title' => t('Select fields for searching'),
	);
	// Create an array to store all selected fields
	$all_selected_fields = array();
	if(!empty($form_state['values']['indexed_tables'])){
    	foreach($form_state['values']['indexed_tables'] as $selected_table){
    		// check if $key is an alphanumeric character.
    		if(!empty($selected_table)){
    			// get list of fields for the selected table
    			$fields = array_keys($mappings[$selected_table]['mappings'][$selected_table]['properties']);
				// note that this form field is within a for loop.
				// It creates a form field for each table, with table fields as being displayed 
				foreach($fields as $key=>$field){
					unset($fields[$key]);
					$fields[$selected_table.'|'.$field] = $selected_table.'|'.$field;
				}

    			$form['search_block']['fields'][$selected_table] = array(
    				'#type' => 'checkboxes',
    				'#title' => t("$selected_table"),
    				'#options' => $fields,
    				'#ajax' => array(
    					'callback' => 'search_block_ajax_callback',
    					'wrapper' => 'search_block'
    				),
    			);
    
				// $form_state['values'][$selected_table] is an array of keys of
				// selected fields from table $selected_table
    			foreach($form_state['values'][$selected_table] as $field){
    				if(!empty($field)){
    					$all_selected_fields[$selected_table][] = $field;
    				}
    			};			

    
    		}
    	}

	}

	

	// Build search box for all selected fields
	$form['search_block']['all_selected_fields'] = array(
		'#type' => 'fieldset',
		'#title' => t('Elastic search block'),
	);		


	// for loop through the first level of $all_selected_fields to
	// get the selected tables
	foreach($all_selected_fields as $table){
		// Second level of for loop gets the selected fields for a given selected table
		foreach($table as $field){
				// build keyword box for each table field
				$form['search_block']['all_selected_fields'][$field] = array(
					'#type' => 'textfield',
					'#title' => $field,
				);
		}
	}


	$form['search_block']['build_search_boxes'] = array(
		'#type' => 'submit',
		'#value' => 'Add elasticsearch block',
		'#submit' => array('_add_elastic_search_block'),
	);	
	

	$form['#validate'][] = '_search_block_name_validate';
	
	return $form;	
}





/**
 * AJAX callback function to generate all fields for a selected table
 */
function search_block_ajax_callback($form, &$form_state){
    return $form;
}



/**
 * search block name validation
 */
function _search_block_name_validate($form, &$form_state){
	if(empty($form_state['values']['search_block_name'])){
		form_set_error('search_block_name', t('Search block name cannot be empty'));
	}
	if(preg_match("/\\s/", $form_state['values']['search_block_name'])){
		form_set_error('search_block_name', t('Space is not allowed in search block name.'));
	}

}


/**
 * elastic_search_form submit callback
 */
function _add_elastic_search_block($form, &$form_state){
	$block_name = $form_state['values']['search_block_name'];
	// store form element for building block list and search interface.
	$_SESSION['elastic_search_blocks'][$block_name] = $form['search_block']['all_selected_fields'];
	variable_set('elastic_search_blocks', $_SESSION['elastic_search_blocks']);

	$form_state['redirect'] = 'admin/config/search/elastic_search/search_blocks';
}


// generate table_list options
function elastic_search_get_table_list_options(){
    // You have to create a new $client to get access to the elasticsearch
    $client = new Elasticsearch\Client();
    $mappings = $client->indices()->getMapping();
    $indices = array_keys($mappings);
    foreach($indices as $table){
        $table_list[$table] = $table;
    }

	return $table_list;	
}



