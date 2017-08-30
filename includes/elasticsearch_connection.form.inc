<?php
// $Id$

/**
 * @file
 * Admin page callback to build form for Elasticsearch connection.
 */
function elasticsearch_connection_form($form, &$form_state)
{

    $form['server_type'] = [
        '#title' => t('File type'),
        '#type' => 'radios',
        '#required' => true,
        '#options' => [
            'local' => t('A local Elasticsearch server.  This will be your primary search database.'),
            'remote' => t('A remote Elasticsearch server.  You can connect any number of additional servers.'),
        ],
    ];

    $form['local_host'] = [
        '#type' => 'fieldset',
        '#title' => t('Elasticsearch Local Server'),
        '#states' => [
            'visible' => [
                ':input[name="server_type"]' => ['value' => 'local'],
            ],
        ],
    ];

    $form['local_host']['host'] = [
        '#type' => 'textfield',
        '#title' => 'Elasticsearch Server URL',
        '#description' => t('URL and port of an elasticsearch server. Examples: http://localhost:9200 or http://127.0.0.1:9200'),
        '#default_value' => variable_get('elasticsearch_host'),
        '#attributes' => [
            'placeholder' => 'http://localhost:9200',
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
    $form['remote_host']['host'] = [
        '#type' => 'textfield',
        '#title' => 'Elasticsearch Server URL',
        '#description' => t('URL and port of an elasticsearch server. Examples: http://localhost:9200 or http://127.0.0.1:9200'),
        '#default_value' => variable_get('elasticsearch_host'),
        '#attributes' => [
            'placeholder' => 'http://localhost:9200',
        ],
    ];
    $form['remote_host']['label'] = [
        '#type' => 'textfield',
        '#title' => 'Elasticsearch Server Label',
        '#description' => t('Label for this elasticsearch server'),
        '#default_value' => variable_get('My Elasticsearch Server'),
        '#attributes' => [],
    ];

    $form['remote_host']['description'] = [
        '#type' => 'textfield',
        '#title' => 'Elasticsearch Server Description',
        '#description' => t('A description for this Elasticsearch server.'),
        '#default_value' => variable_get(''),
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

        //TODO:  Add remote ES servers to this table.

        // if $health is not empty, create a form element to display it.
        if (! empty($health)) {
            $header = array_keys($health);
            $rows[] = array_values($health);
            $output = theme('table', ['header' => $header, 'rows' => $rows]);
            $form['health'] = [
                '#type' => 'markup',
                '#title' => t('Elasticsearch server health'),
                '#description' => t('The table below shows the health information of connected Elasticsearch servers.'),
                '#markup' => '<h4>'.t('Elasticsearch server health').'</h4>'.'<p>'.t('The table below shows the health of connected Elasticsearch servers.').'</p>'.$output,
            ];
        }
    } catch (\Exception $e) {
        $message = $e->getMessage();
    }
    return $form;
}

    /**
     *
     * Table of existing remote DBs
     * TO DO:  modify function to return something that meshes with above health table.
     */

    function elasticsearch_fetch_DBs()
    {
        $results = db_query('SELECT * FROM tripal_elasticsearch_servers')->fetchAssociated();

        return ($results);
    }


/**
 * elasticsearch_connection_form submit
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
