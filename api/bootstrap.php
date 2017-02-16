<?php


require_once drupal_get_path('module', 'tripal_elasticsearch') . '/vendor/autoload.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/BuildElasticIndex.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/CronQueueWorker.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticCharacterFilters.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticConnection.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticIndex.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticSearch.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticTokenFilters.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/FormElementsForIndex.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/GetTableList.php';
//require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/tripal_elasticsearch.api.php';



require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/elasticsearch_cluster_connection/connect_to_elasticsearch_cluster_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/indices_management/tripal_elasticsearch_indexing_website_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/indices_management/tripal_elasticsearch_indexing_database_table_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/indices_management/tripal_elasticsearch_delete_indices_form.inc';


require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/build_search_forms_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/link_results_to_pages_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/alter_search_forms_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/delete_search_forms_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/view_search_forms_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/sitewide_search_box_form.inc';