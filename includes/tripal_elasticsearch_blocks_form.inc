<?php



function ES_table_blocks_form($form, &$form_state, $table_name){
  $result = db_query('SELECT DISTINCT(table_name),search_block_name FROM elastic_search WHERE table_name=:table_name', array(':table_name'=>$table_name));
  foreach($result as $record){
    $table_name = $record->table_name;
    $form[$table_name] = array(
      '#type' => 'fieldset',
      '#title' => $table_name,
      '#tree' => TRUE,
    );
    $columns = db_query('SELECT table_field FROM elastic_search WHERE table_name=:table_name', array(':table_name'=>$table_name))
                 ->fetchAll();
    foreach($columns as $column){
      $item = $column->table_field;
      $form[$table_name][$item] = array(
        '#type' => 'textfield',
        '#title' => $item,
        '#size' => 40,
        '#title_display' => 'after',
      );
    }
    
    $form[$table_name]['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search|'.$table_name),
    );
  }

  return $form;
}


function ES_table_blocks_form_submit($form, &$form_state){
  // Use $form_state['input'] instead of $form_state['values'] to get the correct data!!!
  $_SESSION['elastic_form_input'] = $form_state['input'];
}
