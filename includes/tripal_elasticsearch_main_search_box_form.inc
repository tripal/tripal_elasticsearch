<?php

/** elastic_search_homepage_search_box_form handler**/
function main_search_box_form($form, &$form_state){
  $form['search'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['search']['search_box'] = array(
    '#type' => 'textfield',
    '#size' => 40,
  );  

  $form['search']['search_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );


  return $form;
}



/** elastic_search_homepage_search_box_form submit **/
function elastic_search_homepage_search_box_form_submit($form, &$form_state){
  
    $client = new Elasticsearch\Client();
    $mappings = $client->indices()->getMapping();
    $indices = array_keys($mappings);
    $index = 'index_website';

  //if(in_array($index, $indices)){
      $field = 'node_content';
      //$searchMethod = $form_state['values']['search_methods'];
    $searchMethod = 'query_string';
      $keyword = $form_state['values']['search_box'];

      //--------------------run elastic search----------------------------------
      // Build query statement, just need to replace some of this query string
      // with the input form data
      $body_curl_head = '{';
      $body_head =  '"query" : {"bool" : {"must" : [';
    $elastic_special_chars = array('+', '-', '=', '&&', '||', '>', '<', '!', '(', ')', '{', '}', '[', ']', '^', '"', '~', '*', '?', ':', '\\', '/');
    // trim all whitespace at the beginning and the end
    $keyword = trim($keyword);
    // get the first and the last letter and see if there are '"'.
    $first_2_letters = substr($keyword, 0, 1);
    $last_2_letters = substr($keyword, -1, 1);
    if($first_2_letters == '"' and $last_2_letters == '"'){
      // if the beginning and the end letters are '"', this is assumed as a phrase search
      $keyword = str_replace($elastic_special_chars, ' ', $keyword);
      $keyword = '\"'.$keyword.'\"';
    }
    else{
      // if it is not a phrase search, remove all special characters.
      $keyword = str_replace($elastic_special_chars, ' ', $keyword);
    }
    $body_content = _build_elastic_query($searchMethod, $field, $keyword);
    $highlight = '"highlight":{"pre_tags":["<em><b>"], "post_tags":["</b></em>"], "fields":{"node_content":{"fragment_size":150}}}';
    $body_end = ']}}';
    $body_curl_end = '}';
    
  
        $params['index'] = $index;
        $params['type'] = $index;
    // display the first 1000 records for performance.
      $params['body'] = $body_curl_head.$body_head.$body_content.$body_end.','.$highlight.$body_curl_end;
    $params['size'] = 100;
      $search_results = $client->search($params);  


        $params_count['index'] = $index;
        $params_count['type'] = $index;
        // get hits count. remove 'highlight' settings.
        $params_count['body'] = $body_curl_head.$body_head.$body_content.$body_end.$body_curl_end;
        $_SESSION['websearch_count'] = $client->count($params_count)['count'];


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
  unset($_SESSION['homepage_search_output']);
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
    $title = '<h6><span style="color:red"><em>'.$_SESSION['websearch_count'].'</em></span> records were found. The first ';
    $title .= '<span style="color:red"><em>'. count($_SESSION['homepage_search_output']).'</em></span> records were displayed.</h6>';
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














