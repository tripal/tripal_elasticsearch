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
    $index = 'index_website';

	//if(in_array($index, $indices)){
	    $field = 'node_content';
	    $searchMethod = $form_state['values']['search_methods'];
	    $keyword = $form_state['values']['search_box'];

	    //--------------------run elastic search----------------------------------
    	// Build query statement, just need to replace some of this query string
    	// with the input form data
    	$body_curl_head = '{';
    	$body_head =  '"query" : {"bool" : {"must" : [';
		$keyword = trim($keyword);
		$body_content = _build_elastic_query($searchMethod, $field, $keyword);
		$highlight = '"highlight":{"pre_tags":["<em><b>"], "post_tags":["</b></em>"], "fields":{"node_content":{"fragment_size":150}}}';
		$body_end = ']}}';
		$body_curl_end = '}';
		
	
        $params['index'] = $index;
        $params['type'] = $index;

		// Get the hits count. Note that $client->count does not support hightlights.
		unset($_SESSION['websearch_count']);
		$body_for_count = $body_curl_head.$body_head.$body_content.$body_end.','.$body_curl_end;
		$_SESSION['websearch_count'] = $client->count($params)['count'];

		// display the first 1000 records for performance.
	    $params['body'] = $body_curl_head.$body_head.$body_content.$body_end.','.$highlight.$body_curl_end;
		$params['size'] = 1000;
	    $search_results = $client->search($params);	


		foreach($search_results['hits']['hits'] as $key=>$value){
			if(!empty($value)){
				$node_id = $value['_source']['node_id'];
				$node_title = $value['_source']['node_title'];
				$node_content = implode('......', $value['highlight']['node_content']);
				$node_content = strip_tags($node_content, '<em><b>');

				$homepage_search_output[$key]['node_id'] = $node_id;
				$homepage_search_output[$key]['node_title'] = $node_title;
				$homepage_search_output[$key]['node_content'] = $node_content;
			}
		}
	//}

	// if no keywords input or no records were found, search output is set to be empty
	if(empty($form_state['values']['search_box']) or empty($homepage_search_output)){
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
		$title = '<h6><span style="color:red"><em>'.$_SESSION['websearch_count'].'</em></span> records were found</h6>';
		//$header['title'] = array('data'=>'title', 'field'=>'title');
		//$header['content'] = array('data'=>'content', 'field'=>'content');
		

		foreach($_SESSION['homepage_search_output'] as $value){
			
			$row = '<h5>'.l($value['node_title'], 'node/'.$value['node_id']).'</h5>';
			$row .= '<p>'.$value['node_content'].'</p>';
			$rows[] = array('row' =>$row);
		}	
		$per_page = 10;
		$current_page = pager_default_initialize(count($rows), $per_page);
		// split list into page sized chunks
		$chunks = array_chunk($rows, $per_page, TRUE);
		// show the appropriate items from the list
		$output .= theme('table', array('header'=>array(), 'rows'=>$chunks[$current_page]));
		$output .= theme('pager', array('quantity', count($rows)));
		$output = $title.$output;
		
		// unset $_SESSION['homepage_search_output']
		//unset($_SESSION['homepage_search_output']);
	} else {
		$output .= '<h6>No records were found</h6>';
	}

	return $output;
}














