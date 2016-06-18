<?php




/** elastic_search_form handler.**/
function elastic_search_block_build_form($form, &$form_state) {

  // You have to create a new $client to get access to the elasticsearch
  $client = new Elasticsearch\Client();
  $mappings = $client->indices()->getMapping();
  $indices = array_keys($mappings);


  // $indices is an array consisting of all indexed tables
  $form['search_block'] = array(
    '#type' => 'fieldset',
    '#title' => t('Build tripal_elasticsearch block'),
        '#prefix' => '<div id="search_block">',
        '#suffix' => '</div>',
  );

  // Build search box for all selected fields
  $form['search_block']['all_selected_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('tripal_elasticsearch block'),
  ); 

  $form['search_block']['indexed_tables'] = array(
    '#type' => 'select',
    '#title' => t('Select table for building searc block'),
    '#options' => elastic_search_get_table_list_options(),
    '#default_value' => t('Select a table'),
    '#ajax' => array(
      'callback' => '_elastic_search_block_build_ajax',
      'wrapper' => 'search_block',
    ),
  );


  // Create an array to store all selected fields
  $all_selected_fields = array();
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
          'callback' => '_elastic_search_block_build_ajax',
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
    '#submit' => array('_add_elastic_search_block'),
  );  
  
  
  return $form;  
}





/**
 * AJAX callback function to generate all fields for a selected table
 */
function _elastic_search_block_build_ajax($form, &$form_state){
    return $form;
}



/** elastic_search_block_build_form submit callback **/
// store form elements for building a list of blocks and for building a search interface.
function _add_elastic_search_block($form, &$form_state){


  $table_name = $form_state['values']['indexed_tables'];
  $record = array();
  // Delete the block in the database if that block name already exists
  $delete_search_block = db_delete('elastic_search')
              ->condition('table_name', $table_name)
              ->execute();
  foreach($form_state['values'][$table_name] as $field){
    if(!empty($field)){
      $record['table_name'] = $table_name;
      $record['table_field'] = $field;
      // write record into the elastic_search table in database
      drupal_write_record('elastic_search', $record);
    }
  }
  


  $form_state['redirect'] = 'admin/config/search/elastic_search/search_blocks';
}


// generate table_list options
function elastic_search_get_table_list_options(){
    // You have to create a new $client to get access to the elasticsearch
    $client = new Elasticsearch\Client();
    $mappings = $client->indices()->getMapping();
    $indices = array_keys($mappings);
    $table_list['Select a table'] = 'Select a table';
    foreach($indices as $table){
        $table_list[$table] = $table;
    }

  return $table_list;  
}



