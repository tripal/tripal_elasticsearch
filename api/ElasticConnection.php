<?php

class ElasticConnection
{
    protected $elasticsearch_hosts;

    public function __construct (array $elasticsearch_hosts = ["localhost:9200"])
    {
        $this->elasticsearch_hosts = $elasticsearch_hosts;
    }

    public function make()
    {

        try {

            return Elasticsearch\ClientBuilder::create()->setHosts($this->elasticsearch_hosts)->build();

        } catch (\Exception $e) {

            $message = $e->getMessage();
            drupal_set_message($message, 'warning');

            return false;

        }
    }
}