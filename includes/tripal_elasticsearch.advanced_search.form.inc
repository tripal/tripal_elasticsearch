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

  $form_vals = $form_state['values'] ?? [];

  $values = [
    'term' => trim($_GET['term'] ?? $form_vals['term'] ?? ''),
    'category' => $_GET['category'] ?? $form_vals['category'] ?? '',
    'field' => $_GET['field'] ?? $form_vals['field'] ?? '',
  ];

  $helper = new \ES\Common\BundleHelper();

  $form['wrapper'] = [
    '#prefix' => '<div id="advanced_search_wrapper">',
    '#suffix' => '</div>',
  ];

  $form['wrapper']['category'] = [
    '#type' => 'select',
    '#options' => ['' => 'Any Category'] + $bundles,
    '#ajax' => [
      'callback' => 'tripal_elasticsearch_advanced_search_form_callback',
      'wrapper' => 'advanced_search_wrapper',
    ],
    '#default_value' => $values['category'],
  ];

  if (!empty($values['category'])) {
    $fields = $helper->getFieldsByBundle(
      $helper->getBundleByName($values['category'])
    );
    $field_options = [
      '' => 'Any Field',
    ];

    foreach ($fields as $field) {
      $field_options[$field->name] = $field->label;
    }

    $form['wrapper']['field'] = [
      '#type' => 'select',
      '#options' => $field_options,
      '#default_value' => $values['field'],
    ];
  }

  $form['wrapper']['term'] = [
    '#type' => 'textfield',
    '#default_value' => $values['term'],
  ];

  $form['wrapper']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Search'),
  ];

  $form['#method'] = 'GET';

  $form['wrapper']['results'] = [
    '#markup' => tripal_elasticsearch_advanced_search_results($values),
  ];

  return $form;
}

function tripal_elasticsearch_configure_term_session() {
  $op = $_GET['op'] ?? '';
  $session_term = $_SESSION['es_term'] ?? '';
  $term = $_GET['term'] ?? '';

  if ($op === 'Include More Results') {
    $_SESSION['es_term'] = $term;
  }
  elseif (isset($_SESSION['es_term']) && $session_term !== $term) {
    unset($_SESSION['es_term']);
  }
}

/**
 * @param array $values
 *
 * @return string
 * @depends tripal_elasticsearch_configure_term_session()
 */
function tripal_elasticsearch_advanced_search_results(array $values) {
  tripal_elasticsearch_configure_term_session();

  if (empty($values['category']) && empty($values['term'])) {
    return '';
  }

  $retry = isset($_SESSION['es_term']);

  try {
    $instance = new \ES\Common\Instance();

    if($retry) {
      $values['term'] = $_SESSION['es_term'];
    }

    $results = tripal_elasticsearch_perform_advanced_search(
      $values,
      15,
      $retry
    );

    if ($results['total'] <= 0 && !empty($values['term']) && !$retry) {
      $retry = TRUE;
      $values['term'] = strtolower($values['term']);
      $time = $results['time'];
      $results = tripal_elasticsearch_perform_advanced_search(
        $values,
        15,
        $retry
      );

      if ($results['total'] > 0) {
        $_SESSION['es_term'] = $values['term'];
      }
      $results['time'] += $time;
    }

    $hits = $instance->formatHits($results['results']);

    if ($results['total'] === 0) {
      $content = theme(
        'elasticsearch_tips',
        [
          'message' => '0 results found.',
        ]
      );
    }
    else {
      $content = theme(
        'elasticsearch_results_header',
        [
          'page' => $results['page'],
          'total' => $results['total'],
          'pages' => $results['pages'],
          'time' => $results['time'],
        ]
      );

      $content .= tripal_elasticsearch_get_website_search_result_table(
        $hits,
        FALSE
      );
      $content .= $results['pager'];
    }

    // If we are in the last page and did not yet attempt a retry, check to see
    // if a retry would produce more results and inform the user of that option
    if ($results['pages'] == $results['page'] && !$retry && !empty($values['term'])) {
      $with_retry = tripal_elasticsearch_perform_advanced_search(
        $values,
        15,
        TRUE
      );

      if ($with_retry['total'] > $results['total']) {
        $content .= tripal_elasticsearch_advanced_search_retry_message();
      }
    }

    return $content;
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
 * Create a retry message.
 *
 * @return string
 */
function tripal_elasticsearch_advanced_search_retry_message() {
  $elements = [
    [
      '#type' => 'markup',
      '#markup' => t(
        '<div><i>Only most relevant results have been included</i></div>'
      ),
    ],
    [
      '#type' => 'markup',
      '#markup' => t(
        '<div><i>Would you like to include other results?</i></div>'
      ),
    ],
    [
      '#type' => 'button',
      '#value' => 'Include More Results',
      '#prefix' => '<div>',
      '#suffix' => '</div>',
    ],
  ];

  return drupal_render($elements);
}

/**
 * @param array $values
 * @param int $per_page
 * @param bool $retry
 *
 * @return array
 * @throws \Exception
 */
function tripal_elasticsearch_perform_advanced_search($values, $per_page = NULL, $retry = FALSE) {
  $fields = ['content.*', 'title'];

  if (isset($values['field']) && !empty($values['field'])) {
    $fields = ["content.{$_GET['field']}"];
  }

  $instance = new \ES\Common\Instance();
  $index = ['entities'];
  $category_field = ['bundle_label'];
  if ($instance->hasIndex('website')) {
    $index[] = 'website';
    $category_field[] = 'type';
  }
  $model = new \ES\Models\Model();

  $model->setIndexName(implode(',', $index));

  if (!empty($values['term'])) {
    $model->where($fields, trim($values['term']));
  }

  $category = trim($values['category'] ?? '');
  if (!empty($category)) {
    $model->where($category_field, "$category");
  }

  $model->highlight(['content.*', 'title']);

  $model->retry($retry);

  if (is_null($per_page)) {
    return $model->search();
  }

  return $model->paginate($per_page);
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
