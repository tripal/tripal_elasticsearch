<?php

class ESDaemon extends DrushDaemon {

  protected $loop_interval = 20;

  /**
   * Find and run queues.
   *
   * @param int $iteration_number
   */
  protected function executeTask($iteration_number) {
    // Get and run the dispatcher if any items are in it
    $dispatcher = db_query("SELECT COUNT(item_id) FROM queue WHERE name = 'elasticsearch_dispatcher'")->fetchField();
    if ($dispatcher > 0) {
      $this->runQueue('elasticsearch_dispatcher');
    }

    // Check to see if there are tasks remaining
    $queues = db_query("SELECT name, COUNT(item_id) FROM queue WHERE name LIKE 'elasticsearch_queue%' GROUP BY name ORDER BY name ASC")->fetchAll();

    // Execute all queues
    foreach ($queues as $queue) {
      // Run the queue
      $this->runQueue($queue->name);
    }
  }

  /**
   * Run a queue in its own thread.
   *
   * @param string $queue_name
   */
  protected function runQueue($queue_name) {
    $name = 'queue_' . $queue_name;
    $thread = intval(substr($name, -1));
    if (!($thread > 0)) {
      $thread = 1;
    }

    // We use drush_invoke_process() to fork the daemon safely to run
    // multiple jobs concurrently. We can't use the PHP-daemon built
    // in functionality such as workers & tasks b/c they copy the
    // entire environment. resulting in multiple processes using the same
    // database connection (which causes errors).
    drush_invoke_process('@self', // Obviously run on the current site.
      'cron-run',                 // Run the cron
      [$name],                    // Arguments
      [
        'options' => 'thread=' . $thread,
      ],
      ['fork' => TRUE]            // This tells drush to spawn a new process.
    );
  }
}