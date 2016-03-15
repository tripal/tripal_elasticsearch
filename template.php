<?php


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//================================ Theme functions ==============================

/**
 * Implements hook_theme().
 */
function elastic_search_theme(){
    return array(
        'elastic_search_form' => array(
            'render element' => 'form',
			'path' => './templates',
            'template' => 'elastic-search-form',
        ),
        'elastic_indexing_form' => array(
            'render element' => 'form',
			'path' => drupal_get_path('theme', 'elastic_search') . '/templates',
            'template' => 'elastic-indexing-form',
        ),
    );
}

