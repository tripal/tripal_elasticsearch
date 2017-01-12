<?php

require_once drupal_get_path('module', 'tripal_elasticsearch').'/vendor/autoload.php';

$files = scandir(drupal_get_path('module', 'tripal_elasticsearch').'/elasticsearch_indices_json_templates', SCANDIR_SORT_DESCENDING);
foreach($files as $key=>$value) {
  if(!preg_match('/\.json$/', $value)) {
    unset($files[$key]);
  }
}

var_dump($files);

/*
var_dump(variable_get('elasticsearch_hots'));
$client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('127.0.0.1:9201')))->build();



$elasticsearch_index_json = json_decode(file_get_contents(drupal_get_path('module', 'tripal_elasticsearch').'/elasticsearch_index.json'), true);
var_dump($elasticsearch_index_json);
$response = $client->indices()->create($elasticsearch_index_json);
var_dump($response);
*/
