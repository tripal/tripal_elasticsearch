<?php
// $Id$

/**
 * @file
 * Admin page callback for Tripal Elasticsearch module.
 */
function tripal_elasticsearch_admin_settings() {
	$form['elasticsearch_connection_settings'] = array(
		'#type' => 'textfield',
		'#title' => t('Elasticsearch Connection'),
		'#default_value' => variable_get('elasticsearch_connection_setting', array()),
		'#required' => TRUE,
	);
	return system_settings_form($form);
}