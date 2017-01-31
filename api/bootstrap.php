<?php


require_once drupal_get_path('module', 'tripal_elasticsearch') . '/vendor/autoload.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/BuildElasticIndex.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/CronQueueWorker.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticCharacterFilters.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticConnection.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticIndex.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticTokenFilters.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/FormElementsForIndex.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/GetTableList.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/tripal_elasticsearch.api.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/tripal_elasticsearch_main_search_box_form.inc';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/includes/search/tripal_elasticsearch_blocks_form.inc';
