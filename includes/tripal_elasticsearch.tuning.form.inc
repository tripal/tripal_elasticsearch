<?php

/**
 * Field priority settings.
 * Dynamically creates the table rows and form elements of the table.
 *
 * @return array
 */
function tripal_elasticsearch_get_field_priority_settings() {
  $default_fields = tripal_elasticsearch_default_fields();
  $fields = field_info_instances('TripalEntity');
  $saved_settings = tripal_elasticsearch_tuning_db_settings();
  $rows = [];
  $elements = [];
  $reference = [];

  // Construct the rows
  foreach ($fields as $bundle => $field) {
    foreach ($field as $machine_name => $item) {
      if (in_array($machine_name, $reference)) {
        // Field is a duplicate from another bundle, ignore it.
        continue;
      }
      $reference[] = $machine_name;

      $field_id = $item['field_id'];
      // Set the dropdown options
      $options = [
        "{$field_id}:0" => 'Do not index',
        "{$item['field_id']}:1" => 'High priority',
        "{$item['field_id']}:2" => 'Low priority',
      ];

      // Create the form element
      if (isset($saved_settings[$field_id])) {
        $value = in_array($saved_settings[$field_id], [
          0,
          1,
          2,
        ]) ? $saved_settings[$field_id] : 0;
        $default = "{$field_id}:{$value}";
      }
      else {
        $default = in_array($machine_name, $default_fields) ? "{$item['field_id']}:1" : "{$item['field_id']}:2";
      }

      $element = [
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => $default,
      ];

      // Insert the element into the rows array
      $rows[] = [
        [
          'data' => $item['label'],
        ],
        [
          'data' => $machine_name,
        ],
        [
          // Tell the table that this element exists
          'data' => &$element,
        ],
      ];

      // Tell the form that this element exists
      // Passing by reference to make sure the values are synced
      // between the table element and the form element. The
      // form element is not rendered because we are using #theme => table.
      // @see tripal_elasticsearch_tuning_form()
      $elements[] = &$element;

      unset($element);
    }
  }

  usort($rows, function ($a, $b) {
    return $a[0]['data'] > $b[0]['data'];
  });

  return [
    'rows' => $rows,
    'elements' => $elements,
  ];
}

/**
 * Create the tuning form.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_tuning_form($form, &$form_state) {
  $form['description'] = [
    '#type' => 'markup',
    '#markup' => tripal_elasticsearch_tuning_instructions(),
  ];

  $fields = tripal_elasticsearch_get_field_priority_settings();

  $form['fields'] = [
    '#theme' => 'table',
    '#tree' => TRUE,
    '#header' => [
      'Label',
      'Machine Name',
      'Priority Setting',
    ],
    '#rows' => $fields['rows'],
  ];

  $form['fields'] = array_merge($form['fields'], $fields['elements']);

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => 'Save Settings',
  ];

  return $form;
}

/**
 * Implements hook_FORM_submit()
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_tuning_form_submit($form, &$form_state) {
  if (!tripal_elasticsearch_insert_tuning_settings($form_state['values']['fields'])) {
    form_set_error(0, 'Unable to save settings. Database error encountered.');
  }
  else {
    drupal_set_message('Settings saved successfully!', 'status');
  }
}

/**
 * Get default high priority fields.
 *
 * This is currently a static array but we can move this to the db if
 * it gets large enough.
 *
 * @return array
 */
function tripal_elasticsearch_default_fields() {
  return [
    'sio__annotation',
    'local__abbreviation',
    'taxrank__genus',
    'taxrank__species',
    'ncbitaxon__common_name',
    'schema__description',
    'schema__name',
    'obi__organism',
  ];
}

/**
 * Get instructions.
 *
 * @return string
 */
function tripal_elasticsearch_tuning_instructions() {
  $content = '<h3>Tripal Entity Index Tuning</h3>';
  $content .= '<p>';
  $content .= 'Specify which Tripal fields to index. Each field can be set to have a high or low priority setting. ';
  $content .= 'High priority fields get indexed in the first indexing round while low priority fields get indexed during the second round. ';
  $content .= 'By reducing the number of high priority fields, the first round of indexing will go much faster. ';
  $content .= 'You may also choose to completely ignore a field by setting it to "Do not index".';
  $content .= '</p>';

  return $content;
}

/**
 * Get previously saved tuning settings.
 *
 * @return array
 */
function tripal_elasticsearch_tuning_db_settings() {
  $results = db_query('select * from {tripal_elasticsearch_priority}')->fetchAll();
  $settings = [];
  foreach ($results as $result) {
    $settings[$result->field_id] = $result->priority;
  }

  return $settings;
}

/**
 * @param $settings
 *
 * @return \DatabaseStatementInterface|int
 */
function tripal_elasticsearch_insert_tuning_settings($settings) {
  db_query('DELETE FROM {tripal_elasticsearch_priority}');

  $query = db_insert('tripal_elasticsearch_priority')->fields([
    'field_id',
    'priority',
  ]);
  foreach ($settings as $field) {
    $value = explode(':', $field);
    $query->values([
      'field_id' => $value[0],
      'priority' => $value[1],
    ]);
  }

  return $query->execute();
}
