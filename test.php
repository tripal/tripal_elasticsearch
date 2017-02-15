<?php


$connection = (new ElasticConnection(['localhost:9201']))->make();
$elastic_index = new ElasticIndex($connection);

var_dump($elastic_index->GetFieldMappingTypes('website'));