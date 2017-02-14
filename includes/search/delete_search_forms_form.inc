<?php

/*
 * delete search forms by table names
 */
function delete_search_forms_form ($form, &$form_state) {
    // Options to delete forms
    $res = db_select('tripal_elasticsearch_search_forms', 'e')
        ->fields('e')
        ->execute()
        ->fetchAllAssoc('table_name');
    $options = array();
    foreach (array_keys($res) as $table_name) {
        $options[$table_name] = $table_name;
    }
    // create form for deleting search forms
    $form['delete_search_forms'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Delete search forms'),
        '#options' => $options
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Delete'),
    );

    return $form;
}


/*
 * delete_search_forms_form submit
 */
function delete_search_forms_form_submit ($form, &$form_state) {
    $delete_forms = $form_state['values']['delete_search_forms'];
    foreach ($delete_forms as $table_name) {
        if (!empty($table_name)) {
            db_delete('tripal_elasticsearch_search_forms')
                ->condition('table_name', $table_name)
                ->execute();

            db_delete('tripal_elasticsearch_add_links')
                ->condition('table_name', $table_name)
                ->execute();
        }
    }

}