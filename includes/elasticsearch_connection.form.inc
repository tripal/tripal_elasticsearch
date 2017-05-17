<?php
// $Id$

/**
 * @file
 * Admin page callback to build form for Elasticsearch connection.
 */
function elasticsearch_connection_form($form, &$form_state) {
	$form['host'] = array(
		'#type' => 'textfield',
		'#title' => 'Elasticsearch Server URL',
		'#description' => t('URL and port of an elasticsearch server. Examples: http://localhost:9200'),
		'#default_value' => 'http://localhost:9200',
		'#attributes' => array(
			'placehodler' => 'http://localhost:9200',
		),
	);

	$form['connect'] = array(
		'#type' => 'submit',
		'#value' => t('Connect'),
	);
	return $form;
}