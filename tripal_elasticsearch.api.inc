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

	dpm($params);

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
	$keyword = str_replace('"', '\"', $keyword);
	return $keyword;
}


/**
 * Get node content by node id.
 */
function get_node_content_by_nid($nid, $base_url, $node_title) {
	$page_html = file_get_contents("$base_url/node/$nid");
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