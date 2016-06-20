<?php


function tripal_elasticsearch_add_links_form($form, &$form_state){
  $result = db_query('SELECT DISTINCT(table_name) FROM tripal_elasticsearch');
  $tables = array('Select a table');
  foreach($result as $record){
    $tables[$record->table_name] = $record->table_name;
  }
  $form['add_links'] = array(
    '#type' => 'fieldset',
    '#title' => 'Add page links to search results',
    '#prefix' => '<div id="add_links">',
    '#suffix' => '</div>',
  );
  $form['add_links']['table'] = array(
    '#type' => 'select',
    '#options' => $tables,
    '#ajax' => array(
      'callback' => 'add_links_ajax',
      'wrapper' => 'add_links',
    ),
  );



  if(!empty($form_state['values']['table'])){
    $table_name = $form_state['values']['table'];
    $columns = db_query('SELECT table_field FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name))
                 ->fetchAll();
    foreach($columns as $field){
      $field = $field->table_field;
      $form['add_links'][$field] = array(
        '#type' => 'textfield',
        '#title' => $field,
        '#size' => 60,
        '#title_display' => 'after',
/*
        '#ajax' => array(
          'callback' => 'add_links_ajax',
          'wrapper' => 'add_links',
        ),
*/
      );
    }

  }

  
  $form['add_links']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add links'),
  );

  return $form;
}


function add_links_ajax($form, &$form_state){
  return $form;
}

function tripal_elasticsearch_add_links_form_submit($form, &$form_state){
  $table_name = $form_state['input']['table'];
  $delete_table_name = db_delete('tripal_elasticsearch_add_links')
              ->condition('table_name', $table_name)
              ->execute();
  $columns = db_query('SELECT table_field FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name))
                 ->fetchAll();
  $record = array();
  foreach($columns as $field){
    $field = $field->table_field;
    if(!empty($form_state['input'][$field])){
      $record['table_name'] = $table_name;
      $record['table_field'] = $field;
      $record['page_link'] = $form_state['input'][$field];
    }
    drupal_write_record('tripal_elasticsearch_add_links', $record);
  }

}
