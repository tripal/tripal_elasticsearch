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
function tripal_elasticsearch_gene_search_form(
  $form,
  &$form_state,
  $local = TRUE,
  $organism = NULL,
  $url_query = []
) {
  $constructed_url = '';
  $i = 0;
  foreach ($url_query as $key => $value) {
    if ($i > 0) {
      $constructed_url .= '&';
    }
    $constructed_url .= "{$key}={$value}";
    $i++;

    $form['hidden'][$key] = [
      '#type' => 'value',
      '#value' => $value,
    ];
  }

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

  if ($organism !== NULL) {
    $form['organism'] = [
      '#type' => 'hidden',
      '#value' => $organism,
    ];
  }
  else {
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
  }

  $form['options']['search_term'] = [
    '#type' => 'textfield',
    '#size' => '50',
    '#attributes' => [
      'placeholder' => t('E,g. Kinase or IPR020405'),
      'id' => 'tripal-elasticsearch-search-field',
    ],
    '#description' => '<small>Examples: <a href="?search_term=Heat+Shock&' . $constructed_url . '">Heat Shock</a>, <a href="?search_term=IPR020575&' . $constructed_url . '">IPR020575</a>, <a href="?search_term=GO:0016049&' . $constructed_url . '">GO:0016049</a>, etc.</small>',
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
        '#markup' => tripal_elasticsearch_gene_search_index_results(TRUE),
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
function tripal_elasticsearch_gene_search_index_results_formatter(
  $search_results,
  $host = NULL
) {
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
function tripal_elasticsearch_gene_search_index_results($display_download = FALSE) {
  if (empty($_GET['search_term']) && empty($_GET['organism'])) {
    return '';
  }

  try {
    $es = new ESInstance();
  } catch (Exception $exception) {
    drupal_set_message('The search service is currently unavailable. Please try again later.', 'error');
    watchdog('tripal_elasticsearch', 'Feature/Gene search form resulted in this error: '.$exception->getMessage(), [], WATCHDOG_ERROR);
    return '';
  }

  $query = tripal_elasticsearch_gene_search_index_query_mapper($_GET);

  try {
    $results = $es->setTableSearchParams('gene_search_index', '', $query)
      ->paginate(15);
  } catch (Exception $exception) {
    drupal_set_message('The search service is currently unavailable. Please try again later.', 'error');
    watchdog('tripal_elasticsearch', 'Feature/Gene search form resulted in this error: '.$exception->getMessage(), [], WATCHDOG_ERROR);
    return '';
  }


  $formatted = tripal_elasticsearch_gene_search_index_results_formatter($results['results']);

  $query = $_GET;
  if (isset($query['q'])) {
    unset($query['q']);
  }

  $content = '<div style="margin-bottom: 15px">';
  $content .= '<strong>' . $results['total'] . ' results found</strong>';
  $content .= '<strong style="float: right">Page ' . $results['page'] . ' - ' . $results['pages'] . '</strong>';
  $content .= '</div>';

  if ($display_download && intval($results['total']) > 0) {
    $content .= '<p>';
    $content .= l('Download results as FASTA', 'elasticsearch/gene_download', [
      'query' => [
        'organism' =>isset($_GET['organism']) ? $_GET['organism'] : '',
        'search_term' => $_GET['search_term'],
        'from' => urlencode($_GET['q']),
      ],
      'attributes' => [
        'target' => '_blank',
        'class' => ['btn', 'btn-info'],
      ],
    ]);
    $content .= '</p>';
  }

  if ($results['total'] == 0) {
    $content .= theme('elasticsearch_tips');
  }
  else {
    $content .= $formatted;
    $content .= $results['pager'];
  }

  // Add the necessary JS
  drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/table_search_results_datatable.js');

  return $content;
}

/**
 * Create downloadable FASTA file.
 * This function allows up to 1 million records max.
 *
 * @return void
 */
function tripal_elasticsearch_gene_search_download() {
  if (!isset($_GET['search_term']) || empty($_GET['search_term'])) {
    drupal_not_found();
    return;
  }

  try {
    $es = new ESInstance();
  } catch (Exception $exception) {
    drupal_set_message('Error occurred while generating records for download. Please try again later.', 'error');
    header('Location: /' . urldecode($_GET['from']));
    return;
  }

  ob_clean();

  $query = tripal_elasticsearch_gene_search_index_query_mapper($_GET);

  // Set the mime type
  drupal_add_http_header('Content-Type', 'text/plain');
  drupal_add_http_header('Content-Disposition', 'attachment; filename="gene_search_results.fasta"');

  // Handle IE downloads
  drupal_add_http_header('Pragma', 'public');
  drupal_add_http_header('Cache-Control', 'must-revalidate, post-check=0, pre-check=0');

  try {
    // Stop at 1 million
    $count = min($es->setTableSearchParams('gene_search_index', '', $query)
      ->count(), 1000000);

    $i = 0;
    $chunk = 500;
    while ($i < $count) {
      $records = $es->setTableSearchParams('gene_search_index', '', $query, [
        $i,
        $chunk,
      ])->search();

      foreach ($records as $record) {
        print tripal_elasticsearch_put_fasta_line($record['uniquename'], $record['sequence'], [
          'Sequence Length' => $record['sequence_length'],
          'Type' => $record['type'],
        ]);
      }

      $i += $chunk;
    }
  } catch (Exception $exception) {
    print 'Error occurred while generating file. Please try again later.';
    return;
  }
}

/**
 * Create a FASTA line.
 *
 * @param string $name
 * @param string $sequence
 * @param array $meta
 *
 * @return string
 */
function tripal_elasticsearch_put_fasta_line($name, $sequence, array $meta = []) {
  $line = ">$name";

  // Join meta data
  foreach ($meta as $title => $value) {
    $line .= "; $title: $value";
  }

  $line .= "\n";

  return "{$line}{$sequence}\n";
}