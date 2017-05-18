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
												and start with letters.'),
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
		'#title' => t('Select a table and fields to index'),
		'#options' => array('' => 'Select a table') + drupal_map_assoc( get_table_list() ),
		'#states' => array(
			'invisible' => array(
				':input[name="website_or_table"]' => array('value' => 'website'),
			),
		),
		'#ajax' => array(
			'callback' => 'tripal_elasticsearch_table_fields_ajax_callback',
			'wrapper' => 'tripal_elasticsearch_table_fields_wrapper',
		),
	);

	$table_name = isset($form_state['values']['index_table']) ? $form_state['values']['index_table'] : '';
	$form['table_fields'] = array(
		'#type' => 'fieldset',
		'#title' => t('Select fields to index'),
		'#tree' => true,
		'#options' => drupal_map_assoc( get_column_list($table_name) ),
		'#states' => array(
			'invisible' => array(
				':input[name="index_table"]' => array('value' => ''),
			),
		),
		'#prefix' => '<div id="tripal_elasticsearch_table_fields_wrapper">',
		'#suffix' => '</div>',
	);

	foreach (get_column_list($table_name) as $field) {
		$form['table_fields'][$field] = array(
			'#type' => 'select',
			'#title' => t('Field name: '.$field),
			'#description' => t('Please select a mapping type each field. If no 
													mapping type is selected for a field, that field will
													no be indexed. This can be used to selectively index
													table fields.'),
			'#options' => array('' => 'Select mapping type') + get_field_mapping_types(),
		);
	}

//	$header = array(
//		'field' => 'Field',
//		'mapping_type' => 'Mapping Type',
//	);
//	$options = array();
//	foreach (get_column_list($table_name) as $field) {
//		$options[$field] = array(
//			'field' => $field,
//			'mapping_type' => theme('select', array(
//				'element' => array(
//					'#options' => get_field_mapping_types(),
//				),
//			)),
//		);
//	}

//	$form['table_fields'] = array(
//		'#type' => 'tableselect',
//		'#header' => $header,
//		'#options' => $options,
//		'#empty' => t('No content available.'),
//		'#states' => array(
//			'invisible' => array(
//				':input[name="website_or_table"]' => array('value' => 'website'),
//			),
//		),
//		'#prefix' => '<div id="tripal_elasticsearch_table_fields_wrapper">',
//		'#suffix' => '</div>',
//	);

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
	// get indexing data
	$queue_number = $form_state['values']['queue_number'];
	$index_name = $form_state['values']['index_name'];
	$website_or_table = $form_state['values']['website_or_table'];

	if ($website_or_table == 'website') {
		// indexing website
		$index_type = 'website';
		$index_table = 'node';
	} else {
		// indexing a table
		$index_table = $form_state['values']['index_table'];
		$index_type = $index_table;
		// Add schema prefix 'chado' if $index_table is a chado table.

		$field_mapping_types = array_filter($form_state['values']['table_fields']);
		dpm($field_mapping_types);
	}


	/**
	 * Bind the following data to a cron queue item.
	 * 		index_name
	 * 		index_type
	 * 		index_table
	 * 		selected_table_fields
	 * 		field_mapping_types
	 * 		sql query that will extract values from the database table for the selected table fields.
	 */

	// Get row count of selected table.
	$row_count = db_query("SELECT COUNT(*) FROM {$index_table}")->fetchAssoc()['count'];
	dpm($row_count);

	dpm($form_state['values']);
	$form_state['rebuild'] = TRUE;
}