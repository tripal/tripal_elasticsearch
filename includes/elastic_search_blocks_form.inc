<?php

/** elastic_search_blocks form handler **/
function elastic_search_blocks_form($form, &$form_state){

  // Build search blocks
  $search_block_elements = db_select('elastic_search', 'e')
                ->fields('e')
                ->execute();
  foreach($search_block_elements as $record){
    // Build search_block_name field only once
    if(empty($form[$record->search_block_name]['#title'])){
      $form[$record->search_block_name] = array(
        '#type' => 'fieldset',
        '#title' => $record->search_block_name,
      );
    }
    // Add fields to the fieldset
    $form[$record->search_block_name][$record->table_field] = array(
      '#type' => 'textfield',
      '#title' => $record->table_field,
    );
  }
                  

  // Options to delete blocks
  $res = db_select('elastic_search', 'e')
        ->fields('e')
        ->execute()
        ->fetchAllAssoc('search_block_name');
  $options = array();
  foreach(array_keys($res) as $search_block_name){
    $options[$search_block_name] = $search_block_name;
  }
  // create form for deleting search blocks
  $form['delete_search_blocks'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Delete search blocks'),
    '#options' => $options
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete blocks'),
    '#submit' => array('_delete_elastic_block'),
  );

  
  return $form;
}


/**
 *elastic_search_blocks_form submit callback
*/
function _delete_elastic_block($form, &$form_state){
  $delete_blocks = $form_state['values']['delete_search_blocks'];
  foreach($delete_blocks as $search_block_name){
    if(!empty($search_block_name)){
      db_delete('elastic_search')
        ->condition('search_block_name', $search_block_name)
        ->execute();
    }
  }
  
  $form_state['rebuild'] = TRUE;
}






