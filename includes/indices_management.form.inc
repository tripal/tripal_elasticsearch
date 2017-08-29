<?php
// $Id$

/**
 * @file
 * Administrative forms for management of Elasticsearch indices.
 */

/**
 * render array for tripal_elasticsearch_index_website_form
 */
function tripal_elasticsearch_indexing_form($form, &$form_state) {
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
    '#title' => t('Index website or database table'),
    '#options' => drupal_map_assoc(['website', 'database table', 'entities']),
    '#default_value' => 'website',
  ];

  // Need base url for website indexing.
  $form['website_base_url'] = [
    '#type' => 'textfield',
    '#title' => t('Enter your website base url'),
    '#description' => t('Enter the base url of your website. Examples: "http://www.example.com",  
                           "http://www.example.com:8080". Do not include the forward slash (/) at the end.'),
    '#states' => [
      'visible' => [
        ':input[name="website_or_table"]' => [
          ['value' => 'website'],
          ['value' => 'entities'],
        ],
      ],
    ],

  ];

  $form['index_name'] = [
    '#type' => 'textfield',
    '#title' => t('Enter a unique Elasticsearch index name'),
    '#field_suffix' => 'less than 28 characters',
    '#description' => t('Elasticsearch index name can only contain lowercase letters, numbers and underscores.  It must start with a letter.'),
    // This field is only visible to table indexing.
    '#states' => [
      'invisible' => [
        ':input[name="website_or_table"]' => [
          ['value' => 'website'],
          ['value' => 'entities'],
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
          ['value' => 'website'],
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
 * AJAX callback for table fields
 */
function tripal_elasticsearch_table_fields_ajax_callback($form, &$form_state) {
  return $form['table_fields'];
}

/**
 * tripal_elasticsearch_indexing_form validation
 */
function tripal_elasticsearch_indexing_form_validate($form, &$form_state) {
  // validate base url if it is website indexing
  $type = $form_state['values']['website_or_table'];
  if ($type == 'website' || $type == 'entities') {
    $website_base_url = $form_state['values']['website_base_url'];
    // Define a variable to store website base url.
    variable_set('website_base_url', $website_base_url);
    try {
      $content = @file_get_contents($website_base_url . '/node');
      if ($content === FALSE) {
        $error_message = t('The website base url is invalid. Please check your website base url.');
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
      form_set_error('index_name', t('"website" is reserved for the website index. Please
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
 */
function tripal_elasticsearch_indexing_form_submit($form, &$form_state) {

  $queue_number = $form_state['values']['queue_number'];

  /*
   * Build Elasticsearch index settings, which needs the following parameters
   *    $index_name
   *    $shards
   *    $replicas
   *    $character_filters
   *    $tokenizer
   *    $token_filters
   *    $field_mapping_types
   */

  $website_or_table = $form_state['values']['website_or_table'];

  if ($website_or_table == 'website') {
    // indexing website: has default index settings.
    // $index_name = $form_state['values']['index_name'];
    $index_name = 'website';
    $index_table = 'node';
    $index_type = 'website';
    $character_filters = drupal_map_assoc(['html_string']);
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
    $character_filters = drupal_map_assoc(['html_string']);
    $tokenizer = $form_state['values']['index_settings']['tokenizer'];
    $token_filters = array_filter($form_state['values']['index_settings']['token_filters']);
    $field_mapping_types = $form_state['values']['table_fields'];
    $field_mapping_types = array_filter($form_state['values']['table_fields']);
  }

  build_elasticsearch_index($index_name = $index_name, $shards = 5, $replicas = 0, $character_filters = $character_filters, $tokenizer = $tokenizer, $token_filters = $token_filters, $field_mapping_types = $field_mapping_types);

  /*
   * Bind the following data to a cron queue item.
   *    index_name
   *    index_type
   *    index_table
   *    selected_table_fields
   *    field_mapping_types
   *    sql query that will extract values from the database table for the selected table fields.
   */

  // Get row count of selected table.
  $row_count = db_query("SELECT COUNT(*) FROM {$index_table}")->fetchAssoc()['count'];
  // Get total number of offsets (offset interval is 1000)
  $k = 1000;
  $total_offsets = intval($row_count / $k);
  // Separate table fields with comma
  $comma_separated_fields = implode(',', array_keys($field_mapping_types));
  $order_by_field = array_keys($field_mapping_types)[0];
  foreach (range(0, $total_offsets) as $offset) {
    $id = $offset % $queue_number + 1;
    $cron_queue_id = 'elasticsearch_queue_' . $id;
    $cron_queue = DrupalQueue::get($cron_queue_id);
    $OFFSET = $k * $offset;
    $item = new stdClass();

    // Use the first field to sort the table
    if ($website_or_table == 'website') {
      $sql = "SELECT nid,title,type FROM $index_table ORDER BY $order_by_field LIMIT $k OFFSET $OFFSET";
    }
    elseif ($website_or_table == 'entities') {
      $sql = "SELECT tripal_entity.id AS entity_id, title, label AS bundle_label
              FROM tripal_entity
              JOIN tripal_bundle ON tripal_entity.term_id = tripal_bundle.term_id
              ORDER BY title ASC LIMIT $k OFFSET $OFFSET";
    }
    else {
      $sql = "SELECT $comma_separated_fields FROM $index_table ORDER BY $order_by_field LIMIT $k OFFSET $OFFSET";
    }

    $item->index_name = $index_name;
    $item->index_type = $index_type;
    $item->website_or_table = $website_or_table;
    $item->field_mapping_types = $field_mapping_types;
    $item->sql = $sql;

    $cron_queue->createItem($item);
  }

  $form_state['rebuild'] = TRUE;

  drupal_set_message('Index created successfully');
}

/**
 * Render array for tripal_elasticsearch_delete_indices_form.
 */
function tripal_elasticsearch_delete_indices_form($form, &$form_state) {
  $existing_indices = [];
  try {
    libraries_load('elasticsearch-php');
    $elasticsearch_host = variable_get('elasticsearch_host');
    $client = Elasticsearch\ClientBuilder::create()
                                         ->setHosts([$elasticsearch_host])
                                         ->build();
    // associate index name with indexed table.
    $raw_existing_indices = array_keys($client->indices()->getMapping());
    $mappings = $client->indices()->getMapping();
    $existing_indices = ['entities', 'website'];
    foreach ($raw_existing_indices as $index) {
      if (!in_array($index, ['entities', 'website'])) {
        $new_index = [$index . ' (indexed table: ' . array_keys($mappings[$index]['mappings'])[1] . ')'];
        $existing_indices = array_merge($existing_indices, $new_index);
      }
    }
  } catch (\Exception $e) {
    $message = $e->getMessage();
    drupal_set_message($message, 'warning');
    drupal_set_message("Please check your Elasticsearch Connection.", 'warning');
  }

  $form['indices'] = [
    '#type' => 'checkboxes',
    '#title' => t('Existing indices'),
    '#description' => t('Delete indexed data from Eleasticsearch. Please make sure you know 
												what the data is. This process can not be undone.'),
    '#options' => drupal_map_assoc($existing_indices),
  ];

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Delete'),
  ];

  drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch') . '/js/indices_delete_confirm.js');

  return $form;
}

/**
 * Submit function for the tripal_elasticsearch_delete_indices_form
 */
function tripal_elasticsearch_delete_indices_form_submit($form, &$form_state) {
  $delete_indices = array_filter($form_state['values']['indices']);
  if (!empty($delete_indices)) {
    foreach ($delete_indices as $index) {
      try {
        libraries_load('elasticsearch-php');
        $elasticsearch_host = variable_get('elasticsearch_host');
        $client = Elasticsearch\ClientBuilder::create()
                                             ->setHosts([$elasticsearch_host])
                                             ->build();
        $response = $client->indices()->delete(['index' => $index]);
      } catch (\Exception $e) {
        $message = $e->getMessage();
        drupal_set_message($message, 'warning');
      }
    }
  }
}
