<?php

/**
 * "Create a collection" form.
 *
 * @param $form
 * @param $form_state
 */
function tripal_elasticsearch_collections_form($form, &$form_state) {
  if (!user_is_logged_in()) {
    $content = '<p>';
    $content .= t('To create a collection, you must first ') . l('sign in', 'user') . t(' or ') . l('register a new account', 'user/register');
    $content .= '</p>';
    $form['login'] = [
      '#type' => 'markup',
      '#markup' => $content,
    ];

    return $form;
  }

  $bundles = db_query('SELECT name, label FROM {tripal_bundle}');
  $index_bundles = [];
  $selected_label = FALSE;
  $selected_category = FALSE;

  if (isset($_GET['category']) && !empty($_GET['category'])) {
    $selected_label = $_GET['category'];
  }

  foreach ($bundles->fetchAll() as $bundle) {
    if ($selected_label === $bundle->label) {
      $selected_category = $bundle->name;
    }
    $index_bundles[$bundle->name] = $bundle->label;
  }

  $form['step1'] = [
    '#title' => t('Step 1 - Categories'),
    '#type' => 'fieldset',
    '#attributes' => [
      'class' => ['elastic-modal-step'],
    ],
  ];

  $form['step1']['bundle_type'] = [
    '#title' => t('Select a Category'),
    '#type' => 'radios',
    '#options' => $index_bundles,
    '#ajax' => [
      'callback' => 'tripal_elasticsearch_collections_form_callback',
      'wrapper' => 'collections-step-2',
      'method' => 'html',
      'effect' => 'fade',
    ],
  ];

  if ($selected_category) {
    $form['step1']['bundle_type']['#default_value'] = $selected_category;
  }

  $form['step2'] = [
    '#type' => 'fieldset',
    '#title' => t('Step 2 - Fields'),
    '#attributes' => [
      'class' => ['elastic-modal-step'],
    ],
  ];

  $form['step2']['fields'] = [
    '#markup' => '<div id="collections-step-2"></div>',
  ];

  // Get the selected category and pull the available fields for it
  // This value is usually available after ajax call
  if ($selected_category || (isset($form_state['values']) && $form_state['values']['bundle_type'])) {
    $bundle = isset($form_state['values']) ? $form_state['values']['bundle_type'] : $selected_category;
    $field_ids = tripal_elasticsearch_get_fields_as_options($bundle);
    $defaults = array_filter($field_ids, function ($field) {
      return strstr($field, 'FASTA') !== FALSE;
    });

    $form['step2']['fields'] = [
      '#title' => t('Select One or More Fields'),
      '#description' => t('Please select the fields to include in this data
      collection. Not all of these fields appear in the search results
      above but they are available for this category. By default,
      tab-delimited and comma-separated files are generated for the
      collection using only the fields selected. However, some fields when
      selected will generate other downloadable file formats. Fields that
      generate other file formats are indicated.'),
      '#type' => 'checkboxes',
      '#options' => $field_ids,
      '#prefix' => '<div id="collections-step-2">',
      '#suffix' => '</div>',
      '#default_value' => array_keys($defaults),
    ];
  }

  $form['step3'] = [
    '#type' => 'fieldset',
    '#title' => t('Step 3 - Name'),
    '#attributes' => [
      'class' => ['elastic-modal-step'],
    ],
  ];

  $form['step3']['name'] = [
    '#type' => 'textfield',
    '#title' => t('Name Your Collection'),
    '#required' => TRUE,
  ];

  $form['step3']['description'] = [
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#required' => FALSE,
    '#description' => t('Optionally, you can add a more detailed description for this collection.'),
  ];

  // hidden elements
  $form['step3']['search_query'] = [
    '#type' => 'hidden',
    '#default_value' => isset($_GET['search_box']) ? $_GET['search_box'] : '',
  ];

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => 'Create Collection',
  ];

  return $form;
}

function tripal_elasticsearch_collections_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $collection_name = $values['name'];
  $bundle_name = $values['bundle_type'];
  $fields = $values['fields'];
  $description = $values['description'];
  $query = $values['search_query'];

  // get the bundle label
  $bundle_label = db_query('SELECT name FROM {tripal_bundle} WHERE name=:name', [':name' => $bundle_name])->fetchField();
  $entities = [];

  try {
    $es = new ESInstance();
    $es->setWebsiteSearchParams($query, $bundle_label, '', '', [0, 10], TRUE);
    $count = $es->count();

    for ($i = 0; $i < $count; $i += 500) {
      $results = $es->setWebsiteSearchParams($query, $bundle_label, '', '', [
        $i,
        500,
      ], TRUE)->search();

      foreach ($results as $entity) {
        $entities[] = $entity['entity_id'];
      }
    }
  } catch (Exception $exception) {
    drupal_set_message('Could not create collection due to an internal server error. Please try again later.', 'error');
    return;
  }

  global $user;

  tripal_create_collection([
    'uid' => $user->uid,
    'collection_name' => $collection_name,
    'bundle_name' => $bundle_label,
    'ids' => $entities,
    'fields' => $fields,
    'description' => $description,
  ]);

  drupal_goto('user/' . $user->uid . '/data-collections');
}

/**
 * AJAX Callback.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_collections_form_callback($form, $form_state) {
  return $form['step2']['fields'];
}

/**
 * Get options for downloadable fields.
 *
 * @credit Shamelessly copied from
 *          tripal/views_handlers/tripal_views_handler_area_collections.inc
 *
 * @param $bundle
 *
 * @return mixed
 */
function tripal_elasticsearch_get_fields_as_options($bundle) {
  $fields = field_info_instances('TripalEntity', $bundle);

  foreach ($fields as $field_name => $instance) {
    $field = field_info_field($field_name);
    $field_type = $field['type'];
    if ($instance['field_name'] == 'entity_id') {
      continue;
    }

    // Skip hidden fields.
    if ($instance['display']['default']['type'] == 'hidden') {
      continue;
    }
    $field_label = $instance['label'];

    // Add in in any non CSV or Tab formatters to the label.
    $formatters = [];
    if (tripal_load_include_field_class($field_type)) {
      $instance_settings = $field_type::$default_instance_settings;

      if (array_key_exists('download_formatters', $instance_settings)) {

        foreach ($instance_settings['download_formatters'] as $class_name) {

          if ($class_name != 'TripalTabDownloader' and $class_name != 'TripalCSVDownloader') {
            tripal_load_include_downloader_class($class_name);
            $formatters[] = $class_name::$label;
          }

        }

      }

    }

    if (count($formatters) > 0) {
      $field_label .= ' (' . implode(',', $formatters) . ')';
    }

    // Add the field to those supported.
    $field_ids[$instance['field_id']] = t($field_label);
  }

  return $field_ids;
}
