<?php

/**
 * Implements hook_drush_command().
 */
function tripal_elasticsearch_drush_command() {
  $commands['es-run-tests'] = [
    'description' => 'DO NOT RUN THIS',
    'arguments' => [],
    'options' => [],
    'examples' => [
      'drush es-run-tests' => '',
    ],
  ];

  return $commands;
}

/**
 * This is a test function to figure out where memory leak errors are happening.
 */
function drush_tripal_elasticsearch_es_run_tests() {
  print "Running Tests: \n";
  print "Checking memory load from features: \n";

  $bundles = db_query("SELECT * FROM {tripal_bundle} WHERE label IN ('mRNA', 'mRNA/polypeptide')")->fetchAll();
  if (!count($bundles)) {
    print "[ERROR] Could not find the bundle for mRNA\n";
    exit(1);
  }

  $table = '';
  foreach ($bundles as $bundle) {
    $table = $bundle->name;
  }

  $memory_usage = [];

  $max = db_query("SELECT COUNT(*)
              FROM tripal_entity
              JOIN tripal_bundle ON tripal_entity.term_id = tripal_bundle.term_id
              WHERE status=1 AND bundle=:bundle", [':bundle' => $table])->fetchField();

  for ($i = 0; $i < $max; $i += 100) {
    $records = db_query("SELECT tripal_entity.id AS entity_id, title, tripal_bundle.name AS bundle_label
              FROM tripal_entity
              JOIN tripal_bundle ON tripal_entity.term_id = tripal_bundle.term_id
              WHERE status=1 AND bundle=:bundle
              ORDER BY tripal_entity.id DESC
              OFFSET :offset
              LIMIT 100", [':bundle' => $table, ':offset' => $i])->fetchAll();
    $entity_ids = array_map(function ($record) {
      return $record->entity_id;
    }, $records);

    $memory_usage[$i] = [];
    $mem = memory_get_usage();
    $memory_usage[$i][] = $mem;
    print "Before load memory Usage: " . number_format($mem) . " bytes\n";
    print "Loading entities SELECT * FROM {" . $table . "} ORDER BY entity_id DESC OFFSET $i LIMIT 100\n";
    $entities = tripal_load_entity('TripalEntity', $entity_ids, TRUE);
    $mem = memory_get_usage();
    $memory_usage[$i][] = $mem;
    print "After load memory Usage: " . number_format($mem) . " bytes\n";

    $entities = NULL;

    print "\n ============ \n\n";
  }

  print "Test report:\n";
  $mask = "|%-15s |%-25s |\n";
  printf($mask, 'OFFSET', 'MEMORY');
  foreach ($memory_usage as $offset => $row) {
    printf($mask, $offset, number_format((($row[1] - $row[0]) / 1024) / 1024) . ' MB');
  }
}