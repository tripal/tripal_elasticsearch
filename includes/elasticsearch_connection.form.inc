<?php
// $Id$

/**
 * @file
 * Admin page callback to build form for Elasticsearch connection.
 */


/**
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */

function elasticsearch_connection_form($form, &$form_state)
{

    $form['host'] = [
        '#type' => 'textfield',
        '#title' => 'Elasticsearch Server URL',
        '#description' => t('URL and port of an elasticsearch server. Examples: http://localhost:9200 or http://127.0.0.1:9200'),
        '#default_value' => variable_get('elasticsearch_host'),
        '#attributes' => [
            'placeholder' => 'http://localhost:9200',
        ],
    ];

    $form['server_type'] = [
        '#title' => t('File type'),
        '#type' => 'radios',
        '#required' => true,
        '#options' => [
            'local' => t('A local Elasticsearch server.  This will be your primary search database.'),
            'remote' => t('A remote Elasticsearch server.  You can connect any number of additional servers.'),
        ],
    ];

    $form['remote_host'] = [
        '#type' => 'fieldset',
        '#title' => t('Elasticsearch Local Server'),
        '#states' => [
            'visible' => [
                ':input[name="server_type"]' => ['value' => 'remote'],
            ],
        ],
    ];

    $form['remote_host']['label'] = [
        '#type' => 'textfield',
        '#title' => 'Elasticsearch Server Label',
        '#description' => t('Label for this elasticsearch server'),
        '#default_value' => '',
        '#attributes' => [],
    ];

    $form['remote_host']['description'] = [
        '#type' => 'textfield',
        '#title' => 'Elasticsearch Server Description',
        '#description' => t('A description for this Elasticsearch server.'),
        '#default_value' => '',
        '#attributes' => [],
    ];

    $form['connect'] = [
        '#type' => 'submit',
        '#value' => t('Connect'),
    ];

    $form['health'] = [
        '#type' => 'markup',
        '#markup' => '<h4>'.t('Elasticsearch server health').'</h4>'.'<p>No running Elasticsearch server is connected. Please check your Elasticsearch server URL and port.</p>',
    ];

    try {
        $elasticsearch_host = variable_get('elasticsearch_host');
        libraries_load('elasticsearch-php');
        $client = Elasticsearch\ClientBuilder::create()->setHosts([$elasticsearch_host])->build();
        // obtain cluster health information
        $params['v'] = true;
        $health = $client->cat()->health($params)[0];

        // if $health is not empty, create a form element to display it.
        if (! empty($health)) {
            $header = array_keys($health);
            $rows[] = array_values($health);
            $output = theme('table', ['header' => $header, 'rows' => $rows]);
            $form['health'] = [
                '#type' => 'markup',
                '#title' => t('Elasticsearch Server Health'),
                '#description' => t('The table below shows the health information of connected Elasticsearch servers.'),
                '#markup' => '<h4>'.t('Elasticsearch server health').'</h4>'.'<p>'.t('The table below shows the health of connected Elasticsearch servers.').'</p>'.$output,
            ];
        }
    } catch (\Exception $e) {
        $message = $e->getMessage();
    }

    $remotes = db_query('SELECT * FROM tripal_elasticsearch_servers')->fetchAll();

    if ($remotes) {
        $header = ["URL", "Label", "Description", "Edit", "Delete"];
        $rows = [];
        foreach ($remotes as $remote) {
            $url = $remote->url;
            $label = $remote->label;
            $description = $remote->description;
            $edit = "<a href ='tripal_elasticsearch/edit/{$remote->id}'>".t("edit")."</a>";
            $delete = "<a href ='tripal_elasticsearch/delete/{$remote->id}'>".t("delete")."</a>";

            array_push($rows, [$url, $label, $description, $edit, $delete]);
        }
        $output = "<h4>Elasticsearch Remote Servers </h4><p>Remote Elasticsearch server connections can be viewed, edited, or deleted below.</p>".theme('table', [
                'header' => $header,
                'rows' => $rows,
            ]);
        $form['remotes'] = [
            '#type' => 'markup',
            '#title' => 'Remote Elasticsearch Servers',
            '#markup' => $output,
        ];
    }
    if (! $remotes) {
        $form['remotes'] = [
            '#type' => 'markup',
            '#markup' => '<h4>'.t('Elasticsearch Remote Servers').'</h4>'.'<p>No remote Elasticsearch servers have been added. When remote servers are added, their status will be displayed below.</p>',
        ];
    }

    return $form;
}

/**
 * elasticsearch_connection_form submit
 * Submits the form, setting either the localhost or adding a new remote host
 *
 * @param $form
 * @param $form_state
 */

function elasticsearch_connection_form_submit($form, &$form_state)
{

    $server_type = $form_state['values']['server_type'];
    $url = $form_state['values']['host'];
    $label = $form_state['values']['label'];
    $description = $form_state['values']['description'];

    if ($server_type == "local") {
        libraries_load('elasticsearch-php');
        $client = Elasticsearch\ClientBuilder::create()->setHosts([$url])->build();
        // update the elasticsearch_host variable after form submission.
        variable_set('elasticsearch_host', $url);
    }

    if ($server_type == "remote") {
        $fields = [
            'url' => $url,
            'label' => $label,
            'description' => $description,
        ];
        // Insert records to the database
        db_insert('tripal_elasticsearch_servers')->fields($fields)->execute();
        drupal_set_message('External elasticsearch server added!');
        drupal_goto('admin/tripal/extension/tripal_elasticsearch');
    }
}

/**
 * Implements a function that validates and submits the form that adds new
 * elasticsearch servers.
 *
 */
function elasticsearch_connection_form_validate($form, &$form_state)
{
    $server_type = $form_state['values']['server_type'];
    $url = $form_state['values']['host'];
    $label = $form_state['values']['label'];
    $description = $form_state['values']['description'];

    if (! $server_type) {
        form_set_error('server_type', 'Please select a server type.');
    }

    if ($server_type == "local") {
        //if (! (filter_var($url, FILTER_VALIDATE_URL) || filter_var($url, FILTER_VALIDATE_IP))) {
        //    form_set_error('host', 'Please enter a valid URL for this remote host');
        //}
    }

    if ($server_type == "remote") {
        //if (! (filter_var($url, FILTER_VALIDATE_URL) || filter_var($url, FILTER_VALIDATE_IP))) {
        //    form_set_error('host', 'Please enter a valid URL for this remote host');
        //}
        if ($label == '') {
            form_set_error('label', 'Please enter a label for this remote host');
        }
        if ($description == '') {
            form_set_error('description', 'Please enter a description for this remote host ');
        }
    }
}

/**
 * Creates a confirmation page before deleting a remote server
 *
 * @param $form
 * @param $form_state
 * @param $product_code
 *
 * @return mixed
 */

function remote_delete_confirm($form, &$form_state, $remote_id)
{
    $form['_remote_id'] = [
        '#type' => 'value',
        '#value' => $remote_id,
    ];

    return confirm_form($form, t('Are you sure you want to delete the remote elasticsearch server entry?'), isset($_GET['destination']) ? $_GET['destination'] : "tripal_elasticsearch", t('Are you sure you want to delete the remote elasticsearch server entry '.$remote_id.'?'), t('Delete'), t('Cancel'));
}

/**
 * Deletes db entry for remote server
 *
 * @param $form
 * @param $form_state
 */
function remote_delete_confirm_submit($form, &$form_state)
{
    if ($form_state['values']['confirm']) {
        $remote_id = $form_state['values']['_remote_id'];
        $result = db_query("DELETE FROM {tripal_elasticsearch_servers} where id='{$remote_id}'");
        drupal_set_message(t('Remote server '.$remote_id.' has been deleted successfully.'));
    }
    drupal_goto("admin/tripal/extension/tripal_elasticsearch");
}

function remote_edit_confirm($form, &$form_state, $remote_id)
{
    $form = [];
    $url = '';
    $label = '';
    $description = '';
    $result = db_query("SELECT * FROM {tripal_elasticsearch_servers}
         WHERE id='$remote_id'")->FetchObject();

    if ($result) {
        $url = $result->url;
        $label = $result->label;
        $description = $result->description;
    }
    dpm($description);

    $form['_remote_id'] = [
        '#type' => 'value',
        '#value' => $remote_id,
    ];
    $form['url'] = [
        '#title' => t('URL'),
        '#type' => 'textfield',
        '#default_value' => $url,
        '#description' => t('URL and port of an Elasticsearch server. Examples: http://localhost:9200 or http://127.0.0.1:9200'),
    ];
    $form['label'] = [
        '#title' => t('Label'),
        '#type' => 'textfield',
        '#default_value' => $label,
        '#description' => t('Label for this Elasticsearch server'),

    ];
    $form['description_field'] = [
        '#title' => t('Description'),
        '#type' => 'textfield',
        '#default_value' => $description,
        '#description' => t('Description for this Elasticsearch server'),

    ];

    return confirm_form($form, t(''), isset($_GET['destination']) ? $_GET['destination'] : "admin/tripal/extension/tripal_elasticsearch", t(''), t('Edit'), t('Cancel'));
}

function remote_edit_confirm_submit($form, &$form_state)
{
    $form_values = $form_state['values'];
    dpm($form_values);

    if ($form_values['confirm']) {
        $remote_id = $form_values['_remote_id'];
        $url = $form_values['url'];
        $label = $form_values['label'];
        $description = $form_values['description_field'];

        $query = "UPDATE {tripal_elasticsearch_servers}
         SET url = '$url', label = '$label', description = '$description'
         WHERE id='$remote_id'";
        $result = db_query($query);
        drupal_set_message(t('Remote server '.$label.' has been edited successfully.'));
        drupal_goto("admin/tripal/extension/tripal_elasticsearch");
}