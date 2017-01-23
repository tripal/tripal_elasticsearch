<?php

require_once drupal_get_path('module', 'tripal_elasticsearch').'/includes/indices_management/tripal_elasticsearch_indexing_website_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch').'/includes/indices_management/tripal_elasticsearch_indexing_database_table_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch').'/includes/indices_management/tripal_elasticsearch_delete_indices_form.inc';

/**
 * @file
 * Administrative page callbacks for tripal_elasticsearch module.
 */


/*
 ** page callback to display the admin pages links
 */
function tripal_elasticsearch_admin_page() {
  $pre = 'admin/tripal/extension/tripal_elasticsearch';
  $connect_to_cluster = l('Connect to elasticsearch cluster', $pre.'/connect_to_elasticsearch_cluster');
  $index = l('Indices Management', $pre.'/indices_management');
  $search = l('Search', $pre.'/search');
  $links = array($connect_to_cluster, $index, $search);

  return theme('item_list', array('items'=>$links, 'title'=>t('Tripal Elasticsearch Administration'), 'type'=>'ul'));
}




/**
 * Menu callback for indexing the entire website.
 */
function indexing_website_page(){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  try{
    $client_health = $client->cat()->health();
    return drupal_get_form('tripal_elasticsearch_indexing_website_form');
  } catch (\Exception $e) {
    $message = $e->getMessage();
    drupal_set_message($message, 'warning');
    return $output;
  }
}


/**
 * Menu callback for indexing database tables.
 */
function indexing_database_table_page(){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  try{
    $client_health = $client->cat()->health();
    return drupal_get_form('tripal_elasticsearch_indexing_database_table_form');
  } catch (\Exception $e) {
    $message = $e->getMessage();
    drupal_set_message($message, 'warning');
    return $output;
  }
}

/**
 * menu callback for deleting indices page
 */
function delete_indices_page(){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  try{
    $client_health = $client->cat()->health();
    return drupal_get_form('tripal_elasticsearch_delete_indices_form');
  } catch (\Exception $e) {
    $message = $e->getMessage();
    drupal_set_message($message, 'warning');
  }
}




 
