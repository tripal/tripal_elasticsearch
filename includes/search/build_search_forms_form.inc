<?php
/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/31/17
 * Time: 2:58 PM
 */

function build_search_forms_form ($form, &$form_state) {
    $connection = (new ElasticConnection(["localhost:9201"]))->make();
    $elastic_index = (new ElasticIndex($connection));
    $indices_options = $elastic_index->GetIndices();

    $form['indices'] = array(
        '#type' => 'select',
        '#title' => t('Select an index'),
        '#options' => ['none' => 'Select an index'] + $indices_options,
        '#ajax' => [
            'callback' => 'display_index_mappings_ajax',
            'wrapper' => 'display_index_mappings'
        ]
    );

    $index_name = !empty($form_state['values']['indices']) ? $form_state['values']['indices'] : 'Select an index';

    $form['form_fields'] = [
        '#type' => 'fieldset',
        '#title' => t('Form Fields'),
        '#tree' => true,
        '#prefix' => '<div id="display_index_mappings">',
        '#suffix' => '</div>',
    ];

    try{
        // get fields for a select index
        $index_mappings = $elastic_index->GetMappings($index_name);

        foreach ($index_mappings as $field) {
            $form['form_fields'][$field] = [
                '#type' => 'fieldset',
                '#title' => t($field),
                '#collapsible' => true,
                '#collapsed' => true,
            ];
            $form['form_fields'][$field]['field_label'] = [
                '#type' => 'textfield',
                '#title' => t('Form Field Label'),
                '#default_value' => $field,
                '#attributes' => [
                    'placeholder' => t($field),
                ],
            ];
            $form['form_fields'][$field]['field_type'] = [
                '#type' => 'select',
                '#title' => t('Form Field Input Type'),
                '#options' => [
                    '#textfield' => 'Input Box',
                    '#select' => 'Drop Down'
                ],
            ];
            $form['form_fields'][$field]['field_weight'] = [
                '#type' => 'select',
                '#title' => t('Form Field Weight'),
                '#options' => array_combine(range(1, 20), range(1, 20)),
            ];
        }
    } catch (Exception $exception) {
        //drupal_set_message($exception->getMessage(), 'warning');
    }


    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
    );

    return $form;
}


/*
 * display_index_mappings_ajax callback
 */
function display_index_mappings_ajax ($form, &$form_state) {
    return $form['form_fields'];
}


/*
 * build_search_forms_form submit
 */
function build_search_forms_form_submit ($form, &$form_state) {

    $index_name = $form_state['values']['indices'];
    $table_name = preg_replace('/^chado_/', 'chado.', $index_name);

    // Delete the table and its fields in the database if that table name already exists
    db_delete('tripal_elasticsearch_search_forms')
        ->condition('table_name', $table_name)
        ->execute();

    $table_fields = array_keys($form_state['values']['form_fields']);
    foreach ($table_fields as $field) {
        $record['table_name'] = $table_name;
        $record['table_field'] = $field;
        $record['form_field_label'] = $form_state['values']['form_fields'][$field]['field_label'];
        $record['form_field_type'] = $form_state['values']['form_fields'][$field]['field_type'];
        $record['form_field_weight'] = $form_state['values']['form_fields'][$field]['field_weight'];

        drupal_write_record('tripal_elasticsearch_search_forms', $record);
    }

    drupal_set_message(t('A search form block has been created for table '. $table_name));
}