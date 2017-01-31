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
        '#title' => t('Select items to delete'),
        '#options' => ['none' => 'Select an index'] + $indices_options,
        '#ajax' => [
            'callback' => 'display_index_mappings_ajax',
            'wrapper' => 'display_index_mappings'
        ]
    );

    $index_name = !empty($form_state['values']['indices']) ? $form_state['values']['indices'] : 'Select an index';

    $tableselect_rows = [];
    try{
        // get fields for a select index
        $index_mappings = $elastic_index->GetMappings($index_name);
        foreach ($index_mappings as $field) {
            $tableselect_rows[$field] = array(
                'field' => $field,
                //'mapping_type' => get_elastic_mapping_types(),
                'form_input' => array(
                    'data' => array(
                        '#type' => 'textfield',
                        '#title' => '',
                        '#title_display' => 'invisible',
                    )
                ),
            );
        }
    } catch (Exception $exception) {
        //drupal_set_message($exception->getMessage(), 'warning');
    }

    $header = array(
        'field' => t('Field'),
        'form_input' => t('Form input')
    );

    $form['index_mappings'] = array(
        '#type' => 'tableselect',
        '#tree' => true,
        '#header' => $header,
        '#options' => $tableselect_rows,
        '#empty' => t('No fields available.'),
        '#prefix' => '<div id="display_index_mappings">',
        '#suffix' => '</div>',
    );

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
    return $form['index_mappings'];
}