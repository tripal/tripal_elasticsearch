<?php


/**
 * form handler for deleting indices
 */
function tripal_elasticsearch_delete_indices_form($form, &$form_state)
{
    $connection = (new ElasticConnection(["localhost:9201"]))->make();
    $indices_options = (new ElasticIndex($connection))->GetIndices();

    $form['indices'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Select items to delete'),
        '#options' => $indices_options,
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#attributes' => [
            'id' => ['indices-delete-confirmation']
        ]
    );

    drupal_add_js(drupal_get_path('module', 'tripal_elasticsearch').'/js/indices-delete-confirmation.js');


    return $form;
}




function tripal_elasticsearch_delete_indices_form_submit($form, &$form_state)
{
    $indices = array_filter($form_state['values']['indices'], "is_string");
    $connection = (new ElasticConnection(["localhost:9201"]))->make();
    (new ElasticIndex($connection))->DeleteIndices($indices);
}
