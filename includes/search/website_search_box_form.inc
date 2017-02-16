<?php

function website_search_box_form ($form, &$form_state) {
    $form['website_search_box'] = [
        '#type' => 'textfield',
    ];


    $form['#method'] = 'get';
    $form['submit'] = [
        '#type' => 'submit',
        '#value' => t('Search'),
    ];


    return $form;
}