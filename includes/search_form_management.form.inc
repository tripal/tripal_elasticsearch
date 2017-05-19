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

