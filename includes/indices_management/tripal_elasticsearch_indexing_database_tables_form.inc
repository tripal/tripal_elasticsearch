<?php


/**
 * The fileds options are dependent on the table selected in previous dropdown.
 */
function tripal_elasticsearch_indexing_form($form, &$form_state){
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


    $form['database_table'] = array(
      '#type' => 'select',
      '#title' => t('Select a database table'),
      '#description' => t('Select a database table to index. Select "index_website" to index all web pages.'),
      '#options' => get_table_list(), 
      '#ajax' => array(
        'callback' => 'display_table_fields_ajax',
        'wrapper' => 'display_table_fields',
      ),
    );


    $form['display_table_fields'] = array(
      '#type' => 'textfield',
      '#title' => t('Input base url of your site'),
      '#description' => t('Please input the base url of your site.'),
    );

    $table = !empty($form_state['values']['database_table']) ? $form_state['values']['database_table'] : 'index_website';
    // display table fields
    $form['display_table_fields'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Select table fields to index'),
      '#description' => t('Please select at least on fields'),
      '#options' => get_column_list($table), 
      '#prefix' => '<div id="display_table_fields">',
      '#suffix' => '</div>',
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
 * display table fields ajax callback
 */
function display_table_fields_ajax ($form, &$form_state) {
  $table = $form_state['values']['database_table'];
  if($table == 'index_website') {
    $form['display_table_fields']['#description'] = t('No need to select fields for table "index_website"'); 
  }
  return $form['display_table_fields'];
}




/**
 * submit callback function
 */
function tripal_elasticsearch_indexing_form_submit($form, &$form_state){
  $index = $form_state['values']['indices'];
  $queue_number = $form_state['values']['queue_number'];
  //variable_set('queue_number', $queue_number);
  $table = $form_state['values']['database_table'];
  $table_fields = $form_state['values']['display_table_fields'];
  foreach($table_fields as $key=>$value) {
    if(! is_string($value)) {
      unset($table_fields[$key]);
    }
  }

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
    if ($table == "index_website") {
      $sql = "SELECT title, nid FROM node WHERE status=1 ORDER BY nid LIMIT $k OFFSET $OFFSET;";
    } else {
      $comma_separated_fields = implode(',', array_keys($table_fields)); 
      // use the first field to sort the table
      $order_by_field = array_shift($table_fields);
      $sql = "SELECT ". $comma_separated_fields. " FROM " . $table ." ORDER BY $order_by_field LIMIT $k OFFSET $OFFSET;";
    }
    $item->index = $index;
    $item->table = $table;
    $item->sql = $sql;
    $queue->createItem($item);
  }

}


