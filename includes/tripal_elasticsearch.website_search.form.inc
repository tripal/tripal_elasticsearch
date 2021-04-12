<?php

/**
 * @file
 * Form to create basic elasticsearch search box.
 */

/**
 * Render array for website_search_box_form.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function website_search_box_form($form, &$form_state) {
  global $base_url;

  $default = isset($_GET['search_box']) && $_GET['search_box'] ? $_GET['search_box'] : '';
  $category = isset($_GET['category']) && $_GET['category'] ? $_GET['category'] : '';

  $form['#attributes']['role'] = 'search';
  $form['#attributes']['aria-label'] = 'Sitewide';

  $form['container'] = [
    '#type' => 'container',
    '#attributes' => [
      'class' => ['container-inline'],
    ],
  ];

  $form['container']['search_box'] = [
    '#type' => 'textfield',
    '#size' => 15,
    '#default_value' => $default,
  ];

  if (strstr(current_path(), 'tripal_elasticsearch/search_website') !== FALSE) {
    $form['container']['search_box']['#attributes'] = [
      'autofocus' => 'autofocus',
    ];
  }

  $form['container']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Search'),
  ];

  $form['container']['category'] = [
    '#type' => 'hidden',
    '#value' => $category,
  ];

  $form['#method'] = 'GET';
  $form['#action'] = $base_url . '/tripal_elasticsearch/search_website/' . $category;

  return $form;
}

/**
 * website_search_box_form submit.
 *
 * @param $form
 * @param $form_state
 */
function website_search_box_form_submit($form, &$form_state) {
  /*
   * $keyword = $form_state['values']['search_box'];
  $url = "tripal_elasticsearch/search_website/$keyword";
  // only redirect to the search results page when $keyword is not empty.
  if (empty($keyword)) {
    drupal_goto(current_path());
  }
  else {
    drupal_goto($url);
  }
   */
}

/**
 * Form builder function for table search.
 *
 * @param $form
 * @param $form_state
 * @param $index_name
 *
 * @return mixed
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
  $record = end($result);
  $count = '';

  //$window_range = drupal_map_assoc([1]);
  if (count($index_fields) > 0) {
    // Build search query for table search.
    $query = tripal_elasticsearch_build_search_query_from_field_content_pairs($index_fields);
    try {
      $es = new ESInstance();
      $results_count = $es->setTableSearchParams($index_name, $query)
        ->count();
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
