<?php

class GeneSearchIndexJob extends ESJob {

  /**
   * Index name.
   *
   * @var string
   */
  public $index = 'gene_search_index';

  /**
   * Progress report type.
   *
   * @var string
   */
  public $type = 'Gene Search';

  /**
   * Table name.
   *
   * @var string
   */
  public $table = 'chado.feature';

  /**
   * Bulk indexing size limit.
   *
   * @var int
   */
  public $chunk = 100;

  /**
   * Total number of indexed records.
   *
   * @var int
   */
  protected $total;

  /**
   * GeneSearchIndexJob constructor.
   *
   * @param string $index Name of index to add data to.
   */
  public function __construct($index = NULL) {
    if ($index) {
      $this->index = $index;
    }
  }

  /**
   * Execute the indexing job.
   */
  public function handle() {
    $records = $this->get();
    $this->total = count($records);

    $es = new ESInstance();
    if ($this->total > 1) {
      $es->bulkIndex($this->index, $records);
    }
    elseif ($this->total > 0) {
      $es->createEntry($this->index, $this->table, FALSE, $records[0]);
    }
  }

  /**
   * Get all records to index.
   *
   * @return mixed
   */
  protected function get() {
    $records = db_query('SELECT uniquename, feature_id FROM chado.feature ORDER BY feature_id ASC OFFSET :offset LIMIT :limit', [
      ':offset' => $this->offset,
      ':limit' => $this->limit,
    ])->fetchAll();

    // Eager load all blast hit descriptions and feature annotations
    $this->loadData($records);

    return $records;
  }

  /**
   * Load blast data and annotations into feature records.
   *
   * @param $records
   */
  protected function loadData(&$records) {
    // Get all ids
    $primary_keys = array_map(function ($record) {
      return $record->feature_id;
    }, $records);

    // Load blast data
    $blast_results = $this->loadBlastData($primary_keys);

    // Load annotations
    $annotations = $this->loadAnnotations($primary_keys);

    // Attach data to records
    foreach ($records as $key => $record) {
      $records[$key]->annotations = isset($annotations[$record->feature_id]) ? $annotations[$record->feature_id] : [];
      $records[$key]->blast_hit = isset($blast_results[$record->feature_id]) ? $blast_results[$record->feature_id] : [];
    }
  }

  /**
   * Load blast records for a given set of feature ids.
   *
   * @param array $keys Feature ids
   *
   * @return array
   */
  protected function loadBlastData($keys) {
    $records = db_query('SELECT hit_description, hit_accession FROM chado.blast_hit_data WHERE feature_id IN (:keys)', [':keys' => $keys])->fetchAll();

    $indexed = [];
    foreach ($records as $record) {
      $indexed[$record->feature_id][] = [
        $record->hit_description,
        $record->hit_accession,
      ];
    }

    return $indexed;
  }

  /**
   * Load annotations for a given set of feature ids.
   *
   * @param array $keys Feature ids
   *
   * @return array
   */
  protected function loadAnnotations($keys) {
    $query = "SELECT db.name AS db_name, dbxref.accession, cv.name AS cv_name, feature_id 
              FROM chado.dbxref
              INNER JOIN chado.cvterm ON dbxref.dbxref_id = cvterm.dbxref_id
              INNER JOIN chado.feature_cvterm ON cvterm.cvterm_id = feature_cvterm.cvterm_id
              INNER JOIN chado.db ON dbxref.db_id = db.db_id
              INNER JOIN chado.cv ON cvterm.cv_id = cv.cv_id
              WHERE feature_id IN (:keys)";
    $records = db_query($query, [':keys' => $keys]);

    $indexed = [];
    foreach ($records as $record) {
      $indexed[$record->feature_id][] = [
        $record->db_name,
        $record->cv_name,
        $record->accession,
      ];
    }

    return $indexed;
  }

  /**
   * Total number of indexed records.
   *
   * @return int
   */
  public function total() {
    return $this->total;
  }

  /**
   * Get the count of all features for the dispatcher.
   *
   * @return int
   */
  public function count() {
    return db_query('SELECT COUNT(feature_id) FROM chado.feature')->fetchField();
  }
}