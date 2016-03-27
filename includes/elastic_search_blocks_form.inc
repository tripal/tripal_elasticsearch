<?php

/** elastic_search_blocks form handler **/
function elastic_search_blocks_form($form, &$form_state){
	foreach($_SESSION['elastic_search_blocks'] as $key => $value){
	//foreach($elastic_search_blocks as $key => $value){
		$form[$key] = $value;
		$form[$key]['#title'] = $key;
		$form[$key]['#tree'] = TRUE;
	}

	$form['delete_blocks'] = array(
		'#type' => 'fieldset',
		'#title' => t('Delete search blocks'),
		'#tree' => TRUE,
	);

	$options = array();
	foreach(array_keys($_SESSION['elastic_search_blocks']) as $block){
		$options[$block] = $block;
	}
	$form['delete_blocks']['blocks'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Select search blocks to delete'),
		'#options' => $options,
	);
	$form['delete_blocks']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Delete'),
		'#submit' => array('_delete_elastic_block'),
	);


	return $form;
}


/**
 *elastic_search_blocks_form submit callback
*/
function _delete_elastic_block($form, &$form_state){

	$elastic_search_interface = array();
	foreach($form_state['values']['delete_blocks']['blocks'] as $key=>$block){
		if(!empty($block)){
			unset($_SESSION['elastic_search_blocks'][$block]);
		} else {
			$elastic_search_interface[$key] = $form[$key];
		}
	}
	variable_set('elastic_search_interface', $elastic_search_interface);

	
	$form_state['rebuild'] = TRUE;

}






