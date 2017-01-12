<?php

require_once drupal_get_path('module', 'tripal_elasticsearch').'/includes/tripal_elasticsearch_indexing_form.inc';

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
 * menu callback to display page for creating indices.
 */
function create_index_page () {
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  try{
    $client_health = $client->cat()->health();
    return drupal_get_form('tripal_elasticsearch_indexing_form');
  } catch (\Exception $e) {
    $message = $e->getMessage();
    $output = "<h2><font color='red'>$message. Please check if your elasticsearch cluster is running normally.</font></h2>";
    return $output;
  }
  return '';
}


/**
 * Menu callback which shows administrative pages for indices management.
 */
function indexing_page(){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  try{
    $client_health = $client->cat()->health();
    return drupal_get_form('tripal_elasticsearch_indexing_form');
  } catch (\Exception $e) {
    $message = $e->getMessage();
    $output = "<h2><font color='red'>$message. Please check if your elasticsearch cluster is running normally.</font></h2>";
    return $output;
  }
}

/*
function  index_management_page() {
  $output = t('This page is for elasticsearch indices management');
  return $output;
}
*/
