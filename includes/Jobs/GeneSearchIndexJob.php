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

    $es = new ESInstance();

    // Can't use bulk indexing since we are using array data
    // type (ES error not our fault)
    foreach ($records as $record) {
      $es->createEntry($this->index, $this->table, FALSE, $record);
    }
  }

  /**
   * Get all records to index.
   *
   * @return mixed
   */
  protected function get() {
    $query = 'SELECT F.uniquename,
                     F.feature_id,
                     O.genus AS organism_genus,
                     O.species AS organism_species,
                     O.common_name AS organism_common_name
                FROM chado.feature F
                INNER JOIN chado.organism O ON F.organism_id = O.organism_id
                ORDER BY feature_id ASC OFFSET :offset LIMIT :limit';

    $records = db_query($query, [
      ':offset' => $this->offset,
      ':limit' => $this->limit,
    ])->fetchAll();

    $this->total = count($records);

    if ($this->total > 0) {
      // Eager load all blast hit descriptions and feature annotations
      $this->loadData($records);
    }

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

    // Load urls
    $urls = $this->loadUrlPaths($primary_keys);

    // Attach data to records
    foreach ($records as $key => $record) {
      // Get only features that have annotations or blast hit descriptions
      if (!isset($annotations[$record->feature_id]) && !isset($blast_results[$record->feature_id])) {
        unset($records[$key]);
        continue;
      }

      // Remove any features that we can't link to
      if (!isset($urls[$record->feature_id]) || empty($urls[$record->feature_id])) {
        unset($records[$key]);
        continue;
      }

      $records[$key]->annotations = isset($annotations[$record->feature_id]) ? $annotations[$record->feature_id] : '';
      $records[$key]->blast_hit_descriptions = isset($blast_results[$record->feature_id]) ? $blast_results[$record->feature_id] : '';
      $records[$key]->url = isset($urls[$record->feature_id]) ? $urls[$record->feature_id] : NULL;
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
    $records = db_query('SELECT feature_id, hit_description, hit_accession FROM chado.blast_hit_data WHERE feature_id IN (:keys)', [':keys' => $keys])->fetchAll();

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
   * Load urls of each feature's node or entity.
   *
   * @param $keys
   *
   * @return array
   */
  protected function loadUrlPaths($keys) {
    $indexed = [];

    $chado_feature = db_table_exists('chado_feature');

    foreach ($keys as $id) {
      $url = NULL;
      if (function_exists('tripal_get_chado_entity_id')) {
        $eid = tripal_get_chado_entity_id('feature', $id);
        if ($eid !== NULL) {
          $url = 'bio_data/' . $id;
        }
      }

      if (empty($url) && $chado_feature) {
        $nid = db_query('SELECT nid FROM chado_feature WHERE feature_id=:id', [':id' => $id])->fetchField();
        if ($nid) {
          $url = "node/{$nid}";
        }
      }

      $indexed[$id] = $url;
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