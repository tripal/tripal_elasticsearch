<?php

/** elastic_search_blocks form handler **/
function elastic_search_blocks_form($form, &$form_state){
                 
  // Options to delete blocks
  $res = db_select('elastic_search', 'e')
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
    '#submit' => array('_delete_elastic_block'),
  );

  
  return $form;
}


/**
 *elastic_search_blocks_form submit callback
*/
function _delete_elastic_block($form, &$form_state){
  $delete_blocks = $form_state['values']['delete_search_blocks'];
  foreach($delete_blocks as $table_name){
    if(!empty($table_name)){
      db_delete('elastic_search')
        ->condition('table_name', $table_name)
        ->execute();
    }
  }
  
  $form_state['rebuild'] = TRUE;
}






