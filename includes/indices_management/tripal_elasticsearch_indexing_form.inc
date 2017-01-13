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

    // select a database table to index
    $table_list = get_table_list();
    foreach($table_list as $key=>$table) {
      $table_list[$table] = $table;
      unset($table_list[$key]);
    }
    $form['database_table'] = array(
      '#type' => 'select',
      '#title' => t('Select a database table'),
      '#description' => t('Select a database table to index. Select "index_website" will index all web pages.'),
      '#options' => $table_list, 
      '#ajax' => array(
        'callback' => 'display_table_fields_ajax',
        'wrapper' => 'display_table_fields',
      ),
    );

    // display table fields
    $table = !empty($form_state['values']['database_table']) ? $form_state['values']['database_table'] : 'index_website';
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
 * submit callback function
 */
function tripal_elasticsearch_indexing_form_submit($form, &$form_state){

  run_elasticsearch_indexing($form_state['values']['table_dropdown'], $form_state['values']['queue_N'], $form_state['values']['fields']);

  $form['rebuild'] = TRUE;

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
