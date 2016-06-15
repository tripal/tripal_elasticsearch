<?php

function ES_table_blocks_form($form, &$form_state){
  $result = db_query('SELECT DISTINCT(table_name),search_block_name FROM elastic_search');
  foreach($result as $record){
    $table_name = $record->table_name;
    $form[$table_name] = array(
      '#type' => 'fieldset',
      '#title' => $record->search_block_name, 
    );
    $columns = db_query('SELECT table_field FROM elastic_search WHERE table_name=:table_name', array(':table_name'=>$record_name));
    $table_columns = get_object_vars($columns);
    foreach($table_columns as $column){
      $form[$table_name][$column] = array(
        '#type' => 'textfield',
        'title' => $column,
        '#size' => 40,
        '#title_display' => 'after',
      );
    }
  }

  return $form;
}
