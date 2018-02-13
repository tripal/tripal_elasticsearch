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
  public $chunk = 1000;

  /**
   * Total number of indexed records.
   *
   * @var int
   */
  protected $total;

  /**
   * Tripal version.
   * Specify 2 or 3
   *
   * @var int
   */
  protected $tripal_version;

  /**
   * Bundle table.
   * Will equal "chado_feature" if tripal 2.
   *
   * @var string
   */
  protected $bundle_table;

  /**
   * GeneSearchIndexJob constructor.
   *
   * @param string Bundle table (if tripal 2, use NULL or chado_feature).
   * @param int $tripal_version Version of tripal to index.
   */
  public function __construct($bundle_table, $tripal_version = 3) {
    $this->bundle_table = $bundle_table !== NULL ? $bundle_table : 'chado_feature';
    $this->tripal_version = $tripal_version;
    $this->type = 'Gene Search Index. Bundle: ' . $bundle_table;
  }

  /**
   * Execute the indexing job.
   *
   * @throws \Exception
   */
  public function handle() {
    $records = $this->get();

    $es = new ESInstance();

    // Can't use bulk indexing since we are using array data
    // type (ES error not our fault)
    foreach ($records as $record) {
      $es->createEntry($this->index, $this->table, $record->feature_id, $record);
    }
  }

  /**
   * Get all records to index.
   *
   * @return mixed
   */
  protected function get() {
    $records = db_query($this->getQuery(), [
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
   * Get features depending on the tripal version fron tripal_entity or
   * chado_organism.
   *
   * @return array
   */
  protected function getQuery() {
    if ($this->tripal_version === 3) {
      return $this->tripal3FeaturesQuery();
    }

    return $this->tripal3FeaturesQuery();
  }

  /**
   * Tripal 3 features query.
   *
   * @return string
   */
  protected function tripal3FeaturesQuery() {
    return 'SELECT BT.entity_id as entity_id,
                   F.uniquename,
                   F.feature_id,
                   F.seqlen AS sequence_length,
                   F.residues AS sequence,
                   CV.name AS type,
                   O.genus AS organism_genus,
                   O.species AS organism_species,
                   O.common_name AS organism_common_name
                FROM ' . db_escape_table($this->bundle_table) . ' BT
                INNER JOIN chado.feature F ON BT.record_id = F.feature_id
                INNER JOIN chado.organism O ON F.organism_id = O.organism_id
                INNER JOIN chado.cvterm CV ON F.type_id = CV.cvterm_id 
                INNER JOIN tripal_entity TE ON BT.entity_id = TE.id
                WHERE TE.status = 1 AND BT.mapping_id IN (
                  SELECT mapping_id FROM ' . db_escape_table($this->bundle_table) . '
                  ORDER BY entity_id ASC
                  OFFSET :offset LIMIT :limit
                )';
  }

  /**
   * Tripal 2 features query.
   *
   * @return string
   */
  protected function tripal2FeaturesQuery() {
    return 'SELECT CF.nid as node_id,
                   F.uniquename,
                   F.feature_id,
                   F.seqlen AS sequence_length,
                   F.residues AS sequence,
                   CV.name AS type,
                   O.genus AS organism_genus,
                   O.species AS organism_species,
                   O.common_name AS organism_common_name
                FROM chado_feature CF
                INNER JOIN chado.organism O ON F.organism_id = O.organism_id
                INNER JOIN chado.cvterm CV ON F.type_id = CV.cvterm_id 
                INNER JOIN chado.feature F ON CF.feature_id = F.feature_id
                INNER JOIN node ON node.nid = CF.nid
                WHERE node.status = 1
                ORDER BY node.nid ASC OFFSET :offset LIMIT :limit';
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
    // $urls = $this->loadUrlPaths($primary_keys);

    // Attach data to records
    foreach ($records as $key => $record) {
      // Get only features that have annotations or blast hit descriptions
      if (!isset($annotations[$record->feature_id]) && !isset($blast_results[$record->feature_id])) {
        unset($records[$key]);
        continue;
      }

      // Remove any features that we can't link to
      //      if (!isset($urls[$record->feature_id]) || empty($urls[$record->feature_id])) {
      //        unset($records[$key]);
      //        continue;
      //      }

      $records[$key]->annotations = isset($annotations[$record->feature_id]) ? $annotations[$record->feature_id] : '';
      $records[$key]->blast_hit_descriptions = isset($blast_results[$record->feature_id]) ? $blast_results[$record->feature_id] : '';
      $records[$key]->url = $this->tripal_version === 3 ? "bio_data/{$record->entity_id}" : "node/{$record->node_id}";
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
    if (!db_table_exists('chado.blast_hit_data')) {
      return [];
    }

    $records = db_query('SELECT feature_id, hit_description, hit_accession 
                          FROM chado.blast_hit_data 
                          WHERE feature_id IN (:keys)', [':keys' => $keys])->fetchAll();

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
    $query = "SELECT db.name AS db_name,
                     dbxref.accession AS accession,
                     cv.name AS cv_name,
                     feature_id AS feature_id,
                     cvterm.definition AS definition
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
        $record->definition,
      ];
    }

    return $indexed;
  }

  /**
   * Load urls of each feature's node or entity.
   *
   * @param $keys
   *
   * @deprecated not completely ignore by the indexer now
   * @return array
   */
  protected function loadUrlPaths($keys) {
    $indexed = [];

    $chado_feature = db_table_exists('chado_feature');

    foreach ($keys as $id) {
      $url = NULL;
      if (function_exists('chado_get_record_entity_by_table')) {
        $eid = chado_get_record_entity_by_table('feature', $id);
        if ($eid !== NULL) {
          $url = 'bio_data/' . $eid;
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
    if ($this->tripal_version === 3) {
      return intval(db_query('SELECT COUNT(entity_id) FROM ' . db_escape_table($this->bundle_table) . ' CB
               INNER JOIN tripal_entity TE ON TE.id = CB.entity_id
               WHERE TE.status = 1')->fetchField());
    }

    return intval(db_query('SELECT COUNT(nid) FROM {chado_feature} CF
                     INNER JOIN node N ON N.nid = CF.nid
                     WHERE N.status = 1')->fetchField());
  }

  /**
   * A method to quickly create and dispatch indexing jobs.
   *
   * @param int $round Priority round (1 or 2)
   */
  public static function generateDispatcherJobs() {
    if (db_table_exists('chado_bundle')) {
      // Foreach bundle type, create a dispatcher job.
      $bundles = db_query('SELECT bundle_id FROM {chado_bundle} WHERE data_table = :data_table', [
        ':data_table' => 'feature',
      ])->fetchAll();

      foreach ($bundles as $bundle) {
        $bundle = "chado_bio_data_{$bundle->bundle_id}";

        $job = new GeneSearchIndexJob($bundle, 3);
        $dispatcher = new DispatcherJob($job);
        $dispatcher->dispatch();
      }

      return;
    }

    // Tripal 2 so index chado feature instead
    $job = new GeneSearchIndexJob('chado_feature', 2);
    $dispatcher = new DispatcherJob($job);
    $dispatcher->dispatch();
  }
}
