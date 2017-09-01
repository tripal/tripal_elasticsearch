<?php
/**
 * @file
 * Administrative forms for management of Elasticsearch indices.
 */

/**
 * Render array for tripal_elasticsearch_index_website_form.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_indexing_form($form, &$form_state) {
  // Need base url for website indexing.
  global $base_url;

  $form['instructions'] = [
    '#type' => 'markup',
    '#markup' => '<h1>Create New Index</h1>',
  ];

  $form['website_base_url'] = [
    '#type' => 'item',
    '#title' => t("Base URL"),
    '#markup' => '<p>The Base URL used for this index will be: ' . $base_url . '.  If this doesn\'t match your site, reconfigure your $base_url in your settings.php file.</p>',
    '#states' => [
      'visible' => [
        ':input[name="website_or_table"]' => [
          ['value' => 'website'],
          ['value' => 'entities'],
        ],
      ],
    ],
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

  // Index website or a database table
  $form['website_or_table'] = [
    '#type' => 'select',
    '#title' => t('Index website nodes, website entities, or database table'),
    '#options' => drupal_map_assoc([
      'website nodes',
      'website entities',
      'database table',
    ]),
    '#default_value' => 'website',
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
          ['value' => 'website nodes'],
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
          ['value' => 'website nodes'],
          ['value' => 'website entities'],
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
													no be indexed. This can be used to selectively index
													table fields.'),
      '#options' => array_merge(['' => 'Select mapping type'], get_field_mapping_types()),
    ];
  }

  //	$header = array(
  //		'field' => 'Field',
  //		'mapping_type' => 'Mapping Type',
  //	);
  //	$options = array();
  //	foreach (get_column_list($table_name) as $field) {
  //		$options[$field] = array(
  //			'field' => $field,
  //			'mapping_type' => theme('select', array(
  //				'element' => array(
  //					'#options' => get_field_mapping_types(),
  //				),
  //			)),
  //		);
  //	}

  //	$form['table_fields'] = array(
  //		'#type' => 'tableselect',
  //		'#header' => $header,
  //		'#options' => $options,
  //		'#empty' => t('No content available.'),
  //		'#states' => array(
  //			'invisible' => array(
  //				':input[name="website_or_table"]' => array('value' => 'website'),
  //			),
  //		),
  //		'#prefix' => '<div id="tripal_elasticsearch_table_fields_wrapper">',
  //		'#suffix' => '</div>',
  //	);

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Submit'),
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
  if ($type == 'website nodes' || $type == 'website entities') {
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

  if ($website_or_table == 'website nodes') {
    // indexing website: has default index settings.
    // $index_name = $form_state['values']['index_name'];
    $index_name = 'website';
    $index_table = 'node';
    $index_type = 'website';
    //$character_filters = drupal_map_assoc(['html_string']);
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
  elseif ($website_or_table === 'website entities') {
    // indexing website: has default index settings.
    // $index_name = $form_state['values']['index_name'];
    $index_name = 'entities';
    $index_table = 'tripal_entity';
    $index_type = 'entities';
    $character_filters = drupal_map_assoc(['html_string']);
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
    $index_type = $index_table;
    //		$character_filters = array_filter($form_state['values']['index_settings']['character_filters']);
    //$character_filters = drupal_map_assoc(['html_string']);
    $tokenizer = $form_state['values']['index_settings']['tokenizer'];
    $token_filters = array_filter($form_state['values']['index_settings']['token_filters']);
    $field_mapping_types = array_filter($form_state['values']['table_fields']);
  }

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
  $form = drupal_get_form('tripal_elasticsearch_indexing_form');

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
  $sql = 'SELECT DISTINCT(index_name), table_name FROM {tripal_elasticsearch}';
  $table_indices = db_query($sql)->fetchAll();
  $tables = [];
  foreach ($table_indices as $index) {
    $tables[$index->index_name] = $index->table_name;
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
      $table = $tables[$index];
    }
    else {
      $table = 'Unknown';
    }

    $delete = l('Delete', 'admin/tripal/extension/tripal_elasticsearch/indices_management/delete/' . $index);
    $rows[] = [$index, $table, $delete];
  }

  $output = '<h3>List of Available Indices</h3>';
  $output .= theme('table', [
    'header' => ['Index Name', 'Indexed Table', 'Delete'],
    'rows' => $rows,
  ]);

  $form['indices_list'] = [
    '#type' => 'markup',
    '#markup' => $output
  ];

  return $form;
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
function tripal_elasticsearch_index_delete_confirm($form, &$form_state, $index_name) {
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