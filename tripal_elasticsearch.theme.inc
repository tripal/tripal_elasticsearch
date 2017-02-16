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
    $search_res = $variables['database_table_search_results'];
    $field_content_pairs = $variables['field_content_pairs'];

    $per_page = 10;
    foreach ($field_content_pairs as $field) {
        $header[] = [
            'data' => $field,
            'field' => $field,
        ];
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
    $website_search_results = $variables['website_search_results'];
    //dpm($website_search_results);
    $rows = [];
    foreach ($website_search_results as $record) {
        //dpm($record);
        $nid = $record['nid'];
        $title = '<h1>' . l($record['title'], 'node/' . $nid) . '</h1>';
        $body = '<p>' . $record['body'] . '</p>';
        $rows[] = [
            'row' => $title . $body,
        ];
    }

    $per_page = 10;
    $current_page = pager_default_initialize(count($rows), $per_page);
    $chunks = array_chunk($rows, $per_page, TRUE);
    $output = theme('table', array('header' => [], 'rows' => $chunks[$current_page]));
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
        $categories[] = l($text, 'website-search/' . $type . '?sitewide_search_box=' . $search_content);
    }
    $output = theme('item_list', array('items' => $categories));

    return $output;
}