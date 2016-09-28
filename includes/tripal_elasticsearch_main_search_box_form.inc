<?php

/** elastic_search_homepage_search_box_form handler**/
function main_search_box_form($form, &$form_state){
  $form['search'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('class' => array('container-inline')),
  );
  $form['search']['search_box'] = array(
    '#type' => 'textfield',
    //'#size' => 40,
  );

  $form['search']['search_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );


  return $form;
}


function main_search_box_form_submit($form, &$form_state){
  $_SESSION['main_search_keyword'] = $form_state['values']['search_box'];
  $form_state['redirect'] = 'search-website';
}
