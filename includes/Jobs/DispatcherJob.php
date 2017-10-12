<?php

/**
 * Class DispatcherJob
 * ======================================
 * Dispatch jobs for bulk indexing
 */
class DispatcherJob extends ESJob {

  /**
   * Dispatcher job type.
   *
   * @var string
   */
  public $type = 'Dispatcher';

  /**
   * Hold total number of records.
   *
   * @var int
   */
  protected $total = 0;

  /**
   * The job to dispatch.
   *
   * @var \ESJob
   */
  protected $job;

  /**
   * DispatcherJob constructor.
   *
   * @param \ESJob $job
   */
  public function __construct($job) {
    $this->job = $job;

    $this->total = $this->job->count();

    // Nothing to index. Terminate
    return ESQueue::initProgress($this->job->type, $this->total);
  }

  /**
   * Start dispatching jobs.
   */
  public function handle() {
    $chunk = $this->job->chunk;

    for ($offset = 0; $offset < $this->total; ($offset + $chunk < $this->total) ? ($offset += $chunk) : ($offset += $this->total - $offset)) {
      $this->job->offset($offset)->limit($chunk)->dispatch();
    }
  }

  /**
   * Dispatch the job on the dispatcher queue.
   *
   * @param string $queue_name Defaults to the dispatcher queue.
   *
   * @override parent::dispatch
   */
  public function dispatch($queue_name = NULL) {
    if ($queue_name === NULL) {
      $queue_name = 'elasticsearch_dispatcher';
    }
    parent::dispatch($queue_name);
  }
}