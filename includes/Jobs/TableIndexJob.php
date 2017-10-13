<?php

class TableIndexJob extends ESJob {

  /**
   * Job type to show in progress report.
   *
   * @var string
   */
  public $type = 'Table Index';

  /**
   * Table to index.
   *
   * @var string
   */
  protected $table;

  /**
   * Total number of records this job is handling.
   * Used to report progress.
   *
   * @var int
   */
  protected $total;

  /**
   * ES index name to insert data into.
   *
   * @var string
   */
  protected $index;

  /**
   * Columns in specified table to index.
   *
   * @var array
   */
  protected $fields = [];

  /**
   * TableIndexJob constructor.
   *
   * @param $index
   * @param string $table
   * @param array $fields
   *
   * @see fields
   *
   */
  public function __construct($index, $table, $fields) {
    $this->index = $index;
    $this->type = ucwords($index);
    $this->table = $table;
    $this->fields = $fields;
  }

  /**
   * Run the indexing process.
   */
  public function handle() {
    $records = $this->get();
    $es = new ESInstance();
    $this->total = count($records);

    if ($this->total > 1) {
      $es->bulkIndex($this->index, $records, $this->table);
    }
    elseif ($this->total > 0) {
      $es->createEntry($this->index, $this->table, FALSE, $records[0]);
    }

    $sql = "
    SELECT F.uniquename,
           F.feature_id,
           BLAST.hit_description,
           CVT.cvterm_id
            FROM chado.feature F
            FULL OUTER JOIN chado.blast_hit_data BLAST ON F.feature_id = BLAST.feature_id
            FULL OUTER JOIN chado.feature_cvterm CVT ON F.feature_id = CVT.feature_id
            WHERE BLAST.hit_description IS NOT NULL OR CVT.cvterm_id IS NOT NULL
            ";
  }

  /**
   * Get records to index.
   *
   * @return array
   */
  protected function get() {
    if ($this->limit === NULL) {
      $this->limit(1);
    }

    if ($this->offset === NULL) {
      $this->offset(0);
    }

    $select_fields = implode(',', $this->fields);
    $query = "SELECT {$select_fields} FROM {{$this->table}} ORDER BY {$this->fields[0]} ASC OFFSET :offset LIMIT :limit";

    return db_query($query, [
      ':offset' => $this->offset,
      ':limit' => $this->limit,
    ])->fetchAll();
  }

  /**
   * Total number of records this job is handling.
   * Used to report progress.
   *
   * @return int
   */
  public function total() {
    return $this->total;
  }

  /**
   * Tells the dispatcher how many records the table has.
   *
   * @return int
   */
  public function count() {
    return db_query('SELECT COUNT(*) FROM {' . $this->table . '}')->fetchField();
  }
}