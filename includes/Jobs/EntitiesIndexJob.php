<?php

class EntitiesIndexJob extends ESJob{

  /**
   * Job type to show in progress report.
   *
   * @var string
   */
  public $type;

  /**
   * Index name.
   *
   * @var string
   */
  public $index = 'entities';

  /**
   * Specify the field priority round.
   *
   * @var int
   */
  public $priority_round = 1;

  /**
   * Tripal Entity bundle name.
   * E.g, bio_data_1, bio_data_2, etc.
   *
   * @var string
   */
  public $bundle;

  /**
   * Entity id.
   *
   * @var int
   */
  protected $id;

  /**
   * Number of records.
   *
   * @var int
   */
  protected $total;

  /**
   * Number of items to bulk index.
   *
   * @var int
   */
  public $chunk = 100;

  /**
   * Elasticsearch instance.
   *
   * @var \ESInstance
   */
  protected $es;

  /**
   * Constructor.
   *
   * @param int $bundle_type Which bundle type to process
   * @param int $entity_id Provide a specific entity id to index a single
   *   entity.
   * @param int $round
   */
  public function __construct($bundle, $entity_id = NULL, $round = 1) {
    $this->id = $entity_id;
    $this->bundle = $bundle;
    $this->type = 'Tripal 3 Entities: ' . $bundle . '. Priority Round: ' . ($round === 1 ? 'High' : 'Low');
    $this->priority_round = $round;
  }

  /**
   * Job handler.
   * Bulk index all entries if there are more than one.
   */
  public function handle() {
    $this->es = new ESInstance();
    $records = $this->get();
    $records = $this->loadContent($records);

    if ($this->total > 1) {
      if ($this->priority_round === 1) {
        $this->es->bulkIndex($this->index, $records, $this->index, 'entity_id');
      }
      else {
        $this->es->bulkUpdate($this->index, $records, $this->index, 'entity_id');
      }
    }
    elseif (count($records) > 0) {
      $this->es->createEntry($this->index, $this->index, $records[0]->entity_id, $records[0]);
    }
  }

  /**
   * Load entity content.
   *
   * @param array $records Entities from tripal_entity table.
   *
   * @return array
   */
  protected function loadContent($records) {
    $all = [];
    $this->total = 0;

    // Load entities and applicable fields
    $ids = array_map(function ($record) {
      return $record->entity_id;
    }, $records);
    $fields = field_info_instances('TripalEntity', $this->bundle);

    // Load priority list
    $priority = $this->getPriorityList($fields);
    $entities = tripal_load_entity('TripalEntity', $ids, FALSE, $priority['ids']);
    foreach ($records as $record) {
      $this->total++;

      if (!isset($entities[$record->entity_id])) {
        continue;
      }

      $entity = $entities[$record->entity_id];
      $content = [];
      if (tripal_entity_access('view', $entity)) {
        foreach ($priority['names'] as $field) {
          if (property_exists($entity, $field) && isset($entity->{$field}['und'])) {
            foreach ($entity->{$field}['und'] as $elements) {
              if (!isset($elements['value'])) {
                continue;
              }

              $value = $this->extractValue($elements['value']);

              if (empty($value)) {
                continue;
              }

              $content[] = $value;
            }
          }
        }
      }

      if (empty($content)) {
        continue;
      }

      $prev_entity = $this->es->getRecord('entities', 'entities', $entity->id);
      if ($prev_entity['found']) {
        $content = array_merge($content, $prev_entity['_source']['content']);
      }

      $all[] = (object) [
        'entity_id' => $entity->id,
        'title' => $entity->title,
        'bundle_label' => $record->bundle_label,
        'content' => $content,
      ];
    }

    return $all;
  }

  /**
   * Get a list of priority settings.
   *
   * @return array
   */
  protected function getPriorityList($fields) {
    if ($this->id !== NULL) {
      return $this->getAllFields($fields);
    }

    return $this->prioritizeFields($fields);
  }

  /**
   * Index all fields.
   *
   * @param array $fields
   *
   * @return array
   */
  protected function getAllFields($fields) {
    $return = [
      'names' => [],
      'ids' => [],
    ];

    foreach ($fields as $field => $data) {
      $return['names'] = $field;
      $return['ids'] = $data['field_id'];
    }

    return $return;
  }

  /**
   * Get a list of fields for the current priority round only.
   *
   * @param array $fields field_
   *
   * @return array
   */
  protected function prioritizeFields($fields) {
    if ($this->priority_round < 2) {
      $results = db_query('SELECT * FROM {tripal_elasticsearch_priority} WHERE priority = :priority', [
        ':priority' => 1,
      ])->fetchAll();
    }
    else {
      $results = db_query('SELECT * FROM {tripal_elasticsearch_priority} WHERE priority != :priority', [
        ':priority' => 1,
      ])->fetchAll();
    }

    $indexed = [];

    foreach ($results as $result) {
      $indexed[$result->field_id] = $result->priority;
    }

    $return = [
      'ids' => [],
      'names' => [],
    ];

    foreach ($fields as $field => $data) {
      $id = $data['field_id'];
      if (isset($indexed[$id])) {
        if ($indexed[$id] > 0) {
          $return['names'][] = $field;
          $return['ids'][] = $id;
        }
      }
      elseif ($this->priority_round === 2) {
        // Assume the field is new and a priority setting has not yet been
        // saved for it so automatically consider it low priority and add
        // it to the list.
        $return['names'][] = $field;
        $return['ids'][] = $id;
      }
    }

    return $return;
  }

  /**
   * Extract the value of each field.
   *
   * @param $element
   *
   * @return array
   */
  protected function extractValue($element) {
    $items = [];
    $this->flatten($element, $items);

    // Remove repeated elements
    return array_unique($items);
  }

  /**
   * Recursively flattens the field's value.
   *
   * @param $array
   * @param $items
   */
  protected function flatten($array, &$items) {
    if (is_scalar($array)) {
      $value = stripslashes(trim(strip_tags($array)));
      if (!empty($value)) {
        $items[] = $value;
      }
      return;
    }

    if (is_array($array)) {
      foreach ($array as $b) {
        $this->flatten($b, $items);
      }
    }
  }

  /**
   * Get records to index.
   *
   * @return mixed
   * @throws \Exception
   */
  protected function get() {
    if ($this->id !== NULL) {
      return $this->getSingleEntity();
    }

    if ($this->limit === NULL || $this->offset === NULL) {
      throw new Exception('EntitiesIndexJob: Limit and offset parameters are required if node id is not provided in the constructor.');
    }

    return $this->getMultipleEntities();
  }

  /**
   * Process multiple entities from the DB.
   *
   * @return array
   */
  protected function getMultipleEntities() {
    $query = 'SELECT tripal_entity.id AS entity_id, title, label AS bundle_label
              FROM tripal_entity
              JOIN tripal_bundle ON tripal_entity.term_id = tripal_bundle.term_id
              WHERE status=1 AND bundle=:bundle
              ORDER BY tripal_entity.id DESC
              OFFSET :offset
              LIMIT :limit';

    return db_query($query, [
      ':bundle' => $this->bundle,
      ':limit' => $this->limit,
      ':offset' => $this->offset,
    ])->fetchAll();
  }

  /**
   * Get a single entity record from the DB.
   *
   * @return array
   */
  protected function getSingleEntity() {
    $query = 'SELECT tripal_entity.id AS entity_id, title, label AS bundle_label
              FROM tripal_entity
              JOIN tripal_bundle ON tripal_entity.term_id = tripal_bundle.term_id
              WHERE status=1 AND tripal_entity.id = :id
              ORDER BY  tripal_entity.id DESC';

    return db_query($query, [':id' => $this->id])->fetchAll();
  }

  /**
   * Get total number of items in a job.
   *
   * @return int
   */
  public function total() {
    return $this->total;
  }

  /**
   * Count the total number of available entities.
   * Used for progress reporting by the DispatcherJob.
   *
   * @return int
   */
  public function count() {
    return db_query('SELECT COUNT(id) FROM {tripal_entity} WHERE status=1 AND bundle=:bundle', [':bundle' => $this->bundle])->fetchField();
  }

  /**
   * A method to quickly create and dispatch indexing jobs.
   *
   * @param int $round Priority round (1 or 2)
   */
  public static function generateDispatcherJobs($round = 1) {
    // Foreach bundle type, create a dispatcher job.
    $bundles = db_query('SELECT name FROM {tripal_bundle}')->fetchAll();
    foreach ($bundles as $bundle) {
      $job = new EntitiesIndexJob($bundle->name, NULL, $round === 1 ? 1 : 2);
      $dispatcher = new DispatcherJob($job);
      $dispatcher->dispatch();
    }
  }

  /**
   * Tells the ESQueue class whether this job implements
   * priority queues.
   *
   * @return bool
   */
  public function hasRounds() {
    return TRUE;
  }

  /**
   * Inform progress tracker of current round.
   *
   * @return int
   */
  public function currentRound() {
    return $this->priority_round === 1 ? 1 : 2;
  }

  /**
   * Creates the next priority round.
   *
   * @return bool
   */
  public function createNextRound() {
    if ($this->priority_round >= 2) {
      return FALSE;
    }

    $job = new EntitiesIndexJob($this->bundle, NULL, $this->priority_round + 1);
    $dispatcher = new DispatcherJob($job);
    $dispatcher->dispatch();
    return TRUE;
  }
}