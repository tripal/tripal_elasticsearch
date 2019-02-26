<?php

/**
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_elasticsearch_advanced_search_form($form, &$form_state) {
  try {
    $instance = new \ES\Common\Instance();
    $bundles = $instance->getAllCategories();
  } catch (Exception $exception) {
    $bundles = [];
  }

  $helper = new \ES\Common\BundleHelper();

  $form['wrapper'] = [
    '#prefix' => '<div id="advanced_search_wrapper" class="container-inline">',
    '#suffix' => '</div>',
  ];

  $category = trim(
    $_GET['category'] ?? $form_state['values']['category'] ?? ''
  );

  $form['wrapper']['category'] = [
    '#type' => 'select',
    '#options' => ['' => 'Any Category'] + $bundles,
    '#ajax' => [
      'callback' => 'tripal_elasticsearch_advanced_search_form_callback',
      'wrapper' => 'advanced_search_wrapper',
    ],
    '#default_value' => $category,
  ];

  if (!empty($category)) {
    $fields = $helper->getFieldsByBundle($helper->getBundleByName($category));
    $field_options = [
      '' => 'Any Field',
    ];

    foreach ($fields as $field) {
      $field_options[$field->name] = $field->label;
    }

    $form['wrapper']['field'] = [
      '#type' => 'select',
      '#options' => $field_options,
      '#default_value' => trim($_GET['field'] ?? ''),
    ];
  }

  $form['wrapper']['term'] = [
    '#type' => 'textfield',
    '#default_value' => $_GET['term'] ?? '',
  ];

  $form['wrapper']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Search'),
  ];

  $form['#method'] = 'GET';

  if (isset($_GET['term']) && !empty($_GET['term'])) {
    $form['wrapper']['results'] = [
      '#markup' => tripal_elasticsearch_advanced_search_results($_GET),
    ];
  }

  return $form;
}

/**
 * @param array $values
 *
 * @return string
 */
function tripal_elasticsearch_advanced_search_results(array $values) {
  $fields = ['content.*', 'title'];

  if (isset($_GET['field']) && !empty($_GET['field'])) {
    $fields = ["content.{$_GET['field']}"];
  }

  try {
    $instance = new \ES\Common\Instance();
    $model = new \ES\Models\Model();
    $model->setIndexName('entities');
    $model->where($fields, trim($values['term']));

    $category = trim($values['category'] ?? '');
    if (!empty($category)) {
      $model->where('bundle_label', "\"$category\"");
    }

    $model->highlight(['content.*', 'title']);
    $data = $model->search();
    $hits = $instance->formatHits($data);

    if (count($hits) === 0) {
      return theme(
        'elasticsearch_tips',
        [
          'message' => '0 results found.',
        ]
      );
    }

    return tripal_elasticsearch_get_website_search_result_table($hits);
  } catch (Exception $exception) {
    watchdog(
      'tripal_elasticsearch',
      $exception->getMessage(),
      [],
      WATCHDOG_ERROR
    );
    drupal_set_message(
      'The search service is currently unavailable. Please contact us to resolve the issue.'
    );
  }
}

/**
 * AJAX Callback.
 *
 * @param array $form
 * @param array $form_state
 *
 * @return array
 */
function tripal_elasticsearch_advanced_search_form_callback($form, &$form_state) {
  return $form['wrapper'];
}
