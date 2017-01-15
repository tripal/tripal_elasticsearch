<?php


/**
 * The fileds options are dependent on the table selected in previous dropdown.
 */
function tripal_elasticsearch_indexing_website_form($form, &$form_state){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  try{
    // get a list of created indices
    $indices = array_keys($client->indices()->getMapping());
    foreach($indices as $key=>$index) {
      $indices[$index] = $index;
      unset($indices[$key]);
    }

    $form['indices'] = array(
      '#type' => 'select',
      '#description' => t('Different indices may consists of different character filters, token filters and analyzers,
                         which together determine how your data is indexed and how your data is searchable.'),
      '#title' => t('Indices'),
      '#options' => $indices, 
    );

    // form fields for select N number of cron queues
    $queue_number_array = range(0, 10);
    unset($queue_number_array[0]);
    $form['queue_number'] = array(
      '#type' => 'select',
      '#description' => t('10 queues have been pre-defined for indexing jobs. This number determines the first n
                         queues will be used for indexing jobs. Use more queues if your server has high ability
                         for multi-threads process.'),
      '#title' => t('Number of cron queues'),
      '#options' => $queue_number_array,
    );

    $form['website_base_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Enter your website base url'),
      '#description' => t('Enter the base url of your website. Examples: "http://www.example.com"; 
                           "http://www.example.com:8080"')
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
  } catch (\Exception $e) {
    $form['error'] = array(
      '#type' => 'item',
      '#markup' => $e->getMessage(),
    );
  } 

  return $form;
}



/**
 * submit callback function
 */
function tripal_elasticsearch_indexing_website_form_submit($form, &$form_state){
  $index = $form_state['values']['indices'];
  $queue_number = $form_state['values']['queue_number'];
  $table = 'index_website';
  $website_base_url = $form_state['values']['website_base_url'];

  // get row count of selected table
  $sql = ($table == "index_website") ? "SELECT COUNT(*) FROM node;" : "SELECT COUNT(*) FROM ".$table.";";
  $record = db_query($sql)->fetchAssoc();
  $row_count = $record['count'];

  // get the total number of offsets (offset interval is 1000)
  $k = 1000;
  $total_offsets = intval($row_count / $k);
  foreach (range(0, $total_offsets) as $offset) {
    $queue_id = 'elastic_queue_'.($offset % $queue_number);
    $queue = DrupalQueue::get($queue_id);
    $OFFSET = $k * $offset;
    $item = new stdClass();
    $sql = "SELECT title, nid FROM node WHERE status=1 ORDER BY nid LIMIT $k OFFSET $OFFSET;";
    $item->index = $index;
    $item->table = $table;
    $item->sql = $sql;
    $item->website_base_url = $website_base_url;
    $queue->createItem($item);
  }

}


