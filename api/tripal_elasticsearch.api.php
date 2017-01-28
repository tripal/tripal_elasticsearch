<?php


function sort_2d_array_by_value(&$arr, $field, $order){
usort(
    $arr,
    function ($a, $b) use ($field, $order) {
      if ($order === 'asc') {
        return $a[$field] > $b[$field];
      }
      else {
        return $b[$field] > $a[$field];
      }
    }
  );
return $arr;
}

/**
 * Define a function to sort two dimensional array by values
 *
function sort_2d_array_by_value($arr, $field, $sort){
  $sorted_a = array();
  foreach($arr as $k=>$v){
    // create an array to store the field values
    $field_array[$k] = $v[$field];
  }

  if($sort == 'asc'){
    asort($field_array);
  }
  else{
    arsort($field_array);
  }

  foreach(array_keys($field_array) as $k){
    $sorted_a[] = $arr[$k];
  }

  return $sorted_a;
}*/
//================End of sort_2d_array_by_value() =====================


/**
 * Define a function to get the primary key of a table
 */
/*
function get_primary_key($table_name){
  if(in_array($table_name, get_chado_table_list())){
    $table = 'chado.'.$table_name;
    $primary_key_sql = 'SELECT a.attname
              FROM   pg_index i
              JOIN   pg_attribute a ON a.attrelid = i.indrelid
                               AND a.attnum = ANY(i.indkey)
              WHERE  i.indrelid = \''.$table.'\'::regclass
              AND    i.indisprimary;';

    $primary_key = db_query($primary_key_sql)->fetchAssoc();
  }
  else{
    $table = $table_name;
    $primary_key_sql = 'SELECT a.attname
              FROM   pg_index i
              JOIN   pg_attribute a ON a.attrelid = i.indrelid
                         AND a.attnum = ANY(i.indkey)
              WHERE  i.indrelid = \''.$table.'\'::regclass
              AND    i.indisprimary;';

    $primary_key = db_query($primary_key_sql)->fetchAssoc();
  }


  if(is_array($primary_key)){
    $primary_key = implode($primary_key);
  }

  return $primary_key;

}//============== End of primary key function ================================
*/





/**
 * This function return an array containing tables from the chado schema
 */
function get_chado_table_list() {

    $sql_table_list = "SELECT table_name FROM information_schema.tables WHERE table_schema = 'chado' ORDER BY table_name;";
    $result_table = db_query($sql_table_list);
    $input_table = $result_table->fetchAll();


    $table_list = array();
    $i = 0;
    foreach ($input_table as $value) {
        $table_list[$i] = $value->table_name;
        $i++;
    }

    return $table_list;
}


/**
 * This function takes a table name and return all the column names
 */
function get_column_list($table_name) {
    $sql_column_list = "SELECT column_name FROM information_schema.columns WHERE (table_schema = 'public'OR table_schema = 'chado') AND table_name = :selected_table;";
    $result_column = db_query($sql_column_list, array(':selected_table' => $table_name));
    $input_column = $result_column->fetchAll();
    $column_list = array();

    $k = 0;
    foreach($input_column as $value) {
        $column_list[$k] = $value->column_name;
        $k++;
    }

    return $column_list;
}

/**
 * This function return an array containing a list of table names from the public OR chado schema.
 * All the tables are from the chado schema or public schema.
 */
function get_table_list() {

    $sql_table_list = "SELECT table_name FROM information_schema.tables WHERE (table_schema = 'public' OR table_schema = 'chado') ORDER BY table_name;";
    $result_table = db_query($sql_table_list);
    $input_table = $result_table->fetchAll();


    $table_list = array('index_website');
    $i = 1;
    foreach ($input_table as $value) {
        $table_list[$i] = $value->table_name;
        $i++;
    }

    return $table_list;
}



/**
 * This function transforms an object to an array resursively.<br/>
 */
function objectToArray($d) {
    if (is_object($d)) {
        // Gets the properties of the given object
        // with get_object_vars function
        $d = get_object_vars($d);
    }

    if (is_array($d)) {
        /*
        * Return array converted to object
        * Using __FUNCTION__ (Magic constant)
        * for recursive call
        */
        return array_map(__FUNCTION__, $d);
    }
    else {
        // Return array
        return $d;
    }
}

/**
 * This function transform a nested array to a flattened array.
 */
function flatten($arr, $prefix = '') {
        $out = array();
        foreach ($arr as $key => $value) {
                $key = (!strlen($prefix)) ? $key : "{$prefix}_$key";
                if (is_array($value)) {
                        $out += flatten($value, $key);
                } else {
                        $out[$key] = $value;
                }
        }

        return $out;
}




/*
 * Build elatic search queries
 */
function _build_elastic_search_query($field, $keyword, $searchMethod='query_string'){
  $query_string_template = ' {"'.$searchMethod.'":{"default_field":"_field_", "query":"_keyword_", "default_operator":"OR"}} ';
  $search = array("_field_", "_keyword_");
  $replace = array($field, $keyword);
  $query = str_replace($search, $replace, $query_string_template);

  return $query;
}



/*
 * Escape special characters for elasticsearch
 */
function _remove_special_chars($keyword){
  /*
  $elastic_special_chars = array('+', '-', '=', '&&', '||', '>',
                                 '<', '!', '(', ')', '{', '}', '[',
                                 ']', '^', '"', '~', '*', '?', ':', '\\', '/');


  $keyword = trim($keyword);
  // Check if $keyword starts and ends with double quotations
  $start = substr($keyword, 0, 1);
  $end = substr($keyword, -1, 1);
  $keyword = str_replace($elastic_special_chars, ' ', $keyword);
  if($start == '"' and $end == '"'){
    $keyword = '\"'.$keyword.'\"';
  }
  */

  $keyword = str_replace('"', '\"', $keyword);
  return $keyword;
}



/*
 * This function takes form input and return an array, of which
 * the keys are field names and values are corresponding keywords.
 */
function _get_field_keyword_pairs($form_input){
  $table = array_keys($form_input)[0];
  /*
   * remove search_from parameter
  $search_from = $form_input[$table]['search_from'];
  unset($form_input[$table]['search_from']);
  */
  $field_keyword_pairs = $form_input[$table];
  return array('table'=>$table, 'field_keyword_pairs'=>$field_keyword_pairs, 'search_from'=>0);
}



/*
 * Takes in a table name and field-keyword pairs array and run elasticsearch for site wide search
 */
function _run_elastic_main_search($table, $field_keyword_pairs, $size=100){

  $body_curl_head = '{';
  $body_boolean_head = '"query" : {"bool" : {"must" : [';
  $body_boolean_end = ']}}';
  $body_curl_end = '}';

  $body_query_elements = array();
  foreach($field_keyword_pairs as $field=>$keyword){
    //Put queries in an array
    if(!empty($keyword)){
      $keyword = _remove_special_chars($keyword);
      $body_query_elements[] = _build_elastic_search_query($field, $keyword);
    }
  }
  $body_query = implode(',', $body_query_elements);
  $body = $body_curl_head.$body_boolean_head.$body_query.$body_boolean_end.$body_curl_end;

  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $params = array();
  $params['index'] = $table;
  $params['type'] = $table;
  $params['body'] = $body;
  try{
    $search_hits_count = $client->count($params)['count'];
  
    $highlight = '"highlight":{"pre_tags":["<em><b>"], "post_tags":["</b></em>"], "fields":{"node_content":{"fragment_size":150}}}';
    $body = $body_curl_head.$body_boolean_head.$body_query.$body_boolean_end.','.$highlight.$body_curl_end;
    $params['body'] = $body;
    $params['size'] = $size;
    $search_results = $client->search($params);
  
    $main_search_hits= array();
    foreach($search_results['hits']['hits'] as $key=>$value){
        if(!empty($value)){
          $node_id = $value['_source']['node_id'];
          $node_title = $value['_source']['node_title'];
          $node_content = implode('......', $value['highlight']['node_content']);
          $node_content = strip_tags($node_content, '<em><b>');
  
          $main_search_hits[$key]['node_id'] = $node_id;
          $main_search_hits[$key]['node_title'] = $node_title;
          $main_search_hits[$key]['node_content'] = $node_content;
        }
    }
  
    return array('search_hits_count'=>$search_hits_count, 'main_search_hits'=>$main_search_hits);
  } catch (\Exception $e) {
    $message = $e->getMessage();
    $search_hits_count = 'Error';
    $main_search_hits = $message;
    return array('search_hits_count'=>$search_hits_count, 'main_search_hits'=>$main_search_hits);
  }

}




/*
 * Takes in a table name and field-keyword pairs array and run elasticsearch
 */
function _run_elastic_search($table, $field_keyword_pairs, $from=0, $size=1000){

  $body_curl_head = '{';
  $body_boolean_head = '"query" : {"bool" : {"must" : [';
  $body_boolean_end = ']}}';
  $body_curl_end = '}';

  $body_query_elements = array();
  foreach($field_keyword_pairs as $field=>$keyword){
    //Put queries in an array
    if(!empty($keyword)){
      $keyword = _remove_special_chars($keyword);
      $body_query_elements[] = _build_elastic_search_query($field, $keyword);
    }
  }
  $body_query = implode(',', $body_query_elements);
  $body = $body_curl_head.$body_boolean_head.$body_query.$body_boolean_end.$body_curl_end;

  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $params = array();
  $params['index'] = $table;
  $params['type'] = $table;
  $params['body'] = $body;
  try{
    $search_hits_count = $client->count($params)['count'];
  
    $params['from'] = $from;
    $params['size'] = $size;
    $search_results = $client->search($params);
  
    $search_hits= array();
    foreach($search_results['hits']['hits'] as $key=>$value){
        foreach($field_keyword_pairs as $field=>$keyword){
	  //if(array_key_exists($key, $search_hits)) {
              $search_hits[$key][$field] = $value['_source'][$field];
	  //}
        }
    }
  
    return array('search_hits_count'=>$search_hits_count, 'search_hits'=>$search_hits, 'search_results'=>$search_results);
  } catch (\Exception $e) {
    $message = $e->getMessage();
    $search_hits_count = 'Error';
    $search_hits = $message;
    $search_results = Null;
    return array('search_hits_count'=>$search_hits_count, 'search_hits'=>$search_hits, 'search_results'=>$search_results);
  }

}


/*
 * This function takes in the search_hits array from elastic main search and
 * returns a themed table.
 */
function get_main_search_hits_table($main_search_hits, $main_search_hits_count){
  $output = '';
  if(!empty($main_search_hits)){
    $title = '<h6><span style="color:red"><em>'.$main_search_hits_count.'</em></span> records were found.';
    foreach($main_search_hits as $value){
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
  }
  else{
    $output .= '<h6>No records were found</h6>';
  }

  return $output;
}


/*
 * This function takes in the search_hits array and
 * return a themed table.
 */
function get_search_hits_table($search_hits, $table){
  // Get table header
  $elements = array_chunk($search_hits, 1);
  //dpm($elements);
  $header = array();
  foreach($elements[0] as $value){
    foreach(array_keys($value) as $field){
      $header[] = array('data'=>$field, 'field'=>$field);
    }
  }

  if(isset($_GET['sort']) and isset($_GET['order'])){
    $sorted_hits_records = sort_2d_array_by_value($search_hits, $_GET['order'], $_GET['sort']);
  }
  else{
    // By default, the table is sorted by the first column by ascending order.
    $sorted_hits_records = sort_2d_array_by_value($search_hits, $header[0]['field'], 'asc');
  }

  //Get table rows
  foreach($sorted_hits_records as $values){
    // add links to search results
    $records = db_query('SELECT DISTINCT(table_field), page_link FROM tripal_elasticsearch_add_links WHERE table_name=:table_name', array(':table_name'=>$table))
               ->fetchAll();
    $row = $values;
    foreach($records as $record){
      preg_match_all('/\[.+?\]/', $record->page_link, $matches);
      $pattern = array();
      $replace = array();
      foreach($matches[0] as $match){
        $field = str_replace('[', '', $match);
        $field = str_replace(']', '', $field);
        $pattern[] = $match;
        $replace[] = $values[$field];
      }
      $link = str_replace($pattern, $replace, $record->page_link);
      $row[$record->table_field] = l($values[$record->table_field], $link);
    }
    $rows[] = array_values($row);
  }

  $per_page = 10;
  $current_page = pager_default_initialize(count($rows), $per_page);
  $chunks = array_chunk($rows, $per_page, TRUE);
  $output = theme('table', array('header' => $header, 'rows' => $chunks[$current_page], 'attributes' => array('id' => 'elasticsearch_hits_table')));
  $output .= theme('pager', array('quantity', count($rows)));

  return $output;
}



/*
 * Test if a string is an elasticsearch index
 */
function is_elastic_index($index){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $mappings = $client->indices()->getMapping();
  $indices = array_keys($mappings);
  $res = false;
  if(in_array($index, $indices)){
    $res = true;
  }

  return $res;
}


function run_elasticsearch_indexing($table_dropdown, $queue_N, $fields){
  //Get selected table
  $table_list = get_table_list();
  $selected_table_key = $table_dropdown;
  $selected_table = $table_list[$selected_table_key];

  // Get selected fields=============
  $field_list = get_column_list($selected_table);
  $selected_fields = array();

  if(!empty($fields)){
    foreach($fields as $key){
      // check if $key is an alphanumeric character.
      if(ctype_alnum($key)){
        $selected_fields[] = $field_list[$key];
      }
    }
  }

  // Add 'chado.' to the front of table name if it is a chado table
  if(in_array($selected_table, get_chado_table_list())){
    $selected_table = 'chado.' . $selected_table;
  }

  // separate the selected fields by ','.
  // and insert this string into the sql query statement.
  $comma_separated_fields = implode(", ", $selected_fields);


  // get row count of the selected table
  if($selected_table == 'index_website'){
    $sql_rowcount = "SELECT COUNT(*) FROM node;";
  }
  else {
    $sql_rowcount = "SELECT COUNT(*) FROM ".$selected_table.";";
  }
  $row_count = db_query($sql_rowcount)->fetchAssoc();
  $row_count = implode($row_count);


  // fetch 1000 rows for a single database query
  if($selected_table == 'index_website'){
    // It's better to set a small $k when the doc to be index is very large, e.g. a whole webpage.
    $k = 1000;
  }
  else {
    $k = 1000;
  }
  // the number of database queries for a selected table
  $queue_N = $queue_N; // split items into N queues
  $n = intval($row_count/$k);
  $queue = DrupalQueue::get('elastic_queue');
  $offsets = range(0, $n);
  foreach($offsets as $offset){
    $queue_n = $offset % $queue_N;
    $queue = DrupalQueue::get('elastic_queue_'.$queue_n);
    $offset = $k*$offset;
    $item = new stdClass();
    if($selected_table == 'index_website'){
      // select only published nodes
      $sql = "SELECT title, nid FROM node WHERE status=1 ORDER BY nid LIMIT $k OFFSET $offset;";
    }
    else {
      // Use the first field to order the database table when implement queries.
      if (!empty($selected_fields)) {
        $order_by_field = $selected_fields[0];
      } else if (!empty($field_list)) {
        $order_by_field = $field_list[0];
      }

      $sql = "SELECT ". $comma_separated_fields. " FROM " . $selected_table ." ORDER BY $order_by_field LIMIT $k OFFSET $offset;";
    }
    $item->sql = $sql;
    // also store selected table
    $item->selected_table = $selected_table;
    $queue->createItem($item);
  }
}

function tripal_elasticsearch_add_block($indexed_table, $fields){

  $record = array();
  // Delete the table and its fields in the database if that table name already exists
  $delete_table_name = db_delete('tripal_elasticsearch')
              ->condition('table_name', $indexed_table)
              ->execute();
  $i = 0;
  foreach($fields as $field){
    if(!empty($field)){
      $i++;
      $record['table_name'] = $indexed_table;
      $record['table_field'] = $field;
      $record['form_field_type'] = 'textfield';
      $record['form_field_title'] = $field;
      $record['form_field_default_value'] = '';
      $record['form_field_options'] = '';
      $record['form_field_weight'] = $i;
      // write record into the elastic_search table in database
      drupal_write_record('tripal_elasticsearch', $record);
    }
  }
}

function tripal_elasticsearch_add_links($table_name, $fields) {
  $delete_table_name = db_delete('tripal_elasticsearch_add_links')
              ->condition('table_name', $table_name)
              ->execute();
  $columns = db_query('SELECT table_field FROM tripal_elasticsearch WHERE table_name=:table_name', array(':table_name'=>$table_name))
                 ->fetchAll();
  $record = array();
  foreach($columns as $field){
    $field = $field->table_field;
    if(!empty($fields[$field])){
      $record['table_name'] = $table_name;
      $record['table_field'] = $field;
      $record['page_link'] = $fields[$field];
    }
    drupal_write_record('tripal_elasticsearch_add_links', $record);
  }

}



/*
 * Return cluster health information into an array. If no alive nodes found, this function returns a message.
 */
function get_cluster_health(){
  $client = Elasticsearch\ClientBuilder::create()->setHosts(variable_get('elasticsearch_hosts', array('localhost:9200')))->build();
  $params['v'] = true;
  //$params['help'] = true;
  try{
    $client_health = $client->cat()->health($params);
    $client_health = preg_split('/\s+/', $client_health);
    foreach(range(0, 13) as $i){
      $client_health_arr[$client_health[$i]] = $client_health[$i+14];
    }
    $output = "<h2>Elasticsearch cluster health information:</h2>\n";
    $output .= '<ul>';
    foreach($client_health_arr as $key=>$value){
      $output .= "<li><b>$key:</b> $value</li>";
    }
    $output .= '</ul>';
  } catch (\Exception $e) {
    $message = $e->getMessage();
    $output = "<h2><font color='red'>$message. Please check if your elasticsearch cluster is running normally.</font></h2>";
  }

  return $output;
}
