<?php

/**
 * Provides web services support to Elasticsearch server.
 *
 * @file includes/tripal_elasticsearch.ws.inc
 */

/**
 * Report the status of the elastic search index.
 *
 * @return bool
 */
function tripal_elasticsearch_api_v1_status() {
  try {
    $params['v'] = TRUE;
    $es = new ESInstance();
    $health = current($es->client->cat()->health($params));
  } catch (Exception $exception) {
    return ESResponse::error([
      'status' => 'Inactive',
      'healthy' => FALSE,
    ]);
  }

  if (empty($health)) {
    return ESResponse::error([
      'status' => 'Inactive',
      'healthy' => FALSE,
    ]);
  }

  return ESResponse::success([
    'status' => 'Active',
    'healthy' => TRUE,
  ]);
}