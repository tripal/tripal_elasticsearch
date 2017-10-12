<?php

abstract class ESJob {

  /**
   * Job type.
   * Best option is to add the index type.
   *
   * @var string
   */
  public $type = 'index';

  /**
   * SQL limit.
   *
   * @var int
   */
  protected $limit = NULL;

  /**
   * SQL offset.
   *
   * @var int
   */
  protected $offset = NULL;

  /**
   * Runs the job.
   * This function must be defined by extending classes.
   */
  abstract public function handle();

  /**
   * Get the total count of available records.
   *
   * @return int
   */
  public function count() {
    return 1;
  }

  /**
   * Get the total number of items this job is processing.
   * Override this function to return the total records your job is indexing.
   *
   * @return int
   */
  public function total() {
    return 1;
  }

  /**
   * Set SQL limit.
   *
   * @param int $limit
   *
   * @return $this
   */
  public function limit($limit) {
    $this->limit = $limit;
    return $this;
  }

  /**
   * Set SQL offset.
   *
   * @param int $offset
   *
   * @return $this
   */
  public function offset($offset) {
    $this->offset = $offset;
    return $this;
  }

  /**
   * Dispatch this job to the queue.
   *
   * @param string $queue_nameN ame of queue.
   */
  public function dispatch($queue_name = NULL) {
    ESQueue::dispatch($this, $queue_name);
  }
}