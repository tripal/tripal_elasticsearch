<?php
/**
 * Render array for website_search_box_form.
 */
function website_search_box_form($form, &$form_state) {
	$form['search_box'] = array(
		'#type' => 'textfield',
		'#size' => 15,
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Search'),
	);

	return $form;
}

/**
 * website_search_box_form submit
 */
function website_search_box_form_submit($form, &$form_state) {
	try{
		libraries_load('elasticsearch-php');
		$elasticsearch_host = variable_get('elasticsearch_host');
		$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();
	} catch (\Exception $e) {

	}

	$keyword = $form_state['values']['search_box'];
	// Get all node types from node table.
	$node_types = db_query("SELECT DISTINCT(type) FROM {node}")->fetchCol('type');
	// Save search results count to an associative array with node types as keys.
	$search_website_count_by_category = array();
	foreach ($node_types as $node_type) {
		// Build search params.
		$params = build_website_search_params(
			$search_content = _remove_special_chars($keyword),
			$node_type = $node_type
		);
		// To get a total number of all search results, we need to unset 'from' and 'size' from $params.
		unset($params['from']);
		unset($params['size']);
		try{
			$count = $client->count($params)['count'];
			if ($count != 0) {
				$search_website_count_by_category[$node_type] = $count;
			}
		} catch (\Exception $e) {

		}
	}

	// Add total count to the $search_website_count_by_category array.
	$total_count = array('all categories' => array_sum($search_website_count_by_category));
	$search_website_count_by_category = $total_count + $search_website_count_by_category;

	// Save the count data and keyword to a variable.
	variable_set('website_search_by_node_type', array(
		'keyword' => $keyword,
		'count' => $search_website_count_by_category
	));

	$url = "tripal_elasticsearch/search_website/$keyword";
	// only redirect to the search results page when $keyword is not empty.
	if (empty($keyword)) {
		drupal_goto(current_path());
	} else {
		drupal_goto($url);
	}
}



/**
 * Render array to build a form for creating table search interface.
 */
function table_search_interface_building_form($form, &$form_state) {
	$form['help'] = array(
		'#type' => 'markup',
		'#markup' => t('This administrative page is used to build search blocks for 
				indexed data.'),
	);

	try{
		libraries_load('elasticsearch-php');
		$elasticsearch_host = variable_get('elasticsearch_host');
		$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();
		$existing_indices = $client->indices()->getMapping();
		if (count($existing_indices) == 0) {
			$form['markup'] = array(
				'#type' => 'markup',
				'#markup' => '<h3>No index data exists.</h3>'
			);
			return $form;
		}
	} catch (\Exception $e) {
		$form['markup'] = array(
			'#type' => 'markup',
			'#markup' => '<h3>'.$e->getMessage().'</h3>',
		);
		return $form;
	}

	$index_name_options = array(''=>'Select an index') + drupal_map_assoc(array_keys($existing_indices));
	// 'website' is reserved for website indexing and search, no need to build search
	// block for this index.
	if (isset($index_name_options['website']))
		unset($index_name_options['website']);
	$form['index_name'] = array(
		'#type' => 'select',
		'#title' => t('Select an index to build search interface'),
		'#options' => $index_name_options,
		'#ajax' => array(
			'callback' => 'tripal_elasticsearch_index_fields_ajax_callback',
			'wrapper' => 'tripal_elasticsearch_index_fields_ajax_wrapper',
		),
	);

	$index_name = isset($form_state['values']['index_name']) ? $form_state['values']['index_name'] : '';
	if (!empty($index_name)) {
		$index_profile = $existing_indices[$index_name];
		$table_name = array_keys($existing_indices[$index_name]['mappings'])[1];
		$index_fields = array_keys($index_profile['mappings']['_default_']['properties']);

		$form['table_name'] = array(
			'#type' => 'value',
			'#value' => $table_name,
		);
	} else {
		$index_fields = array();
	}

	$form['index_fields'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Select index fields'),
		'#description' => t('Only selected fields will show up on the table search interface.'),
		'#options' => drupal_map_assoc($index_fields),
		'#prefix' => '<div id="tripal_elasticsearch_index_fields_ajax_wrapper">',
		'#suffix' => '</div>',
		'#states' => array(
			'invisible' => array(
				':input[name="index_name"]' => array('value' => ''),
			),
		),
	);


	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
		'#states' => array(
			'invisible' => array(
				':input[name="index_name"]' => array('value' => ''),
			),
		),
	);

	return $form;
}


/**
 * index fields ajax callback.
 */
function tripal_elasticsearch_index_fields_ajax_callback($form, &$form_state) {
	return $form['index_fields'];
}


/**
 * table_search_interface_building_form validation.
 */
function table_search_interface_building_form_validate($form, $form_state) {
	$index_fields = array_filter($form_state['values']['index_fields']);
	if (empty($index_fields)) {
		form_set_error('index_fields', t('Please select at least one index field.'));
	}
}

/**
 * table_search_interface_building_form submit.
 */
function table_search_interface_building_form_submit($form, &$form_state) {
	$index_name = $form_state['values']['index_name'];
	$index_fields = array_filter($form_state['values']['index_fields']);
	$table_name = $form_state['values']['table_name'];
	// Delete record if it already exists in database table 'tripal_elasticsearch'.
	$sql = "SELECT index_name FROM {tripal_elasticsearch} WHERE index_name = :index_name";
	$result = db_query($sql, array(':index_name'=>$index_name))->fetchCol('index_name');
	if (!empty($result)) {
		try {
			db_delete('tripal_elasticsearch')
				->condition('index_name', $index_name)
				->execute();
			drupal_set_message('You have successfully updated search block for index -<b>'.$index_name.'</b>-.');
		} catch (\Exception $e) {
			drupal_set_message(t($e->getMessage()), 'warning');
		}

	}

	$record = array();
	foreach ($index_fields as $key=>$field) {
		$record['index_name'] = $index_name;
		$record['table_name'] = $table_name;
		$record['index_field'] = $field;
		$record['form_field_title'] = $field;
		$record['form_field_type'] = 'textfield';
		$record['form_field_description'] = '';
		$record['form_field_options'] = '';
		$record['form_field_weight'] = 0;
		try{
			drupal_write_record('tripal_elasticsearch', $record);
		} catch (\Exception $e) {
			drupal_set_message(t($e->getMessage()), 'warning');
		}
	}
}


/**
 * Form builder function for table search.
 */
function tripal_elasticsearch_build_search_block_form($form, &$form_state, $index_name) {
	$sql = "SELECT * FROM {tripal_elasticsearch} WHERE index_name = :index_name";
	$result = db_query($sql, array(':index_name'=>$index_name))->fetchAll();
	$index_fields = array();
	foreach ($result as $record) {
		switch ($record->form_field_type) {
			case 'textfield':
				$form[$record->index_field] = array(
					'#type' => 'textfield',
					//'#title' => t($record->form_field_title),
					'#field_suffix' => t('<b>'.$record->form_field_title.'</b>'),
					'#size' => 25,
					'#weight' => $record->form_field_weight,
					'#ajax' => array(
						'callback' => 'search_content_ajax_callback',
						'wrapper' => 'search_content_ajax_wrapper',
						'keypress' => FALSE,
					),
				);
				break;
			case 'select':
				$form[$record->index_field] = array(
					'#type' => 'select',
					//'#title' => t($record->form_field_title),
					'#field_suffix' => t('<b>'.$record->form_field_title.'</b>'),
					'#options' => drupal_map_assoc(range(0,10)),
					'#weight' => $record->form_field_weight,
					'#ajax' => array(
						'callback' => 'search_content_ajax_callback',
						'wrapper' => 'search_content_ajax_wrapper',
						'keypress' => FALSE,
					),
				);
				break;
		}


		if (isset($form_state['values'][$record->index_field])) {
			$index_fields[$record->index_field] = $form_state['values'][$record->index_field];
		}
	}

	$index_type = $record->table_name;
	$count = '';
	$window_range = range(1,1);
	if (count($index_fields) > 0) {
		// Build search query for table search.
		$query = build_search_query_from_field_content_pairs($index_fields);
		// Build table search params.
		$params = build_table_search_params($index = $index_name, $type = $index_type, $query);
		unset($params['size']);
		unset($params['from']);
		try{
			libraries_load('elasticsearch-php');
			$elasticsearch_host = variable_get('elasticsearch_host');
			$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();
			$count = $client->count($params)['count'];
			$window_range = range(1, ceil($count/1000));
			$count = '<font color="red"><b>'.$count.'</b> results match your search.</font>';
		} catch (\Exception $e) {
			drupal_set_message($e->getMessage(), 'warning');
		}
	}

	$form['count'] = array(
		'#type' => 'markup',
		'#markup' => t($count),
		'#prefix' => '<div id="count_ajax_wrapper">',
		'#suffix' => '</div>',
		'#weight' => 99,
	);


	$form['select_window'] = array(
		'#title' => t('Please select a window number to display results.'),
		'#description' => t('To improve search performance, 
				a window with 1000 results is used to display search results. For example, 
				the 1th window will display the first 1000 search records. The 4th window
				will display search records from 4001 to 5000.'),
		'#type' => 'select',
		'#options' => drupal_map_assoc($window_range),
		'#field_prefix' => 'show',
		'#field_suffix' => 'th 1000 results',
		'#prefix' => '<div id="select_window_ajax_wrapper">',
		'#suffix' => '</div>',
		'#weight' => 100,
	);

	$form['index_name'] = array(
		'#type' => 'hidden',
		'#value' => $index_name,
	);

	$form['actions'] = array(
		'#type' => 'actions',
		'#weight' => 99,
	);
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Show Search Results'),
	);
	$form['actions']['download_table'] = array(
		'#type' => 'submit',
		'#value' => t('Download Table'),
	);


	$form['#method'] = 'get';
	return $form;
}


/**
 * form submit for tripal_elasticsearch_build_search_block_form.
 */
function tripal_elasticsearch_build_search_block_form_submit($form, &$form_state) {
	$form_state['rebuild'] = TRUE;
}

/**
 * ajax search_content_ajax_callback
 */
function search_content_ajax_callback($form, &$form_state) {
	$commands = array();
	$commands[] = ajax_command_replace('#count_ajax_wrapper', drupal_render($form['count']));
	$commands[] = ajax_command_replace("#select_window_ajax_wrapper", drupal_render($form['select_window']));
	return array('#type' => 'ajax', '#commands' => $commands);
}


/**
 * Form builder for table_search_block_altering_form.
 */
function table_search_block_altering_form($form, &$form_state) {
	// Get all index_name from database table tripal_elasticsearch.
	$sql = "SELECT DISTINCT index_name FROM {tripal_elasticsearch}";
	$search_blocks = db_query($sql)->fetchCol('index_name');

	if (empty($search_blocks)) {
		$form['no_search_block'] = array(
			'#type' => 'markup',
			'#markup' => t('No search block is available. Please build search block first.')
		);
		return $form;
	}

	$form['search_block'] = array(
		'#type' => 'select',
		'#title' => t('Search blocks'),
		'#options' => array('' => 'Select a search block') + drupal_map_assoc($search_blocks),
		'#ajax' => array(
			'callback' => 'search_block_name_ajax_callback',
			'wrapper' => 'search_block_alter_ajax_wrapper',
		)
	);

	$form['field_wrapper'] = array(
		'#type' => 'fieldset',
		'#title' => t('Available fields'),
		'#prefix' => '<div id="search_block_alter_ajax_wrapper">',
		'#suffix' => '</div>',
		'#states' => array(
			'invisible' => array(
				':input[name="search_block"]' => array('value' => '')
			),
		),
		'#tree' => TRUE,
	);

	$search_block = isset($form_state['values']['search_block']) ? $form_state['values']['search_block']:'';
	if (!empty($search_block)) {
		$sql = "SELECT * FROM {tripal_elasticsearch} WHERE index_name = :index_name";
		$result = db_query($sql, array(':index_name' => $search_block));
		foreach ($result as $record) {
			$field = $record->index_field;
			$form['field_wrapper'][$field] = array(
				'#type' => 'fieldset',
				'#title' => t('Alter field: <b>'.$field.'</b>'),
				'#collapsible' => TRUE,
				'#collapsed' => TRUE,
			);
			$form['field_wrapper'][$field]['title'] = array(
				'#type' => 'textfield',
				'#title' => t('Available fields'),
				'#title' => t('Field title'),
			);
			$form['field_wrapper'][$field]['description'] = array(
				'#type' => 'textarea',
				'#title' => t('Field description'),
				'#description' => t('Enter some text to show users some searching examples.'),
			);
			$form['field_wrapper'][$field]['type'] = array(
				'#type' => 'select',
				'#title' => t('Field type'),
				'#options' => drupal_map_assoc(array(
					'textfield',
					'select',
				)),
				'#default_value' => 'textfield',
			);
			$form['field_wrapper'][$field]['options'] = array(
				'#type' => 'textarea',
				'#title' => t('Options for select field'),
				// display options field only when the type field value is 'select'.
				'#states' => array(
					'visible' => array(
						":input[name=\"field_wrapper[$field][type]\"]" => array('value' => 'select')
					),
				),
			);
			$form['field_wrapper'][$field]['weight'] = array(
				'#type' => 'select',
				'#title' => t('Field weight'),
				'#options' => drupal_map_assoc(range(0,20)),
			);

		}
	}

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
		'#states' => array(
			'invisible' => array(
				':input[name="search_block"]' => array('value' => '')
			),
		),
	);
	return $form;
}


/**
 * ajax callback search_block_name_ajax_callback
 */
function search_block_name_ajax_callback($form, &$form_state) {
	return $form['field_wrapper'];

//	$commands = array();
//	$commands[] = ajax_command_replace('#search_block_alter_ajax_wrapper', $form['field_wrapper']);
//	return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Form submit for table_search_block_altering_form.
 */
function table_search_block_altering_form_submit($form, &$form_state) {
	$index_name = $form_state['values']['search_block'];
	$sql = "SELECT DISTINCT table_name FROM {tripal_elasticsearch}";
	$table_name = db_query($sql)->fetchCol('table_name')[0];

	// Delete old records.
	db_delete("tripal_elasticsearch")
		->condition('index_name', $index_name)
		->execute();

	$index_fields = array_keys($form_state['values']['field_wrapper']);

	$record = array();
	foreach ($index_fields as $field) {
		$record['index_name'] = $index_name;
		$record['table_name'] = $table_name;
		$record['index_field'] = $field;
		$record['form_field_title'] = $form_state['values']['field_wrapper'][$field]['title'];
		$record['form_field_type'] = $form_state['values']['field_wrapper'][$field]['type'];
		$record['form_field_description'] = $form_state['values']['field_wrapper'][$field]['description'];
		$record['form_field_options'] = $form_state['values']['field_wrapper'][$field]['options'];
		$record['form_field_weight'] = $form_state['values']['field_wrapper'][$field]['weight'];
		drupal_write_record('tripal_elasticsearch', $record);
	}

	drupal_set_message('Search block settings for <b>'.$index_name.'</b> has been successfully updated.');
	$form_state['rebuild'] = TRUE;
}
