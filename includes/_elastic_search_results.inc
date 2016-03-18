<?php



// callback function within the form's submit function
// the purpose is to redirect to another page to display the
// results after the form has been submitted.
function _elastic_search_results(){


/**
	$output = '<hr>';
	
	//get the first element of an array
	//get headers with clicking sorting
	$elements = array_chunk($_SESSION['search_output_table'], 1, 1);
	foreach($elements[0] as $value){
		foreach(array_keys($value) as $field){
			$header[] = array('data'=> $field, 'field'=>$field);
		}
	}


//++++++++++++++++++++++CURRENTLY WORKING+++++++++++++++++++++++
	// implementation of sort by clicking
	$sorted_output = $_SESSION['search_output_table']; // default is unsorted
	if(isset($_GET['sort']) && isset($_GET['order'])){
		$sorted_output = sort_2d_array_by_value($_SESSION['search_output_table'], $_GET['order'], $_GET['sort']);
	}
//+++++++++++++++++++++CURRENTLY WORKING++++++++++++++++++++++++++



	foreach($sorted_output as $values){
//		$header[] = array_keys($values);
		$values['uniquename'] = l($values['uniquename'], 'feature/'.$values['uniquename']); // add link to feature uniquename.
		$rows[] = array_values($values);
	}		

	$per_page = 10;
	// Initialize the pager
	$current_page = pager_default_initialize(count($rows), $per_page);
	// Split your list into page sized chunks
	$chunks = array_chunk($rows, $per_page, TRUE);
	// Show the appropriate items from the list	
	$output .= theme('table', array('header' => $header, 'rows' => $chunks[$current_page] ));
	$output .= theme('pager', array('quantity', count($rows)));			

*/
	return $output;

}//================ The end of callback function for results display ============
//+++++++++++++++++ The end of callback function for results display ++++++++++++




