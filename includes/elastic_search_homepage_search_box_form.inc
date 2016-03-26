<?php

/** elastic_search_homepage_search_box_form handler**/
function elastic_search_homepage_search_box_form($form, &$form_state){
	$form['search_methods'] = array(
		'#type' => 'select',
		'#options' => array(
			'match' => t('match'),
			'fuzzy' => t('fuzzy match'),
			'match_phrase' => t('phrase match'),
		),
	);
	$form['search_box'] = array(
		'#type' => 'textfield',
	);	

	$form['search_submit'] = array(
		'#type' => 'submit',
		'#value' => t('Elasticsearch'),
		//'#submit' => array('_homepage_search_box_form_submit'),
	);


	return $form;
}


/** hook_preprocess_theme() implementation**/
function elastic_search_preprocess_elastic_search_homepage_search_box_form(&$variables){

	$form = $variables['form'];
	if($form){
		drupal_add_css(drupal_get_path('module', 'elastic_search').'/css/homepage-search-box.css');
	}

	$variables['search_methods'] = render($form['search_methods']);
	$variables['search_box'] = render($form['search_box']);
	$variables['search_submit'] = render($form['search_submit']);	

	$variables['children'] = drupal_render_children($form);
	
}

/** elastic_search_homepage_search_box_form submit **/
function elastic_search_homepage_search_box_form_submit($form, &$form_state){
	

    $client = new Elasticsearch\Client();
    $mappings = $client->indices()->getMapping();
    $indices = array_keys($mappings);
    $index = 'field_data_body';


	if(in_array($index, $indices)){
	    $field = 'body_value';
	    $searchMethod = $form_state['values']['search_methods'];
	    $keyword = $form_state['values']['search_box'];

	    //--------------------run elastic search----------------------------------
    	// Build query statement, just need to replace some of this query string
    	// with the input form data
    	$body_curl_head = '{';
    	$body_head =  '"query" : {"bool" : {"must" : [';
		$body_content = _build_elastic_query($searchMethod, $field, $keyword);
		$highlight = '"highlight":{"pre_tags":["<em><b>"], "post_tags":["</b></em>"], "fields":{"body_value":{"fragment_size":150}}}';
		$body_end = ']}}';
		$body_curl_end = '}';
		
        $params['index'] = $index;
        $params['type'] = $index;
	    $params['body'] = $body_curl_head.$body_head.$body_content.$body_end.','.$highlight.$body_curl_end;
		$params['size'] = 10000;
	    $search_results = $client->search($params);	
		foreach($search_results['hits']['hits'] as $key=>$value){
			$entity_id = $value['_source']['entity_id'];
			$body_value = implode('......', $value['highlight']['body_value']);
			$body_value = strip_tags($body_value, '<em><b>');
			$homepage_search_output[$key]['entity_id'] = $entity_id;
			$homepage_search_output[$key]['body_value'] = $body_value;
		}
	}


	if(empty($form_state['values']['search_box'])){
		$_SESSION['homepage_search_output'] = '';
	} else {
		$_SESSION['homepage_search_output'] = $homepage_search_output;
	}

	$form_state['redirect'] = 'search-website';
}



/** function to return homepage elasticsearch output **/
function _homepage_elastic_search_output(){
	$output = '';
	if(!empty($_SESSION['homepage_search_output'])){
		$record_count = count($_SESSION['homepage_search_output']);
		$title = '<h3><em>'.$record_count.'</em> records were found</h3>';
		$header['page'] = array('data'=>'page', 'field'=>'pages');
		$header['page link'] = array('data'=>'page link', 'field'=>'page link');
	

		foreach($_SESSION['homepage_search_output'] as $value){
			$item1 = $value['body_value'];
			$item2 = l(t('read more'), 'node/'.$value['entity_id']);
			$rows[] = array($item1, $item2);
		}	
	
		$per_page = 10;
		$current_page = pager_default_initialize(count($rows), $per_page);
		// split list into page sized chunks
		$chunks = array_chunk($rows, $per_page, TRUE);
		// show the appropriate items from the list
		$output .= theme('table', array('header'=>$header, 'rows'=>$chunks[$current_page]));
		$output .= theme('pager', array('quantity', count($rows)));
		$output = $title.$output;
		
		// unset $_SESSION['homepage_search_output']
		//unset($_SESSION['homepage_search_output']);
	} else {
		$output .= '<h3>No records were found</h3>';
	}

	return $output;
}














