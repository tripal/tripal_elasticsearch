<?php

class EntitiesIndexJob extends ESJob {

  /**
   * Job type to show in progress report.
   *
   * @var string
   */
  public $type = 'Tripal 3 Entities';

  /**
   * Index name.
   *
   * @var string
   */
  public $index = 'entities';

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
  public $chunk = 10;

  /**
   * Constructor.
   *
   * @param int $entity_id Provide a specific entity id to index a single
   *   entity.
   */
  public function __construct($entity_id = NULL) {
    $this->id = $entity_id;
  }

  /**
   * Job handler.
   * Bulk index all entries if there are more than one.
   */
  public function handle() {
    $es = new ESInstance();
    $records = $this->get();
    $records = $this->loadContent($records);

    if ($this->total > 1) {
      $es->bulkIndex($this->index, $records, $this->index, 'entity_id');
    }
    else {
      if ($this->total > 0) {
        $es->createEntry($this->index, $this->index, $records[0]->entity_id, $records[0]);
      }
    }
  }

  /**
   * Load entity content.
   *
   * @param $records
   *
   * @return array
   */
  protected function loadContent($records) {
    $all = [];
    $this->total = 0;

    $ids = array_map(function ($record) {
      return $record->entity_id;
    }, $records);
    $entities = tripal_load_entity('TripalEntity', $ids);

    foreach ($records as $record) {
      $this->total++;

      if (!isset($entities[$record->entity_id])) {
        continue;
      }

      $entity = $entities[$record->entity_id];
      $content = [];
      if (tripal_entity_access('view', $entity)) {
        $fields = field_info_instances($entity->type, $entity->bundle);
        foreach ($fields as $field => $value) {
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
 * Extract the value of each field.
 *
 * @param $element
 *
 * @return array
 */
protected
function extractValue($element) {
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
protected
function flatten($array, &$items) {
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
protected
function get() {
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
protected
function getMultipleEntities() {
  $query = 'SELECT tripal_entity.id AS entity_id, title, label AS bundle_label
              FROM tripal_entity
              JOIN tripal_bundle ON tripal_entity.term_id = tripal_bundle.term_id
              WHERE status=1
              ORDER BY tripal_entity.id DESC
              OFFSET :offset
              LIMIT :limit';

  return db_query($query, [
    ':limit' => $this->limit,
    ':offset' => $this->offset,
  ])->fetchAll();
}

/**
 * Get a single entity record from the DB.
 *
 * @return array
 */
protected
function getSingleEntity() {
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
public
function total() {
  return $this->total;
}

/**
 * Count the total number of available entities.
 * Used for progress reporting by the DispatcherJob.
 *
 * @return int
 */
public
function count() {
  return db_query('SELECT COUNT(id) FROM {tripal_entity} WHERE status=1')->fetchField();
}
}