<?php
// $Id$

/**
 * @file
 * Administrative forms for management of Elasticsearch indices.
 */

/**
 * render array for index_website_form
 */
function tripal_elasticsearch_indexing_form($form, &$form_state) {
	$form['queue_number'] = array(
		'#type' => 'select',
		'#title' => t('Number of cron queues'),
		'#description' => t('10 queues have been pre-defined for indexing jobs. This number determines the number of
                         queues will be used for indexing jobs. Use more queues if your server has high ability
                         for multi-threads process.'),
		'#options' => drupal_map_assoc(range(1,10)),
	);
	$form['index_name'] = array(
		'#type' => 'textfield',
		'#title' => t('Enter a unique Elasticsearch index name'),
		'#description' => t('Elasticsearch index name can only contain lowercase letters and underscores, 
												and start with letters'),
		'#size' => 25,
	);
	$form['website_or_table'] = array(
		'#type' => 'select',
		'#title' => t('Index website or database table'),
		'#options' => drupal_map_assoc(array('website', 'database table')),
		'#default_value' => 'website',
	);
	$form['index_table'] = array(
		'#type' => 'select',
		'#title' => t('Select a table to index'),
		'#options' => drupal_map_assoc( get_table_list() ),
		'#default_value' => 'node',
		'#states' => array(
			'visible' => array(
				':input[name="website_or_table"]' => array('value' => 'database table'),
			),
		),
		'#ajax' => array(
			'callback' => 'tripal_elasticsearch_table_fields_ajax_callback',
			'wrapper' => 'tripal_elasticsearch_table_fields_wrapper',
		),
	);

	$table_name = isset($form_state['values']['index_table']) ? $form_state['values']['index_table'] : 'node';
	$form['table_fields'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Select fields to index'),
		'#options' => drupal_map_assoc( get_column_list($table_name) ),
		'#states' => array(
			'visible' => array(
				':input[name="website_or_table"]' => array('value' => 'database table'),
			),
		),
		'#prefix' => '<div id="tripal_elasticsearch_table_fields_wrapper">',
		'#suffix' => '</div>',
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
	);
	return $form;
}


/**
 * AJAX callback for table fields
 */
function tripal_elasticsearch_table_fields_ajax_callback($form, &$form_state) {
	return $form['table_fields'];
}

/**
 * tripal_elasticsearch_indexing_form submission.
 */
function tripal_elasticsearch_indexing_form_submit($form, &$form_state) {
	dpm($form_state['values']);
}