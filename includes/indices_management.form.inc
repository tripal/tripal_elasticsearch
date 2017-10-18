<?php
/**
 * @file
 * Administrative forms for management of Elasticsearch indices.
 */

/**
 * Render array for tripal_elasticsearch_index_website_form.
 *
 * @param array $form
 * @param array $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_indexing_form($form, &$form_state) {
  try {
    $es = new ESInstance();

    // associate index name with indexed table.
    $indices = $es->getIndices();
  } catch (\Exception $exception) {
    drupal_set_message($exception->getMessage(), 'warning');
    drupal_set_message("Please check your Elasticsearch Connection.", 'warning');
    return;
  }

  $form['instructions'] = [
    '#type' => 'markup',
    '#markup' => '<h1>Create New Index</h1>
<p>You can create indices for your local Elasticsearch server here.  There are three types of indices:  website-wide <b>node</b>, website-wide <b>Tripal entity</b>, and <b>database table</b>.  
The website-wide indices are intended to index all Tripal 2 or Tripal 3 content, respectively, and cannot be configured beyond Token settings.
  <b>Database table</b> indices can be customized by indexing and displaying specific fields. </p><p>Please note that that the indices of remote Elasticsearch servers cannot be created or edited.</p>',
  ];

  $index_types = [
    'database' => 'Database Table',
  ];

  //Only one of each index type below is allowed.
  if (!in_array('website', $indices)) {
    $index_types['website'] = 'Website Nodes';
  }
  if (!in_array('gene_search_index', $indices)) {
    $index_types['gene_search'] = 'Gene Search';
  }
  if (!in_array('entities', $indices)) {
    $index_types['entities'] = 'Website Tripal Entities';
  }

  // Index website or a database table
  $form['index_type'] = [
    '#type' => 'select',
    '#title' => t('Index type'),
    '#description' => t('If you already have a website-wide node or tripal entities index, it must be edited or deleted in the indices list page.  Website Tripal entities index is available for Tripal version 3 only.'),
    '#options' => $index_types,
    '#default_value' => isset($index_types['website']) ? 'website' : 'database',
  ];

  $form['index_name'] = [
    '#type' => 'textfield',
    '#title' => t('Enter a unique Elasticsearch index name'),
    '#field_suffix' => 'less than 28 characters',
    '#description' => t('Elasticsearch index name can only contain lowercase letters, numbers and underscores. It must start with a letter.'),
    // This field is only visible to table indexing.
    '#states' => [
      'visible' => [
        ':input[name="index_type"]' => ['value' => 'database'],
      ],
    ],
    '#size' => 25,
  ];

  // Elasticsearch index settings
  $form['index_settings'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Elasticsearch index settings'),
    '#description' => t('These settings determine how your data will be indexed and made searchable.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="index_type"]' => [
          ['value' => 'database'],
        ],
      ],
    ],
  ];
  //	$form['index_settings']['character_filters'] = array(
  //		'#type' => 'checkboxes',
  //		'#title' => t('Character filters'),
  //		'#options' => get_character_filter_options(),
  //	);
  $form['index_settings']['tokenizer'] = [
    '#type' => 'select',
    '#title' => t('Tokenizer'),
    '#options' => get_tokenizer_options(),
  ];
  $form['index_settings']['token_filters'] = [
    '#type' => 'checkboxes',
    '#title' => t('Token filters'),
    '#options' => get_token_filter_options(),
  ];

  // Table fields and mapping types
  $form['index_table'] = [
    '#type' => 'select',
    '#title' => t('Select a table and fields to index'),
    '#options' => array_merge(['' => 'Select a table'], drupal_map_assoc(tripal_elasticsearch_get_table_list())),
    '#states' => [
      'visible' => [
        ':input[name="index_type"]' => [
          ['value' => 'database'],
        ],
      ],
    ],
    '#ajax' => [
      'callback' => 'tripal_elasticsearch_table_fields_ajax_callback',
      'wrapper' => 'tripal_elasticsearch_table_fields_wrapper',
    ],
  ];

  $table_name = isset($form_state['values']['index_table']) ? $form_state['values']['index_table'] : '';
  $columns = tripal_elasticsearch_get_column_list($table_name);

  $form['table_fields'] = [
    '#type' => 'fieldset',
    '#title' => t('Select fields to index'),
    '#tree' => TRUE,
    '#options' => drupal_map_assoc($columns),
    '#states' => [
      'invisible' => [
        ':input[name="index_table"]' => ['value' => ''],
      ],
    ],
    '#prefix' => '<div id="tripal_elasticsearch_table_fields_wrapper">',
    '#suffix' => '</div>',
  ];

  foreach ($columns as $field) {
    $form['table_fields'][$field] = [
      '#type' => 'select',
      '#title' => t('Field name: ' . $field),
      '#description' => t('Please select a mapping type for each field. If no 
													mapping type is selected for a field, that field will
													not be indexed. This can be used to selectively index
													table fields.'),
      '#options' => array_merge(['' => 'Select mapping type'], get_field_mapping_types()),
    ];
  }

  $form['exposed'] = [
    '#type' => 'checkbox',
    '#title' => t('Expose Index to Cross-Site Search'),
    '#description' => t("Check this box to expose your index to cross-site search.  If exposed, other Tripal sites using this module will be able to search your sites and display the results on their own.  Exposing an index is READ ONLY."),
    '#default_value' => FALSE,
  ];

  $form['url'] = [
    '#type' => 'textfield',
    '#title' => 'Results URL Path',
    '#description' => t('In order for other sites to link back to your results page, you must specify a path where the form for this index can be reached.'),
    '#attributes' => [
      'placeholder' => 'E.g, elasticsearch/index-name',
    ],
    '#states' => [
      'visible' => [
        ':input[name="index_type"]' => [
          ['value' => 'database'],
          ['value' => 'gene_search'],
        ],
        ':input[name="exposed"]' => ['value' => 1],
      ],
    ],
  ];

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Create New Index'),
  ];

  return $form;
}

/**
 * AJAX callback for table fields.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_table_fields_ajax_callback($form, &$form_state) {
  return $form['table_fields'];
}

/**
 * tripal_elasticsearch_indexing_form validation
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_indexing_form_validate($form, &$form_state) {
  $type = $form_state['values']['index_type'];
  $url = $form_state['values']['url'];
  //TODO: check validity of URL instead of checking if it exists
  if ($type == "database") {
    // URLs are required for database and gene_search
    if (!$url) {
      form_set_error('url', t('Please provide a valid url'));
    }
    // Index name validation.
    $index_name = $form_state['values']['index_name'];
    if (strlen($index_name) > 28) {
      form_set_error('index_name', t('String length cannot be greater than 28.'));
    }
    if (!preg_match('/^[A-Za-z][A-Za-z0-9_]+$/', $index_name)) {
      form_set_error('index_name', t('index name can only contain lowercase letters, 
								numbers and underscores, and must start with a letter.'));
    }
    // 'website' is reserved for website indexing and search, use a different name
    // as index name for table search.
    if ($index_name == 'website') {
      form_set_error('index_name', t('"website" is reserved for the website nodes index. Please
					use a different name.'));
    }
    // At least one table field need to be selected.
    $table_fields = array_filter($form_state['values']['table_fields']);
    if (empty($table_fields)) {
      form_set_error('table_fields', t('Please specify a mapping type for at least one field.'));
    }
  }
  if ($type == "gene_search_index") {
    if (!$url) {
      form_set_error('url', t('Please provide a valid url'));
    }
  }
}

/**
 * tripal_elasticsearch_indexing_form submission.
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_indexing_form_submit($form, &$form_state) {
  //Create the new index
  tripal_elasticsearch_create_index($form_state['values']);
}

/**
 * Render array for tripal_elasticsearch_delete_indices_form.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed|null
 */
function tripal_elasticsearch_delete_indices_form($form, &$form_state) {
  try {
    $client = (new ESInstance())->client;

    // associate index name with indexed table.
    $mappings = $client->indices()->getMapping();
  } catch (\Exception $exception) {
    drupal_set_message($exception->getMessage(), 'warning');
    drupal_set_message("Please check your Elasticsearch Connection.", 'warning');
    return NULL;
  }

  $existing_indices = [];
  foreach (array_keys($mappings) as $index) {
    if (isset(array_keys($mappings[$index]['mappings'])[1])) {
      if (!in_array($index, ['entities', 'website'])) {
        $existing_indices[$index] = $index . ' (indexed table: <b>' . array_keys($mappings[$index]['mappings'])[1] . '</b>)';
      }
      else {
        $existing_indices[$index] = $index . ' (index for site-wide search)';
      }
    }
    else {
      $existing_indices[$index] = $index . ' (empty index)';
    }
  }

  $form['indices'] = [
    '#type' => 'checkboxes',
    '#title' => t('Existing indices'),
    '#description' => t('delete indexed data from Eleasticsearch. Please make sure you know what the data is. This process can NOT be undone.'),
    '#options' => $existing_indices,
  ];

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Delete'),
  ];

  drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/indices_delete_confirm.js');

  return $form;
}

/**
 * Submit function for the tripal_elasticsearch_delete_indices_form.
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_delete_indices_form_submit($form, &$form_state) {
  $delete_indices = array_filter($form_state['values']['indices']);
  if (!empty($delete_indices)) {
    foreach ($delete_indices as $index) {
      try {
        $es = new ESInstance();
        $es->deleteIndex($index);
        db_query('DELETE FROM {tripal_elasticsearch_indices} WHERE index_name=:index_name', [
          ':index_name' => $index,
        ]);
      } catch (\Exception $exception) {
        drupal_set_message($exception->getMessage(), 'warning');
      }
    }
  }
}

/**
 * Indices List Page.
 *
 * @return array|mixed
 */
function tripal_elasticsearch_indices_list_page() {
  try {
    $es = new ESInstance();

    // associate index name with indexed table.
    $indices = $es->getIndices();
  } catch (\Exception $exception) {
    drupal_set_message($exception->getMessage(), 'warning');
    drupal_set_message("Please check your Elasticsearch Connection.", 'warning');
    return '';
  }

  // Get table indices
  $sql = 'SELECT DISTINCT(index_name), table_name, exposed FROM {tripal_elasticsearch_indices}';
  $table_indices = db_query($sql)->fetchAll();
  $tables = [];
  foreach ($table_indices as $index) {
    $tables[$index->index_name]["table"] = $index->table_name;
    $tables[$index->index_name]["exposed"] = $index->exposed;
  }

  $rows = [];
  foreach ($indices as $index) {
    if ($index === 'entities') {
      $table = 'Indexes Tripal Entities';
    }
    elseif ($index === 'website') {
      $table = 'Indexes Drupal Nodes';
    }
    elseif (isset($tables[$index])) {
      $table = $tables[$index]["table"];
    }
    else {
      $table = 'Unknown';
    }
    if (isset($tables[$index])) {
      $exposed = $tables[$index]["exposed"];
    }
    $exposed_text = "private";

    if (isset($exposed) && $exposed) {
      $exposed_text = "public";
    }

    $edit = l('Edit', 'admin/tripal/extension/tripal_elasticsearch/indices_management/edit/' . $index);
    $delete = l('Delete', 'admin/tripal/extension/tripal_elasticsearch/indices_management/delete/' . $index);
    $rows[] = [$index, $table, $exposed_text, $edit, $delete];
  }

  $output = '<h2>List of Available Indices</h2>';
  $output .= theme('table', [
    'header' => ['Index Name', 'Indexed Table', 'Exposed', 'Edit', 'Delete'],
    'rows' => $rows,
  ]);
  $link = l('Create Index', 'admin/tripal/extension/tripal_elasticsearch/indices_management/create');
  $output .= '<p>To create a new index, click the ' . $link . ' tab above.</p>';

  return $output;
}

/**
 * @param $form
 * @param $form_state
 * @param $index_name
 *
 * @return mixed
 */
function tripal_elasticsearch_index_edit_confirm(
  $form,
  &$form_state,
  $index_name
) {

  $form = [];
  $exposed = FALSE;
  $url = null;
  //Get table name from index name.
  if ($index_name == 'website') {
    $index_type = "website";
    $table_name = 'node';
    $form['table_name'] = [
      '#type' => 'value',
      '#value' => 'website',
    ];
  }
  elseif ($index_name == 'entities') {
    $index_type = "entities";
    $table_name = 'tripal_entity';
    $form['table_name'] = [
      '#type' => 'value',
      '#value' => 'entities',
    ];
  }
  elseif ($index_name == 'gene_search_index') {
    $index_type = "gene_search";
    $table_name = 'tripal_entity';
    $form['table_name'] = [
      '#type' => 'value',
      '#value' => 'chado.feature',
    ];
  }
  else {
    $index_type = "table";
    $sql = "SELECT DISTINCT * FROM {tripal_elasticsearch_indices} WHERE index_name =:index_name";
    $result = db_query($sql, [':index_name' => $index_name])->fetch();
    $table_name = $result->table_name;
    $form['table_name'] = [
      '#type' => 'value',
      '#value' => $table_name,
    ];
  }
  // need to query ALL types for exposed setting.
  $sql = "SELECT DISTINCT * FROM {tripal_elasticsearch_indices} WHERE index_name =:index_name";
  $result = db_query($sql, [':index_name' => $index_name])->fetch();
  $exposed = $result ? $result->exposed : 0;
  dpm($result);
  $url = $result ? $result->url : null;

  $form['index_name'] = [
    '#type' => 'value',
    '#value' => $index_name,
  ];
  $form['index_type'] = [
    '#type' => 'value',
    '#value' => $index_type,
  ];

  // Get existing index settings and mappings.
  //TODO: what about default tables?
  try {
    $es = new ESInstance();
    $index_settings = $es->getIndexSettings($index_name);
    $index_mappings = $es->getIndexMappings($index_name);
  } catch (Exception $exception) {
    drupal_set_message($exception->getMessage(), 'error');
    return;
  }

  $preset_token_settings = $index_settings[$index_name]["settings"]["index"]["analysis"]["analyzer"][$index_name];

  $preset_mappings = [];

  if ($index_type == 'table') {
    if (array_key_exists($table_name, $index_mappings[$index_name]["mappings"])) {
      $preset_mappings = $index_mappings[$index_name]["mappings"][$table_name]["properties"];
    }
  }

  if (!$preset_mappings && $index_type == "table") {
    // there is no dbtable mapping settings yet.  Cron still needs to run!
    drupal_set_message("The index $index_name has no database mapping.  Please make sure that your CRON job to create the index has finished running.", 'error');
  }

  $index_settings = ["tokenizer" => "standard", "token_filters" => ""];

  if (array_key_exists("tokenizer", $preset_token_settings)) {
    $index_settings["tokenizer"] = $preset_token_settings["tokenizer"];

    if (array_key_exists("filter", $preset_token_settings)) {

      $index_settings["token_filters"] = $preset_token_settings["filter"];
    }
  }

  //Snapshot the settings that require indexing so we can figure out if we need to re-index

  $previous_values = ['index_settings' => $index_settings, ];

  $form['previous_values'] = [
    '#type' => 'values',
    '#value' => $previous_values,
  ];

  $form['instructions'] = [
    '#type' => 'markup',
    '#markup' => '<h1>Edit Index: ' . $index_name . '</h1>
<p>This form will allow you to edit the tokenizer settings and field settings for the index <b>' . $index_name . '.</b>  Please note that changing settings will automatically rebuild the index.</p>',
  ];

  // Elasticsearch index settings
  $form['index_settings'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Elasticsearch index settings'),
    '#description' => t('These settings determine how your data will be indexed and made searchable.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];

  $form['index_settings']['tokenizer'] = [
    '#type' => 'select',
    '#title' => t('Tokenizer'),
    '#options' => get_tokenizer_options(),
    '#default_value' => $index_settings["tokenizer"],
  ];

  $form['index_settings']['token_filters'] = [
    '#type' => 'checkboxes',
    '#title' => t('Token filters'),
    '#options' => get_token_filter_options(),
    '#default_value' => $index_settings["token_filters"] ? $index_settings["token_filters"] : [],
  ];
  // if this is website nodes or entities, return here.
  $form['exposed'] = [
    '#type' => 'checkbox',
    '#title' => t('Expose Index to Cross-Site Search'),
    '#description' => t("Check this box to expose your index to cross-site search.  If exposed, other Tripal sites using this module will be able to search your sites and display the results on their own.  Exposing an index is READ ONLY."),
    '#default_value' => $exposed == 1 ? TRUE : FALSE,

  ];


  // url should be set for exposed genes and data tables only
  if ($index_type == "gene_search" || $index_type == "table") {
    $form['url'] = [
      '#type' => 'textfield',
      '#title' => 'Results URL Path',
      '#description' => t('In order for other sites to link back to your results page, you must specify a path where the form for this index can be reached.'),
      '#attributes' => [
        'placeholder' => 'E.g, elasticsearch/index-name',
      ],
      '#states' => [
        'visible' => [
          ':input[name="exposed"]' => ['value' => 1],//TODO: This doesnt update
        ],
      ],
    ];
  }

  if ($index_name == 'website' || $index_name == 'entities' || $index_name == 'gene_search_index') {
    $cancel_path = 'admin/tripal/extension/tripal_elasticsearch/indices_management';

    return confirm_form($form, 'Edit Index', $cancel_path, '', 'Submit Edits');
  }

  $columns = tripal_elasticsearch_get_column_list($table_name);
  $form['table_fields'] = [
    '#type' => 'fieldset',
    '#title' => t('Select fields to index'),
    '#description' => t('Please select a mapping type for each field. If no 
													mapping type is selected for a field, that field will
													not be indexed. This can be used to selectively index
													table fields.'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#options' => drupal_map_assoc($columns),

  ];

  foreach ($columns as $field) {
    $preset = '';
    if ($preset_mappings && array_key_exists($field, $preset_mappings)) {
      $preset = $preset_mappings[$field]["type"];
    }

    $form['table_fields'][$field] = [
      '#type' => 'select',
      '#title' => t('Field name: ' . $field),
      '#default_value' => $preset,
      '#options' => array_merge(['' => 'Select mapping type'], get_field_mapping_types()),
    ];
  }

  $description = 'Edit settings for ' . $index_name;
  $cancel_path = 'admin/tripal/extension/tripal_elasticsearch/indices_management';

  return confirm_form($form, 'Edit Index', $cancel_path, $description, 'Submit Edits');
}

/**
 * Submit editing for index.
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_index_edit_confirm_submit($form, &$form_state) {
  $index = $form_state['values']['index_name'];

  // Delete the existing index in ES
  tripal_elasticsearch_delete_index($index);

  //create the new index
  tripal_elasticsearch_create_index($form_state['values']);
}

/**
 * Index Deletion Confirmation Form.
 *s
 *
 * @param $form
 * @param $form_state
 * @param $index_name
 *
 * @return mixed
 */
function tripal_elasticsearch_index_delete_confirm(
  $form,
  &$form_state,
  $index_name
) {
  $form = [];
  $description = 'Are you sure you want to delete the ' . $index_name . ' index?';
  $cancel_path = 'admin/tripal/extension/tripal_elasticsearch/indices_management';
  $form['index_name'] = [
    '#type' => 'hidden',
    '#value' => $index_name,
  ];
  return confirm_form($form, 'Are you sure?', $cancel_path, $description, 'Delete');
}

/**
 * Index deletion.
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_index_delete_confirm_submit($form, &$form_state) {
  $index = $form_state['values']['index_name'];

  tripal_elasticsearch_delete_index($index);
  drupal_set_message('Index "' . $index . '" deleted successfully.');
  drupal_goto('admin/tripal/extension/tripal_elasticsearch/indices_management');
}

/** Creates an index.  Used by the create and edit index functions.
 *
 * @param $values
 */
function tripal_elasticsearch_create_index(&$values) {
  $index_type = $values['index_type'];
  $exposed = $values['exposed'] ? 1 : 0;
  $url = $values['url'];

  // populate settings for website or entities.
  if ($index_type === 'website') {
    // indexing website: has default index settings.
    $index_name = 'website';
    $index_table = 'node';
    $tokenizer = 'standard';
    $token_filters = drupal_map_assoc([
      'standard',
      'lowercase',
    ]);
    $field_mapping_types = [
      'nid' => 'integer',
      'type' => 'string',
      'title' => 'string',
      'content' => 'string',
    ];
  }
  elseif ($index_type === 'entities') {
    $index_name = 'entities';
    $index_table = 'tripal_entity';
    $tokenizer = 'standard';
    $token_filters = drupal_map_assoc([
      'standard',
      'lowercase',
    ]);
    $field_mapping_types = [
      'entity_id' => 'integer',
      'bundle_label' => 'string',
      'title' => 'string',
      'content' => 'string',
    ];
  }
  elseif ($index_type === 'gene_search') {
    $index_name = 'gene_search_index';
    $index_table = 'chado.feature';
    $tokenizer = 'standard';
    $token_filters = drupal_map_assoc([
      'standard',
      'lowercase',
    ]);
    $field_mapping_types = [
      'uniquename' => 'string',
      'feature_id' => 'integer',
      'organism_genus' => 'string',
      'organism_species' => 'string',
      'organism_common_name' => 'string',
      'annotations' => 'string',
      'blast_hit_descriptions' => 'string',
    ];
  }
  else { //checks and defaults for custom index type
    $index_name = $values['index_name'];
    $index_table = $values['table_name'];
    $tokenizer = $values['index_settings']['tokenizer'];
    $token_filters = array_filter($values['index_settings']['token_filters']);
    $field_mapping_types = array_filter($values['table_fields']);

    if (!$index_name) {
      form_set_error('Error: No index name supplied for custom index');
      return;
    }
    if (!$field_mapping_types) {
      form_set_error('No field mapping types supplied for custom index');
    }
    if (!$tokenizer) {
      // Use standard tokenizer and token filters if none supplied
      $tokenizer = 'standard';
    }
    if (!$token_filters) {
      $token_filters = drupal_map_assoc([
        'standard',
        'lowercase',
      ]);
    }
  }

  //insert index record into database for all types.
  $insert = [
    ':index_name' => $index_name,
    ':table_name' => $index_table,
    ':exposed' => $exposed,
    ':url' => $url,
  ];
  dpm($insert);

  db_query('INSERT INTO {tripal_elasticsearch_indices} (index_name, table_name, exposed, url) VALUES (:index_name, :table_name, :exposed, :url)', $insert);

  // Create the index.
  try {
    $es = new ESInstance();
    $es->setIndexParams($index_name, 5, 0, $tokenizer, $token_filters, $field_mapping_types)
      ->createIndex();
  } catch (Exception $exception) {
    form_set_error($exception->getMessage());
    watchdog('tripal_elasticsearch', $exception->getMessage());
    return;
  }

  // Dispatch jobs to populate the index
  switch ($index_type) {
    case 'website':
      $job = new NodesIndexJob();
      break;
    case 'entities':
      $job = new EntitiesIndexJob();
      break;
    case 'gene_search':
      $job = new GeneSearchIndexJob($index_name);
      break;
    default:
      $fields = array_keys($field_mapping_types);
      $job = new TableIndexJob($index_name, $index_table, $fields);
  }
  $dispatcher = new DispatcherJob($job);
  $dispatcher->dispatch();

  $form_state['rebuild'] = TRUE;

  $base_url = variable_get('website_base_url');
  $cron_url = l($base_url . '/admin/config/system/cron', 'admin/config/system/cron');

  drupal_set_message("The indexing job for {$index_name} has been submitted to your CRON queue.  You can view the status of your CRON jobs at {$cron_url}");
  drupal_goto('admin/tripal/extension/tripal_elasticsearch/indices_management');
}

/**Deletes an index from elasticsearch and from the db
 *
 * @param $index
 */
function tripal_elasticsearch_delete_index($index) {
  // Find the index and delete it from ES
  try {
    $es = new ESInstance();
    $es->deleteIndex($index);
  } catch (Exception $exception) {
    drupal_set_message($exception->getMessage(), 'error');
    return;
  }

  // Find the index and delete it from the DB
  $result = db_query("DELETE FROM {tripal_elasticsearch} WHERE index_name=:index_name", [':index_name' => $index]);

  if (!$result) {
    drupal_set_message("$index could not be deleted from the database.", 'error');
    return;
  }

  // Delete any progress indicator
  db_query('DELETE FROM {tripal_elasticsearch_queues} WHERE index_name=:index_name', [':index_name' => $index]);

  drupal_set_message('Index "' . $index . '" deleted successfully.');
  drupal_goto('admin/tripal/extension/tripal_elasticsearch/indices_management');
}

/**
 * Admin Progress Tracker Page
 */
function tripal_elasticsearch_progress_page($callback = FALSE) {
  $content = '';
  if (!$callback) {
    drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/tripal_elasticsearch_progress_tracker.js');
    $content .= '<div id="progress-report-page">';
  }

  $content .= '<h1>Indexing Progress Tracker</h1>';

  $data = ESQueue::progress();

  $content .= '<h3>Overall Progress</h3>';

  if ($data->remaining === 0) {
    $content .= '<p>All items have been indexed. 0 Items remaining.</p>';
  }
  else {
    $content .= theme_progress_bar([
      'percent' => $data->percent,
      'message' => 'Indexing ' . $data->total . ' items. ' . $data->remaining . ' Items remaining. ' . $data->time . ' seconds elapsed.',
    ]);
  }

  if (count($data->queues) > 1) {
    foreach ($data->queues as $queue => $progress) {
      $content .= '<h3>' . $queue . '</h3>';

      if ($progress->remaining === 0) {
        $content .= '<p>All items have been indexed. 0 Items remaining</p>';
      }
      else {
        $content .= theme_progress_bar([
          'percent' => $progress->percent,
          'message' => $progress->remaining . ' Items remaining. ' . $progress->time . ' seconds passed',
        ]);
      }
    }
  }

  if (!$callback) {
    $content .= '</div>';
  }

  return $content;
}

/**
 * Get progress page without header or footer html.
 */
function tripal_elasticsearch_get_progress() {
  // Prevents rendering the entire page.
  print tripal_elasticsearch_progress_page(TRUE);
}