<?php

class NodesIndexJob extends ESJob{

  /**
   * Job type to show in progress report.
   *
   * @var string
   */
  public $type = 'Website Nodes';

  /**
   * Index name.
   *
   * @var string
   */
  public $index = 'website';

  /**
   * Node id.
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
  public $chunk = 50;

  /**
   * NodesIndexJob constructor.
   *
   * @param int $node_id
   */
  public function __construct($node_id = NULL) {
    $this->id = $node_id;
  }

  /**
   * Job handler.
   * Bulk index all entries if there are more than one.
   *
   * @throws \Exception
   */
  public function handle() {
    $es = new ESInstance();
    $records = $this->get();

    $this->total = count($records);

    if ($this->total > 1) {
      $es->bulkIndex($this->index, $this->loadContent($records), 'nid');
    }
    elseif ($this->total > 0) {
      $node = $this->loadContent($records);
      if (empty($node)) {
        return;
      }

      $record = end($node);

      $es->createEntry($this->index, $record->nid, $record);
    }
  }

  /**
   * Load cleaned up content of nodes and attach it to record.
   *
   * @param array $records
   *
   * @return array
   */
  protected function loadContent($records) {
    $all = [];

    $nids = array_map(function ($record) {
      return $record->nid;
    }, $records);

    $nodes = node_load_multiple($nids);

    foreach ($records as $record) {
      if (!isset($nodes[$record->nid])) {
        continue;
      }

      $node = $nodes[$record->nid];

      if (!node_access('view', $node)) {
        // Anonymous user is not allowed to access this node
        // so don't index it
        continue;
      }

      $modules = module_implements('node_view');
      foreach ($modules as $module) {
        module_invoke($module, 'node_view', $node, 'full', NULL);
      }

      $view = node_view($node, 'full');
      $content = render($view);

      if (empty($content)) {
        continue;
      }

      // Ignore nodes with empty titles
      $title = trim($record->title);
      if (empty($title)) {
        continue;
      }

      $all[] = (object) [
        'nid' => $record->nid,
        'title' => $title,
        'type' => $record->type,
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
    // remove generated jQuery script
    $page_html = preg_replace('/<script\b[^>]*>.*<\/script>/isU', "",
      $page_html);
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
      $query = 'SELECT nid, title, node_type.type AS type
                  FROM {node} 
                  JOIN {node_type} ON node_type.type = node.type
                  WHERE status=1 AND nid=:id';
      return db_query($query, [':id' => $this->id])->fetchAll();
    }

    if ($this->limit === NULL || $this->offset === NULL) {
      throw new Exception('NodesIndexJob: Limit and offset parameters are required if node id is not provided in the constructor.');
    }

    $query = 'SELECT nid, title, node_type.name AS type
                FROM {node}
                JOIN {node_type} ON node_type.type = node.type
                WHERE status=1
                ORDER BY nid DESC
                LIMIT :limit 
                OFFSET :offset';
    return db_query($query, [
      ':limit' => $this->limit,
      ':offset' => $this->offset,
    ])->fetchAll();
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
   * Count the number of available nodes.
   * Used for progress reporting by the DispatcherJob.
   *
   * @return int
   */
  public function count() {
    return db_query('SELECT COUNT(nid) FROM {node} N
                      WHERE status=1 AND type != :type',
      [':type' => 'blastdb'])->fetchField();
  }
}
