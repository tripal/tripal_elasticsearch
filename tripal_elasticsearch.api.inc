<?php

function tripal_elasticsearch_escape($value = '') {
  return htmlspecialchars($value, ENT_QUOTES, 'UTF-8', FALSE);
}

/**
 * Define several arrays for elasticsearch indexing settings
 */
function get_character_filter_options() {
  return drupal_map_assoc([
    'html_strip',
    'mapping',
    'pattern_replace',
  ]);
}

function get_tokenizer_options() {
  return drupal_map_assoc([
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
  ]);
}

function get_token_filter_options() {
  return drupal_map_assoc([
    'standard',
    'asciifolding',
    'length',
    'lowercase',
    'upppercase',
  ]);
}

/**
 * Return an array of Elasticsearch field mapping types.
 */
function get_field_mapping_types() {
  return drupal_map_assoc([
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
  ]);
}

/**
 * Function to build elasticsearch index.
 */
function build_elasticsearch_index($index_name,
                                   $shards = 5,
                                   $replicas = 0,
                                   $character_filters = [],
                                   $tokenizer,
                                   $token_filters = [],
                                   $field_mapping_types = []) {
  $analysis = [
    'analyzer' => [
      $index_name => [
        'type' => 'custom',
        'tokenizer' => $tokenizer,
        //				'char_filter' => array_keys($character_filters),
        'filter' => array_keys($token_filters),
      ],
      //			'char_filter' => array_keys($character_filters),
      //			'filter' => array_keys($token_filters),
    ],
  ];

  $settings = [
    'number_of_shards' => $shards,
    'number_of_replicas' => $replicas,
    'analysis' => $analysis,
  ];

  foreach ($field_mapping_types as $field => $mapping_type) {
    $properties[$field] = [
      'type' => $mapping_type,
      'fields' => [
        'raw' => [
          'type' => $mapping_type,
          'index' => 'not_analyzed',
        ],
      ],
    ];
  }

  $mappings = [
    '_default_' => [
      'properties' => $properties,
    ],
  ];

  $params = [
    'index' => $index_name,
    'body' => [
      'settings' => $settings,
      'mappings' => $mappings,
    ],
  ];

  libraries_load('elasticsearch-php');
  $elasticsearch_host = variable_get('elasticsearch_host');
  $client = Elasticsearch\ClientBuilder::create()
    ->setHosts([$elasticsearch_host])
    ->build();
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
  $table_list = [];
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
  }
  else {
    $sql_column_list = "SELECT column_name FROM information_schema.columns WHERE table_schema = 'public' AND table_name = :table_name";
  }
  $column_list_results = db_query($sql_column_list, [':table_name' => $table_name])->fetchAll();

  $column_list = [];
  foreach ($column_list_results as $value) {
    $column_list[] = $value->column_name;
  }
  return $column_list;
}

/**
 * This function return an array containing a list of table names from the
 * public and chado schema.
 */
function get_table_list() {
  $sql_public_table_list = "SELECT table_name FROM information_schema.tables WHERE (table_schema = 'public') ORDER BY table_name";
  $sql_chado_table_list = "SELECT table_name FROM information_schema.tables WHERE (table_schema = 'chado') ORDER BY table_name";
  $public_table_results = db_query($sql_public_table_list)->fetchAll();
  $chado_table_results = db_query($sql_chado_table_list)->fetchAll();

  $public_tables = [];
  $chado_tables = [];
  foreach ($public_table_results as $value) {
    $public_tables[] = $value->table_name;
  }
  foreach ($chado_table_results as $value) {
    $chado_tables[] = 'chado.' . $value->table_name;
  }
  return array_merge($public_tables, $chado_tables);
}


/*
 * Escape special characters for elasticsearch
 */
function _remove_special_chars($keyword) {
  //	$keyword = str_replace('"', '\"', $keyword);
  return $keyword;
}


/**
 * Get node content by node id.
 */
function get_node_content_by_nid($nid, $base_url, $node_title) {
  $page_html = file_get_contents($base_url . "/node/" . $nid);
  // remove raw sequences
  $pattern_1 = preg_quote('<pre class="tripal_feature-sequence">') . "(.|\n)*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_1 . "!U", ' ', $page_html);
  // remove query sequences
  $pattern_2 = preg_quote('<pre>Query') . "(.|\n)*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_2 . "!U", ' ', $page_html);
  // add one space to html tags to avoid words catenation after stripping html tags
  $page_html = str_replace('<', ' <', $page_html);
  // remove generated jQuery script
  $page_html = preg_replace('/<script\b[^>]*>(.*?)<\/script>/is', "", $page_html);
  // make page title searchable
  $page_content = $node_title . ' ' . strip_tags($page_html);
  // merge multiple spaces into one
  return preg_replace('!\s+!', ' ', $page_content);
}


/**
 * Build search query from field content pairs.
 */
function build_search_query_from_field_content_pairs(array $field_content_pairs, $query_method = 'query_string') {
  $queries = [];
  foreach ($field_content_pairs as $field => $content) {
    if (!empty($content)) {
      $queries[] = [
        $query_method => [
          "default_field" => $field,
          "query" => _remove_special_chars($content),
          "default_operator" => "OR",
        ],
      ];
    }
  }
  $query = [
    "bool" => [
      "must" => $queries,
    ],
  ];
  return $query;
}


/**
 * Function to build website search params.
 */
function build_website_search_params($search_content = '', $node_type = '', $index = 'website', $type = 'website', $from = 0, $size = 1000) {
  $queries = [];
  $queries[] = [
    "query_string" => [
      "default_field" => "content",
      "query" => $search_content,
      "default_operator" => "OR",
    ],
  ];
  if (!empty($node_type)) {
    $queries[] = [
      "match" => [
        "type" => $node_type,
      ],
    ];
  }
  $query = [
    "bool" => [
      "must" => $queries,
    ],
  ];

  $highlight = [
    "pre_tags" => ["<em><b>"],
    "post_tags" => ["</b></em>"],
    "fields" => [
      "content" => [
        "fragment_size" => 150,
      ],
    ],
  ];
  $params = [];
  $params['index'] = $index;
  $params['type'] = $type;
  $params['body'] = [
    'query' => $query,
    'highlight' => $highlight,
  ];
  $params['from'] = $from;
  $params['size'] = $size;
  return $params;
}


/**
 * Function to build table search params.
 */
function build_table_search_params($index, $type, $query, $from = 0, $size = 1000, $sort_direction = 'asc') {
  $params = [];
  $params['index'] = $index;
  $params['type'] = $type;
  // sort the table by the first field by default
  //$sort_field = array_keys($field_content_pairs)[0];
  $params['body'] = [
    'query' => $query,
    //		'sort' => [
    //			$sort_field . ".raw" => $sort_direction,
    //		]
  ];
  $params['from'] = $from;
  $params['size'] = $size;
  return $params;
}


/**
 * Function for Elasticsearch website.
 */
function tripal_elasticsearch_search_website($params) {
  // Build Elasticsearch client
  libraries_load('elasticsearch-php');
  $elasticsearch_host = variable_get('elasticsearch_host');
  $client = Elasticsearch\ClientBuilder::create()
    ->setHosts([$elasticsearch_host])
    ->build();

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
 * Function to for Elasticsearch table.
 *
 * @param $params Elasticsearch api parameters
 * @param $per_page number of results per page
 * @param $current_page the current page number
 */
function tripal_elasticsearch_search_table($params, $per_page = 10, $current_page = 0) {
  // Build Elasticsearch client
  libraries_load('elasticsearch-php');
  $elasticsearch_host = variable_get('elasticsearch_host');
  $client = Elasticsearch\ClientBuilder::create()
    ->setHosts([$elasticsearch_host])
    ->build();

  if (!isset($params['from'])) {
    $params['from'] = $per_page * $current_page;
    $params['size'] = 10;
  }

  try {
    $hits = $client->search($params);
  } catch (Exception $e) {
    drupal_set_message($e->getMessage());
  }

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
function get_website_search_results_category_list($keyword) {
  try {
    libraries_load('elasticsearch-php');
    $elasticsearch_host = variable_get('elasticsearch_host');
    $client = Elasticsearch\ClientBuilder::create()
      ->setHosts([$elasticsearch_host])
      ->build();
  } catch (\Exception $e) {

  }

  // Get all node types from node table.
  $node_types = db_query("SELECT DISTINCT(type) FROM {node}")->fetchCol('type');
  // Save search results count to an associative array with node types as keys.
  $search_website_count_by_category = [];
  foreach ($node_types as $node_type) {
    // Build search params.
    $params = build_website_search_params(
      $search_content = _remove_special_chars($keyword),
      $node_type = $node_type
    );
    // To get a total number of all search results, we need to unset 'from' and 'size' from $params.
    unset($params['from']);
    unset($params['size']);
    try {
      $count = $client->count($params)['count'];
      if ($count != 0) {
        $search_website_count_by_category[$node_type] = $count;
      }
    } catch (\Exception $e) {

    }
  }

  // Add total count to the $search_website_count_by_category array.
  $total_count = ['all categories' => array_sum($search_website_count_by_category)];
  $search_website_count_by_category = $total_count + $search_website_count_by_category;
  $website_search_by_node_type = [
    'keyword' => $keyword,
    'count' => $search_website_count_by_category,
  ];

  $all_categories_text = 'All categories (' . $website_search_by_node_type['count']['all categories'] . ')';
  $all_categories_link = 'tripal_elasticsearch/search_website/' . $website_search_by_node_type['keyword'];
  $output = l($all_categories_text, $all_categories_link);
  unset($website_search_by_node_type['count']['all categories']);

  $items = [];
  foreach ($website_search_by_node_type['count'] as $category => $count) {
    $text = $category . ' (' . $count . ')';
    $url = 'tripal_elasticsearch/search_website/' . $category . '/' . $website_search_by_node_type['keyword'];
    $items[] = l($text, $url);
  }

  $output .= theme('item_list', ['items' => $items]);
  return $output;
}


/**
 * Get website search result table.
 * $search_results: an associative array with three keys: nid, title, and
 * highlight.
 */
function get_website_search_result_table($search_results) {
  $output = '';
  $rows = [];
  foreach ($search_results as $item) {
    $row = '<h3>' . l($item['title'], 'node/' . $item['nid']) . '</h3>';
    $row .= '<p>' . $item['highlight'] . '</p>';
    $rows[] = [$row];
  }

  $per_page = 10;
  $current_page = pager_default_initialize(count($rows), $per_page);

  // split list into page sized chunks
  $chunks = array_chunk($rows, $per_page, TRUE);

  $output .= theme('table', ['header' => [], 'rows' => $chunks[$current_page]]);
  $output .= theme('pager', ['quantity', count($rows)]);

  return $output;
}


/**
 * Get table search result table.
 *
 * @param $search_results the trimmed search results
 * @param $index_name
 * @param $total the total number of results
 *
 * @return markup
 */
function get_table_search_result_table($search_results, $index_name, $total) {
  // Get field url
  $sql = "SELECT * FROM {tripal_elasticsearch_links} WHERE index_name = :index_name";
  $result = db_query($sql, [':index_name' => $index_name]);
  $field_url_pairs = [];
  foreach ($result as $record) {
    $field = $record->index_field;
    $url = $record->field_url;
    $field_url_pairs[$field] = $url;
    //		$field_url_pairs[$field] = preg_replace('/\[(.+)\]/U', '\$${1}', $url);
  }

  $rows = $search_results;
  $filtered_results = array_filter($field_url_pairs, function ($item) {
    return !empty($item);
  });

  //add links to search results.
  if (!empty($filtered_results)) {
    $processed_search_results = [];
    foreach ($search_results as $row) {
      foreach ($field_url_pairs as $field => $url) {
        if ($url != '') {
          // replace field variables in url with field values.
          preg_match_all('/\[.+?\]/', $url, $matches);
          $field_variables = $matches[0];
          array_walk($field_variables, function (&$value) {
            $value = preg_replace('/\[(.+)\]/U', '${1}', $value);
          });
          $replacement = [];
          foreach ($field_variables as $field_variable) {
            $replacement[] = $row[$field_variable];
          }
          $url = str_replace($matches[0], $replacement, $url);

          // theme search results.
          $field_search_result = $row[$field];
          $row[$field] = l($field_search_result, $url);
        }
      }
      $processed_search_results[] = $row;
    }
    $rows = $processed_search_results;
  }

  // theme search results as a table.
  $output = '';
  $headers = [];
  $fields = [];
  $i = 0;
  if (!empty($search_results)) {
    foreach ($search_results[0] as $key => $value) {
      $header = [
        'data' => ucwords(str_replace('_', ' ', $key)),
        'field' => $key,
      ];

      if ($_GET['order'] === $key) {
        $header['sort'] = $_GET['sort'];
      }
      else {
        if ($i === 0) {
          $header['sort'] = 'asc';
        }
      }
      $headers[] = $header;
      $i++;
    }
  }

  //	$rows = $processed_search_results;
  //$per_page = 10;
  //$current_page = pager_default_initialize(count($rows), $per_page);
  // split list into page sized chunks
  //$chunks = array_chunk($rows, $per_page, TRUE);
  $output .= theme('table', [
    'header' => $headers,
    'rows' => $search_results,
    'attributes' => ['id' => ['tripal-elastic-search-results-table']],
  ]);
  $output .= theme('pager', ['quantity', $total]);

  return $output;
}