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