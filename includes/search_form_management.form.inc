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
	$keyword = $form_state['values']['search_box'];
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

	$index_name_options = array_merge(array(''=>'Select an index'), drupal_map_assoc(array_keys($existing_indices)));
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
					'#description' => $record->form_field_description,
//					'#field_suffix' => t('<b>'.$record->form_field_title.'</b>'),
					'#title' => t($record->form_field_title),
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
				$select_options_text = $record->form_field_options;
//				$select_options_pairs = preg_split("/[\s,]+/", $select_options_text, -1, PREG_SPLIT_NO_EMPTY);
				preg_match_all('/\[.+?\]/', $select_options_text, $matches);
				$select_options_pairs = $matches[0];
				$select_options_keys = array_map(function($string) {
					$string = preg_replace('/\[/', '', $string);
					return explode('|', $string)[0];
				}, $select_options_pairs);
				$select_options_values = array_map(function($string) {
					$string = preg_replace('/\]/', '', $string);
					return explode('|', $string)[1];
				}, $select_options_pairs);
				$select_options = array_merge(array('' => 'Select'), array_combine($select_options_keys, $select_options_values));
				$form[$record->index_field] = array(
					'#type' => 'select',
					'#description' => $record->form_field_description,
//					'#field_suffix' => t('<b>'.$record->form_field_title.'</b>'),
					'#title' => t($record->form_field_title),
					'#options' => $select_options,
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
	$window_range = array('' => 0);
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
			$window_range = array_merge(array('' => 0), range(1, ceil($count/1000)));
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
		'#weight' => 97,
	);


	$form['select_window'] = array(
		'#title' => t('Please select a window number to display results.'),
		'#description' => t('To improve search performance, 
				a window with 1000 results is used to display search results. For example, 
				the 1th window will display the first 1000 search records. The 4th window
				will display search records from 4001 to 5000.'),
		'#type' => 'select',
		'#options' => drupal_map_assoc($window_range),
		'#field_prefix' => 'show the',
		'#field_suffix' => 'th 1000 results',
		'#prefix' => '<div id="select_window_ajax_wrapper">',
		'#suffix' => '</div>',
		'#weight' => 98,
		'#ajax' => array(
			'callback' => 'select_window_ajax_callback',
		),
	);



	$form['save_to_file'] = array(
		'#type' => 'radios',
		'#title' => t('Do you want to save your results into a file?'),
		'#options' => array(
			'yes' => 'yes',
			'no' => 'no',
		),
		'#states' => array(
			'invisible' => array(
				':input[name="select_window"]' => array('value' => '0'),
			),
		),
		'#ajax' => array(
			'callback' => 'save_to_file_ajax_callback',
		),
		'#weight' => 99,
	);

	$form['index_name'] = array(
		'#type' => 'hidden',
		'#value' => $index_name,
	);

//	$form['actions'] = array(
//		'#type' => 'actions',
//		'#weight' => 100,
//	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Show Search Results'),
		'#states' => array(
			'visible' => array(
				':input[name="save_to_file"]' => array('checked' => TRUE),
			),
		),
		'#weight' => 100,
	);
//	$form['actions']['download_table'] = array(
//		'#type' => 'submit',
//		'#value' => t('Download Table'),
//	);

	$form['#method'] = 'get';

	return $form;
}

function tripal_elasticsearch_build_search_block_form_validate($form, &$form_state) {

}

/**
 * Form validation for tripal_elasticsearch_build_search_block_form.
 */
function tripal_elasticsearch_build_search_block_form_submit($form, &$form_state, $index_name) {

}

/**
 * Ajax callback for select_window.
 */
function select_window_ajax_callback($form, &$form_state) {
	return $form['select_window'];
}

/**
 * Ajax callback for save_to_file.
 */
function save_to_file_ajax_callback($form, &$form_state) {
	return $form['save_to_file'];
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
		'#options' => array_merge(array('' => 'Select a search block') , drupal_map_assoc($search_blocks)),
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
		$index_name = $search_block;
		// Get table name and then query the table in database for unique values for specific fields.
		$table_name = db_query("SELECT DISTINCT table_name FROM {tripal_elasticsearch} LIMIT 1")
			->fetchCol('table_name')[0];
		$result = db_query($sql, array(':index_name' => $index_name));
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
				'#description' => t('Enter some text to show users some searching examples,
						or give some searching instructions.'),
			);

			// Get unique values for this field in the database table.
			$sql = "SELECT DISTINCT $field FROM {$table_name} LIMIT 51";
			$field_values = db_query($sql)->fetchCol();
			$type_options = (count($field_values) > 50)? array('textfield' => 'textfield') : array (
				'textfield' => 'textfield',
				'select' => 'select',
			);
			$form['field_wrapper'][$field]['type'] = array(
				'#type' => 'select',
				'#title' => t('Field type'),
				'#description' => t('The field type in the search form. <font color="red"><b>textfield
					</b></font> indicates an input box. <font color="red"><b>select</b></font> indicates
					a dropdown menu box. If this field has more than 50 unique values in the database table,
					only the <font color="red"><b>textfield</b></font> will be available.'),
				'#options' => $type_options,
				'#default_value' => 'textfield',
			);

			asort($field_values);
			array_walk($field_values, function(&$value, &$key) {
				$value = "[$value|$value]";
			});
			$options_text = implode(', ', $field_values);
			$form['field_wrapper'][$field]['options'] = array(
				'#type' => 'textarea',
				'#title' => t('Options for select field'),
				'#description' => t('This is used to convert an input box to a dropdown menu box
							in your search form. Enter key-value pairs into this box in the format of
							<font color="red"><b>key|value</b></font>. The keys should be the true values
							in your database table, and the values can be anything that you want to show
							to your users. <font color="red"><b>key|value pairs</b></font> have to be placed within
							the <font color="red"><b>[ ] (example: [key|value])</b></font>.'),
				'#default_value' => $options_text,
				// display options field only when the type field value is 'select'.
				'#states' => array(
					'invisible' => array(
						":input[name=\"field_wrapper[$field][type]\"]" => array('value' => 'textfield')
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


/**
 * Form builder for table_search_block_altering_form.
 */
function link_results_to_pages_form($form, &$form_state) {
	// Give an example to help users.
	$form['help'] = array(
		'#type' => 'fieldset',
		'#title' => t('Example and Instruction'),
		'#collapsible' => TRUE,
		'#collapsed' => TRUE,
	);
	$help_markup = '<p>This is administrative page allows you to add an url to the table
									search results. Below is an example to show you how to associate
									urls with table fields.</p>';
	$help_markup .= '<p>You can add either a <font color="red"><b>consistent</b></font> url 
									(see the organism_id field below) or a <font color="red"><b>variable</b></font> 
									url (see the abbreviation field below). Variable urls are built with search results 
									from specified fields. <font color="red"><b>Field names</b></font> within the 
									<font color="red"><b>[ ]</b></font> in the url will be replaced with corresponding 
									field search results.</p>';
	$form['help']['help'] = array(
		'#type' => 'markup',
		'#markup' => t($help_markup),
	);
	$form['help']['example_form'] = array(
		'#type' => 'fieldset',
		'#title' => t('Available fields'),
	);
	$form['help']['example_form']['organism_id'] = array(
		'#type' => 'textfield',
		'#field_suffix' => 'organism_id',
		'#attributes' => array(
			'placeholder' => 'https://www.ncbi.nlm.nih.gov',
		),
	);
	$form['help']['example_form']['organism_abbreviation'] = array(
		'#type' => 'textfield',
		'#field_suffix' => 'abbreviation',
		'#attributes' => array(
			'placeholder' => 'organism/[genus]/[species]',
		),
	);
	$form['help']['example_form']['organism_genus'] = array(
		'#type' => 'textfield',
		'#field_suffix' => 'genus',
	);
	$form['help']['example_form']['organism_species'] = array(
		'#type' => 'textfield',
		'#field_suffix' => 'species',
	);


	// Get all index_name from database table tripal_elasticsearch.
	$sql = "SELECT DISTINCT index_name FROM {tripal_elasticsearch}";
	$search_blocks = db_query($sql)->fetchCol('index_name');

	if (empty($search_blocks)) {
		$form['no_search_block'] = [
			'#type' => 'markup',
			'#markup' => t('No search block is available. Please build search block first.')
		];
		return $form;
	}

	$form['search_block'] = [
		'#type' => 'select',
		'#title' => t('Search blocks'),
		'#options' => array_merge(array('' => 'Select a search block'), drupal_map_assoc($search_blocks)),
		'#ajax' => [
			'callback' => 'link_results_to_pages_ajax_callback',
			'wrapper' => 'link_results_to_pages_ajax_wrapper',
		]
	];

	$form['field_wrapper'] = array(
		'#type' => 'fieldset',
		'#title' => t('Available fields'),
		'#tree' => TRUE,
		'#states' => array(
			'invisible' => array(
				':input[name="search_block"]' => array('value' => ''),
			),
		),
		'#prefix' => '<div id="link_results_to_pages_ajax_wrapper">',
		'#suffix' => '</div>',
	);

	$search_block = isset($form_state['values']['search_block']) ? $form_state['values']['search_block']:'';
	if (!empty($search_block)) {
		$sql = "SELECT * FROM {tripal_elasticsearch} WHERE index_name = :index_name";
		$index_name = $search_block;
		$result = db_query($sql, array(':index_name' => $index_name));
		foreach ($result as $record) {
			$field = $record->index_field;
			$form['field_wrapper'][$field] = array(
				'#type' => 'textfield',
				'#field_suffix' => $field,
			);
		}
	}

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
		'#states' => array(
			'invisible' => array(
				':input[name="search_block"]' => array('value' => ''),
			),
		),
	);

	return $form;
}


/**
 * Ajax callback for link_results_to_pages_form.
 */
function link_results_to_pages_ajax_callback($form, &$form_state) {
	return $form['field_wrapper'];
}


/**
 * Form submit for link_results_to_pages_form.
 */
function link_results_to_pages_form_submit($form, &$form_state) {
	$index_name = $form_state['values']['search_block'];
	$sql = "SELECT DISTINCT table_name FROM {tripal_elasticsearch}";
	$table_name = db_query($sql)->fetchCol('table_name')[0];
	$record = array();
	// Delete old links first.
	db_delete('tripal_elasticsearch_links')
		->condition('index_name', $index_name)
		->execute();
	foreach ($form_state['values']['field_wrapper'] as $field => $url) {
		$record['index_name'] = $index_name;
		$record['table_name'] = $table_name;
		$record['index_field'] = $field;
		$record['field_url'] = $url;
		drupal_write_record('tripal_elasticsearch_links', $record);
	}

	drupal_set_message(t('Links have been successfully updated.'));
	$form_state['rebuild'] = TRUE;
}