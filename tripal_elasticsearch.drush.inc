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
    $table = 'chado_' . $bundle->name;
  }

  $memory_usage = [];

   for ($i = 193845; $i < (193845 + 700); $i += 100) {
//  for ($i = 0; $i < (100); $i += 100) {
    $records = db_query("SELECT * FROM {" . $table . "} ORDER BY entity_id DESC OFFSET $i LIMIT 100")->fetchAll();
    $entity_ids = array_map(function ($record) {
      return $record->entity_id;
    }, $records);

    $memory_usage[$i] = [];
    $mem = memory_get_usage();
    $memory_usage[$i][] = $mem;
    print "Before load memory Usage: " . number_format($mem) . " bytes\n";
    print "Loading entities SELECT * FROM {" . $table . "} ORDER BY entity_id DESC OFFSET $i LIMIT 100\n";
    $entities = tripal_load_entity('TripalEntity', $entity_ids);
    $mem = memory_get_usage();
    $memory_usage[$i][] = $mem;
    print "After load memory Usage: " . number_format($mem) . " bytes\n";

    print "\n ============ \n\n";
  }

  print "Test report:\n";
  $mask = "|%-15s |%-25s |\n";
  printf($mask, 'OFFSET', 'MEMORY');
  foreach ($memory_usage as $offset => $row) {
    printf($mask, $offset,  number_format((($row[1] - $row[0]) / 1024) / 1024). ' MB');
  }
}