<?php

/** elastic_search_blocks form handler **/
function delete_tripal_elasticsearch_blocks_form($form, &$form_state){
                 
  // Options to delete blocks
  $res = db_select('tripal_elasticsearch', 'e')
        ->fields('e')
        ->execute()
        ->fetchAllAssoc('table_name');
  $options = array();
  foreach(array_keys($res) as $table_name){
    $options[$table_name] = $table_name;
  }
  // create form for deleting search blocks
  $form['delete_search_blocks'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Delete search blocks'),
    '#options' => $options
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );

  
  return $form;
}


/**
 *elastic_search_blocks_form submit callback
*/
function delete_tripal_elasticsearch_blocks_form_submit($form, &$form_state){
  $delete_blocks = $form_state['values']['delete_search_blocks'];
  foreach($delete_blocks as $table_name){
    if(!empty($table_name)){
      db_delete('tripal_elasticsearch')
        ->condition('table_name', $table_name)
        ->execute();

      db_delete('tripal_elasticsearch_add_links')
        ->condition('table_name', $table_name)
        ->execute();
    }
  }
  
  $form_state['rebuild'] = TRUE;
}






