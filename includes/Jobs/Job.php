<?php

namespace ES\Jobs;

abstract class Job{

  /**
   * Holds the index name.
   *
   * @var string
   */
  public $index = 'index';

  /**
   * Job type.
   * Best option is to add the index type.
   *
   * @var string
   */
  public $type = 'index';

  /**
   * Number of items to index in bulk.
   *
   * @var int
   */
  public $chunk = 1;

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
   * Automatically set by \ES\Common\Queue
   *
   * @var string
   */
  public $queue_name;

  /**
   * Runs the job.
   * This function must be defined by extending classes.
   */
  abstract public function handle();

  /**
   * Get the total count of available records.
   *
   * Define this method such that it returns all available records
   * even if it is not going to be handled by this job. This method
   * is called at the start of bulk indexing job by the dispatcher
   * to determine the number of available records and set the limit
   * and offset of each job.
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
    Queue::dispatch($this, $queue_name);
  }

  /**
   * Tells the \ES\Common\Queue class whether that this job implements
   * priority rounds.
   *
   * @return bool
   */
  public function hasRounds() {
    return FALSE;
  }

  /**
   * Creates and dispatches the next round.
   *
   * @return bool TRUE on created or FALSE on done.
   */
  public function createNextRound() {
    return FALSE;
  }

  /**
   * Specifies which round this job is running.
   *
   * @return int
   */
  public function currentRound() {
    return 1;
  }
}
