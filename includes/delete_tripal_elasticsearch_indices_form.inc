<?php

function delete_tripal_elasticsearch_indices_form($form, &$form_state){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $mappings = $client->indices()->getMapping();
  $indices = array_keys($mappings);
  $options = array();
  foreach($indices as $index){
    $options[$index] = $index;
  }

  $form['tripal_elasticsearch_indices'] = array(
    '#type' => 'checkboxes',
    '#title' => t('tripal_elasticsearch indices'),
    '#options' => $options,
  );

  $form['delete_indices'] = array(
    '#type' => 'submit',
    '#value' => t('Delete tripal_elasticsearch indices'),
    '#prefix' => '<div id="delete-indices-submit">',
    '#suffix' => '</div>',
  );


  return $form;
}



function delete_tripal_elasticsearch_indices_form_submit($form, &$form_state){
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
    foreach($form_state['values']['tripal_elasticsearch_indices'] as $index){
        if(!empty($index)){
            $params['index'] = $index;
            $client->indices()->delete($params);
        }
    }

}
