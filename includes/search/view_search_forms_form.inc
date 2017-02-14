<?php
/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 2/14/17
 * Time: 10:49 AM
 */

/*
$table_name = 'chado.cvterm';
$res = db_select('tripal_elasticsearch_search_forms', 'e')
    ->fields('e')
    ->condition('table_name', $table_name)
    ->execute();

foreach ($res as $record) {
    var_dump($record);
}
*/


function view_search_forms_form ($form, &$form_state, $table_name) {
    $res = db_select('tripal_elasticsearch_search_forms', 'e')
        ->fields('e')
        ->condition('table_name', $table_name)
        ->execute();

    foreach ($res as $record) {
        $table_name = $record->table_name;
        $table_field = $record->table_field;
        $table_field_label = $record->form_field_label;
        $table_field_type = $record->form_field_type;
        $table_field_weight = $record->form_field_weight;

        $form[$table_name] = [
            '#type' => 'fieldset',
        ];
        if ($table_field_type == '#select') {
            $form[$table_name][$table_field] = [
                '#type' => $table_field_type,
                '#title' => t($table_field_label),
                '#weight' => $table_field_weight,
                '#options' => [
                    'op1' => 'op1',
                    'op2' => 'op2',
                ]
            ];
        } else {
            $form[$table_name][$table_field] = [
                '#type' => $table_field_type,
                '#title' => t($table_field_label),
                '#weight' => $table_field_weight,
            ];
        }

        $form['submit'] = [
            '#type' => 'submit',
            '#value' => t('Search')
        ];

    }

    dpm($form);

    return $form;
}
