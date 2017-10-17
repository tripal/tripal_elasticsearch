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
function tripal_elasticsearch_gene_search_form($form, &$form_state) {
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
      'style' => 'max-width: 250px;'
    ],
    '#options' => array_merge($default_organism, drupal_map_assoc($organisms)),
    '#default_value' => 'Any Organism',
    '#required' => TRUE,
  ];

  $form['options']['search_term'] = [
    '#type' => 'textfield',
    '#size' => '150',
    '#attributes' => [
      'placeholder' => t('E,g. Kinase or IPR020405'),
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

  drupal_add_js([
    'remotes' => $remotes,
    'action' => 'setupTableIndexPage',
    'index' => 'gene_search_index',
    // Optional form fields to index fields mapper
    'field_mapper' => [
      'organism_common_name' => 'organism',
      'organism_genus' => 'organism',
      'organism_species' => 'organism',
      'uniquename' => 'search_term',
      'annotations' => 'search_term',
      'blast_hit_descriptions' => 'search_term',
    ],
  ], 'setting');
  drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/tripal_elasticsearch.js');

  return $form;
}