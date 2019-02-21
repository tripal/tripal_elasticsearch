<?php
namespace ES\Jobs;

/**
 * Class DispatcherJob
 * ======================================
 * Dispatch jobs for bulk indexing.
 */
class DispatcherJob extends Job{

  /**
   * Dispatcher job type.
   *
   * @var string
   * @see Job::$type
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
   * @var \ES\Jobs
   */
  protected $job;

  /**
   * DispatcherJob constructor.
   *
   * @param \ES\Jobs $job The job object.
   */
  public function __construct($job) {
    $this->job = $job;
  }

  /**
   * Start dispatching jobs.
   */
  public function handle() {
    $chunk = $this->job->chunk;
    $this->total = $this->job->count();
    $round = $this->job->hasRounds() ? $this->job->currentRound() : 1;
    \ES\Common\Queue::initProgress($this->job->type, $this->job->index, $this->total,
      $round);

    for ($offset = 0; $offset < $this->total; $offset += $chunk) {
      $this->job->offset($offset)->limit($chunk)->dispatch();
    }
  }

  /**
   * Dispatch the job on the dispatcher queue.
   *
   * @param string $queue_name
   *   Defaults to the dispatcher queue.
   *
   * @see Job::dispatch
   */
  public function dispatch($queue_name = NULL) {
    if ($queue_name === NULL) {
      $queue_name = 'elasticsearch_dispatcher';
    }
    parent::dispatch($queue_name);
  }

  /**
   * Get the job.
   *
   * @return \ES\Jobs
   */
  public function job() {
    return $this->job;
  }

}
