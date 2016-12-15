<?php

require_once drupal_get_path('module', 'tripal_elasticsearch').'/api/tripal_elasticsearch.api.php';

function connect_to_elasticsearch_cluster_form($form, &$form_state){

  $form['cluster_health'] = array(
    '#markup' => get_cluster_health(), 
  );
  //$connectionPoll = (array) $client->transport->connectionPool;
  //dpm($connectionPoll);
  //dpm($client->cat()->health());
  $form['elasticsearch_cluster'] = array(
    '#type' => 'fieldset',
    '#title' => t('Any host from an elasticsearch cluster'),
    '#tree' => true, 
  );
  $form['elasticsearch_cluster']['host'] = array(
    '#type' => 'textfield',
    '#title' => t(''),
    '#attributes' => array(
      'placeholder' => t('127.0.0.1: 9200'),
    ),
  );
  $form['elasticsearch_cluster']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}



/**
 * connect_to_elasticsearch_cluster_form submit callback 
 */
function connect_to_elasticsearch_cluster_form_submit($form, &$form_state){
  variable_set('elasticsearch_hosts', array_filter($form_state['values']['elasticsearch_cluster']));

  
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $params['v'] = true;
  //dpm($client->cat()->health($params));
}
