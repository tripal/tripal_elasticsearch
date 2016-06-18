<?php


/**
 * The fileds options are dependent on the table selected in previous
 * dropdown.
 */

function tripal_elasticsearch_indexing_form($form, &$form_state){


  // Get the list of tables to populate the table dropdown
  $options_first = get_table_list();

  // If we have a value for the table dropdown from
  // $form_state['values'] we use this both as the default value for
  // the first dropdown and also as a parameter to pass to the
  // function that retrieves the options for the available fields.
  $selected = isset($form_state['values']['table_dropdown']) ? $form_state['values']['table_dropdown']:key($options_first);



  $form['indexing_table'] = array(
    '#type' => 'fieldset',
    '#title' => t('tripal_elasticsearch indexing'),
    // The entire enclosing div created here gets replaced when table_dropdown
    // is changed.
    '#prefix' => '<div id="dropdown_fields_replace">',
    '#suffix' => '</div>',
  );

  $form['indexing_table']['table_dropdown'] = array(
    '#type' => 'select',
    '#title' => 'Select a table to index',
    '#options' => $options_first,
    '#default_value' => $selected,
    // bind an ajax callback to the change event (which is the default for the
    // select form type) of the table dropdown. It will replace the available
    // fields when rebuild
    '#ajax' => array(
      'callback' => 'table_dependent_dropdown_callback',
      'wrapper' => 'dropdown_fields_replace',
    ),
  );

  

  $form['indexing_table']['indexing_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select fields to index'),
  );

  // Check if a table is already indexed.
  $client = new Elasticsearch\Client();
  $mappings = $client->indices()->getMapping();
  $indices = array_keys($mappings);
  if(in_array($options_first[$selected], $indices)){
    $form['indexing_table']['indexing_fields']['checking_table'] = array(
      '#type' => 'item',
      '#markup' => '<font color="red"><b>'.t("$options_first[$selected] already exists in elasticsearch.<br /> 
Please select another item to index, or DELETE this item first and then re-index it!"). '</b></font>',
    );
  }


  $form['indexing_table']['indexing_fields']['fields'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Fields available to index',
    // when the form is rebuilt during ajax processing, the $selected variable
    // will now have the new value and so the fields will change.
    // $selected is a key instead of a table name.
    // To get a table name, we need to use $options_first[$selected].
    '#options' => get_column_list($options_first[$selected]),
  );


  $form['indexing_table']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Index'),
  );




  return $form;
}




/**
 * Selects just the available fields checkboxes to be returned for re-rendering
 *
 * The version here has been re-loaded with a different set of options and is sent
 * back to the page to be updated.
 *
 * @return renderable array (the checkboxes).
 */
function table_dependent_dropdown_callback($form, &$form_state){
    return $form['indexing_table'];//['fields'];
}


/**
 * submit callback function
 */
function tripal_elasticsearch_indexing_form_submit($form, &$form_state){
  //Get selected table
  $table_list = get_table_list();
  $selected_table_key = $form_state['values']['table_dropdown'];
  $selected_table = $table_list[$selected_table_key];

  // Get selected fields=============
  $field_list = get_column_list($selected_table);
  $selected_fields_keys = $form_state['values']['fields'];
  $_SESSION['selected_fields'] = array();

  if(!empty($selected_fields_keys)){
    foreach($selected_fields_keys as $key){
      // check if $key is an alphanumeric character.
      if(ctype_alnum($key)){
        $_SESSION['selected_fields'][] = $field_list[$key];
      }
    }
  }


  // Add 'chado.' to the front of table name if it is a chado table
  if(in_array($selected_table, get_chado_table_list())){
    $selected_table = 'chado.' . $selected_table;
  }

  // separate the selected fields by ','.
  // and insert this string into the sql query statement.
  $comma_separated_fields = implode(", ", $_SESSION['selected_fields']);
  $_SESSION['comma_separated_fields'] = implode(", ", $_SESSION['selected_fields']);
    // Use the first field to order the database table when implement queries.
    $order_by_field = $_SESSION['selected_fields'][0];
    //====="SELECT * FROM chado.blast_hit_data ORDER BY RANDOM() LIMIT 1000;"


  // get row count of the selected table
  if($selected_table == 'index_website'){
    $sql_rowcount = "SELECT COUNT(*) FROM node;";
  }
  else {
    $sql_rowcount = "SELECT COUNT(*) FROM ".$selected_table.";";
  }
  $row_count = db_query($sql_rowcount)->fetchAssoc();
  $row_count = implode($row_count);


  // fetch 1000 rows for a single database query
  if($selected_table == 'index_website'){
    // It's better to set a small $k when the doc to be index is very large, e.g. a whole webpage.
    $k = 100;
  }
  else {
    $k = 1000;
  }
  // the number of database queries for a selected table
  $n = intval($row_count/$k);
  $queue = DrupalQueue::get('elastic_queue');
  $offsets = range(0, $n);
  foreach($offsets as $offset){
    $offset = $k*$offset;
    $item = new stdClass();
    if($selected_table == 'index_website'){
      // select only published nodes
      $sql = "SELECT title, nid FROM node WHERE status=1 ORDER BY nid LIMIT $k OFFSET $offset;";
    }
    else {
      $sql = "SELECT ". $_SESSION['comma_separated_fields']. " FROM " . $selected_table ." ORDER BY $order_by_field LIMIT $k OFFSET $offset;";
    }
    $item->sql = $sql;
    // also store selected table
    $item->selected_table = $selected_table;
    $queue->createItem($item);  
  }

  $form['rebuild'] = TRUE;

}



