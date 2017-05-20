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
		$index_fields = array_keys($index_profile['mappings']['_default_']['properties']);
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