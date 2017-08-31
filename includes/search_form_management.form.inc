<?php
/**
 * Render array for website_search_box_form.
 */
function website_search_box_form($form, &$form_state) {
  $form['container'] = [
    '#type' => 'container',
    '#attributes' => [
      'class' => ['container-inline'],
    ],
  ];
  $form['container']['search_box'] = [
    '#type' => 'textfield',
    '#size' => 15,
  ];

  $form['container']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Search'),
  ];

  return $form;
}

/**
 * website_search_box_form submit
 */
function website_search_box_form_submit($form, &$form_state) {
  $keyword = $form_state['values']['search_box'];
  $url = "tripal_elasticsearch/search_website/$keyword";
  // only redirect to the search results page when $keyword is not empty.
  if (empty($keyword)) {
    drupal_goto(current_path());
  }
  else {
    drupal_goto($url);
  }
}

/**
 * Render array to build a form for creating table search interface.
 */
function table_search_interface_building_form($form, &$form_state) {
  $form['help'] = [
    '#type' => 'markup',
    '#markup' => t('This administrative page is used to build search blocks for 
				indexed data.'),
  ];

  try {
    libraries_load('elasticsearch-php');
    $elasticsearch_host = variable_get('elasticsearch_host');
    $client = Elasticsearch\ClientBuilder::create()
      ->setHosts([$elasticsearch_host])
      ->build();
    $existing_indices = $client->indices()->getMapping();
    if (count($existing_indices) == 0) {
      $form['markup'] = [
        '#type' => 'markup',
        '#markup' => '<h3>No index data exists.</h3>',
      ];

      return $form;
    }
  } catch (\Exception $e) {
    $form['markup'] = [
      '#type' => 'markup',
      '#markup' => '<h3>' . $e->getMessage() . '</h3>',
    ];

    return $form;
  }

  $index_name_options = array_merge(['' => 'Select an index'], drupal_map_assoc(array_keys($existing_indices)));
  // 'website' and 'entities' are reserved for website indexing and search, no need to build search
  // block for this index.
  if (isset($index_name_options['website'])) {
    unset($index_name_options['website']);
  }
	if (isset($index_name_options['entities'])) {
		unset($index_name_options['entities']);
	}
  $form['index_name'] = [
    '#type' => 'select',
    '#title' => t('Select an index to build a search interface'),
    '#options' => $index_name_options,
    '#ajax' => [
      'callback' => 'tripal_elasticsearch_index_fields_ajax_callback',
      'wrapper' => 'tripal_elasticsearch_index_fields_ajax_wrapper',
    ],
  ];

  $index_name = isset($form_state['values']['index_name']) ? $form_state['values']['index_name'] : '';
  if (!empty($index_name)) {
    $index_profile = $existing_indices[$index_name];
    $mappings_keys = drupal_map_assoc(array_keys($existing_indices[$index_name]['mappings']));
    unset($mappings_keys['_default_']);
    $table_name = array_keys($mappings_keys)[0];
    $index_fields = array_keys($index_profile['mappings']['_default_']['properties']);

    $form['table_name'] = [
      '#type' => 'value',
      '#value' => $table_name,
    ];
  }
  else {
    $index_fields = [];
  }

  $form['index_fields'] = [
    '#type' => 'checkboxes',
    '#title' => t('Select index fields'),
    '#description' => t('Only selected fields will show up on the table search interface.'),
    '#options' => drupal_map_assoc($index_fields),
    '#prefix' => '<div id="tripal_elasticsearch_index_fields_ajax_wrapper">',
    '#suffix' => '</div>',
    '#states' => [
      'invisible' => [
        ':input[name="index_name"]' => ['value' => ''],
      ],
    ],
  ];

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#states' => [
      'invisible' => [
        ':input[name="index_name"]' => ['value' => ''],
      ],
    ],
  ];

  return $form;
}

/**
 * index fields ajax callback.
 */
function tripal_elasticsearch_index_fields_ajax_callback($form, &$form_state) {
  return $form['index_fields'];
}

/**
 * table_search_interface_building_form validation.
 */
function table_search_interface_building_form_validate($form, $form_state) {
  $index_fields = array_filter($form_state['values']['index_fields']);
  if (empty($index_fields)) {
    form_set_error('index_fields', t('Please select at least one index field.'));
  }
}

/**
 * table_search_interface_building_form submit.
 */
function table_search_interface_building_form_submit($form, &$form_state) {
  $index_name = $form_state['values']['index_name'];
  $index_fields = array_filter($form_state['values']['index_fields']);
  $table_name = $form_state['values']['table_name'];
  // Delete record if it already exists in database table 'tripal_elasticsearch'.
  $sql = "SELECT index_name FROM {tripal_elasticsearch} WHERE index_name = :index_name";
  $result = db_query($sql, [':index_name' => $index_name])->fetchCol('index_name');
  if (!empty($result)) {
    try {
      db_delete('tripal_elasticsearch')
        ->condition('index_name', $index_name)
        ->execute();
      drupal_set_message('You have successfully updated search block for index -<b>' . $index_name . '</b>-.');
    } catch (\Exception $e) {
      drupal_set_message(t($e->getMessage()), 'warning');
    }
  }

  $record = [];
  foreach ($index_fields as $key => $field) {
    $record['index_name'] = $index_name;
    $record['table_name'] = $table_name;
    $record['index_field'] = $field;
    $record['form_field_title'] = $field;
    $record['form_field_type'] = 'textfield';
    $record['form_field_description'] = '';
    $record['form_field_options'] = '';
    $record['form_field_weight'] = 0;
    try {
      drupal_write_record('tripal_elasticsearch', $record);
    } catch (\Exception $e) {
      drupal_set_message(t($e->getMessage()), 'warning');
    }
  }
}

/**
 * Get the total count for results.
 *
 * @param $params array
 * @param $limit int The index.max_results_window setting in ES
 *                    the limit can be set at elasticsearch.yml file
 *
 * @return mixed
 */
function tripal_elasticsearch_results_count($params, $limit = 1000000) {
  libraries_load('elasticsearch-php');
  $elasticsearch_host = variable_get('elasticsearch_host');
  $client = Elasticsearch\ClientBuilder::create()
    ->setHosts([$elasticsearch_host])
    ->build();

  unset($params['from']);
  unset($params['size']);

  $c = $client->count($params);

  return min($c['count'], $limit);
}

/**
 * Form builder function for table search.
 */
function tripal_elasticsearch_build_search_block_form($form, &$form_state, $index_name) {
  $sql = "SELECT * FROM {tripal_elasticsearch} WHERE index_name = :index_name";
  $result = db_query($sql, [':index_name' => $index_name])->fetchAll();
  $index_fields = [];
  foreach ($result as $record) {
    switch ($record->form_field_type) {
      case 'textfield':
        $form[$record->index_field] = [
          '#type' => 'textfield',
          '#description' => $record->form_field_description,
          '#title' => t($record->form_field_title),
          '#size' => 25,
          '#weight' => $record->form_field_weight,
          /*'#ajax' => [
            'callback' => 'search_content_ajax_callback',
            'wrapper' => 'search_content_ajax_wrapper',
            'keypress' => TRUE,
          ],*/
          '#default_value' => isset($_GET[$record->index_field]) ? $_GET[$record->index_field] : '',
        ];
        break;
      case 'select':
        $select_options_text = $record->form_field_options;

        preg_match_all('/\[.+?\]/', $select_options_text, $matches);
        $select_options_pairs = $matches[0];

        $select_options_keys = array_map(function ($string) {
          $string = preg_replace('/\[/', '', $string);

          return explode('|', $string)[0];
        }, $select_options_pairs);

        $select_options_values = array_map(function ($string) {
          $string = preg_replace('/\]/', '', $string);

          return explode('|', $string)[1];
        }, $select_options_pairs);

        $select_options = array_merge(['' => 'Select'], array_combine($select_options_keys, $select_options_values));

        $default_value = '';
        if (isset($_GET[$record->index_field])) {
          $default_value = $_GET[$record->index_field];
        }

        $form[$record->index_field] = [
          '#type' => 'select',
          '#description' => $record->form_field_description,
          '#title' => t($record->form_field_title),
          '#options' => $select_options,
          '#weight' => $record->form_field_weight,
          /*'#ajax' => [
            'callback' => 'search_content_ajax_callback',
            'wrapper' => 'search_content_ajax_wrapper',
            'keypress' => FALSE,
          ],*/
          '#default_value' => $default_value,
        ];
        break;
    }

    if (isset($form_state['values'][$record->index_field])) {
      $index_fields[$record->index_field] = $form_state['values'][$record->index_field];
    }
  }

  // get the number of total search results.
  $index_type = $record->table_name;
  $count = '';
  //$window_range = drupal_map_assoc([1]);
  if (count($index_fields) > 0) {
    // Build search query for table search.
    $query = build_search_query_from_field_content_pairs($index_fields);
    // Build table search params.
    $params = build_table_search_params($index_name, $index_type, $query);
    unset($params['size']);
    unset($params['from']);
    try {
      $results_count = tripal_elasticsearch_results_count($params);
      $count = '<span style="color: red"><b>' . $results_count . '</b> results match your search.</span>';
    } catch (\Exception $e) {
      drupal_set_message($e->getMessage(), 'warning');
    }
  }

  $form['count'] = [
    '#type' => 'markup',
    '#markup' => t($count),
    '#prefix' => '<div id="count_ajax_wrapper">',
    '#suffix' => '</div>',
    '#weight' => 97,
  ];

  // Create a value form field to store index name. This value will be passed to $_GET.
  $form['index_name'] = [
    '#type' => 'hidden',
    '#value' => $index_name,
  ];

  $form['index_type'] = [
    '#type' => 'hidden',
    '#value' => $index_type,
  ];

  $form['#method'] = 'get';

  // submit button
  $form['actions'] = ['#type' => 'actions'];
  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Search'),
    '#weight' => 100,
  ];

  return $form;
}

function tripal_elasticsearch_build_search_block_form_validate($form, &$form_state) {
}

/**
 * Form validation for tripal_elasticsearch_build_search_block_form.
 */
function tripal_elasticsearch_build_search_block_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback for select_window.
 */
function select_window_ajax_callback($form, &$form_state) {
  return $form['select_window'];
}

/**
 * Ajax callback for save_to_file.
 */
function save_to_file_ajax_callback($form, &$form_state) {
  return $form['save_to_file'];
}

/**
 * ajax search_content_ajax_callback
 */
function search_content_ajax_callback($form, &$form_state) {
  $commands = [];
  $commands[] = ajax_command_replace('#count_ajax_wrapper', drupal_render($form['count']));
  $commands[] = ajax_command_replace("#select_window_ajax_wrapper", drupal_render($form['select_window']));

  return ['#type' => 'ajax', '#commands' => $commands];
}

/**
 * Form builder for table_search_block_altering_form.
 */
function table_search_block_altering_form($form, &$form_state) {
  // Get all index_name from database table tripal_elasticsearch.
  $sql = "SELECT DISTINCT index_name FROM {tripal_elasticsearch}";
  $search_blocks = db_query($sql)->fetchCol('index_name');

  if (empty($search_blocks)) {
    $form['no_search_block'] = [
      '#type' => 'markup',
      '#markup' => t('No search block is available. Please build search block first.'),
    ];

    return $form;
  }

  $form['search_block'] = [
    '#type' => 'select',
    '#title' => t('Search blocks'),
    '#options' => array_merge(['' => 'Select a search block'], drupal_map_assoc($search_blocks)),
    '#ajax' => [
      'callback' => 'search_block_name_ajax_callback',
      'wrapper' => 'search_block_alter_ajax_wrapper',
    ],
  ];

  $form['field_wrapper'] = [
    '#type' => 'fieldset',
    '#title' => t('Available fields'),
    '#prefix' => '<div id="search_block_alter_ajax_wrapper">',
    '#suffix' => '</div>',
    '#states' => [
      'invisible' => [
        ':input[name="search_block"]' => ['value' => ''],
      ],
    ],
    '#tree' => TRUE,
  ];

  $search_block = isset($form_state['values']['search_block']) ? $form_state['values']['search_block'] : '';
  if (!empty($search_block)) {
    $sql = "SELECT * FROM {tripal_elasticsearch} WHERE index_name = :index_name";
    $index_name = $search_block;
    // Get table name and then query the table in database for unique values for specific fields.
    $result = db_query($sql, [':index_name' => $index_name]);
    foreach ($result as $record) {
      $field = $record->index_field;
      $form['field_wrapper'][$field] = [
        '#type' => 'fieldset',
        '#title' => t('Alter field: <b>' . $field . '</b>'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ];
      $form['field_wrapper'][$field]['title'] = [
        '#type' => 'textfield',
        '#title' => t('Available fields'),
        '#title' => t('Field title'),
        '#default_value' => t($record->form_field_title),
      ];
      $form['field_wrapper'][$field]['description'] = [
        '#type' => 'textarea',
        '#title' => t('Field description'),
        '#description' => t('A description for this search.  Enter some text to show users some example searches,
						or give some instructions.'),
        '#default_value' => t($record->form_field_description),
      ];

      //$table_name = $record->table_name;

      // Get unique values for this field in the database table.
      //$table_name = db_query("SELECT DISTINCT table_name FROM {tripal_elasticsearch} LIMIT 1")->fetchCol('table_name')[0];
      //$sql = "SELECT DISTINCT $field FROM {$table_name} LIMIT 51";
      //$field_values = db_query($sql);
      $type_options = [
        'textfield' => 'textfield',
        'select' => 'select',
      ];
      //$field_values = $field_values->fetchCol();
      $form['field_wrapper'][$field]['type'] = [
        '#type' => 'select',
        '#title' => t('Field type'),
        '#description' => t('The field type in the search form. <strong style="color: red">textfield
					</strong> indicates an input box. <strong style="color: red">select</strong> indicates
					a dropdown menu box. If this field has more than 50 unique values in the database table,
					only the <strong style="color: red">textfield</strong> will be available.'),
        '#options' => $type_options,
        '#default_value' => $record->form_field_type,
      ];

      //asort($field_values);
      /*$options_arr = [];
      foreach ($field_values as $field_value) {
        $options_arr[] = '["' . $field_value . '"|' . $field_value . ']';
      }*/

      $options_text = $record->form_field_options;
      $form['field_wrapper'][$field]['options'] = [
        '#type' => 'textarea',
        '#title' => t('Options for select field'),
        '#description' => t('This is used to convert an input box to a dropdown menu box
							in your search form. Enter key-value pairs into this box in the format of
							<b style="color: red;">key|value</b>. The keys should be the true values
							in your database table, and the values can be anything that you want to show
							to your users. <b style="color: red;">key|value pairs</b> have to be placed within
							the <b style="color: red;">[ ] (example: [key|value])</b>.'),
        '#default_value' => $options_text,
        // display options field only when the type field value is 'select'.
        '#states' => [
          'invisible' => [
            ":input[name=\"field_wrapper[$field][type]\"]" => ['value' => 'textfield'],
          ],
        ],
      ];
      $form['field_wrapper'][$field]['weight'] = [
        '#type' => 'select',
        '#title' => t('Field weight'),
        '#options' => drupal_map_assoc(range(0, 20)),
        '#default_value' => $record->form_field_weight,
      ];
    }
  }

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#states' => [
      'invisible' => [
        ':input[name="search_block"]' => ['value' => ''],
      ],
    ],
  ];

  return $form;
}

/**
 * ajax callback search_block_name_ajax_callback
 */
function search_block_name_ajax_callback($form, &$form_state) {
  return $form['field_wrapper'];

  //	$commands = array();
  //	$commands[] = ajax_command_replace('#search_block_alter_ajax_wrapper', $form['field_wrapper']);
  //	return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * Form submit for table_search_block_altering_form.
 */
function table_search_block_altering_form_submit($form, &$form_state) {
  $index_name = $form_state['values']['search_block'];
  $sql = "SELECT DISTINCT table_name FROM {tripal_elasticsearch}";
  $table_name = db_query($sql)->fetchCol('table_name')[0];

  // Delete old records.
  db_delete("tripal_elasticsearch")
    ->condition('index_name', $index_name)
    ->execute();

  $index_fields = array_keys($form_state['values']['field_wrapper']);

  $record = [];
  foreach ($index_fields as $field) {
    $record['index_name'] = $index_name;
    $record['table_name'] = $table_name;
    $record['index_field'] = $field;
    $record['form_field_title'] = $form_state['values']['field_wrapper'][$field]['title'];
    $record['form_field_type'] = $form_state['values']['field_wrapper'][$field]['type'];
    $record['form_field_description'] = $form_state['values']['field_wrapper'][$field]['description'];
    if ($record['form_field_type'] === 'textfield') {
      $record['form_field_options'] = '';
    }
    else {
      $options = $form_state['values']['field_wrapper'][$field]['options'];
      //$options = explode(',', $options);
      //$sortable = [];
      $record['form_field_options'] = $options;
    }
    $record['form_field_weight'] = $form_state['values']['field_wrapper'][$field]['weight'];
    drupal_write_record('tripal_elasticsearch', $record);
  }

  drupal_set_message('Search block settings for <b>' . $index_name . '</b> has been successfully updated.');
  $form_state['rebuild'] = TRUE;
}

/**
 * Form builder for table_search_block_altering_form.
 */
function link_results_to_pages_form($form, &$form_state) {
  // Give an example to help users.
  $form['help'] = [
    '#type' => 'fieldset',
    '#title' => t('Example and Instruction'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  $help_markup = '<p>This is administrative page allows you to add an url to the table
									search results. Below is an example to show you how to associate
									urls with table fields.</p>';
  $help_markup .= '<p>You can add either a <b>consistent</b> url 
									(see the organism_id field below) or a <b>variable</b> 
									url (see the abbreviation field below). Variable urls are built with search results 
									from specified fields. <b>Field names</b> within the 
									<b>[ ]</b> in the url will be replaced with corresponding 
									field search results.</p>';
  $form['help']['help'] = [
    '#type' => 'markup',
    '#markup' => t($help_markup),
  ];
  $form['help']['example_form'] = [
    '#type' => 'fieldset',
    '#title' => t('Available fields'),
  ];
  $form['help']['example_form']['organism_id'] = [
    '#type' => 'textfield',
    '#field_suffix' => 'organism_id',
    '#attributes' => [
      'placeholder' => 'https://www.ncbi.nlm.nih.gov',
    ],
  ];
  $form['help']['example_form']['organism_abbreviation'] = [
    '#type' => 'textfield',
    '#field_suffix' => 'abbreviation',
    '#attributes' => [
      'placeholder' => 'organism/[genus]/[species]',
    ],
  ];
  $form['help']['example_form']['organism_genus'] = [
    '#type' => 'textfield',
    '#field_suffix' => 'genus',
  ];
  $form['help']['example_form']['organism_species'] = [
    '#type' => 'textfield',
    '#field_suffix' => 'species',
  ];

  // Get all index_name from database table tripal_elasticsearch.
  $sql = "SELECT DISTINCT index_name FROM {tripal_elasticsearch}";
  $search_blocks = db_query($sql)->fetchCol('index_name');

  if (empty($search_blocks)) {
    $form['no_search_block'] = [
      '#type' => 'markup',
      '#markup' => t('No search block is available. Please build search block first.'),
    ];

    return $form;
  }

  $form['search_block'] = [
    '#type' => 'select',
    '#title' => t('Search blocks'),
    '#options' => array_merge(['' => 'Select a search block'], drupal_map_assoc($search_blocks)),
    '#ajax' => [
      'callback' => 'link_results_to_pages_ajax_callback',
      'wrapper' => 'link_results_to_pages_ajax_wrapper',
    ],
  ];

  $form['field_wrapper'] = [
    '#type' => 'fieldset',
    '#title' => t('Available fields'),
    '#tree' => TRUE,
    '#states' => [
      'invisible' => [
        ':input[name="search_block"]' => ['value' => ''],
      ],
    ],
    '#prefix' => '<div id="link_results_to_pages_ajax_wrapper">',
    '#suffix' => '</div>',
  ];

  $search_block = isset($form_state['values']['search_block']) ? $form_state['values']['search_block'] : '';
  if (!empty($search_block)) {
    $sql = "SELECT * FROM {tripal_elasticsearch} WHERE index_name = :index_name";
    $index_name = $search_block;
    $result = db_query($sql, [':index_name' => $index_name]);
    foreach ($result as $record) {
      $field = $record->index_field;
      $form['field_wrapper'][$field] = [
        '#type' => 'textfield',
        '#field_suffix' => $field,
      ];
    }
  }

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#states' => [
      'invisible' => [
        ':input[name="search_block"]' => ['value' => ''],
      ],
    ],
  ];

  return $form;
}

/**
 * Ajax callback for link_results_to_pages_form.
 */
function link_results_to_pages_ajax_callback($form, &$form_state) {
  return $form['field_wrapper'];
}

/**
 * Form submit for link_results_to_pages_form.
 */
function link_results_to_pages_form_submit($form, &$form_state) {
  $index_name = $form_state['values']['search_block'];
  $sql = "SELECT DISTINCT table_name FROM {tripal_elasticsearch}";
  $table_name = db_query($sql)->fetchCol('table_name')[0];
  $record = [];
  // Delete old links first.
  db_delete('tripal_elasticsearch_links')
    ->condition('index_name', $index_name)
    ->execute();
  foreach ($form_state['values']['field_wrapper'] as $field => $url) {
    $record['index_name'] = $index_name;
    $record['table_name'] = $table_name;
    $record['index_field'] = $field;
    $record['field_url'] = $url;
    drupal_write_record('tripal_elasticsearch_links', $record);
  }

  drupal_set_message(t('Links have been successfully updated.'));
  $form_state['rebuild'] = TRUE;
}