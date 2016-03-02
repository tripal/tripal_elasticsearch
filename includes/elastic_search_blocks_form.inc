<?php



function elastic_search_blocks_form($form, &$form_state){
	foreach($_SESSION['elastic_search_blocks'] as $key => $value){
		$form[$key] = $value;
		$form[$key]['#title'] = 'Elastic_search_block_'.$key;
		$form[$key]['#tree'] = TRUE;
		$form[$key]['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Delete',
			'#submit' => array('_delete_elastic_block'),
		);
	}
	return $form;
}




/**
 *elastic_search_blocks_form submit callback
*/
function _delete_elastic_block($form, &$form_state){
	drupal_set_message('block deleted');
	dpm($form_state['values']);
}
