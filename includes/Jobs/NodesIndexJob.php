<?php

class NodesIndexJob extends ESJob {

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
  protected $index = 'website';

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
  public $total;

  /**
   * Number of items to bulk index.
   *
   * @var int
   */
  public $chunk = 10;

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
   */
  public function handle() {
    $es = new ESInstance();
    $records = $this->get();
    $this->total = count($records);

    if ($this->total > 1) {
      $es->bulkIndex($this->index, $this->loadContent($records));
    }
    else {
      if ($this->total > 0) {
        $record = $this->loadContent($records)[0];
        $es->createEntry($this->index, $this->index, FALSE, $record[0]);
      }
    }
  }

  /**
   * Load cleaned up content of nodes and attach it to record.
   *
   * @param array $records
   *
   * @return array
   */
  protected function loadContent(array $records) {
    return array_map(function ($record) {
      $node = node_load($record->nid);
      $view = node_view($node, 'full');
      $content = render($view);
      return (object) [
        'nid' => $record->nid,
        'title' => $record->title,
        'type' => $record->type,
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
      $query = 'SELECT nid, title, type FROM {node} WHERE status=1 AND nid=:id';
      return db_query($query, [':id' => $this->id])->fetchAll();
    }

    if ($this->limit === NULL || $this->offset === NULL) {
      throw new Exception('NodesIndexJob: Limit and offset parameters are required if node id is not provided in the constructor.');
    }

    $query = 'SELECT nid, title, type FROM {node} WHERE status=1 ORDER BY nid DESC LIMIT :limit OFFSET :offset';
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
    return db_query('SELECT COUNT(nid) FROM {node} WHERE status=1')->fetchField();
  }
}