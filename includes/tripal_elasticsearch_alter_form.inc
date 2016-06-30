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
                 ->fetchAll();
    foreach($columns as $field){
      $field = $field->table_field;
      $form['alter_field'][$field] = array(
        '#type' => 'fieldset',
        '#title' => $field,
      );
      $form['alter_field'][$field]['input_type'] = array(
        '#type' => 'radios',
        '#title' => t('Input type'),
        '#default_value' => t('textfield'),
        '#options' => array('select'=>'select', 'textfield'=>'textfield'),
      );
      $form['alter_field'][$field]['input_options'] = array(
        '#type' => 'textarea',
        '#title' => t('Enter options'),
      );
      $form['alter_field'][$field]['input_weight'] = array(
        '#type' => 'select',
        '#title' => t('Field weight'),
        '#options' => range(0, 50)
      );
    }


    $form['alter_field']['submit'] = array(
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
 * tripal_elasticsearch_form_tripal_elasticsearch_alter_form_alter hook.
 */
function tripal_elasticsearch_form_tripal_elasticsearch_alter_form_alter($form, &$form_state){
  dpm('test text');
}

/*
 * tripal_elasticsearch_alter_form submit
 */
function tripal_elasticsearch_alter_form_submit($form, &$form_state){
  dpm($form_state['values']);
}




function tripal_elasticsearch_form_alter(&$form, &$form_state, $form_id){
  dpm($form);
}
