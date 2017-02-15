<?php

function sitewide_search_box_form ($form, &$form_state) {
    $form['sitewide_search_box'] = [
        '#type' => 'textfield',
    ];


    $form['#method'] = 'get';
    $form['submit'] = [
        '#type' => 'submit',
        '#value' => t('Search'),
    ];


    return $form;
}