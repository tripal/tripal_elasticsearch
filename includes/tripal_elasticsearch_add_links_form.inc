<?php


function tripal_elasticsearch_add_links_form($form, &$form_state){
  $result = db_query('SELECT DISTINCT(table_name) FROM tripal_elasticsearch');
  $tables = array('Select a table');
  foreach($result as $record){
    $tables[$record->table_name] = $record->table_name;
  }
  $form['add_links'] = array(
    '#type' => 'fieldset',
    '#title' => 'Add page links to search results',
    '#prefix' => '<div id="add_links">',
    '#suffix' => '</div>',
  );
  $form['add_links']['table'] = array(
    '#type' => 'select',
    '#options' => $tables,
    '#ajax' => array(
      'callback' => 'add_links_ajax',
      'wrapper' => 'add_links',
    ),
  );



  if(!empty($form_state['values']['table'])){
    $table_name = $form_state['values']['table'];
    $columns = db_query('SELECT table_field FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name))
                 ->fetchAll();
    foreach($columns as $field){
      $field = $field->table_field;
      $form['add_links'][$field] = array(
        '#type' => 'textfield',
        '#title' => $field,
        '#size' => 60,
        '#title_display' => 'after',
/*
        '#ajax' => array(
          'callback' => 'add_links_ajax',
          'wrapper' => 'add_links',
        ),
*/
      );
    }

  }


  $form['add_links']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add links'),
  );

  return $form;
}


function add_links_ajax($form, &$form_state){
  return $form;
}

function tripal_elasticsearch_add_links_form_submit($form, &$form_state){
  tripal_elasticsearch_add_links($form_state['input']['table'], $form_state['input']);
}




function tripal_elasticsearch_add_links_example_form($form, &$form_state){
  $form['add_links'] = array(
    '#type' => 'fieldset',
    '#title' => 'Example: add page links to search results',
    '#collapsible' => true,
    '#collapsed' => true,
  );

  $description = '<h2>Add page links to search results</h2>';
  $description .= '<p>This page allows you to add dynamic URL to search results. ';
  $description .= 'Here dynamic URL means that the URL dependens on the seach result values. ';
  $description .= 'For example, the search form for organism has four fields: <b>abbreviation</b>, <b>common_name</b>, <b>genus</b>, <b>species</b>. ';
  $description .= 'We want to link some search results to corresponding oranism pages. ';
  $description .= 'Assuming that all organism pages have the same pattern: <b>organism/genus/species</b> ';
  $description .= '(all URLs have the same part "organism/" but are different at "genus" and "species"),';
  $description .= 'and the different parts in URLs only depend on the values of search results, ';
  $description .= 'in this case, we can link search results to their corresponding organism pages by adding dynamic links to the fields. </p>';
  $description .= '<p><b>A dynamic link is created by replacing variable values in URL with corresponding field names within "[]".</b></p>';
  $description .= '<p>The example below shows how to add links to search results in fields <b>abbreviation</b> and <b>species</b>.</p>';

  $form['add_links']['description'] = array(
    '#type' => 'item',
    '#markup' => $description,
  );

  $form['add_links']['abbreviation'] = array(
    '#type' => 'textfield',
    '#title' => t('abbreviation'),
    '#title_display' => 'after',
    '#default_value' => t('organism/[genus]/[species]'),
  );
  $form['add_links']['common_name'] = array(
    '#type' => 'textfield',
    '#title' => t('common_name'),
    '#title_display' => 'after',
  );
  $form['add_links']['genus'] = array(
    '#type' => 'textfield',
    '#title' => t('genus'),
    '#title_display' => 'after',
  );
  $form['add_links']['species'] = array(
    '#type' => 'textfield',
    '#title' => t('species'),
    '#title_display' => 'after',
    '#default_value' => t('organism/[genus]/[species]'),
  );

  return $form;
}
