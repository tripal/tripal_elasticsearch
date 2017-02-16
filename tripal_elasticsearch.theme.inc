<?php
/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 2/15/17
 * Time: 2:35 PM
 */

/*
 * database_table_search_results theme hook
 */
function theme_database_table_search_results ($variables) {

    // get elastic query data
    $table_name = $variables['table_name'];
    $index = preg_replace('/^chado./', 'chado_', $table_name);
    $type = $table_name;
    $field_content_pairs = $_GET[$index];

    // get sort field
    if (isset($_GET['sort']) && in_array($_GET['order'], array_keys($field_content_pairs))) {
        $sort_field = $_GET['order'];
        $sort_direction = $_GET['sort'];
    } else {
        $sort_field = array_keys($field_content_pairs)[0];
        $sort_direction = 'asc';
    }

    // run elastic search
    $connection = (new ElasticConnection(["127.0.0.1:9201"]))->make();
    $elastic_search = new ElasticSearch($connection);
    $query = $elastic_search->build_search_query_from_field_content_pairs($field_content_pairs);
    $params = $elastic_search->build_table_search_params($index=$index, $type=$type, $query=$query, $from=0, $size=10000, $sort_field=$sort_field, $sort_direction=$sort_direction);
    $search_res = $elastic_search->table_search($params);

    $per_page = 10;
    foreach (array_keys($search_res[0]) as $field) {
        $header[$field] = [
            'data' => $field,
            'field' => $field,
        ];
    }

    if (empty($search_res)) {
        return 'No records were found.';
    }
    $rows = $search_res;
    $current_page = pager_default_initialize(count($rows), $per_page);
    $chunks = array_chunk($rows, $per_page, TRUE);
    $output = theme('table', array('header' => $header, 'rows' => $chunks[$current_page]));
    $output .= theme('pager', array('quantity', count($rows)));

    return $output;
}


/*
 *  theme_website_search_results theme hook
 */
function theme_website_search_results ($variables) {
    $search_content = $variables['search_content'];

    // run elastic search
    $connection = (new ElasticConnection(["127.0.0.1:9201"]))->make();
    $elastic_search = new ElasticSearch($connection);
    $params = $elastic_search->build_website_search_params($index = 'website', $type = 'website', $search_content = $search_content, $from = 0, $size = 1000);

    $count = $elastic_search->search_count($params);

    $website_search_results = $elastic_search->website_search($params);
    $rows = [];
    foreach ($website_search_results as $record) {
        //dpm($record);
        $nid = $record['nid'];
        $title = '<h1>' . l($record['title'], 'node/' . $nid) . '</h1>';
        $highlight = '<p>' . $record['highlight'] . '</p>';
        $rows[] = [
            'row' => $title . $highlight,
        ];
    }

    if (empty($rows)) {
        $output = "<h1>0 records were found</h1>";
        return $output;
    }
    $per_page = 10;
    $current_page = pager_default_initialize(count($rows), $per_page);
    $chunks = array_chunk($rows, $per_page, TRUE);
    $output = "<h1>$count records were found</h1>";
    $output .= theme('table', array('header' => [], 'rows' => $chunks[$current_page]));
    $output .= theme('pager', array('quantity', count($rows)));


    return $output;
}


/*
 * website_search_categories theme hook
 */
function theme_website_search_categories ($variables) {
    $website_search_results = $variables['website_search_results'];
    $search_content = $variables['search_content'];

    $node_types = [];
    foreach ($website_search_results as $item) {
        $node_types[] = $item['type'];
    }
    $node_type_by_count = array_count_values($node_types);
    $categories = [];
    foreach ($node_type_by_count as $type=>$count) {
        $text = "$type ($count)";
        $categories[] = l($text, 'website-search/' . $type . '?website_search_box=' . $search_content);
    }
    $output = theme('item_list', array('items' => $categories));

    return $output;
}