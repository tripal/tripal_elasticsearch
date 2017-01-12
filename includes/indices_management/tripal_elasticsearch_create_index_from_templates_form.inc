<?php

function tripal_elasticsearch_create_index_from_templates_form ($form, &$form_state) {
  // get index json templates file names
  $index_json_templates = array();
  $index_json_templates = scandir(drupal_get_path('module', 'tripal_elasticsearch').'/elasticsearch_indices_json_templates', SCANDIR_SORT_ASCENDING);
  foreach($index_json_templates as $key=>$value) {
    if(!preg_match('/\.json$/', $value)) {
      unset($index_json_templates[$key]);
    } else {
      $index_json_templates[$value] = $value;
      unset($index_json_templates[$key]);
    }
  }
  $form['index_json_templates'] = array(
    '#type' => 'select',
    '#title' => t('Index JSON templates'),
    '#options' => $index_json_templates,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  
  return $form;
}
