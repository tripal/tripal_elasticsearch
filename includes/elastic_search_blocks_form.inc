<?php



function elastic_search_blocks_form($form, &$form_state){
	foreach($_SESSION['elastic_search_blocks'] as $key => $value){
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
	unset($_SESSION['elastic_search_blocks'][$block_name]);

	db_delete('elastic_search_interface')
		->condition('search_block_name', $block_name, '=')
		->execute();

	$form_state['rebuild'] = TRUE;
}

