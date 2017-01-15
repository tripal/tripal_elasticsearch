<?php


/**
 * form handler for deleting indices
 */
function tripal_elasticsearch_delete_indexed_data_form($form, &$form_state){
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
    $mappings_options = $types;
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



function tripal_elasticsearch_delete_indexed_data_form_submit($form, &$form_state){
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();

/*
    foreach($form_state['values']['tripal_elasticsearch_indices'] as $index){
        if(!empty($index)){
            $params['index'] = $index;
            $client->indices()->delete($params);
        }
    }
*/
}
