<?php

class GeneSearchIndexJob extends ESJob{

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
  public $chunk = 500;

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
   * To index a single entity.
   *
   * @var int
   */
  protected $entity_id = NULL;

  /**
   * GeneSearchIndexJob constructor.
   *
   * @param string Bundle table (if tripal 2, use NULL or chado_feature).
   * @param int $tripal_version Version of tripal to index.
   */
  public function __construct($bundle_table, $tripal_version = 3, $entity_id = NULL) {
    $this->bundle_table = $bundle_table !== NULL ? $bundle_table : 'chado_feature';
    $this->tripal_version = $tripal_version;
    $this->type = 'Gene Search Index. Bundle: ' . $bundle_table;
    $this->entity_id = $entity_id;
  }

  /**
   * Execute the indexing job.
   *
   * @throws \Exception
   */
  public function handle() {
    if (!is_null($this->entity_id)) {
      $records = $this->getSingleEntity();
    }
    else {
      $records = $this->get();
    }

    $this->total = count($records);

    try {
      $es = new ESInstance();

      // Can't use bulk indexing since we are using array data type (ES error not our fault)
      foreach ($records as $record) {
        $es->createOrUpdate(
          $this->index,
          $record->feature_id,
          $record
        );
      }
    } catch (Exception $exception) {
      echo "ERROR: " . $exception->getMessage() . "\n";
      tripal_report_error(
        'tripal_elasticsearch',
        TRIPAL_ERROR,
        $exception->getMessage()
      );
    }
  }

  /**
   * Get all records to index.
   *
   * @return mixed
   */
  protected function get() {
    $records = db_query(
      $this->getQuery(),
      [
        ':offset' => $this->offset,
        ':limit' => $this->limit,
      ]
    )->fetchAll();

    if (!empty($records)) {
      // Eager load all blast hit descriptions and feature annotations
      $this->loadData($records);
    }

    return $records;
  }

  /**
   * @return mixed
   * @throws \Exception
   */
  protected function getSingleEntity() {
    if (is_null($this->entity_id)) {
      throw new Exception(
        'Please provide a valid entity id in GeneSearchIndexer'
      );
    }

    $records = db_query(
      'SELECT BT.entity_id as entity_id,
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
                WHERE TE.status = 1 AND BT.entity_id=:entity_id',
      [
        ':entity_id' => $this->entity_id,
      ]
    )->fetchAll();

    if (!empty($records)) {
      // Eager load all blast hit descriptions and feature annotations
      $this->loadData($records);
    }

    return $records;
  }

  /**
   * Get features depending on the tripal version fron tripal_entity or
   * chado_organism.
   *
   * @return string
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
   * @param int $entity_id Entity id
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
                  SELECT mapping_id FROM {' . db_escape_table(
        $this->bundle_table
      ) . '}
                  ORDER BY entity_id ASC
                  OFFSET :offset
                  LIMIT :limit
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
                FROM {chado_feature} CF
                INNER JOIN chado.organism O ON F.organism_id = O.organism_id
                INNER JOIN chado.cvterm CV ON F.type_id = CV.cvterm_id
                INNER JOIN chado.feature F ON CF.feature_id = F.feature_id
                INNER JOIN node ON node.nid = CF.nid
                WHERE node.status = 1 AND CF.nid IN (
                    SELECT nid FROM {chado_feature}
                    ORDER BY nid ASC
                    OFFSET :offset LIMIT :limit;
                )';
  }

  /**
   * Load blast data and annotations into feature records.
   *
   * @param $records
   */
  protected function loadData(&$records) {
    // Get all ids
    $primary_keys = array_map(
      function ($record) {
        return $record->feature_id;
      },
      $records
    );

    // Load blast data
    $blast_results = $this->loadBlastData($primary_keys);

    // Load annotations
    $annotations = $this->loadAnnotations($primary_keys);

    // Load urls
    // $urls = $this->loadUrlPaths($primary_keys);

    // Load feature relationships
    $related = $this->loadRelatedFeatures($primary_keys);

    // Attach data to records
    foreach ($records as $key => $record) {
      // Get only features that have annotations or blast hit descriptions
      //      if (!isset($annotations[$record->feature_id]) && !isset($blast_results[$record->feature_id])) {
      //        unset($records[$key]);
      //        continue;
      //      }

      $records[$key]->annotations = isset($annotations[$record->feature_id]) ? $annotations[$record->feature_id] : '';
      $records[$key]->blast_hit_descriptions = isset($blast_results[$record->feature_id]) ? $blast_results[$record->feature_id] : '';
      $records[$key]->url = $this->tripal_version === 3 ? "bio_data/{$record->entity_id}" : "node/{$record->node_id}";
      $records[$key]->related_features = isset($related[$record->feature_id]) ? $related[$record->feature_id] : [];
    }
  }

  /**
   * Add related features to the index.
   *
   * @param array $keys
   *
   * @return array
   */
  protected function loadRelatedFeatures($keys) {
    // Get feature bundles
    $records = db_query(
      'SELECT object_id,
                                subject_id,
                                f1.uniquename as object_uniquename,
                                f2.uniquename as subject_uniquename
                        FROM chado.feature_relationship fr
                        INNER JOIN chado.feature f1 ON f1.feature_id = fr.object_id
                        INNER JOIN chado.feature f2 ON f2.feature_id = fr.subject_id
                        WHERE fr.object_id IN (:keys_1) OR fr.subject_id IN (:keys_2)',
      [
        ':keys_1' => $keys,
        ':keys_2' => $keys,
      ]
    );

    $indexed = [];
    foreach ($records as $record) {
      $indexed[$record->object_id][] = $record->subject_uniquename;
      $indexed[$record->subject_id][] = $record->object_uniquename;
    }

    return $indexed;
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

    $records = db_query(
      'SELECT feature_id, hit_description, hit_accession
                          FROM {chado.blast_hit_data}
                          WHERE feature_id IN (:keys)',
      [':keys' => $keys]
    )->fetchAll();

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
              FROM {chado.dbxref}
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
      return intval(
        db_query(
          'SELECT COUNT(entity_id) FROM ' . db_escape_table(
            $this->bundle_table
          ) . ' CB
               INNER JOIN tripal_entity TE ON TE.id = CB.entity_id
               WHERE TE.status = 1'
        )->fetchField()
      );
    }

    return intval(
      db_query(
        'SELECT COUNT(nid) FROM {chado_feature} CF
                     INNER JOIN node N ON N.nid = CF.nid
                     WHERE N.status = 1'
      )->fetchField()
    );
  }

  /**
   * A method to quickly create and dispatch indexing jobs.
   *
   * @param boolean $clear_queue Whether to remove old jobs before
   *                              submitting new ones
   */
  public static function generateDispatcherJobs(bool $clear_queue = FALSE, int $chunk_size = 500) {
    // Clear all entries from the queue
    if ($clear_queue) {
      // Clear all entries from the queue
      $sql = 'SELECT item_id, data FROM queue q WHERE name LIKE :name';
      $results = db_query(
        $sql,
        [':name' => db_like('elasticsearch') . '%']
      )->fetchAll();
      $delete = [];

      foreach ($results as $result) {
        $class = unserialize($result->data);
        if ($class instanceof DispatcherJob && $class->job() instanceof EntitiesIndexJob) {
          $delete[] = $result->item_id;
        }
        elseif ($class instanceof EntitiesIndexJob) {
          $delete[] = $result->item_id;
        }
      }
    }

    if (db_table_exists('chado_bundle')) {
      // Foreach bundle type, create a dispatcher job.
      $bundles = db_query(
        'SELECT bundle_id FROM {chado_bundle} WHERE data_table = :data_table',
        [
          ':data_table' => 'feature',
        ]
      )->fetchAll();

      foreach ($bundles as $bundle) {
        $bundle = "chado_bio_data_{$bundle->bundle_id}";

        $job = new GeneSearchIndexJob($bundle, 3);
        $job->chunk = $chunk_size;
        $dispatcher = new DispatcherJob($job);
        $dispatcher->dispatch();
      }

      return;
    }

    // Tripal 2 so index chado feature instead
    $job = new GeneSearchIndexJob('chado_feature', 2);
    $job->chunk = 500;
    $dispatcher = new DispatcherJob($job);
    $dispatcher->dispatch();
  }
}
