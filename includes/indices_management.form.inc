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
  // Need base url for website indexing.
  global $base_url;

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

  $form['website_base_url'] = [
    '#type' => 'item',
    '#title' => t("Base URL"),
    '#markup' => 'The Base URL used for this index will be: ' . $base_url . '.  If this doesn\'t match your site, reconfigure the $base_url variable in your Drupal settings.php file.',
  ];

  // Cron queue numbers
  $form['queue_number'] = [
    '#type' => 'select',
    '#title' => t('Number of cron queues'),
    '#description' => t('10 queues have been pre-defined for indexing jobs. This number determines the number of
                         queues that will be used for indexing jobs. Use more queues if your server has higher capacity
                         for multi-thread processes.'),
    '#options' => drupal_map_assoc(range(1, 10)),
  ];

  $index_types = [
    'database' => 'Database Table',
  ];

  if (!in_array('website', $indices)) {
    $index_types['nodes'] = 'Website Nodes';
  }

  if (!in_array('entities', $indices)) {
    $index_types['entities'] = 'Website Tripal Entities';
  }

  // Index website or a database table
  $form['website_or_table'] = [
    '#type' => 'select',
    '#title' => t('Index type'),
    '#description' => t('If you already have a website-wide node or tripal entities index, it must be edited or deleted in the table below.  Website Tripal entities are available for Tripal version 3 only.'),
    '#options' => $index_types,
    '#default_value' => isset($index_types['nodes']) ? 'nodes' : 'database',
    '#attributes' => (in_array('website', $indices) && in_array('entities', $indices)) ? ['disabled' => 'disabled'] : ['enabled' => 'enabled'],
    //Disable if there is already a node and entity index.
  ];

  $form['index_name'] = [
    '#type' => 'textfield',
    '#title' => t('Enter a unique Elasticsearch index name'),
    '#field_suffix' => 'less than 28 characters',
    '#description' => t('Elasticsearch index name can only contain lowercase letters, numbers and underscores. It must start with a letter.'),
    // This field is only visible to table indexing.
    '#states' => [
      'invisible' => [
        ':input[name="website_or_table"]' => [
          ['value' => 'nodes'],
          ['value' => 'website entities'],
        ],
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
      'invisible' => [
        ':input[name="website_or_table"]' => [
          ['value' => 'website'],
          ['value' => 'entities'],
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
    '#options' => array_merge(['' => 'Select a table'], drupal_map_assoc(get_table_list())),
    '#states' => [
      'invisible' => [
        ':input[name="website_or_table"]' => [
          ['value' => 'nodes'],
          ['value' => 'entities'],
        ],
      ],
    ],
    '#ajax' => [
      'callback' => 'tripal_elasticsearch_table_fields_ajax_callback',
      'wrapper' => 'tripal_elasticsearch_table_fields_wrapper',
    ],
  ];

  $table_name = isset($form_state['values']['index_table']) ? $form_state['values']['index_table'] : '';
  $form['table_fields'] = [
    '#type' => 'fieldset',
    '#title' => t('Select fields to index'),
    '#tree' => TRUE,
    '#options' => drupal_map_assoc(get_column_list($table_name)),
    '#states' => [
      'invisible' => [
        ':input[name="index_table"]' => ['value' => ''],
      ],
    ],
    '#prefix' => '<div id="tripal_elasticsearch_table_fields_wrapper">',
    '#suffix' => '</div>',
  ];

  foreach (get_column_list($table_name) as $field) {
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
  // validate base url if it is website indexing
  $type = $form_state['values']['website_or_table'];
  if ($type == 'nodes' || $type == 'entities') {
    global $base_url;
    // Define a variable to store website base url.
    variable_set('website_base_url', $base_url);
    try {
      $content = @file_get_contents($base_url . '/node');
      if ($content === FALSE) {
        $error_message = t('The website base url is invalid. You can reconfigure your $base_url variable in your settings.php file.');
        form_set_error('website_base_url', $error_message);
      }
    } catch (Exception $e) {
      form_set_error('website_base_url', $e->getMessage());
    }
  }
  else {
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
}

/**
 * tripal_elasticsearch_indexing_form submission.
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_indexing_form_submit($form, &$form_state) {
  $queue_number = $form_state['values']['queue_number'];
  // Build Elasticsearch index settings.
  $website_or_table = $form_state['values']['website_or_table'];
  if ($website_or_table === 'nodes') {
    // indexing website: has default index settings.
    $exposed = $form_state['values']['index_table'];
    $index_name = 'website';
    $index_table = 'node';
    $index_type = 'website';
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
  elseif ($website_or_table === 'entities') {
    // indexing website: has default index settings.
    // $index_name = $form_state['values']['index_name'];
    $index_name = 'entities';
    $index_table = 'tripal_entity';
    $index_type = 'entities';
    $exposed = $form_state['values']['index_table'];
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
  else {
    // indexing a table: has customized index settings
    $index_name = $form_state['values']['index_name'];
    $index_table = $form_state['values']['index_table'];
    $exposed = $form_state['values']['exposed'];
    $index_type = $index_table;
    $tokenizer = $form_state['values']['index_settings']['tokenizer'];
    $token_filters = array_filter($form_state['values']['index_settings']['token_filters']);
    $field_mapping_types = array_filter($form_state['values']['table_fields']);
  }

  //insert index record into database for all types.
  $insert = [
    ':index_name' => $index_name,
    ':table_name' => $index_table,
    ':exposed' => intval($exposed) ? 1 : 0,
  ];
  db_query('INSERT INTO {tripal_elasticsearch_indices} (index_name, table_name, exposed) VALUES (:index_name, :table_name, :exposed)', $insert);

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

  // Populate the index
  $es->populateIndex($website_or_table, $index_table, $index_name, $index_type, $field_mapping_types, $queue_number);

  $form_state['rebuild'] = TRUE;

  drupal_set_message('Index created successfully');
  drupal_goto('admin/tripal/extension/tripal_elasticsearch/indices_management');
}

/**
 * Render array for tripal_elasticsearch_delete_indices_form.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_delete_indices_form($form, &$form_state) {
  try {
    $client = (new ESInstance())->client;

    // associate index name with indexed table.
    $mappings = $client->indices()->getMapping();
  } catch (\Exception $exception) {
    drupal_set_message($exception->getMessage(), 'warning');
    drupal_set_message("Please check your Elasticsearch Connection.", 'warning');
    return;
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
    '#description' => t('DELETE indexed DATA FROM Eleasticsearch. Please make sure you know 
												what the DATA IS. This process can NOT be undone.'),
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
    return;
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
function tripal_elasticsearch_index_edit_confirm($form, &$form_state, $index_name) {

  $form = [];
  $exposed = FALSE;
  //Get table name from index name.
  if ($index_name == 'website') {
    $website_or_table = "nodes";
    $form['table_name'] = [
      '#type' => 'value',
      '#value' => 'website',
    ];
  }
  elseif ($index_name == 'entities') {
    $website_or_table = "entities";
    $form['table_name'] = [
      '#type' => 'value',
      '#value' => 'entities',
    ];
  }
  else {
    $website_or_table = "table";
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
  $exposed = $result->exposed;

  $form['index_name'] = [
    '#type' => 'value',
    '#value' => $index_name,
  ];
  $form['website_or_table'] = [
    '#type' => 'value',
    '#value' => $website_or_table,
  ];

  //get existing index settings and mappings.
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

  if ($website_or_table == 'table') {
    if (array_key_exists($table_name, $index_mappings[$index_name]["mappings"])) {
      $preset_mappings = $index_mappings[$index_name]["mappings"][$table_name]["properties"];
    }
  }

  if (!$preset_mappings && $website_or_table == "table") {
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

  $form['instructions'] = [
    '#type' => 'markup',
    '#markup' => '<h1>Edit Index: ' . $index_name . '</h1>
<p>This form will allow you to edit the tokenizer settings and field settings for the index <b>' . $index_name . '.</b>  Please note that changing settings will automatically rebuild the index.</p>',
  ];

  $form['queue_number'] = [
    '#type' => 'select',
    '#title' => t('Number of cron queues'),
    '#description' => t('10 queues have been pre-defined for indexing jobs. This number determines the number of
                         queues that will be used for indexing jobs. Use more queues if your server has higher capacity
                         for multi-thread processes.'),
    '#options' => drupal_map_assoc(range(1, 10)),
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

  if ($index_name == 'website' || $index_name == 'entities') {


    $cancel_path = 'admin/tripal/extension/tripal_elasticsearch/indices_management';

    return confirm_form($form, 'Edit Index', $cancel_path, '', 'Submit Edits');
  }

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
    '#options' => drupal_map_assoc(get_column_list($table_name)),

  ];

  foreach (get_column_list($table_name) as $field) {

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

  $description = 'Edit settings for' . $index_name;
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
  try {
    $es = new ESInstance();
    $es->deleteIndex($index);
  } catch (Exception $exception) {
    drupal_set_message($exception->getMessage(), 'error');
    return;
  }
  // Find the index and delete it from the DB

  $result = db_query("DELETE FROM {tripal_elasticsearch_indices} WHERE index_name=:index_name", [':index_name' => $index]);
  if (!$result) {
    drupal_set_message("$index could not be deleted from the database.", 'error');
    return;
  }
  //Create the new index

  //TODO:  THIS CODE IS VERY REDUNDANT WITH CREATE SUBMIT.  REFACTOR.

  $queue_number = $form_state['values']['queue_number'];
  // Build Elasticsearch index settings.
  $website_or_table = $form_state['values']['website_or_table'];
  if ($website_or_table === 'nodes') {
    // indexing website: has default index settings.
    $exposed = $form_state['values']['exposed'];
    $index_name = 'website';
    $index_table = 'node';
    $index_type = 'website';
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
  elseif ($website_or_table === 'entities') {
    $exposed = $form_state['values']['exposed'];
    $index_name = 'entities';
    $index_table = 'tripal_entity';
    $index_type = 'entities';
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
  else {
    // indexing a table: has customized index settings
    $index_name = $form_state['values']['index_name'];
    $index_table = $form_state['values']['table_name'];
    $index_type = $index_table;
    $exposed = $form_state['values']['exposed'];
    $tokenizer = $form_state['values']['index_settings']['tokenizer'];
    $token_filters = array_filter($form_state['values']['index_settings']['token_filters']);
    $field_mapping_types = array_filter($form_state['values']['table_fields']);
  }

  //insert index record into database for all types.
  $insert = [
    ':index_name' => $index_name,
    ':table_name' => $index_table,
    ':exposed' => $exposed,
  ];

  db_query('INSERT INTO {tripal_elasticsearch_indices} (index_name, table_name, exposed) VALUES (:index_name, :table_name, :exposed)', $insert);

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

  // Populate the index
  $es->populateIndex($website_or_table, $index_table, $index_name, $index_type, $field_mapping_types, $queue_number);

  $form_state['rebuild'] = TRUE;

  drupal_set_message('Index update submitted and will be executed by CRON.');
  drupal_goto('admin/tripal/extension/tripal_elasticsearch/indices_management');
}

/**
 * Index Deletion Confirmation Form.
 *
 * @param $form
 * @param $form_state
 * @param $index_name
 *
 * @return mixed
 */
function tripal_elasticsearch_index_delete_confirm(
  $form, &$form_state, $index_name
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

  drupal_set_message('Index "' . $index . '" deleted successfully.');
  drupal_goto('admin/tripal/extension/tripal_elasticsearch/indices_management');
}
