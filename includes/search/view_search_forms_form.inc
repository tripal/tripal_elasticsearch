<?php
/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 2/14/17
 * Time: 10:49 AM
 */


function view_search_forms_form ($form, &$form_state, $table_name) {
    //$table_name = 'chado.cvterm';
    $res = db_select('tripal_elasticsearch_search_forms', 'e')
        ->fields('e')
        ->condition('table_name', $table_name)
        ->execute();

    $form['_table_name'] = [
        '#type' => 'hidden',
        '#value' => $table_name,
    ];

    $form[$table_name] = [
        '#type' => 'fieldset',
        '#title' => $table_name,
        '#tree' => true,
    ];

    $form_state['storage']['fields'] = [];
    foreach ($res as $record) {
        $table_name = $record->table_name;
        $table_field = $record->table_field;
        // store table fields
        $form_state['storage']['fields'][] = $table_field;
        $form_field_label = $record->form_field_label;
        $form_field_type = $record->form_field_type;
        $table_field_weight = $record->form_field_weight;

        if ($form_field_type == 'select') {
            $form[$table_name][$table_field] = [
                '#type' => $form_field_type,
                '#title' => t($form_field_label),
                '#weight' => $table_field_weight,
                '#options' => [
                    'op1' => 'op1',
                    'op2' => 'op2',
                ]
            ];
        } else {
            $form[$table_name][$table_field] = [
                '#type' => $form_field_type,
                '#title' => t($form_field_label),
                '#weight' => $table_field_weight,
            ];
        }

    }

    $form['submit'] = [
        '#type' => 'submit',
        '#value' => t('Search')
    ];

    $form['#method'] = 'get';

    $form_state['rebuild'] = true;

    return $form;
}


function view_search_forms_form_submit ($form, &$form_state) {

    $connection = (new ElasticConnection(["127.0.0.1:9201"]))->make();
    $elastic_search = new ElasticSearch($connection);

    $table_name = $form_state['storage']['table_name'];
    $index = preg_replace('/^chado./', 'chado_', $table_name);
    $type = $table_name;

    foreach ($form_state['storage']['fields'] as $field) {
        $field_content_pairs[$field] = $form_state['values'][$field];
    }
    $query = $elastic_search->build_search_query_from_field_content_pairs($field_content_pairs);
    $params = $elastic_search->build_search_params($index = $index, $type = $type, $query = $query);
    $search_res = $elastic_search->search($params);

    dpm($search_res);



    // display search results in a table

    $per_page = 10;
    foreach ($form_state['storage']['fields'] as $field) {
        $header[] = [
            'data' => $field,
            'field' => $field,
        ];
    }
    $rows = $search_res;
    $current_page = pager_default_initialize(count($rows), $per_page);
    $chunks = array_chunk($rows, $per_page, TRUE);
    $output = theme('table', array('header' => $header, 'rows' => $chunks[$current_page]));
    $output .= theme('pager', array('quantity', count($rows)));

    $form['search_results'] = [
        '#markup' => $output,
    ];



    $form_state['rebuild'] = true;
}





