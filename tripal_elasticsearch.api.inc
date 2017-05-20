<?php

/**
 * Define several arrays for elasticsearch indexing settings
 */
function get_character_filter_options() {
	return drupal_map_assoc(array(
		'html_strip',
		'mapping',
		'pattern_replace',
		));
}

function get_tokenizer_options() {
	return drupal_map_assoc(array(
		'standard',
		'letter',
		'lowercase',
		'whitespace',
		'uax_url_email',
		'classic',
		'ngram',
		'edge_ngram',
		'keyword',
		'pattern',
		'path_hierarchy',
	));
}

function get_token_filter_options() {
	return drupal_map_assoc(array(
		'standard',
		'asciifolding',
		'length',
		'lowercase',
		'upppercase',
	));
}

/**
 * Return an array of Elasticsearch field mapping types.
 */
function get_field_mapping_types() {
	return drupal_map_assoc(array(
		'string',
		'keyword',
		'date',
		'long',
		'double',
		'boolean',
		'ip',
		'object',
		'nested',
		'geo_point',
		'geo_shape',
		'completion',
	));
}

/**
 * Function to build elasticsearch index.
 */
function build_elasticsearch_index($index_name,
																	 $shards = 5,
																	 $replicas = 0,
																	 $character_filters = array(),
																	 $tokenizer,
																	 $token_filters = array(),
																	 $field_mapping_types = array()) {
	$analysis = array(
		'analyzer' => array(
			$index_name => array(
				'type' => 'custom',
				'tokenizer' => $tokenizer,
//				'char_filter' => array_keys($character_filters),
				'filter' => array_keys($token_filters),
			),
//			'char_filter' => array_keys($character_filters),
//			'filter' => array_keys($token_filters),
		)
	);

	$settings = array(
		'number_of_shards' => $shards,
		'number_of_replicas' => $replicas,
		'analysis' => $analysis,
	);

	foreach ($field_mapping_types as $field=>$mapping_type) {
		$properties[$field] = array(
			'type' => $mapping_type,
			'fields' => array(
				'raw' => array(
					'type' => $mapping_type,
					'index' => 'not_analyzed',
				),
			)
		);
	}

	$mappings = array(
		'_default_' => array(
			'properties' => $properties,
		),
	);

	$params = array(
		'index' => $index_name,
		'body' => array(
			'settings' => $settings,
			'mappings' => $mappings,
		),
	);

	libraries_load('elasticsearch-php');
	$elasticsearch_host = variable_get('elasticsearch_host');
	$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();
	try {
		$response = $client->indices()->create($params);
	} catch (\Exception $e) {
		$message = $e->getMessage();
		drupal_set_message(t($message), 'warning');
	}
}


/**
 * This function return an array containing tables from the chado schema
 */
function get_chado_table_list() {
	$sql_table_list = "SELECT table_name FROM information_schema.tables WHERE table_schema = 'chado' ORDER BY table_name;";
	$result_table = db_query($sql_table_list);
	$input_table = $result_table->fetchAll();
	$table_list = array();
	foreach ($input_table as $value) {
		$table_list[] = $value->table_name;
	}
	return $table_list;
}
/**
 * This function takes a table name and return all the column names
 */
function get_column_list($table_name) {

	// Detect which schema the table belongs to. If it has prefix 'chado.', it is
	// the chado schema, otherwise it is from the public schema.
	if (preg_match('/^chado\./', $table_name)) {
		$table_name = preg_replace('/^chado\./', '', $table_name);
		$sql_column_list = "SELECT column_name FROM information_schema.columns WHERE table_schema = 'chado' AND table_name = :table_name";
	} else {
		$sql_column_list = "SELECT column_name FROM information_schema.columns WHERE table_schema = 'public' AND table_name = :table_name";
	}
	$column_list_results = db_query($sql_column_list, array(':table_name' => $table_name))->fetchAll();

	$column_list = array();
	foreach($column_list_results as $value) {
		$column_list[] = $value->column_name;
	}
	return $column_list;
}

/**
 * This function return an array containing a list of table names from the public and chado schema.
 */
function get_table_list() {
	$sql_public_table_list = "SELECT table_name FROM information_schema.tables WHERE (table_schema = 'public') ORDER BY table_name";
	$sql_chado_table_list = "SELECT table_name FROM information_schema.tables WHERE (table_schema = 'chado') ORDER BY table_name";
	$public_table_results = db_query($sql_public_table_list)->fetchAll();
	$chado_table_results = db_query($sql_chado_table_list)->fetchAll();

	$public_tables = array();
	$chado_tables = array();
	foreach ($public_table_results as $value) {
		$public_tables[] = $value->table_name;
	}
	foreach ($chado_table_results as $value) {
		$chado_tables[] = 'chado.'.$value->table_name;
	}
	return $public_tables + $chado_tables;
}


/*
 * Escape special characters for elasticsearch
 */
function _remove_special_chars($keyword){
//	$keyword = str_replace('"', '\"', $keyword);
	return $keyword;
}


/**
 * Get node content by node id.
 */
function get_node_content_by_nid($nid, $base_url, $node_title) {
	$page_html = file_get_contents($base_url."/node/".$nid);
	// remove raw sequences
	$pattern_1 = preg_quote('<pre class="tripal_feature-sequence">'). "(.|\n)*".preg_quote('</pre>');
	$page_html = preg_replace("!".$pattern_1."!U", ' ', $page_html);
	// remove query sequences
	$pattern_2 = preg_quote('<pre>Query')."(.|\n)*".preg_quote('</pre>');
	$page_html = preg_replace("!".$pattern_2."!U", ' ', $page_html);
	// add one space to html tags to avoid words catenation after stripping html tags
	$page_html = str_replace( '<', ' <', $page_html);
	// remove generated jQuery script
	$page_html =  preg_replace('/<script\b[^>]*>(.*?)<\/script>/is', "", $page_html);
	// make page title searchable
	$page_content = $node_title.' '.strip_tags($page_html);
	// merge multiple spaces into one
	return preg_replace('!\s+!', ' ', $page_content);
}


/**
 * Build search query from field content pairs.
 */
function build_search_query_from_field_content_pairs (array $field_content_pairs, $query_method = 'query_string') {
	$queries = [];
	foreach ($field_content_pairs as $field => $content) {
		if (!empty($content)) {
			$queries[] = [
				$query_method => [
					"default_field" => $field,
					"query" => _remove_special_chars($content),
					"default_operator" => "OR"
				]
			];
		}
	}
	$query = [
		"bool" => [
			"must" => $queries
		]
	];
	return $query;
}


/**
 * Function to build website search params.
 */
function build_website_search_params ($search_content='', $node_type = '', $index='website', $type='website', $from=0, $size=1000) {
	$queries = [];
	$queries[] = [
		"query_string" => [
			"default_field" => "content",
			"query" => $search_content,
			"default_operator" => "OR"
		]
	];
	if (!empty($node_type)) {
		$queries[] = [
			"match" => [
				"type" => $node_type,
			]
		];
	}
	$query = [
		"bool" => [
			"must" => $queries
		]
	];

	$highlight = [
		"pre_tags" => ["<em><b>"],
		"post_tags" => ["</b></em>"],
		"fields" => [
			"content" =>  [
				"fragment_size" => 150
			]
		]
	];
	$params = [];
	$params['index'] = $index;
	$params['type'] = $type;
	$params['body'] = [
		'query' => $query,
		'highlight' => $highlight
	];
	$params['from'] = $from;
	$params['size'] = $size;
	return $params;
}


/**
 * Function to build table search params.
 */
function build_table_search_params ($index, $type, $query, $from=0, $size=1000, $sort_field, $sort_direction = 'asc' ) {
	$params = [];
	$params['index'] = $index;
	$params['type'] = $type;
	// sort the table by the first field by default
	//$sort_field = array_keys($field_content_pairs)[0];
	$params['body'] = [
		'query' => $query,
		'sort' => [
			$sort_field . ".raw" => $sort_direction,
		]
	];
	$params['from'] = $from;
	$params['size'] = $size;
	return $params;
}


/**
 * Function to return search results count.
 */
//function search_count ($params) {
//	unset($params['from']);
//	unset($params['size']);
//	unset($params['scroll']);
//	$count = client->count($params);
//	$count = $count['count'];
//	return $count;
//}


/**
 * Function to Elasticsearch website.
 */
function tripal_elasticsearch_search_website ($params) {
	// Build Elasticsearch client
	libraries_load('elasticsearch-php');
	$elasticsearch_host = variable_get('elasticsearch_host');
	$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();

	$hits = $client->search($params);
	$search_res = [];
	foreach ($hits['hits']['hits'] as $hit) {
		$highlight = implode('......', $hit['highlight']['content']);
		$hit['_source']['highlight'] = $highlight;
		$search_res[] = $hit['_source'];
	}
	return $search_res;
}

/**
 * Function to Elasticsearch table.
 */
function tripal_elasticsearch_search_table ($params) {
	// Build Elasticsearch client
	libraries_load('elasticsearch-php');
	$elasticsearch_host = variable_get('elasticsearch_host');
	$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();

	$hits = $client->search($params);
	$search_res = [];
	foreach ($hits['hits']['hits'] as $hit) {
		$search_res[] = $hit['_source'];
	}
	return $search_res;
}


/**
 * Function to theme search results count by node type.
 * This function returns an html list.
 */
function get_website_search_results_category_list() {
	$website_search_by_node_type = variable_get('website_search_by_node_type', array());

	$all_categories_text = 'All categories ('. $website_search_by_node_type['count']['all categories'] . ')';
	$all_categories_link = 'tripal_elasticsearch/search_website/'. $website_search_by_node_type['keyword'];
	$output = l($all_categories_text, $all_categories_link);
	unset($website_search_by_node_type['count']['all categories']);

	$items = array();
	foreach ($website_search_by_node_type['count'] as $category=>$count) {
		$text = $category . ' (' . $count . ')';
		$url = 'tripal_elasticsearch/search_website/'.$category.'/'.$website_search_by_node_type['keyword'];
		$items[] = l($text, $url);
	}

	$output .= theme('item_list', array('items' => $items));
	return $output;
}



/**
 * Get website search result table.
 * $search_results: an associative array with three keys: nid, title, and highlight.
 */
function get_website_search_result_table ($search_results) {
	$output = '';
	$rows = array();
	foreach ($search_results as $item) {
		$row = '<h3>'.l($item['title'], 'node/'.$item['nid']).'</h3>';
		$row .= '<p>'.$item['highlight'].'</p>';
		$rows[] = array($row);
	}
//	dpm($rows);
	$per_page = 10;
	$current_page = pager_default_initialize(count($rows), $per_page);

	// split list into page sized chunks
	$chunks = array_chunk($rows, $per_page, TRUE);

	$output .= theme('table', array('header'=>array(), 'rows'=>$chunks[$current_page]));
	$output .= theme('pager', array('quantity', count($rows)));

	return $output;
}