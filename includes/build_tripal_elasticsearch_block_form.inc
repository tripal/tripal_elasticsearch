<?php
/*
 * Select an indexed table and its fields to build form for search
 */
function build_tripal_elasticsearch_block_form($form, &$form_state) {

  // You have to create a new $client to get access to the elasticsearch
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $mappings = $client->indices()->getMapping();
  $indices = array_keys($mappings);

  // $indices is an array consisting of all indexed tables
  $form['search_block'] = array(
    '#type' => 'fieldset',
    '#title' => t('Build tripal_elasticsearch block'),
    '#prefix' => '<div id="search_block">',
    '#suffix' => '</div>',
  );

  // Build search block for all selected fields
  $form['search_block']['all_selected_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('tripal_elasticsearch block'),
  );

  $form['search_block']['indexed_tables'] = array(
    '#type' => 'select',
    '#title' => t('Select table for building search block'),
    '#options' => get_indexed_tables(),
    '#default_value' => t('Select a table'),
    '#ajax' => array(
      'callback' => 'build_tripal_elasticsearch_block_form_ajax',
      'wrapper' => 'search_block',
    ),
  );


  if(!empty($form_state['values']['indexed_tables'])){
    $selected_table = $form_state['values']['indexed_tables'];
    if(!empty($selected_table)){
      // get list of fields for the selected table
      $fields = array_keys($mappings[$selected_table]['mappings'][$selected_table]['properties']);

      // note that this form field is within a for loop.
      // It creates a form field for each table, with table fields as being displayed
      foreach($fields as $key=>$field){
        unset($fields[$key]);
        //$fields[$selected_table.'|'.$field] = $selected_table.'|'.$field;
        $fields[$field] = $field;
      }

      $form['search_block'][$selected_table] = array(
        '#type' => 'checkboxes',
        '#title' => t("$selected_table"),
        '#options' => $fields,
        '#ajax' => array(
          'callback' => 'build_tripal_elasticsearch_block_form_ajax',
          'wrapper' => 'search_block'
        ),
      );

      // $form_state['values'][$selected_table] is an array of keys of
      // selected fields from table $selected_table
      foreach($form_state['values'][$selected_table] as $field){
        if(!empty($field)){
                  $form['search_block']['all_selected_fields'][$field] = array(
                      '#type' => 'textfield',
                      '#title' => $field,
                  );
        }
      };
    } // end check empty $selected_table
  } // end checking empty $form_state['values']['indexed_table']



  $form['search_block']['build_search_boxes'] = array(
    '#type' => 'submit',
    '#value' => 'Add elasticsearch block',
    '#submit' => array('add_tripal_elasticsearch_block'),
  );


  return $form;
}


/**
 * AJAX callback function to generate all fields for a selected table
 */
function build_tripal_elasticsearch_block_form_ajax($form, &$form_state){
    return $form;
}



/*
 * Add select table and fields to database table elastic_search
 * If the selected table already exists in the database, it will
 * be overwritten.
 */
function add_tripal_elasticsearch_block($form, &$form_state){

  $table_name = $form_state['values']['indexed_tables'];
  tripal_elasticsearch_add_block($form_state['values']['indexed_tables'], $form_state['values'][$table_name]);

  $form_state['redirect'] = 'admin/tripal/extension/tripal_elasticsearch/tripal_elasticsearch_add_links';
}


// generate table_list options
function get_indexed_tables(){
    // You have to create a new $client to get access to the elasticsearch
    $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
    $mappings = $client->indices()->getMapping();
    $indices = array_keys($mappings);
    $table_list['Select a table'] = 'Select a table';
    foreach($indices as $table){
        $table_list[$table] = $table;
    }

  return $table_list;
}
