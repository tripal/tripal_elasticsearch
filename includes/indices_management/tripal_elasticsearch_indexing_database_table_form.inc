<?php

require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/FormElementsForIndex.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticConnection.php';
require_once drupal_get_path('module', 'tripal_elasticsearch') . '/api/ElasticIndex.php';



/**
 * The fileds options are dependent on the table selected in previous dropdown.
 */
function tripal_elasticsearch_indexing_database_table_form($form, &$form_state)
{
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
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

        $form['database_table'] = array(
            '#type' => 'select',
            '#title' => t('Select a database table'),
            '#description' => t('Select a database table to index'),
            '#options' => array('Select a table' => 'Select a table') + get_table_list(),
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
        $tableselect_rows = array();
        foreach (get_column_list($table) as $field) {
            $tableselect_rows[$field] = array(
                'field' => $field,
                //'mapping_type' => get_elastic_mapping_types(),
                'mapping_type' => array(
                    'data' => array(
                        '#type' => 'select',
                        '#title' => 'Mapping Type',
                        '#title_display' => 'invisible',
                        '#name' => $field,
                        '#options' => get_elastic_mapping_types(),
                    )
                ),
            );
        }
        $header = array(
            'field' => t('Field'),
            'mapping_type' => t('Mapping type'),
        );
        $form['display_table_fields'] = array(
            '#type' => 'tableselect',
            '#tree' => true,
            '#header' => $header,
            '#options' => $tableselect_rows,
            '#empty' => t('No fields available.'),
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
function display_table_fields_ajax($form, &$form_state)
{
    return $form['display_table_fields'];
}


/**
 * submit callback function
 */
function tripal_elasticsearch_indexing_database_table_form_submit($form, &$form_state)
{
    /**
     * can only access all input values through $_POST. How to fix this??
     * we need the following data for indexing job
     *      table name
     *      selected table fields
     *      corresponding mapping types to each selected table fields
     *      index name
     */
    $table_name = $_POST['database_table'];
    $table_fields = array_values($_POST['display_table_fields']);
    $field_mapping_types = [];
    foreach ($table_fields as $field) {
        $field_mapping_types[$field] = $_POST[$field];
    }
    $index_name = $table_name;


    $connection = ElasticConnection::make();
    $client = ElasticIndex($connection);


    /**
     * bind the following data to an item object and add the items to a queue
     *      index name
     *      table name
     *      selected table fields
     *      index mapping types
     *      sql query that will extract values from the database table for the selected table fields.
     */
    // get row count of selected table
    $sql = "SELECT COUNT(*) FROM " . $table_name . ";";
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

        $comma_separated_fields = implode(',', array_keys($table_fields));
        // use the first field to sort the table
        $order_by_field = array_shift($table_fields);
        $sql = "SELECT " . $comma_separated_fields . " FROM " . $table_name . " ORDER BY $order_by_field LIMIT $k OFFSET $OFFSET;";

        /**
         *  Bind the following values to the queue items
         *      index name
         *      table name
         *      sql query
         *      field mapping types
         */
        $item->index = $index_name;
        $item->table = $table_name;
        $item->sql = $sql;
        $item->field_mapping_types = $field_mapping_types;
        $queue->createItem($item);
    }

}


/**
 * form select element for elastic mapping types
 */
function get_elastic_mapping_types()
{
    $mapping_types = array(
        'text' => 'text',
        'keyword' => 'keyword',
        'date' => 'date',
        'long' => 'long',
        'double' => 'double',
        'boolean' => 'boolean',
        'ip' => 'ip',
        'object' => 'object',
        'nested' => 'nested',
        'geo_point' => 'geo_point',
        'geo_shape' => 'geo_shape',
        'completion' => 'completion',
    );
    //return theme('select', array('element' => $element));
    return $mapping_types;
}