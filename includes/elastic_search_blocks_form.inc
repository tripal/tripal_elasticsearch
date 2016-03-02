<?php



function elastic_search_blocks_form($form, &$form_state){
	$form = $_SESSION['elastic_search_blocks'];
	return $form;
}
