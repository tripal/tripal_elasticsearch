<?php

/*
 * Select all tables and corresponding fields from database table elastic_search
 * and then build search forms
 */
function tripal_elasticsearch_blocks_form($form, &$form_state, $table_name){
  $result = db_query('SELECT DISTINCT(table_name) FROM elastic_search WHERE table_name=:table_name', array(':table_name'=>$table_name));
  foreach($result as $record){
    $table_name = $record->table_name;
    $form[$table_name] = array(
      '#type' => 'fieldset',
      '#title' => '',
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
  
    $form[$table_name]['search_from'] = array(
      '#type' => 'textfield',
      '#title' => t('(1000 search hits will be displayed at maximum. Enter an integer to set up an offset.)'),
      '#size' => 30,
      '#title_display' => 'after',
      '#attributes' => array(
        'placeholder' => t('No input or enter an integer'),
      ),
    );
    
    $form[$table_name]['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search|'.$table_name),
    );
  }

  return $form;
}


function tripal_elasticsearch_blocks_form_submit($form, &$form_state){
  // Use $form_state['input'] instead of $form_state['values'] to get the correct data!!!
  $_SESSION['elastic_form_input'] = $form_state['input'];
}


