<?php

function delete_elastic_indexed_tables_form($form, &$form_state){
	$client = new Elasticsearch\Client();
	$mappings = $client->indices()->getMapping();
	$indices = array_keys($mappings);
	$options = array();
	foreach($indices as $index){
		$options[$index] = $index;
	}

	$form['elastic_indices'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Elastic indices'),
		'#options' => $options,
	);

	$form['delete_indices'] = array(
		'#type' => 'submit',
		'#value' => t('Delete indexed tables'),
		'#prefix' => '<div id="delete-indices-submit">',
		'#suffix' => '</div>',
	);

	$form['#submit'][] = '_delete_elastic_indexed_tables_submit';


	return $form;
}



/** delete_elastic_indexed_tables_form submit callback **/
function _delete_elastic_indexed_tables_submit($form, &$form_state){
    $client = new Elasticsearch\Client();
    foreach($form_state['values']['elastic_indices'] as $index){
        if(!empty($index)){
            $params['index'] = $index;
            $client->indices()->delete($params);

            /* delete all search blocks that depend on the delete indices */
            $elastic_search_blocks = variable_get('elastic_search_blocks');
            foreach($elastic_search_blocks as $block_name=>$value){
                $indexed_table = element_children($value)[0];
                $indexed_table = explode('|', $indexed_table)[0];
                if($index == $indexed_table){
                    unset($elastic_search_blocks[$block_name]);
                    variable_set('elastic_search_blocks', $elastic_search_blocks);
                }
            }
        }
    }

    $form_state['redirect'] = 'admin/config/search/elastic_search/delete_elastic_indexed_tables';
}
