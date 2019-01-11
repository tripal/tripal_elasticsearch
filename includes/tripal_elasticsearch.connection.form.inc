<?php

/**
 * @file
 * Admin page callback to build form for Elasticsearch connection.
 */

/**
 * Connection Form.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function elasticsearch_connection_form($form, &$form_state) {
  $instructions         = '<h1>Add Elasticsearch Servers</h1>';
  $instructions         .= '<p>This administrative page allows you to add or manage local and remote Elasticsearch server connections.  To configure an Elasticsearch server for your site, please see <a href="https://github.com/tripal/tripal_elasticsearch"> the Readme documentation for this module</a>.</p>';
  $form['instructions'] = [
    '#type'   => 'markup',
    '#markup' => $instructions,
  ];

  $form['server_type'] = [
    '#title'         => t('Server Type'),
    '#type'          => 'radios',
    '#required'      => TRUE,
    '#options'       => [
      'local'  => t(
        'A local Elasticsearch server.  This will be your primary search database, indexing content on the current site.'
      ),
      'remote' => t(
        'A remote Elasticsearch server.  You can connect any number of additional servers, enabling cross-site searching.'
      ),
    ],
    '#default_value' => 'local',
  ];

  $form['local_host'] = [
    '#type'   => 'fieldset',
    '#title'  => t('Elasticsearch Local Server'),
    '#states' => [
      'visible' => [
        ':input[name="server_type"]' => ['value' => 'local'],
      ],
    ],
  ];

  $form['local_host']['local_url'] = [
    '#type'          => 'textfield',
    '#title'         => t('Elasticsearch Server URL'),
    '#description'   => t(
      'URL and port of an Elasticsearch server. Examples: http://localhost:9200 or http://127.0.0.1:9200'
    ),
    '#default_value' => variable_get('elasticsearch_host'),
    '#attributes'    => [
      'placeholder' => 'Example: http://localhost:9200',
    ],
  ];

  $form['remote_host'] = [
    '#type'   => 'fieldset',
    '#title'  => t('Elasticsearch Remote Server'),
    '#states' => [
      'visible' => [
        ':input[name="server_type"]' => ['value' => 'remote'],
      ],
    ],
  ];

  $form['remote_host']['remote_url'] = [
    '#type'        => 'textfield',
    '#title'       => 'Server URL',
    '#description' => t(
      'URL of a Tripal site that has Tripal Elasticsearch enabled.'
    ),
    '#attributes'  => [
      'placeholder' => 'Example: http://example.com',
    ],
  ];

  $form['remote_host']['label'] = [
    '#type'          => 'textfield',
    '#title'         => 'Server Label',
    '#description'   => t('Label for this Elasticsearch server.'),
    '#default_value' => '',
    '#attributes'    => [],
  ];

  $form['remote_host']['logo'] = [
    '#type'        => 'managed_file',
    '#title'       => 'Logo',
    '#description' => t('Optional logo file'),
    '#upload_location' => 'public://tripal_elasticsearch/',
  ];

  $form['remote_host']['description'] = [
    '#type'          => 'textfield',
    '#title'         => 'Server Description',
    '#description'   => t('A description for this Elasticsearch server.'),
    '#default_value' => '',
    '#attributes'    => [],
  ];

  $form['remote_host']['connect'] = [
    '#type'  => 'submit',
    '#value' => t('Save Remote Host'),
  ];

  $form['local_host']['connect_local'] = [
    '#type'  => 'submit',
    '#value' => variable_get('elasticsearch_host')
      ? t('Update Local Host')
      : t(
        'Connect to Local Host'
      ),
  ];

  $form['health'] = [
    '#type'   => 'markup',
    '#markup' => '<h4>' . t('Elasticsearch Server Health') . '</h4>'
      . '<p>No running Elasticsearch server is connected. Please check your Elasticsearch server URL and port.</p>',
  ];

  try {
    $client = (new ESInstance())->client;
    // Obtain cluster health information.
    $params['v'] = TRUE;
    $health      = $client->cat()->health($params)[0];

    // If $health is not empty, create a form element to display it.
    if (!empty($health)) {
      $header         = array_keys($health);
      $rows[]         = array_values($health);
      $output         = theme('table', ['header' => $header, 'rows' => $rows]);
      $form['health'] = [
        '#type'        => 'markup',
        '#title'       => t('Elasticsearch Server Health'),
        '#description' => t(
          'The table below shows the health  of your local Elasticsearch server.'
        ),
        '#markup'      => '<h4>' . t('Local Elasticsearch Server Health')
          . '</h4>' . '<p>' . t(
            'The table below shows the health of your local Elasticsearch server.'
          ) . '</p>' . $output,
      ];
    }
  } catch (\Exception $e) {
    drupal_set_message($e->getMessage(), 'warning');
  }

  $remotes =
    db_query('SELECT * FROM {tripal_elasticsearch_servers}')->fetchAll();

  if ($remotes) {
    $header = ["Logo", "URL", "Label", "Description", "Status", "Edit", "Delete"];
    $rows   = [];

    foreach ($remotes as $remote) {
      $logo = '';
      if(!empty($remote->logo)) {
        $logo_img = file_load($remote->logo);
        $logo = '<img src="'.file_create_url($logo_img->uri).'" style="max-height:50px; width: auto"/> ';
      }
      $url         = $remote->url;
      $label       = $remote->label;
      $description = $remote->description;
      $status      =
        "<span class='elastic-status-circle' id='remote-host-{$remote->id}-circle'></span> <span id='remote-host-{$remote->id}'>Loading</span>";
      $edit        = l(
        "Edit",
        "admin/tripal/extension/tripal_elasticsearch/connection/edit/{$remote->id}"
      );
      $delete      = l(
        "Delete",
        "admin/tripal/extension/tripal_elasticsearch/connection/delete/{$remote->id}"
      );

      $rows[] = [$logo, $url, $label, $description, $status, $edit, $delete];
    }

    $output = "<h4>Elasticsearch Remote Servers </h4>";
    $output .= "<p>Remote Elasticsearch server connections can be viewed, edited, or deleted below.</p>";
    $output .= theme(
      'table', [
        'header' => $header,
        'rows'   => $rows,
      ]
    );

    $form['remotes'] = [
      '#type'   => 'markup',
      '#title'  => 'Remote Elasticsearch Servers',
      '#markup' => $output,
    ];
  }

  if (!$remotes) {
    $form['remotes'] = [
      '#type'   => 'markup',
      '#markup' => '<h4>' . t('Elasticsearch Remote Servers') . '</h4>'
        . '<p>No remote Elasticsearch servers have been added. When remote servers are added, their status will be displayed below.</p>',
    ];
  }

  global $base_url;

  drupal_add_js(
    drupal_get_path('module', 'tripal_elasticsearch') . '/js/axios.min.js'
  );
  drupal_add_js(
    drupal_get_path(
      'module', 'tripal_elasticsearch'
    ) . '/js/tripal_elasticsearch.js'
  );
  drupal_add_js(
    [
      'remotes' => $remotes,
      'action'  => 'getStatus',
      'base'    => $base_url,
    ], 'setting'
  );

  return $form;
}

/**
 * Elasticsearch_connection_form submit
 * Submits the form, setting either the localhost or adding a new remote host.
 *
 * @param $form
 * @param $form_state
 */
function elasticsearch_connection_form_submit($form, &$form_state) {
  $server_type = $form_state['values']['server_type'];
  $url         = $server_type == 'remote' ? $form_state['values']['remote_url']
    : $form_state['values']['local_url'];
  $label       = $form_state['values']['label'];
  $description = $form_state['values']['description'];

  if ($server_type == "local") {
    // Update the elasticsearch_host variable after form submission.
    variable_set('elasticsearch_host', $url);
  }


  if ($server_type == "remote") {
    // Load the file via file.fid.
    $file = file_load($form_state['values']['logo']);
    if($file) {
      // Change status to permanent.
      $file->status = FILE_STATUS_PERMANENT;
      // Save.
      $file = file_save($file);
    }

    $fields = [
      'url'         => $url,
      'label'       => $label,
      'description' => $description,
      'logo'        => $file ? $file->fid : '',
    ];

    // Insert records to the database.
    db_insert('tripal_elasticsearch_servers')->fields($fields)->execute();
    drupal_set_message('External elasticsearch server added!');
    drupal_goto('admin/tripal/extension/tripal_elasticsearch');
  }
}

/**
 * Implements a function that validates and submits the form that adds new
 * elasticsearch servers.
 *
 * @param $form
 * @param $form_state
 */
function elasticsearch_connection_form_validate($form, &$form_state) {
  $server_type = $form_state['values']['server_type'];
  $url         = $server_type === 'local' ? $form_state['values']['local_url']
    : $form_state['values']['remote_url'];
  $label       = $form_state['values']['label'];
  $description = $form_state['values']['description'];

  if (empty($server_type)) {
    form_set_error('server_type', 'Please select a server type.');
  }

  if ($url == '') {
    form_set_error('url', 'Please enter a url for this host');
  }
  else {
    if (filter_var($url, FILTER_VALIDATE_URL) === FALSE) {
      form_set_error(
        $server_type === 'remote' ? 'remote_url' : 'local_url',
        'Please enter a valid URL for the host.'
      );
    }
  }

  if ($server_type == "remote") {
    if (empty($label)) {
      form_set_error('label', 'Please enter a label for this remote host.');
    }

    if (empty($description)) {
      form_set_error(
        'description', 'Please enter a description for this remote host.'
      );
    }

    // Check to make sure the server doesn't already exist.
    $count = db_query(
      'SELECT COUNT(*) FROM {tripal_elasticsearch_servers} WHERE url=:url',
      [':url' => $url]
    )->fetchObject();
    if ($count->count > 0) {
      form_set_error(
        'remote_url',
        'The remote server URL already exists in the database.  Please edit or delete it in the Elasticsearch Remote Servers table below.'
      );
    }
  }
}

/**
 * Creates a confirmation page before deleting a remote server.
 *
 * @param $form
 * @param $form_state
 * @param $product_code
 *
 * @return mixed
 */
function remote_delete_confirm($form, &$form_state, $remote_id) {
  $form['_remote_id'] = [
    '#type'  => 'value',
    '#value' => $remote_id,
  ];

  $server = db_query(
    "SELECT * FROM {tripal_elasticsearch_servers} WHERE id=:id",
    [':id' => $remote_id]
  )->fetchObject();

  return confirm_form(
    $form,
    t('Are you sure you want to delete the remote Elasticsearch server entry?'),
    isset($_GET['destination']) ? $_GET['destination'] : "tripal_elasticsearch",
    t(
      'Are you sure you want to delete the remote elasticsearch server "'
      . $server->label . '"?'
    ), t('Delete'), t('Cancel')
  );
}

/**
 * Deletes db entry for remote server.
 *
 * @param $form
 * @param $form_state
 */
function remote_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $remote_id = $form_state['values']['_remote_id'];
    $remote    = db_query(
      'SELECT label FROM {tripal_elasticsearch_servers} WHERE id=:id',
      [':id' => $remote_id]
    )->fetchObject();
    db_query(
      "DELETE FROM {tripal_elasticsearch_servers} WHERE id=:id",
      [':id' => $remote_id]
    );
    drupal_set_message(
      t('Remote server ' . $remote->label . ' has been deleted successfully.')
    );
  }

  drupal_goto("admin/tripal/extension/tripal_elasticsearch");
}

/**
 * Edit form confirmation.
 *
 * @param $form
 * @param $form_state
 * @param $remote_id
 *
 * @return mixed
 */
function remote_edit_confirm($form, &$form_state, $remote_id) {
  $form   = [];
  $result = db_query(
    "SELECT * FROM {tripal_elasticsearch_servers} WHERE id=:remote_id",
    [':remote_id' => $remote_id]
  )->FetchObject();

  if ($result) {
    $url         = $result->url;
    $label       = $result->label;
    $description = $result->description;
  }
  else {
    drupal_not_found();

    return drupal_exit();
  }

  $form['_remote_id'] = [
    '#type'  => 'value',
    '#value' => $remote_id,
  ];

  $form['instructions'] = [
    '#type'   => 'markup',
    '#markup' => t('<h1>Editing "' . $label . '"</h1>'),
  ];

  $form['url'] = [
    '#title'         => t('URL'),
    '#type'          => 'textfield',
    '#default_value' => $url,
    '#description'   => t(
      'URL and port of an Elasticsearch server. Examples: http://localhost:9200 or http://127.0.0.1:9200'
    ),
  ];

  $form['label'] = [
    '#title'         => t('Label'),
    '#type'          => 'textfield',
    '#default_value' => $label,
    '#description'   => t('Label for this Elasticsearch server'),
  ];

  $form['description_field'] = [
    '#title'         => t('Description'),
    '#type'          => 'textfield',
    '#default_value' => $description,
    '#description'   => t('Description for this Elasticsearch server'),
  ];

  return confirm_form(
    $form, t(''), isset($_GET['destination']) ? $_GET['destination']
    : "admin/tripal/extension/tripal_elasticsearch", t(''), t('Edit'),
    t('Cancel')
  );
}

/**
 * Remote form submit after confirmation has been obtained.
 *
 * @param $form
 * @param $form_state
 */
function remote_edit_confirm_submit($form, &$form_state) {
  $form_values = $form_state['values'];

  if ($form_values['confirm']) {
    $remote_id   = $form_values['_remote_id'];
    $url         = $form_values['url'];
    $label       = $form_values['label'];
    $description = $form_values['description_field'];

    $query = "UPDATE {tripal_elasticsearch_servers}
         SET url=:url, {label}=:label, description=:description
         WHERE id=:id";

    db_query(
      $query, [
        ':id'          => $remote_id,
        ':description' => $description,
        ':label'       => $label,
        ':url'         => $url,
      ]
    );

    drupal_set_message(
      t('Remote server ' . $label . ' has been edited successfully.')
    );
    drupal_goto("admin/tripal/extension/tripal_elasticsearch");
  }
}
