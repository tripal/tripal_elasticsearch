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
  protected $index = 'entities';

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
  public $total;

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
    $this->total = count($records);

    $records = $this->loadContent($records);

    if ($this->total > 1) {
      $es->bulkIndex($this->index, $records);
    }
    else {
      if ($this->total > 0) {
        $es->createEntry($this->index, $this->index, FALSE, $records[0]);
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
    return array_map(function ($record) {
      $entity = entity_load('TripalEntity', [$record->entity_id]);
      $view = entity_view('TripalEntity', $entity, 'full');
      $content = render($view);
      return (object) [
        'entity_id' => $record->entity_id,
        'title' => $record->title,
        'bundle_label' => $record->bundle_label,
        'content' => $this->cleanHTML($content),
      ];
    }, $records);
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
    // remove generated jQuery script
    $page_html = preg_replace('/<script\b[^>]*>.*<\/script>/isU', "", $page_html);
    // remove css stuff
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
   * Get total number of items in job.
   *
   * @return int
   */
  public function total() {
    return $this->total;
  }

  /**
   * Count the number of available entity.
   * Used for progress reporting by the DispatcherJob.
   *
   * @return int
   */
  public function count() {
    return db_query('SELECT COUNT(id) FROM {tripal_entity} WHERE status=1')->fetchField();
  }
}