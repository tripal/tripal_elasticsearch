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
    'uppercase',
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
  $pattern_1 = preg_quote('<pre class="tripal_feature-sequence">') . ".*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_1 . "!sU", ' ', $page_html);
  // remove query sequences
  $pattern_2 = preg_quote('<pre>Query') . ".*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_2 . "!sU", ' ', $page_html);
  // remove blast alignments if tripal_analysis_blast is installed
  $pattern_3 = preg_quote('<pre class="blast_align">') . ".*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_3 . "!sU", ' ', $page_html);
  // add one space to html tags to avoid words concatenated after stripping html tags
  $page_html = str_replace('<', ' <', $page_html);
  // remove generated jQuery script
  $page_html = preg_replace('/<script\b[^>]*>.*<\/script>/isU', "", $page_html);
  // remove css stuff
  $page_html = preg_replace('/<style\b[^>]*>.*<\/style>/isU', "", $page_html);
  // make page title searchable
  $page_html = $node_title . ' ' . strip_tags($page_html);
  // merge multiple spaces into one
  return preg_replace('!\s+!', ' ', $page_html);
}

/**
 * Get node content by node id.
 */
function tripal_elasticsearch_get_page_content_by_id($id, $base_url, $title, $is_node = TRUE) {
  if ($is_node) {
    $page_html = file_get_contents($base_url . "/node/" . $id);
  }
  else {
    $page_html = file_get_contents($base_url . "/bio_data/" . $id);
  }
  // remove raw sequences
  $pattern_1 = preg_quote('<pre class="tripal_feature-sequence">') . ".*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_1 . "!sU", ' ', $page_html);
  // remove query sequences
  $pattern_2 = preg_quote('<pre>Query') . ".*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_2 . "!sU", ' ', $page_html);
  // remove blast alignments if tripal_analysis_blast is installed
  $pattern_3 = preg_quote('<pre class="blast_align">') . ".*" . preg_quote('</pre>');
  $page_html = preg_replace("!" . $pattern_3 . "!sU", ' ', $page_html);
  // add one space to html tags to avoid words concatenated after stripping html tags
  $page_html = str_replace('<', ' <', $page_html);
  // remove generated jQuery script
  $page_html = preg_replace('/<script\b[^>]*>.*<\/script>/isU', "", $page_html);
  // remove css stuff
  $page_html = preg_replace('/<style\b[^>]*>.*<\/style>/isU', "", $page_html);
  // make page title searchable
  $page_html = $title . ' ' . strip_tags($page_html);
  // merge multiple spaces into one
  return preg_replace('!\s+!', ' ', $page_html);
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
 * Function to theme search results count by node type.
 * This function returns an html list.
 *
 * @return string
 */
function get_website_search_results_category_list($keyword) {
  try {
    $es = new ESInstance();
    $indices = $es->getIndices();
    $types = $es->getAllCategories();
  } catch (Exception $exception) {
    tripal_report_error('tripal_elasticsearch', 'TRIPAL_WARNING', $exception->getMessage());
    watchdog('tripal_elasticsearch', $exception->getMessage());
    return '';
  }

  $index_name = [];

  if (in_array('website', $indices)) {
    $index_name[] = 'website';
  }

  if (in_array('entities', $indices)) {
    $index_name[] = 'entities';
  }

  // Save search results count to an associative array with node types as keys.
  $search_website_count_by_category = [];
  foreach ($types as $node_type) {
    try {
      $count = $es->setWebsiteSearchParams($keyword, $node_type, implode(',', $index_name))
                  ->count();
      if ($count > 0) {
        $search_website_count_by_category[$node_type] = $count;
      }
    } catch (Exception $exception) {
      tripal_report_error('tripal_elasticsearch', 'TRIPAL_WARNING', $exception->getMessage());
      watchdog('tripal_elasticsearch', $exception->getMessage());
    }
  }

  // Add total count to the $search_website_count_by_category array.
  $total_count = ['all categories' => array_sum($search_website_count_by_category)];
  $search_website_count_by_category = $total_count + $search_website_count_by_category;
  $website_search_by_node_type = [
    'keyword' => $keyword,
    'count' => $search_website_count_by_category,
  ];

  $output = '<h4>Filter by Category</h4>';

  $all_categories_text = 'All categories (' . $website_search_by_node_type['count']['all categories'] . ')';
  $all_categories_link = 'tripal_elasticsearch/search_website';
  $output .= l($all_categories_text, $all_categories_link, [
    'query' => [
      'search_box' => $website_search_by_node_type['keyword'],
    ],
    'attributes' => [
      'class' => ['es-category-item', 'es-category-item__all'],
    ],
  ]);
  unset($website_search_by_node_type['count']['all categories']);

  $items = [];
  foreach ($website_search_by_node_type['count'] as $category => $count) {
    $text = $category . ' (' . $count . ')';
    $url = 'tripal_elasticsearch/search_website/' . $category;
    $items[] = l($text, $url, [
      'query' => [
        'search_box' => $website_search_by_node_type['keyword'],
        'category' => $category,
      ],
      'attributes' => [
        'class' => ['es-category-item'],
      ],
    ]);
  }

  $output .= theme('item_list', ['items' => $items]);

  return $output;
}

/**
 * Get website search result table.
 *
 * @param array $search_results an associative array with three keys: nid,
 *   title, and highlight.
 * @param boolean $pager Whether to render a pager
 * @param string $base_url Specify the site that owns the results.
 *
 * @return string
 */
function get_website_search_result_table($search_results, $pager = TRUE, $base_url = NULL) {
  // grab page title and highlight content from search results.
  $rows = [];
  foreach ($search_results as $item) {
    $item = (object) $item;
    if (isset($item->nid)) {
      $row = '<h3>' . l($item->title, 'node/' . $item->nid) . '</h3>';
      if ($base_url) {
        $row = '<h3><a href="' . $base_url . '/node/' . $item->nid . '">' . $item->title . '</a></h3>';
      }
    }
    else {
      $row = '<h3>' . l($item->title, 'bio_data/' . $item->entity_id) . '</h3>';
      if ($base_url) {
        $row = '<h3><a href="' . $base_url . '/bio_data/' . $item->entity_id . '">' . $item->title . '</a></h3>';
      }
    }

    if (property_exists($item, 'highlight')) {
      $row .= '<p>' . $item->highlight . '</p>';
    }

    $rows[] = [$row];
  }

  $output = theme('table', [
    'header' => [],
    'rows' => $rows,
    'attributes' => [
      'id' => 'es-search-results-table',
    ],
  ]);
  if ($pager) {
    $output .= theme('pager', ['quantity', count($rows)]);
  }

  return $output;
}

/**
 * Get table search result table.
 *
 * @param array $search_results the trimmed search results
 * @param string $index_name
 * @param int $total the total number of results
 *
 * @return mixed
 */
function get_table_search_result_table($search_results, $index_name, $total) {
  // Get field url
  $sql = "SELECT * FROM {tripal_elasticsearch_links} WHERE index_name = :index_name";
  $result = db_query($sql, [':index_name' => $index_name]);

  $sql = "SELECT form_field_title, index_field FROM {tripal_elasticsearch} WHERE index_name = :index_name";
  $header_values = db_query($sql, [':index_name' => $index_name]);
  $titles = [];
  foreach ($header_values as $title) {
    $titles[$title->index_field] = $title->form_field_title;
  }

  $field_url_pairs = [];
  foreach ($result as $record) {
    $field = $record->index_field;
    $url = $record->field_url;
    $field_url_pairs[$field] = $url;
    //		$field_url_pairs[$field] = preg_replace('/\[(.+)\]/U', '\$${1}', $url);
  }

  $filtered_results = array_filter($field_url_pairs, function ($item) {
    return !empty($item);
  });

  $rows = [];
  foreach ($search_results as $result) {
    $row = [];
    foreach ($result as $key => $value) {
      if (!isset($titles[$key])) {
        continue;
      }
      $row[$key] = $value;
    }

    $rows[] = $row;
  }

  // Add links to search results.
  if (!empty($filtered_results)) {
    $processed_search_results = [];
    foreach ($search_results as $row) {
      $row = (array) $row;
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
  $i = 0;
  if (!empty($search_results)) {
    foreach ($search_results[0] as $key => $value) {
      if (!isset($titles[$key])) {
        continue;
      }

      $title = $titles[$key];

      $header = [
        'field' => $key,
        'data' => $key,
        'data-elastic-title' => $title,
      ];

      if (isset($_GET['order']) && $_GET['order'] === $key) {
        $header['sort'] = isset($_GET['sort']) ? $_GET['sort'] : 'asc';
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
    'rows' => $rows,
    'attributes' => ['id' => ['tripal-elastic-search-results-table']],
  ]);
  $output .= theme('pager', ['quantity', $total]);

  return $output;
}