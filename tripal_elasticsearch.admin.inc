<?php

/*
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/elasticsearch_cluster_connection/connect_to_elasticsearch_cluster_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/indices_management/tripal_elasticsearch_indexing_website_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/indices_management/tripal_elasticsearch_indexing_database_table_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/indices_management/tripal_elasticsearch_delete_indices_form.inc';
*/

require drupal_get_path('module', 'tripal_elasticsearch') . '/api/bootstrap.php';

/**
 * @file
 * Administrative page callbacks for tripal_elasticsearch module.
 */


/**
 * page callback to connect_to_elasticsearch_cluster_page
 */
function connect_to_elasticsearch_cluster_page()
{

    return drupal_get_form('connect_to_elasticsearch_cluster_form');

}

/*
 ** page callback to display the admin pages links
 */
function tripal_elasticsearch_admin_page()
{
    $pre = 'admin/tripal/extension/tripal_elasticsearch';
    $connect_to_cluster = l('Connect to elasticsearch cluster', $pre . '/connect_to_elasticsearch_cluster');
    $index = l('Indices Management', $pre . '/indices_management');
    $search = l('Search', $pre . '/search');
    $links = array($connect_to_cluster, $index, $search);

    return theme('item_list', array('items' => $links, 'title' => t('Tripal Elasticsearch Administration'), 'type' => 'ul'));
}


/**
 * Menu callback for indexing the entire website.
 */
function indexing_website_page()
{
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
    try {
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
function indexing_database_table_page()
{
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
    try {
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
function delete_indices_page()
{
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
    try {
        $client_health = $client->cat()->health();
        return drupal_get_form('tripal_elasticsearch_delete_indices_form');
    } catch (\Exception $e) {
        $message = $e->getMessage();
        drupal_set_message($message, 'warning');
    }
}



/*
 * build_search_forms_page callback
 */
function build_search_forms_page ()
{
    // create a connection port
    // $connection = (new ElasticConnection(["127.0.0.1:9201"]))->make();
    return drupal_get_form ('build_search_forms_form');

    /*
    if ($connection) {
        return drupal_get_form ('build_search_forms_form');
    } else {
        drupal_set_message ('Elasticsearch cluster is disconnected.', 'warning');
        return '';
    }
    */

}
 

/*
 * link_results_to_pages_page callback
 */
function link_results_to_pages_page ()
{
    $example_form = drupal_get_form('link_results_to_pages_example_form');
    $form = drupal_get_form('link_results_to_pages_form');
    $output = drupal_render($example_form).drupal_render($form);

    return $output;

}

/*
 * alter_search_forms_page callback
 */
function alter_search_forms_page ()
{
    // create a connection port
    $connection = (new ElasticConnection(["127.0.0.1:9201"]))->make();
    if ($connection) {
        return drupal_get_form ('alter_search_forms_form');
    } else {
        drupal_set_message ('Elasticsearch cluster is disconnected.', 'warning');
        return '';
    }


}

/*
 * delete_search_forms_page callback
 */
function delete_search_forms_page ()
{
    // create a connection port
    $connection = (new ElasticConnection(["127.0.0.1:9201"]))->make();
    if ($connection) {
        return drupal_get_form ('delete_search_forms_form');
    } else {
        drupal_set_message ('Elasticsearch cluster is disconnected.', 'warning');
        return '';
    }

}