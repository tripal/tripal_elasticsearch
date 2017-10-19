<?php
/**
 * Gene search forms.
 *
 * @file gene_search.form.inc
 */

/**
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_gene_search_form($form, &$form_state, $local = TRUE) {
  $form['#attributes']['id'] = 'cross-site-search-form';

  $form['options'] = [
    '#type' => 'fieldset',
    '#attributes' => [
      'class' => [
        'container-inline',
        'es-cross-site-search-form',
      ],
    ],
  ];

  $default_organism = ['' => 'Any Organism'];
  $organism_list = chado_query('SELECT genus, species, common_name FROM {organism}')->fetchAll();
  $organisms = array_map(function ($organism) {
    $name = "{$organism->genus} {$organism->species}";
    if (!empty($organism->common_name)) {
      $name .= " ({$organism->common_name})";
    }

    return $name;
  }, $organism_list);

  $form['options']['organism'] = [
    '#type' => 'select',
    '#attributes' => [
      'id' => 'tripal-elasticsearch-search-category',
      'style' => 'max-width: 250px;',
    ],
    '#options' => array_merge($default_organism, drupal_map_assoc($organisms)),
    '#default_value' => isset($_GET['organism']) ? $_GET['organism'] : '',
    '#required' => TRUE,
  ];

  $form['options']['search_term'] = [
    '#type' => 'textfield',
    '#size' => '50',
    '#attributes' => [
      'placeholder' => t('E,g. Kinase or IPR020405'),
      'id' => 'tripal-elasticsearch-search-field',
    ],
    '#description' => '<small>Examples: <a href="?search_term=Heat+Shock">Heat Shock</a>, <a href="?search_term=IPR020405">IPR020405</a>, <a href="?search_term=GO:0016049">GO:0016049</a>, etc.</small>',
    '#required' => TRUE,
    '#default_value' => isset($_GET['search_term']) ? $_GET['search_term'] : '',
  ];

  $form['options']['search_button'] = [
    '#type' => 'submit',
    '#value' => t('Search'),
    '#attributes' => [
      'id' => 'tripal-elasticsearch-search-button',
    ],
  ];

  if (!$local) {
    $form['results_block'] = [
      '#type' => 'markup',
      '#markup' => '<div id="tripal-elasticsearch-results-block"></div>',
    ];

    drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/axios.min.js');
    $remotes = db_query('SELECT * FROM {tripal_elasticsearch_servers}')->fetchAll();
    // Add local server as the first remote
    array_unshift($remotes, tripal_elasticsearch_get_local_server_entry());
    drupal_add_js([
      'remotes' => $remotes,
      'action' => 'setupTableIndexPage',
      'index' => 'gene_search_index',
    ], 'setting');
    drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/tripal_elasticsearch.js');
  }
  else {
    if (!empty($_GET['search_term']) || !empty($_GET['organism'])) {
      $form['results'] = [
        '#type' => 'markup',
        '#markup' => tripal_elasticsearch_gene_search_index_results(),
      ];
    }
  }

  $form['#method'] = 'get';

  return $form;
}

/**
 * Map arguments to ES query.
 *
 * @param array $arguments Usually $_GET request variable
 *
 * @return array
 */
function tripal_elasticsearch_gene_search_index_query_mapper($arguments) {
  $queries = [];

  $organism = isset($arguments['organism']) ? $arguments['organism'] : NULL;
  if (!empty($organism)) {
    $organism = explode(' ', $organism);
    $genus = array_shift($organism);
    $species = array_shift($organism);

    $queries[] = [
      'query_string' => [
        'fields' => ['organism_genus', 'organism_species'],
        "query" => "$genus $species",
        "default_operator" => "OR",
      ],
    ];
  }

  if (isset($arguments['search_term'])) {

    $search_term = str_replace(':', ' AND ', $arguments['search_term']);
    
    $queries[] = [
      'query_string' => [
        'query' => $search_term,
        'fields' => [
          '_all',
        ],
      ],
    ];
  }

  $query = [
    "bool" => [
      "must" => $queries,
    ],
  ];

  return $query;
}

/**
 * Format the results.
 *
 * @param array $search_results
 * @param string $host
 *
 * @return string
 */
function tripal_elasticsearch_gene_search_index_results_formatter($search_results, $host = NULL) {
  $header = ['Gene Identifier', 'Blast Hit Descriptions', 'Annotations'];
  $rows = [];
  if ($host) {
    $base_url = trim($host->url, '/');
  }
  foreach ($search_results as $result) {
    $result = (array) $result;
    if ($host) {
      $url = "<a href='{$base_url}/{$result['url']}'>{$result['uniquename']}</a>";
    }
    else {
      $url = l($result['uniquename'], $result['url']);
    }

    $result = (array) $result;
    $rows[] = [
      $url,
      '<div class="es-squish">' . elasticsearch_recursive_implode('<br>', $result['blast_hit_descriptions']) . '</div>',
      '<div class="es-squish">' . elasticsearch_recursive_implode('<br>', $result['annotations']) . '</div>',
    ];
  }

  return theme('table', [
    'header' => $header,
    'rows' => $rows,
  ]);
}

/**
 * Get gene search results.
 *
 * @return string
 */
function tripal_elasticsearch_gene_search_index_results() {
  if (empty($_GET['search_term']) && empty($_GET['organism'])) {
    return '';
  }

  try {
    $es = new ESInstance();
  } catch (Exception $exception) {
    drupal_set_message($exception->getMessage(), 'error');
    return '';
  }

  $query = tripal_elasticsearch_gene_search_index_query_mapper($_GET);
  $results = $es->setTableSearchParams('gene_search_index', '', $query)
                ->paginate(15);
  $formatted = tripal_elasticsearch_gene_search_index_results_formatter($results['results']);

  $content = '<div style="margin-bottom: 22px">';
  $content .= '<strong>' . $results['total'] . ' results found</strong>';
  $content .= '<strong style="float: right">Page ' . $results['page'] . ' - ' . $results['pages'] . '</strong>';
  $content .= '</div>';
  $content .= $formatted;
  $content .= $results['pager'];

  // Add the necessary JS
  drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/table_search_results_datatable.js');

  return $content;
}