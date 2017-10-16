<?php

/**
 * Provides web services support to Elasticsearch server.
 *
 * @file includes/tripal_elasticsearch.ws.inc
 */

/**
 * Report the status of the elastic search index.
 *
 * @return bool
 */
function tripal_elasticsearch_api_v1_status() {
  try {
    $params['v'] = TRUE;
    $es = new ESInstance();
    $health = current($es->client->cat()->health($params));
  } catch (Exception $exception) {
    return ESResponse::error([
      'status' => 'Inactive',
      'healthy' => FALSE,
    ]);
  }

  if (empty($health)) {
    return ESResponse::error([
      'status' => 'Inactive',
      'healthy' => FALSE,
    ]);
  }

  return ESResponse::success([
    'status' => 'Active',
    'healthy' => TRUE,
  ]);
}

/**
 * Allow https/http connections to communicate.
 *
 * @param $remote_id
 *
 * @return bool
 */
function tripal_elasticsearch_api_v1_remote_status($remote_id) {
  $remote_id = abs(intval(trim($remote_id)));

  if (!is_int($remote_id)) {
    return ESResponse::error([
      'remote_id' => ['Invalid remote server id'],
    ]);
  }

  // Get the remote host url
  $host = db_query('SELECT url FROM {tripal_elasticsearch_servers} WHERE id=:id', [':id' => $remote_id])->fetchObject();
  if (!$host) {
    return ESResponse::error([
      'remote_id' => ['Remote server record does not exist'],
    ]);
  }

  $url = trim(trim($host->url), '/') . '/elasticsearch/api/v1/status';

  try {
    $response = ESRequest::get($url);
  } catch (Exception $exception) {
    return ESResponse::error(['server' => $exception->getMessage()], 500);
  }

  return ESResponse::success($response->data);
}

/**
 * Send a search request to the given host and return rendered results.
 *
 * @param $remote_host_id
 *
 * @return bool
 */
function tripal_elasticsearch_api_v1_search($remote_host_id) {
  $remote_host_id = abs(intval(trim($remote_host_id)));

  if (!isset($_GET['terms'])) {
    return ESResponse::error(['terms' => ['Please provide search terms']]);
  }

  $size = 0;
  if (isset($_GET['size'])) {
    $size = abs(intval($_GET['size']));
  }

  if ($remote_host_id === 0) {
    $response = tripal_elasticsearch_api_v1_local_search(FALSE);
    $host = tripal_elasticsearch_get_local_server_entry();
  }
  else {
    $host = db_query('SELECT * FROM {tripal_elasticsearch_servers} WHERE id=:id', [':id' => $remote_host_id])->fetchObject();
    $url = rtrim(trim($host->url), '/') . '/elasticsearch/api/v1/local-search';

    try {
      $response = ESRequest::get($url, [
        'terms' => $_GET['terms'],
        'category' => isset($_GET['category']) ? $_GET['category'] : '',
        'size' => $size,
      ]);
    } catch (Exception $exception) {
      return ESResponse::error(['server' => $exception->getMessage()], 500);
    }
  }

  $markup = get_website_search_result_table($response->data->results, FALSE, $host->url);

  // Render results into fields and send them back
  return ESResponse::success([
    'count' => $response->data->count,
    'url' => $response->data->url,
    'markup' => $markup,
  ]);
}

/**
 * Perform a local search.
 * This method is normally called by other sites looking for
 * results from our site.
 *
 * @param bool $http Whether this is an http request or an internal
 *                    request. (TRUE for http)
 *                   Set this option to FALSE to retrieve an object instead
 *                    of printing a JSON string.
 */
function tripal_elasticsearch_api_v1_local_search($http = TRUE) {
  global $base_url;

  // Validate the request
  $terms = isset($_GET['terms']) ? $_GET['terms'] : '';
  $size = isset($_GET['size']) ? abs(intval($_GET['size'])) : 0;
  $category = isset($_GET['category']) ? $_GET['category'] : NULL;

  if ($size !== 0 && $size > 20) {
    return ESResponse::error([
      'size' => ['Please provide a valid size between 1 and 20'],
    ]);
  }

  if ($size === 0) {
    $size = 5;
  }

  try {
    $es = new ESInstance();
    $results = $es->searchWebIndices($terms, $size, $category);
  } catch (Exception $exception) {
    return ESResponse::error($exception->getMessage(), 500);
  }

  return ESResponse::success(array_merge($results, [
    'url' => rtrim($base_url, '/') . url('tripal_elasticsearch/search_website/' . $category, [
        'query' => [
          'search_box' => $terms,
          'category' => $category,
        ],
      ]),
  ]), $http);
}

/**
 * Perform a table index search.
 *
 * @param $index_name
 * @param $remote_host_id
 *
 * @return array|bool|object
 */
function tripal_elasticsearch_api_v1_table_index_search($index_name, $remote_host_id) {
  $remote_id = intval($remote_host_id);
  $terms = isset($_GET['annotations']) ? $_GET['annotations'] : '';

  if (empty($terms)) {
    return ESResponse::error([
      'search_terms' => ['Please provide a search term'],
    ]);
  }

  if ($remote_id === 0) {
    try {
      $response = tripal_elasticsearch_api_v1_table_index_local_search($index_name, FALSE);
      $results = $response->data;
    } catch (Exception $exception) {
      return ESResponse::error([
        'results' => [$exception->getMessage()],
      ]);
    }
  }
  else {
    $host = db_query('SELECT * FROM {tripal_elasticsearch_servers} WHERE id=:id', [':id' => $remote_id])->fetchObject();

    if (!$host) {
      return ESResponse::error([
        'remote_id' => ['Invalid remote host provided.'],
      ]);
    }

    $url = trim($host->url, '/') . 'elasticsearch/' . $index_name . '/local-search';
    try {
      $results = ESRequest::get($url);
    } catch (Exception $exception) {
      return ESResponse::error([
        'remote_id' => [
          'The host returned an invalid response',
          $exception->getMessage(),
        ],
      ]);
    }
  }

  if (!$results) {
    return ESResponse::error([
      'results' => [
        'Elasticsearch error. Could not perform search.',
      ],
    ]);
  }

  return ESResponse::success($results);
}

/**
 * Perform a local table index search.
 *
 * @param $index_name
 * @param bool $http
 *
 * @return array|bool|object
 * @throws \Exception
 */
function tripal_elasticsearch_api_v1_table_index_local_search($index_name, $http = TRUE) {
  try {
    $es = new ESInstance();
    $fields = $es->getIndexFields($index_name);
    $field_content_pairs = [];

    foreach ($fields as $field) {
      if (isset($_GET[$field])) {
        $field_content_pairs[$field] = $_GET[$field];
      }
    }

    $query = build_search_query_from_field_content_pairs($field_content_pairs);
    $results = $es->setTableSearchParams($index_name, '', $query, [0, 2]);
  } catch (Exception $exception) {
    if ($http) {
      return ESResponse::error([
        'results' => [$exception->getMessage()],
      ]);
    }
    else {
      throw $exception;
    }
  }

  $count = $results->count();
  $search_results = $results->search();

  global $base_url;
  return ESResponse::success([
    'count' => $count,
    'url' => $base_url,
    'markup' => theme('table', [
      'header' => $count > 0 ? array_keys($search_results[0]) : [],
      'rows' => $search_results,
    ]),
  ], $http);
}

/**
 * Get local categories.
 *
 * @return array|bool|object
 */
function tripal_elasticsearch_api_v1_categories() {
  return ESResponse::success([
    [
      'label' => 'Gene',
      'table' => 'chado.feature',
    ],
    [
      'label' => 'mRNA',
      'table' => 'chado.feature',
    ],
    [
      'label' => 'Organism',
      'table' => 'chado.organism',
    ],
  ]);
}

/**
 * Get the search page.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_cross_site_search_form($form, &$form_state) {
  $form['options'] = [
    '#type' => 'fieldset',
    '#attributes' => [
      'class' => [
        'container-inline',
        'es-cross-site-search-form',
      ],
    ],
  ];

  $default_category = ['Any Type' => 'Any Type'];
  try {
    $es = new ESInstance();
    $categories = drupal_map_assoc($es->getAllCategories());
  } catch (Exception $exception) {
    $categories = [];
  }

  $form['options']['category'] = [
    '#type' => 'select',
    '#attributes' => [
      'id' => 'tripal-elasticsearch-search-category',
    ],
    '#options' => array_merge($default_category, $categories),
    '#default_value' => 'Any Type',
    '#required' => TRUE,
  ];

  $form['options']['search_term'] = [
    '#type' => 'textfield',
    '#size' => '50',
    '#attributes' => [
      'placeholder' => t('E,g. Fraxinus Excelsior mRNA'),
      'id' => 'tripal-elasticsearch-search-field',
    ],
    '#required' => TRUE,
  ];

  $form['options']['search_button'] = [
    '#type' => 'submit',
    '#value' => t('Search'),
    '#attributes' => [
      'id' => 'tripal-elasticsearch-search-button',
    ],
  ];

  $form['results_block'] = [
    '#type' => 'markup',
    '#markup' => '<div id="tripal-elasticsearch-results-block"></div>',
  ];

  $form['#atached']['js'][] = [
    'data' => drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/axios.min.js'),
    'type' => 'file',
  ];

  $remotes = db_query('SELECT * FROM {tripal_elasticsearch_servers}')->fetchAll();

  // Add local server as the first remote
  array_unshift($remotes, tripal_elasticsearch_get_local_server_entry());

  drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/tripal_elasticsearch.js');
  drupal_add_js([
    'remotes' => $remotes,
    'action' => 'setupSearchPage',
  ], 'setting');

  return $form;
}

/**
 * Mock the local server as a remote server for AJAX to work.
 *
 * @return object
 */
function tripal_elasticsearch_get_local_server_entry() {
  global $base_url;

  return (object) [
    'id' => 0,
    'label' => variable_get('site_name', 'Drupal'),
    'description' => '',
    'url' => $base_url,
  ];
}