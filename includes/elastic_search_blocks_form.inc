<?php

/** elastic_search_blocks form handler **/
function elastic_search_blocks_form($form, &$form_state, $elastic_search_blocks){
	//foreach($_SESSION['elastic_search_blocks'] as $key => $value){
	foreach($elastic_search_blocks as $key => $value){
		$form[$key] = $value;
		$form[$key]['#title'] = $key;
		$form[$key]['#tree'] = TRUE;
		$form[$key]['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Delete|'.$key,
			'#submit' => array('_delete_elastic_block'),
		);
	}

	return $form;
}


/**
 *elastic_search_blocks_form submit callback
*/
function _delete_elastic_block($form, &$form_state){
	$block_name = substr($form_state['values']['op'], 7);
	//unset($_SESSION['elastic_search_blocks'][$block_name]);
	$elastic_search_blocks = variable_get('elastic_search_blocks');
	unset($elastic_search_blocks[$block_name]);
	variable_set('elastic_search_blocks', $elastic_search_blocks);

	$form_state['rebuild'] = TRUE;
}






