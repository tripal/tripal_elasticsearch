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
    $columns = db_query('SELECT * FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name))
                 ->fetchAll();
    foreach($columns as $column){
      $item = $column->table_field;
      $form_field_type = $column->form_field_type;
      switch($form_field_type){
        case 'textfield':
          $form[$table_name][$item] = array(
            '#type' => $column->form_field_type,
            '#title' => $column->form_field_title,
            '#default_value' => $column->form_field_default_value,
            '#size' => 40,
            '#title_display' => 'after',
            '#weight' => $column->form_field_weight,
          );
          break;
        case 'select':
          $options_first_items = array('_SelectAnyItem_' => 'Any');
          $form_field_options = explode(',', $column->form_field_options);
          // make key = value
          foreach($form_field_options as $value){
            $value = trim($value);
            $options_rest_items[$value] = $value;
          }
          // The first option is 'Any'
          $options = array_merge($options_first_items, $options_rest_items);
          $form[$table_name][$item] = array(
            '#type' => $column->form_field_type,
            '#title' => $column->form_field_title,
            '#default_value' => $options['_SelectAnyItem_'],
            '#options' => $options,
            '#title_display' => 'after',
            '#weight' => $column->form_field_weight,
          );
          break;
      }

      //$weight = $column->form_field_weight;
    }

    /*
    $form[$table_name]['search_from'] = array(
      '#type' => 'textfield',
      '#title' => t('(1000 search hits will be displayed at maximum. Enter an integer to set up an offset.)'),
      '#size' => 30,
      '#title_display' => 'after',
      '#weight' => 997,
      '#attributes' => array(
        'placeholder' => t('No input or enter an integer'),
      ),
    );
    */ 

    $form[$table_name]['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search'),
      '#submit' => array('tripal_elasticsearch_blocks_form_submit_search'),
      '#weight' => 998,
    );

    $form[$table_name]['download'] = array(
      '#type' => 'submit',
      '#value' => t('Download table'),
      '#submit' => array('tripal_elasticsearch_blocks_form_submit_download'),
      '#weight' => 999,
    );

    $form[$table_name]['usage_description'] = array(
      '#type' => 'item',
      '#markup' => '<br/><p><b>Search:</b> clicking on the search button without keywords input will return all available records.</p>',
      // <p><b>Download:</b> clicking "Download" without keywords input will download all available records as a csv file. Clicking "Download" with keywords input will implement a search and download the returned search records into a csv file.</p>',
      '#weight' => 1000,
    );
  }

  return $form;
}


function tripal_elasticsearch_blocks_form_submit_search($form, &$form_state){
  $table = array_keys($form_state['input'])[0];
  // Use $form_state['input'] instead of $form_state['values'] to get the correct data!!!
  $_SESSION['elastic_form_input'] = $form_state['input'];
  // If select 'Any', set select options as empty so that searching will return all hits.
  foreach(array_keys($_SESSION['elastic_form_input'][$table]) as $field){
    if($_SESSION['elastic_form_input'][$table][$field] == '_SelectAnyItem_'){
      $_SESSION['elastic_form_input'][$table][$field] = '';
    }
  }

  $form_state['rebuild'] = True;
}


function tripal_elasticsearch_blocks_form_submit_download($form, &$form_state){
  //$table = _get_field_keyword_pairs($form_state['input'])['table'];
  //$field_keyword_pairs = _get_field_keyword_pairs($form_state['input'])['field_keyword_pairs'];
  $table = _get_field_keyword_pairs($_SESSION['elastic_form_input'])['table'];
  $field_keyword_pairs = _get_field_keyword_pairs($_SESSION['elastic_form_input'])['field_keyword_pairs'];
  // Allow to download 100,000 records at maximum
  if(is_elastic_index($table)){
    ini_set('memory_limit', '-1');
    $search_hits = _run_elastic_search($table, $field_keyword_pairs, $from=0, $size=100000)['search_hits'];

    // Create a file pointer connected to the output stream
    $random_str = random_string($str_length = 5);
    $destination = file_destination('temporary://'.'tripal_elasticsearch_'.$random_str.'.csv', FILE_EXISTS_RENAME);
    // write content into file
    $output = fopen($destination, 'w');
    foreach($search_hits as $hit){
      fputcsv($output, $hit);
    }

    // download file
    drupal_add_http_header('Content-Type', 'text/csv; utf-8');
    file_transfer($destination, array());
  }
  else{
    drupal_set_message(t('No data available.'), 'error');
  }
}


function random_string($str_length){
  $random_string = substr(str_shuffle("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0, $str_length);
  return $random_string;
}
