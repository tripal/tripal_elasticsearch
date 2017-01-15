<?php


/**
 * form handler for deleting indices
 */
function tripal_elasticsearch_delete_indices_form($form, &$form_state){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $mappings = $client->indices()->getMapping();
  $indices = array_keys($mappings);
  foreach($indices as $key=>$index){
    $indices[$index] = $index;
    unset($indices);
  }

  $form['indices'] = array(
    '#type' => 'checkbox',
    '#title' => t('Choose an item to delete'),
    '#options' => array('Select an index' => 'Select an index') + $indices,
    '#default_value' => 'mappings',
    '#ajax' => array(
      'callback' => 'display_mappings_ajax',
      'wrapper' => 'display_mappings_options',
    ),
  );

  $selected_index = !empty($form_state['values']['indices']) ? $form_state['values']['indices'] : '';
  

  $form['delete_indices'] = array(
    '#type' => 'submit',
    '#value' => t('Delete tripal_elasticsearch indices'),
    '#prefix' => '<div id="delete-indices-submit">',
    '#suffix' => '</div>',
  );


  return $form;
}



function tripal_elasticsearch_delete_indices_form_submit($form, &$form_state){
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
    foreach($form_state['values']['tripal_elasticsearch_indices'] as $index){
        if(!empty($index)){
            $params['index'] = $index;
            $client->indices()->delete($params);
        }
    }

}
