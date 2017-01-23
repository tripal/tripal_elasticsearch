<?php


/**
 * form handler for deleting indices
 */
function tripal_elasticsearch_delete_indices_form($form, &$form_state){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $mappings = $client->indices()->getMapping();
  $indices = array_keys($mappings);
  $indices_options = array();
  foreach($indices as $index){
    $indices_options[$index] = $index;
  }

  $form['indices'] = array(
    '#type' => 'select',
    '#title' => t('Choose an item to delete'),
    '#options' => array('Select an index' => 'Select an index') + $indices_options,
    '#default_value' => 'mappings',
    '#ajax' => array(
      'callback' => 'display_mappings_ajax',
      'wrapper' => 'display_mappings',
    ),
  );

  if(!empty($form_state['values']['indices'])) {
    $selected_index = $form_state['values']['indices'];
    $types = array_keys($mappings[$selected_index]['mappings']);
    foreach($types as $type) {
      $mappings_options[$type] = $type;
    }
  } else {
    $mappings_options = array();
  }
  $form['mappings'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select mappings'),
    '#description' => t('Mappings are indexed database tables.'),
    '#options' => $mappings_options,
    '#prefix' => '<div id="display_mappings">',
    '#suffix' => '</div>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  


  return $form;
}


/**
 * display_mappings ajax
 */
function display_mappings_ajax($form, &$form_state) {

  return $form['mappings'];
}


/**
 * form validation
 */
function tripal_elasticsearch_delete_indexed_data_form_validate($form, &$form_state) {
  if(empty($form_state['values']['mappings'])) {
    form_set_error('mappings', t('Please select index and mappings'));
  }
}

function tripal_elasticsearch_delete_indexed_data_form_submit($form, &$form_state){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $index = $form_state['values']['indices'];
  $types = $form_state['values']['mappings'];
  try {
    foreach($types as $type) {
      $params = array();
      $params['index'] = array($index);
      $params['type'] = $type;
      dpm($params);
      $response = $client->indices()->deleteMapping($params);
    }
  } catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'warning');
  }

}
