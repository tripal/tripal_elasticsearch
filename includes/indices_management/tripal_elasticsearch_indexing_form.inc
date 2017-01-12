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

  $form['indexing_table']['queue_N'] = array(
    '#type' => 'select',
    '#title' => 'Number of cron queues',
    '#default_value' => 1,
    '#description' => t('10 queues have been pre-defined for indexing jobs. This number determines the first n
                         queues will be used for indexing jobs. Use more queues if your server has high ability
                         for multi-threads process.'),
    '#options' => array(1=>'1',
                        2=>'2',
                        3=>'3',
                        4=>'4',
                        5=>'5',
                        6=>'6',
                        7=>'7',
                        8=>'8',
                        9=>'9',
                        10=>'10')
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
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
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
    '#suffix' => t('Please do not re-submit.'),
    '#value' => t('Submit'),
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

  run_elasticsearch_indexing($form_state['values']['table_dropdown'], $form_state['values']['queue_N'], $form_state['values']['fields']);

  $form['rebuild'] = TRUE;

}
