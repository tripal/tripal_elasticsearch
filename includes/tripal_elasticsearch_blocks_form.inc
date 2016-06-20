<?php

  require_once drupal_get_path('module', 'tripal_elasticsearch').'/api/tripal_elasticsearch.api.php';

/*
 * Select all tables and corresponding fields from database table elastic_search
 * and then build search forms
 */
function tripal_elasticsearch_blocks_form($form, &$form_state, $table_name){
  $result = db_query('SELECT DISTINCT(table_name) FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name));
  foreach($result as $record){
    $table_name = $record->table_name;
    $form[$table_name] = array(
      '#type' => 'fieldset',
      '#title' => '',
      '#tree' => TRUE,
    );
    $columns = db_query('SELECT table_field FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name))
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
      '#value' => t('Search'),
      '#submit' => array('tripal_elasticsearch_blocks_form_submit_search'),
    );

    $form[$table_name]['download'] = array(
      '#type' => 'submit',
      '#value' => t('Download all search hits as csv file'),
      '#submit' => array('tripal_elasticsearch_blocks_form_submit_download'), 
    );
  }

  return $form;
}


function tripal_elasticsearch_blocks_form_submit_search($form, &$form_state){
  // Use $form_state['input'] instead of $form_state['values'] to get the correct data!!!
  $_SESSION['elastic_form_input'] = $form_state['input'];
}


function tripal_elasticsearch_blocks_form_submit_download($form, &$form_state){
  $table = _get_field_keyword_pairs($form_state['input'])['table'];
  $field_keyword_pairs = _get_field_keyword_pairs($form_state['input'])['field_keyword_pairs'];
  // Allow to download 1,000,000 records at maximum
  if(is_elastic_index($table)){
    ini_set('memory_limit', '-1');
    $search_hits = _run_elastic_search($table, $field_keyword_pairs, $from=0, $size=10000)['search_hits'];
  
    // Create a file pointer connected to the output stream
    $random_str = random_string($str_length = 5);
    $destination = file_destination('temporary://'.'/tripal_elasticsearch_'.$random_str.'.csv', FILE_EXISTS_RENAME);
    $output = fopen($destination, 'w');
  
    foreach($search_hits as $hit){
      fputcsv($output, $hit);
    }
  
    // strip off 'temprary://'
    $filename = substr($destination, 12);
    drupal_goto('sites/default/files/tmp/'. $filename);
  }
  else{
    drupal_set_message(t('No data available.'), 'error');
  }
}


function random_string($str_length){
  $random_string = substr(str_shuffle("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, $str_length);
  return $random_string;
}
