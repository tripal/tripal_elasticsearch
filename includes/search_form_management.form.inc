<?php
/**
 * @file
 * Administrative forms for managing search blocks.
 */

/**
 * Render array to build a form for creating or editing table search interfaces
 * and the options for each field.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function table_search_interface_building_form($form, &$form_state) {

  $block_exists = FALSE;  //set constant to false to accommodate when no index is selected.

  $form['help'] = [
    '#type' => 'markup',
    '#markup' => t('This administrative page is used to build search blocks for indexed data. You cannot and do not need to build or alter search blocks for website node or website entity indices.  To begin, select a data table index below.'),
  ];

  try {
    $client = (new ESInstance())->client;
    $existing_indices = $client->indices()->getMapping();
  } catch (\Exception $e) {
    $form['markup'] = [
      '#type' => 'markup',
      '#markup' => '<h3>' . $e->getMessage() . '</h3>',
    ];

    return $form;
  }
  // 'website' and 'entities' are reserved for website indexing and search, no need to build search
  // block for this index.
  if (isset($existing_indices['website'])) {
    unset($existing_indices['website']);
  }
  if (isset($existing_indices['entities'])) {
    unset($existing_indices['entities']);
  }

  if (count($existing_indices) == 0) {
    $form['markup'] = [
      '#type' => 'markup',
      '#markup' => '<h3>No index data exists.</h3>',
    ];

    return $form;
  }

  $index_name_options = array_merge(['' => 'Select an index'], drupal_map_assoc(array_keys($existing_indices)));

  $form['index_name'] = [
    '#type' => 'select',
    '#title' => t('Select an index to build a search interface'),
    '#options' => $index_name_options,
    '#ajax' => [
      'callback' => 'tripal_elasticsearch_manage_ajax_callback',
      'wrapper' => 'tripal_elasticsearch_manage_block',
    ],
  ];

  $index_name = isset($form_state['values']['index_name']) ? $form_state['values']['index_name'] : '';
  if (!empty($index_name)) {

    //get the dbtable mapping
    $sql = "SELECT DISTINCT table_name FROM {tripal_elasticsearch_indices} WHERE index_name = :index_name ";
    $dbtable = db_query($sql, [':index_name' => $form_state['values']['index_name']])->fetchCol('table_name');
    $table_name = $dbtable[0];
    if (!$table_name) {

      drupal_set_message(t("Error: no database table associated with this index.  Recreate the index and try again."), 'error');
    }

    $index_profile = $existing_indices[$index_name];

    $index_fields = array_keys($index_profile['mappings']['_default_']['properties']);

    $form['table_name'] = [
      '#type' => 'value',
      '#value' => $table_name,
    ];
    $sql = "SELECT DISTINCT index_name FROM {tripal_elasticsearch}";
    $result = db_query($sql)->fetchCol('index_name');
    $blocks = drupal_map_assoc($result);
    $block_exists = in_array($index_name, $blocks);
  }
  else {
    $index_fields = [];
  }

  $form['manage_block'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#prefix' => '<div id="tripal_elasticsearch_manage_block">',
    '#suffix' => '</div>',
    '#states' => [
      //only show this field if its checked.
      'invisible' => [
        ':input[name="index_name"]' => ['value' => ''],
      ],
    ],
  ];

  // if the block already exists get info for it.

  $start_check = [];// which fields should be selected at the start in index_fields checkbox?

  if (!empty($index_name) && $block_exists) {
    $sql = "SELECT * FROM {tripal_elasticsearch} WHERE index_name = :index_name";
    $existing_block = db_query($sql, [':index_name' => $form_state['values']['index_name']])->fetchAll();
    $recast_blocks = [];
    foreach ($existing_block as $key => $existing_settings) {
      $field_name = $existing_settings->index_field;
      $recast_blocks[$field_name] = $existing_settings;
      $start_check[$field_name] = $field_name;
    }

    //Stick field choices in state

    $sql = "SELECT * FROM {tripal_elasticsearch_links} WHERE index_name = :index_name";
    $existing_links = db_query($sql, [':index_name' => $form_state['values']['index_name']])->fetchAll();
    $recast_links = [];
    foreach ($existing_links as $key => $link) {
      $field_name = $link->form_field_title;
      $recast_links[$field_name] = $link->field_url;
    }
  }

  $form['manage_block']['index_fields'] = [
    '#type' => 'checkboxes',
    '#title' => t('Select index fields'),
    '#description' => t('Select which index fields will show up on the table search interface. Click on the settings dropdown below to configure a given field.'),
    '#options' => drupal_map_assoc($index_fields),
    '#default_value' => $start_check,
  ];

  foreach ($start_check as $choice) {
    $form['manage_block']['index_fields'][$choice] = [
      '#type' => 'checkbox',
      '#return_value' => $choice,
      '#default_value' => $choice,
      '#attributes' => ['class' => ['hidden']],
    ];
  }

  foreach ($index_fields as $field) {
    //set up blank variables
    $settings["form_field_title"] = $field;
    $settings["form_field_description"] = '';
    $settings["form_field_options"] = '';
    $settings["form_field_weight"] = '10';
    $settings["url"] = '';
    $settings["form_field_type"] = "textfield";

    if ($block_exists) {
      if ($recast_blocks[$field]) {
        $settings["form_field_title"] = $recast_blocks[$field]->form_field_title;
        $settings["form_field_description"] = $recast_blocks[$field]->form_field_description;
        $settings["form_field_options"] = $recast_blocks[$field]->form_field_options;
        $settings["form_field_weight"] = $recast_blocks[$field]->form_field_weight;
        $settings["form_field_type"] = $recast_blocks[$field]->form_field_type;
      }
    }
    if ($existing_links) {
      if ($recast_links[$field]) {
        $settings["url"] = $recast_links[$field];
      }
    }
    //set up expandable block for each field
    $form['manage_block']['field_settings'][$field] = [
      '#type' => 'fieldset',
      '#title' => t('Settings for field: <b>' . $field . '</b>'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#states' => [
        //only show this field if its checked.
        'visible' => [
          ':input[name="manage_block[index_fields][' . $field . ']"]' => [
            ['checked' => TRUE],
          ],
        ],
      ],
    ];
    $form['manage_block']['field_settings'][$field]['title'] = [
      '#type' => 'textfield',
      '#title' => t('Available fields'),
      '#title' => t('Field title'),
      '#default_value' => t($settings["form_field_title"]),
    ];
    $form['manage_block']['field_settings'][$field]['description'] = [
      '#type' => 'textarea',
      '#title' => t('Field description'),
      '#description' => t('A description for this search.  Enter some text to show users some example searches,
						or give some instructions.'),
      '#default_value' => t($settings["form_field_description"]),
    ];

    $type_options = [
      'textfield' => 'textfield',
      'select' => 'select',
    ];
    $form['manage_block']['field_settings'][$field]['type'] = [
      '#type' => 'select',
      '#title' => t('Field type'),
      '#description' => t('The field type available to users in the search form. <strong style="color: red">textfield
					</strong> indicates an input box. <strong style="color: red">select</strong> indicates
					a dropdown menu box. If this field has more than 50 unique values in the database table,
					only the <strong style="color: red">textfield</strong> will be available.'),
      '#options' => $type_options,
      '#default_value' => $settings["form_field_type"],
    ];

    $form['manage_block']['field_settings'][$field]['options'] = [
      '#type' => 'textarea',
      '#title' => t('Options for select field'),
      '#description' => t('This is used to convert an input box to a dropdown menu box
							in your search form. Enter key-value pairs into this box in the format of
							<b style="color: red;">key|value</b>. The keys should be the true values
							in your database table, and the values can be anything that you want to show
							to your users. <b style="color: red;">key|value pairs</b> have to be placed within
							the <b style="color: red;">[ ] (example: [key|value])</b>.'),
      '#default_value' => $settings->form_field_options,
      // display options field only when the type field value is 'select'.
      '#states' => [
        'invisible' => [
          ":input[name=\"manage_block[field_settings][$field][type]\"]" => ['value' => 'textfield'],
        ],
      ],
    ];


      $form['manage_block']['field_settings'][$field]['weight'] = [
      '#type' => 'select',
      '#title' => t('Field weight'),
      '#description' => t('The search rank weight for this field.  A higher weight means matches in this field will influence the relevance more.'),
      '#options' => drupal_map_assoc(range(0, 20)),
      '#default_value' => $settings["form_field_weight"],
    ];
    $form['manage_block']['field_settings'][$field]['url_link'] = [
      '#title' => 'URL linking',
      '#type' => 'textfield',
      '#description' => t('A URL linking for this field.  You can add a static url (https://www.ncbi.nim.nih.gov) or a dynamic field (organism/[genus]/[species]'),
      '#default_value' => $settings["url"],

    ];
  }

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Submit'),
  ];

  return ($form);
}

/**
 * Manage block callback.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_manage_ajax_callback($form, &$form_state) {
  return $form['manage_block'];
}

/**
 * table_search_interface_building_form validation.
 *
 * @param $form
 * @param $form_state
 */
function table_search_interface_building_form_validate($form, $form_state) {
  $index_fields = array_filter($form_state['values']['manage_block']['index_fields']);
  if (empty($index_fields)) {
    form_set_error('index_fields', t('Please select at least one index field.'));
  }
}

/**
 * table_search_interface_building_form submit.
 *
 * @param $form
 * @param $form_state
 */
function table_search_interface_building_form_submit($form, &$form_state) {
  $index_name = $form_state['values']['index_name'];
  $index_fields = array_filter($form_state['values']['manage_block']['index_fields']);
  $field_settings = array_filter($form_state['values']['manage_block']['field_settings']);
  $table_name = $form_state['values']['table_name'];
  // Delete record if it already exists in database table 'tripal_elasticsearch' and 'tripal_elasticsearch_links.
  $sql = "SELECT index_name FROM {tripal_elasticsearch} WHERE index_name = :index_name";
  $result = db_query($sql, [':index_name' => $index_name])->fetchCol('index_name');
  if (!empty($result)) {
    try {
      db_delete('tripal_elasticsearch')
        ->condition('index_name', $index_name)
        ->execute();
      db_delete('tripal_elasticsearch_links')
        ->condition('index_name', $index_name)
        ->execute();
    } catch (\Exception $e) {
      drupal_set_message(t($e->getMessage()), 'warning');
    }
  }

  //Loop through index fields that are checked.
  foreach ($index_fields as $key => $field) {

    $field_setting = $field_settings[$field];
    $record = [];
    $record['index_name'] = $index_name;
    $record['table_name'] = $table_name;
    $record['index_field'] = $field;
    $record['form_field_description'] = $field_setting['description'];
    $record['form_field_type'] = $field_setting['type'];
    $record['form_field_weight'] = $field_setting['weight'];
    $record['form_field_title'] = $field_setting['title'];
    if ($record['form_field_type'] === 'textfield') {
      $record['form_field_options'] = '';
    }
    else {
      $record['form_field_options'] = $field_setting['options'];
    }
    try {
      drupal_write_record('tripal_elasticsearch', $record);
    } catch (\Exception $e) {
      drupal_set_message(t($e->getMessage()), 'warning');
    }
    //now set URL
    $record = [];
    $record['index_name'] = $index_name;
    $record['table_name'] = $table_name;
    $record['index_field'] = $field;
    $record['field_url'] = $field_setting['url_link'];
    try {
      drupal_write_record('tripal_elasticsearch_links', $record);
    } catch (\Exception $e) {
      drupal_set_message(t($e->getMessage(), "warning"));
    }
  }

  drupal_set_message("Search block set!");
}

/**
 * implement hook_form_FORM_ID_alter().
 * set checkbox default values for table_search_interface_building_form
 */
function tripal_elasticsearch_form_table_search_interface_building_form_alter(
  &$form,
  &$form_state,
  $form_id
) {
  if (!isset($form['manage_block']['index_fields'])) {
    return;
  }
  else {
    $index_fields = $form['manage_block']['index_fields']['#options'];
    $default_value = $form['manage_block']['index_fields']['#default_value'];
    $checked_fields = [];
    foreach ($default_value as $checked_field) {
      $checked_fields[] = $index_fields[$checked_field];
    }
    foreach ($index_fields as $field) {
      if (in_array($field, array_values($checked_fields))) {
        $form['manage_block']['index_fields'][$field] = [
          '#type' => 'checkbox',
          '#default_value' => $field,
        ];
      }
      else {
        $form['manage_block']['index_fields'][$field] = [
          '#type' => 'checkbox',
          '#return_value' => $field,
        ];
      }
    }
  }
}