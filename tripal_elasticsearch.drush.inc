<?php

/**
 * Implements hook_drush_command().
 */
function tripal_elasticsearch_drush_command() {

  $commands['generate-fields-cache'] = [
    'description' => 'This is my example command.',
    'aliases' => ['dfc'],
    'arguments' => [],
    'options' => [],
    'examples' => [
      'drush dfc' => 'Loads all tripal entities.',
    ],
  ];

  return $commands;
}

/**
 * Drush command logic.
 * drush_[COMMAND_NAME]().
 */
function drush_tripal_elasticsearch_generate_fields_cache() {
  error_reporting(0);
  print "Generating fields cache\n";
  $offset = 0;
  $time = time();
  $total = db_query('select count(id) from tripal_entity')->fetchField();
  while ($records = db_query('select id from tripal_entity order by id asc offset :offset limit 50', [':offset' => $offset])->fetchAll()) {
    $ids = [];
    foreach ($records as $record) {
      $ids[] = $record->id;
    }

    tripal_load_entity('TripalEntity', $ids);

    $progress = $offset === 0 ? '0%' : (($offset / $total) * 100) . '%';
    $elapsed = time() - $time;
    $unit = 'seconds';
    if ($elapsed > 60) {
      $elapsed /= 60;
      $unit = 'minutes';

      if ($elapsed > 60) {
        $elapsed /= 60;
        $unit = 'hours';

        if ($elapsed > 24) {
          $elapsed /= 24;
          $unit = 'days';
        }
      }
    }

    print $progress . " Time elapsed: $elapsed $unit\n";
    $offset += 50;
  }

  print "Fields cache for entities has been reset.\n";
}