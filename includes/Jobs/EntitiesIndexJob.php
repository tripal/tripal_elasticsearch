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
  public $chunk = 100;

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

    // TODO: use tripal_get_field_types to get fields and check if
    // TODO: the index() property exists to use it
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
    global $base_url;
    $url = variable_get('es_base_url', $base_url);
    $all = [];
    $this->total = 0;

    foreach ($records as $record) {
      $this->total++;

      $content = @file_get_contents("{$url}/bio_data/{$record->entity_id}");

      if ($content === FALSE) {
        continue;
      }

      $all[] = (object) [
        'entity_id' => $record->entity_id,
        'title' => $record->title,
        'bundle_label' => $record->bundle_label,
        'content' => $this->cleanHTML($content),
      ];
    }
    return $all;
  }

  /**
   * Clean up html content.
   *
   * @param $content
   *
   * @return string
   */
  protected function cleanHTML($content) {
    $pattern_1 = preg_quote('<pre class="tripal_feature-sequence">') . ".*" . preg_quote('</pre>');
    $page_html = preg_replace("!" . $pattern_1 . "!sU", ' ', $content);
    // remove query sequences
    $pattern_2 = preg_quote('<pre>Query') . ".*" . preg_quote('</pre>');
    $page_html = preg_replace("!" . $pattern_2 . "!sU", ' ', $page_html);
    // remove blast alignments if tripal_analysis_blast is installed
    $pattern_3 = preg_quote('<pre class="blast_align">') . ".*" . preg_quote('</pre>');
    $page_html = preg_replace("!" . $pattern_3 . "!sU", ' ', $page_html);
    // add one space to html tags to avoid words concatenated after stripping html tags
    $page_html = str_replace('<', ' <', $page_html);
    // Remove the head tag and its contents
    $page_html = preg_replace('/<head\b[^>]*>.*<\/head>/isU', "", $page_html);
    // remove scripts
    $page_html = preg_replace('/<script\b[^>]*>.*<\/script>/isU', "", $page_html);
    // remove css styles
    $page_html = preg_replace('/<style\b[^>]*>.*<\/style>/isU', "", $page_html);

    return strip_tags($page_html);
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
    return db_query('SELECT COUNT(id) FROM {tripal_entity} WHERE status=1')->fetchField();
  }
}