<?php

function tripal_elasticsearch_alter_form($form, &$form_state){
  $result = db_query('SELECT DISTINCT(table_name) FROM tripal_elasticsearch');
  $tables = array('Select a table');
  foreach($result as $record){
    $tables[$record->table_name] = $record->table_name;
  }
  $form['alter_field'] = array(
    '#type' => 'fieldset',
    '#title' => 'Alter form fields',
    '#tree' => true,
    '#prefix' => '<div id="alter_field">',
    '#suffix' => '</div>',
  );
  $form['alter_field']['table'] = array(
    '#type' => 'select',
    '#options' => $tables,
    '#ajax' => array(
      'callback' => 'alter_field_ajax',
      'wrapper' => 'alter_field',
    ),
  );
  if(!empty($form_state['values']['alter_field']['table'])){
    $table_name = $form_state['values']['alter_field']['table'];
    $columns = db_query('SELECT table_field FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name))
                 ->fetchCol(0);
    foreach($columns as $field){
      //$field = $field->table_field;
      // count unique entries in this field
      // if entry number is larger than 1000, only textfield is available for field type
      // if entry number is equal or less than 1000, both textfield and select (drop-down list)
      // are available for field type
      $one_fieldtype = array('textfield' => 'textfield');
      $two_fieldtypes = array('select'=>'select (drop-down list)', 'textfield'=>'textfield');
      $sql_unique_entries = "SELECT COUNT(DISTINCT($field)) FROM {".$table_name."}";
      $unique_entries = chado_query($sql_unique_entries)->fetchCol('count');
      $entry_count = $unique_entries[0]; 
      $options = ($entry_count > 100)? $one_fieldtype: $two_fieldtypes;
      $options_content = '';
      if($entry_count > 100){
        $options = $one_fieldtype;
      } else {
        $options = $two_fieldtypes;
        $sql = "SELECT DISTINCT($field) FROM {".$table_name."}";
        $options_content = chado_query($sql)->fetchCol(0);
      }
      //========
      $form['alter_field'][$field] = array(
        '#type' => 'fieldset',
        '#title' => $field,
      );
      $form['alter_field'][$field]['field_type'] = array(
        '#type' => 'radios',
        '#title' => t('Field type'),
        '#default_value' => t('textfield'),
        //'#options' => array('select'=>'select (drop-down list)', 'textfield'=>'textfield'),
        '#options' => $options,
        '#description' => t('Only textfield type is available if this field has more than 100 unique entries.'),
      );
      $form['alter_field'][$field]['field_title'] = array(
        '#type' => 'textfield',
        '#title' => t('Field title'),
        '#default_value' => $field,
      );
      $form['alter_field'][$field]['field_default_value'] = array(
        '#type' => 'textfield',
        '#title' => t('Field default value')
      );
      $form['alter_field'][$field]['field_options'] = array(
        '#type' => 'textarea',
        '#title' => t('Field options'),
        '#default_value' => implode(',', $options_content),
      );
      $form['alter_field'][$field]['field_weight'] = array(
        '#type' => 'select',
        '#title' => t('Field weight'),
        '#options' => range(0, 50)
      );
    }


    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );


  }

  return $form;
}


/*
 * tripal_elasticsearch_alter_form ajax
 */
function alter_field_ajax($form, &$form_state){
  return $form;
}



/*
 * tripal_elasticsearch_alter_form submit
 */
function tripal_elasticsearch_alter_form_submit($form, &$form_state){
  $table_name = $form_state['values']['alter_field']['table'];
  // Delete the table and its fields in the database if that table name already exists
  $delete_table_name = db_delete('tripal_elasticsearch')
              ->condition('table_name', $table_name)
              ->execute();
  unset($form_state['values']['alter_field']['table']);
  $table_fields = array_keys($form_state['values']['alter_field']);
  foreach($table_fields as $table_field){
    $record['table_name'] = $table_name;
    $record['table_field'] = $table_field;
    $record['form_field_type'] = $form_state['values']['alter_field'][$table_field]['field_type'];
    $record['form_field_title'] = $form_state['values']['alter_field'][$table_field]['field_title'];
    $record['form_field_value'] = $form_state['values']['alter_field'][$table_field]['field_default_value'];
    if($record['form_field_type'] == 'textfield'){
      $record['form_field_options'] = '';
    } else {
      $record['form_field_options'] = $form_state['values']['alter_field'][$table_field]['field_options'];
    }
    $record['form_field_weight'] = $form_state['values']['alter_field'][$table_field]['field_weight'];
  
    drupal_write_record('tripal_elasticsearch', $record);
  }

}



