<?php


/**
 * form handler to create index from a template file.
 */
function tripal_elasticsearch_create_index_from_templates_form ($form, &$form_state) {
  // get index json templates file names
  $index_json_templaes_array_begin = array('Select a template' => 'Plase select a template');
  $index_json_templates = scandir(drupal_get_path('module', 'tripal_elasticsearch').'/elasticsearch_indices_json_templates', SCANDIR_SORT_ASCENDING);
  foreach($index_json_templates as $key=>$value) {
    if(!preg_match('/\.json$/', $value)) {
      unset($index_json_templates[$key]);
    } else {
      $index_json_templates[$value] = $value;
      unset($index_json_templates[$key]);
    }
  }
  $index_json_templates_array = $index_json_templaes_array_begin + $index_json_templates;


  $form = array();
  $form['index_json_templates'] = array(
    '#type' => 'select',
    '#title' => t('Index JSON templates'),
    '#options' => $index_json_templates_array,
    '#ajax' => array(
      'callback' => 'index_json_template_content_ajax',
      'wrapper' => 'index_json_template_content',
    ),
  );

  // get selected index json template
  if(!empty($form_state['values']['index_json_templates'])) {
    if($form_state['values']['index_json_templates'] == 'Select a template') {
      $selected_index_json_template = 'Please select a template';
    } else {
      $selected_index_json_template = $form_state['values']['index_json_templates'];
      $file_path = drupal_get_path('module', 'tripal_elasticsearch').'/elasticsearch_indices_json_templates/'.$selected_index_json_template;
      $selected_index_json_template = file_get_contents($file_path);
      //$selected_index_json_template = json_encode($selected_index_json_template, JSON_PRETTY_PRINT);
      //$selected_index_json_template = drupal_json_output($selected_index_json_template);
      //dpm(drupal_json_output($selected_index_json_template));
      //$selected_index_json_template = preg_replace('/\\\n/', '<br />', $selected_index_json_template);
      //$selected_index_json_template = json_decode($selected_index_json_template);
    }
  } else {
    $selected_index_json_template = 'Please select a template';
  }

  $form['index_json_template_content'] = array(
    '#type' => 'item',
    '#prefix' => '<div id="index_json_template_content">',
    '#suffix' => '</div>',
    '#markup' => $selected_index_json_template, 
  );
 
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}



/**
 * index_json_template_content ajax callback
 */
function index_json_template_content_ajax ($form, $form_state) {
  return $form['index_json_template_content'];
}


/**
 * tripal_elasticsearch_create_index_from_templates_form submit callback
 */
function tripal_elasticsearch_create_index_from_templates_form_submit ($form, &$form_state) {
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $file_path = drupal_get_path('module', 'tripal_elasticsearch').'/elasticsearch_indices_json_templates/'.$form_state['values']['index_json_templates'];
  $selected_index_json_template = file_get_contents($file_path);
  $params = json_decode($selected_index_json_template, true);
  try{
    $client->indices()->create($params);
  } catch (\Exception $e) {
    $message = $e->getMessage();
    drupal_set_message($message, 'warning');
  }
}
