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
		'#description' => t('URL and port of an elasticsearch server. Examples: http://localhost:9200 or http://127.0.0.1:9200'),
		'#default_value' => variable_get('elasticsearch_host'),
		'#attributes' => array(
			'placeholder' => 'http://localhost:9200',
		),
	);

	$form['connect'] = array(
		'#type' => 'submit',
		'#value' => t('Connect'),
	);

	$form['health'] = array(
		'#type' => 'markup',
		'#markup' => '<h4>'. t('Elasticsearch server health').'</h4>'.
			'<p>No running Elasticsearch server is connected. Please check your Elasticsearch server URL and port.</p>'
	);

	try{
		$elasticsearch_host = variable_get('elasticsearch_host');
		libraries_load('elasticsearch-php');
		$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();
		// obtain cluster health information
		$params['v'] = true;
		$health = $client->cat()->health($params)[0];

		// if $health is not empty, create a form element to display it.
		if (!empty($health)) {
			$header = array_keys($health);
			$rows[] = array_values($health);
			$output = theme('table', array('header'=>$header, 'rows'=>$rows));
			$form['health'] = array(
				'#type' => 'markup',
				'#title' => t('Elasticsearch server health'),
				'#description' => t('The table below shows the health information of connected Elasticsearch servers.'),
				'#markup' => '<h4>'. t('Elasticsearch server health').'</h4>'.
					           '<p>'.t('The table below shows the health of connected Elasticsearch servers.').'</p>'.
										 $output,
			);
		}
	} catch (\Exception $e) {
		$message = $e->getMessage();
	}

	return $form;
}

/**
 * elasticsearch_connection_form submit
 */
function elasticsearch_connection_form_submit($form, &$form_state) {
	$elasticsearch_host = $form_state['values']['host'];

	libraries_load('elasticsearch-php');
	$client = Elasticsearch\ClientBuilder::create()->setHosts(array($elasticsearch_host))->build();
	// update the elasticsearch_host variable after form submission.
	variable_set('elasticsearch_host', $elasticsearch_host);

}