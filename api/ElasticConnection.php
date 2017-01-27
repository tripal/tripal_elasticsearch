<?php

class ElasticConnection
{
    public static function make()
    {

        try {

            return Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();

        } catch (\Exception $e) {

            $message = $e->getMessage();
            drupal_set_message($message, 'warning');

        }
    }
}