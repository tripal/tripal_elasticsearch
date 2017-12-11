<?php

class ESQueue{

  /**
   * Name of the queue progress table.
   *
   * @var string
   */
  const QUEUE_TABLE = 'tripal_elasticsearch_queues';

  /**
   * Available ES queues.
   *
   * @var array
   */
  protected $queues = [];

  /**
   * Queue with the minimum number of jobs.
   *
   * @var string
   */
  protected $min_queue = 'elasticsearch_queue_1';

  /**
   * Queue with the maximum number of jobs.
   *
   * @var string
   */
  protected $max_queue = 'elasticsearch_queue_1';

  /**
   * Whether the generateQueueCount function has been ran before.
   *
   * @var bool
   */
  protected $generated_queue_count = FALSE;

  /**
   * ESQueue constructor.
   * Populate available queues.
   */
  public function __construct() {
    // Generate 10 default queues
    for ($i = 1; $i <= 5; $i++) {
      $this->queues["elasticsearch_queue_{$i}"] = 0;
    }
  }

  /**
   * Create a progress report.
   *
   * @return object
   */
  public static function progress() {
    $query = 'SELECT index_name,
                    priority,
                    SUM(total) AS total,
                    SUM(completed) AS completed,
                    MAX(last_run_at) AS last_run_at,
                    MIN(started_at) AS started_at
                    FROM {' . self::QUEUE_TABLE . '}
                    GROUP BY index_name, priority
                    ORDER BY index_name ASC, priority ASC';
    $queues = db_query($query)->fetchAll();

    $progress = [];
    $total = 0;
    $completed = 0;
    $progress_last_run_at = 0;
    $progress_started_at = time();

    foreach ($queues as $queue) {
//      if ($queue->total === $queue->completed) {
//        continue;
//      }

      if($queue->completed > $queue->total) {
        static::fixProgress($queue);
      }

      $last_run = new DateTime();
      $last_run->setTimestamp($queue->last_run_at);

      $started_at = new DateTime();
      $started_at->setTimestamp($queue->started_at);

      $progress_last_run_at = max($progress_last_run_at, $queue->last_run_at);
      $progress_started_at = min($progress_started_at, $queue->started_at);

      $total += $queue->total;
      $completed += $queue->completed;
      $round_name = ' Round: ' . (intval($queue->priority) === 1 ? 'High' : 'Low');
      $progress[$queue->index_name . $round_name] = (object) [
        'total' => $queue->total,
        'completed' => $queue->completed,
        'remaining' => $queue->total - $queue->completed,
        'percent' => number_format(($queue->completed / ($queue->total ?: 1)) * 100, 2),
        'last_run_at' => $last_run,
        'started_at' => $started_at,
        'time' => $queue->last_run_at - $queue->started_at,
        'priority' => $queue->priority
      ];
    }

    return (object) [
      'queues' => $progress,
      'total' => $total,
      'completed' => $completed,
      'remaining' => $total - $completed,
      'percent' => number_format(($completed / ($total ?: 1)) * 100, 2),
      'time' => count($progress) > 0 ? $progress_last_run_at - $progress_started_at : 0,
    ];
  }

  public static function fixProgress(&$queue) {
    $queue->completed = $queue->total;

    db_query('UPDATE {'.self::QUEUE_TABLE.'} SET completed=total WHERE completed > total');
  }

  /**
   * Dispatch a new job. Uses the queue that has minimum items if queue name is
   * not provided.
   *
   * @param \ESJob $job
   * @param string $queue_name
   *
   * @return boolean
   */
  public static function dispatch($job, $queue_name = NULL) {
    if ($queue_name === NULL) {
      $queue = new static();
      $queue_name = $queue->getMinQueue();
    }

    $job->queue_name = $queue_name;
    return DrupalQueue::get($queue_name)->createItem($job);
  }

  /**
   * Execute a given job.
   *
   * @param \ESJob $job
   *
   * @throws \Exception
   */
  public static function run($job) {
    if ($job instanceof ESJob) {
      try {
        $job->handle();
        $total = $job->total();
      } catch (Exception $exception) {
        watchdog('tripal_elasticsearch', $exception->getMessage(), [], WATCHDOG_ERROR);
        $total = $job->chunk ?: 1;
      }

      $remaining = static::updateProgress($job->type, $total);
      if ($remaining === 0 && $job->hasRounds()) {
        $job->createNextRound();
      }

      return;
    }

    throw new Exception('Elasticsearch Queue: ' . get_class($job) . ' is an invalid job type. Jobs must extend the ESJob class');
  }

  /**
   * Initialize the progress tracker for a specific type.
   *
   * @param string $type The label of the index.
   * @param string $index_name Name of the index.
   * @param int $total the total number of records going to the queue (not
   *                    number of jobs).
   *
   * @return DatabaseStatementInterface
   */
  public static function initProgress(
    $type,
    $index_name,
    $total = 1,
    $priority = 1
  ) {
    $counter_table = self::QUEUE_TABLE;
    $query = 'SELECT total, completed FROM {' . $counter_table . '} WHERE type=:type';
    $queue = db_query($query, [':type' => $type])->fetchObject();

    // If type already exists, reset progress
    if ($queue) {
      return db_query('UPDATE {' . $counter_table . '} SET total=:total, last_run_at=:time, completed=:completed, started_at=:started_at WHERE type=:type', [
        ':type' => $type,
        ':total' => $total,
        ':completed' => 0,
        ':time' => time(),
        ':started_at' => time(),
      ]);
    }

    // Initialize a new progress report for index name
    return db_query('INSERT INTO {' . $counter_table . '} (index_name, type, total, completed, last_run_at, started_at, priority) VALUES (:index_name, :type, :total, 0, :last_run_at, :started_at, :priority)', [
      ':type' => $type,
      ':index_name' => $index_name,
      ':total' => $total,
      ':last_run_at' => time(),
      ':started_at' => time(),
      ':priority' => $priority,
    ]);
  }

  /**
   * Update the number of items in the counter.
   *
   * @param string $index_name the index name.
   * @param int $by the number to decrement by.
   *
   * @return DatabaseStatementInterface|boolean
   */
  public static function updateProgress($type, $by = 1) {
    $counter_table = self::QUEUE_TABLE;
    $query = 'SELECT type, completed, total FROM {' . $counter_table . '} WHERE type=:type';
    $queue = db_query($query, [':type' => $type])->fetchObject();

    if ($queue) {
      db_query('UPDATE {' . $counter_table . '} SET completed=:completed, last_run_at=:last_run_at WHERE type=:type', [
        ':type' => $type,
        ':completed' => $queue->completed + $by,
        ':last_run_at' => time(),
      ]);

      return $queue->total - ($queue->completed + $by);
    }

    return FALSE;
  }

  /**
   * Get the queue with the minimum jobs.
   *
   * @return string
   */
  public function getMinQueue() {
    $this->generateQueueCount();

    return $this->min_queue;
  }

  /**
   * Get the queue with the maximum jobs.
   *
   * @return string
   */
  public function getMaxQueue() {
    $this->generateQueueCount();

    return $this->max_queue;
  }

  /**
   * Generate the queue count and set the min and max queues.
   */
  protected function generateQueueCount() {
    // Run only if the count hasn't been generated yet
    if ($this->generated_queue_count) {
      return;
    }

    $this->generated_queue_count = TRUE;

    $max = NULL;
    $min = NULL;

    $queues = db_query("SELECT name, COUNT(name) as count FROM {queue} WHERE name LIKE 'elasticsearch%' GROUP BY name ORDER BY count ASC")->fetchAll();

    foreach ($queues as $queue) {
      if (!isset($this->queues[$queue->name])) {
        continue;
      }

      $this->queues[$queue->name] = $queue->count;
    }

    foreach ($this->queues as $queue => $count) {
      if ($max === NULL) {
        $max = $count;
        $this->max_queue = $queue;

        $min = $count;
        $this->min_queue = $queue;
      }

      if ($count > $max) {
        $max = $count;
        $this->max_queue = $queue;
      }

      if ($count < $min) {
        $min = $count;
        $this->min_queue = $queue;
      }
    }
  }
}