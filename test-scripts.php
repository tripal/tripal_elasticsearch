<?php

require_once drupal_get_path('module', 'tripal_elasticsearch').'/vendor/autoload.php';

$client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array()))->build();

var_dump($client->health());
