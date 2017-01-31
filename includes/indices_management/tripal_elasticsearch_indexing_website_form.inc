<?php


/**
 * The fileds options are dependent on the table selected in previous dropdown.
 */
function tripal_elasticsearch_indexing_website_form($form, &$form_state)
{
    try {

        $FormElementsForIndex = new FormElementsForIndex();

        $form['index'] = array(
            '#type' => 'fieldset',
            '#title' => t('Index Settings'),
            '#description' => t('You can use the default settings for your index'),
            '#collapsible' => True,
            '#collapsed' => True,
        );
        $form['index']['character_filters'] = $FormElementsForIndex->CharacterFiltersElements();
        $form['index']['tokenizer'] = $FormElementsForIndex->TokenizerElements();
        $form['index']['token_filters'] = $FormElementsForIndex->TokenFiltersElements();

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
                           "http://www.example.com:8080". Do not add forward slash (/) at the end.')
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
 * form validation function
 */
function tripal_elasticsearch_indexing_website_form_validate($form, &$form_state)
{

    $sql = "SELECT nid FROM node WHERE status=1 ORDER BY nid LIMIT 1;";
    $result = db_query($sql)->fetchAll();
    foreach ($result as $record) {
        $nid = $record->nid;
    }
    $website_base_url = $form_state['values']['website_base_url'];
    try {
        @ $content = file_get_contents($website_base_url . '/node/' . $nid);
        if ($content === false) {
            $error_message = t('The website base url is invalid. Please check your website base url.');
            form_set_error('website_base_url', $error_message);
        }
    } catch (Exception $e) {
        form_set_error('website_base_url', $e->getMessage());
    }
}


/**
 * submit callback function
 */
function tripal_elasticsearch_indexing_website_form_submit($form, &$form_state)
{

    $queue_number = $form_state['values']['queue_number'];
    $website_base_url = $form_state['values']['website_base_url'];
    $table_name = "node"; //$_POST['database_table'];

    // get index name
    $index_name = "website"; //preg_replace('/\./', '_', $table_name);
    // build character filters array
    $character_filters = array_filter(array_values($form_state['values']['CharacterFiltersElements']), "is_string");
    //$ElasticCharacterFilters = new ElasticCharacterFilters();
    foreach ($character_filters as $character_filter)
    {
        $character_filters_arr["my_" . $character_filter] = call_user_func("ElasticCharacterFilters::$character_filter");
    }
    // build token filters array
    $token_filters = array_filter(array_values($form_state['values']['TokenFiltersElements']), "is_string");
    //$ElasticTokenFilters = new ElasticTokenFilters();
    foreach ($token_filters as $token_filter)
    {
        $token_filters_arr["my_" . $token_filter] = call_user_func("ElasticTokenFilters::$token_filter");
    }
    // get tokenizer
    $tokenizer = $form_state['values']['TokenizerElements'];

    // get field_mapping_types
    $field_mapping_types = [
        "nid" => "long",
        "title" => "string",
        "type" => "string",
        "body" => "string",
    ];

    // create a connection port
    $connection = (new ElasticConnection(["127.0.0.1:9201"]))->make();

    // build index
    $build_elastic_index = new BuildElasticIndex($client = $connection,
        $index = $index_name,
        $number_of_shards = 5,
        $nuber_of_replicas = 0,
        $character_filters = $character_filters_arr,
        $token_filters = $token_filters_arr,
        $tokenizer = $tokenizer,
        $field_mapping_types = $field_mapping_types);

    $build_elastic_index->create_index();




    // get row count of selected table
    $sql = "SELECT COUNT(*) FROM node;";
    $record = db_query($sql)->fetchAssoc();
    $row_count = $record['count'];

    // get the total number of offsets (offset interval is 1000)
    $k = 1000;
    $total_offsets = intval($row_count / $k);
    foreach (range(0, $total_offsets) as $offset) {
        $queue_id = 'elastic_queue_' . ($offset % $queue_number);
        $queue = DrupalQueue::get($queue_id);
        $OFFSET = $k * $offset;
        $item = new stdClass();
        $sql = "SELECT title,nid,type FROM node WHERE status=1 ORDER BY nid LIMIT $k OFFSET $OFFSET;";


        /**
         *  Bind the following values to the queue items
         *      index name
         *      table name
         *      sql query
         *      field mapping types
         */
        $item->index = $index_name;
        $item->table = $table_name;
        $item->is_website = True;
        $item->sql = $sql;
        $item->website_base_url = $website_base_url;
        $item->field_mapping_types = $field_mapping_types;
        $queue->createItem($item);
    }

}




/**
 *
 */ 
