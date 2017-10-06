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
    return ESResponse::error($exception->getMessage(), 500);
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
    $response = tripal_elasticsearch_api_v1_local_search(true);
    $host = tripal_elasticsearch_get_local_server_entry();
  }
  else {
    $host = db_query('SELECT * FROM {tripal_elasticsearch_servers} WHERE id=:id', [':id' => $remote_host_id])->fetchObject();
    $url = rtrim(trim($host->url), '/') . '/elasticsearch/api/v1/local-search';

    try {
      $response = ESRequest::get($url, [
        'terms' => $_GET['terms'],
        'category' => $_GET['category'] ?: '',
        'size' => $size,
      ]);
    } catch (Exception $exception) {
      return ESResponse::error($exception->getMessage(), 500);
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
 * @param bool $local Whether this is an internal request or an http request.
 */
function tripal_elasticsearch_api_v1_local_search($local = true) {
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
    $results = $es->searchAllIndices($terms, $size, $category);
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
  ]), $local);
}

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

  $remotes = db_query('SELECT * FROM {tripal_elasticsearch_servers}')->fetchAll();

  // Add local server as the first remote
  array_unshift($remotes, tripal_elasticsearch_get_local_server_entry());

  drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/axios.min.js');
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