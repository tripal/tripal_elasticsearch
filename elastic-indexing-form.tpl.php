<?php




	print '<div id="elastic_indexing_form">';
	print $variables['elastic_indexing_form']['indexing_table'];
//	print $variables['elastic_indexing_form']['indexing_fields'];
//	print $variables['elastic_indexing_form']['submit'];

	print $variables['elastic_indexing_form']['indexed_block'];
	// Hidden elements are very important. If I didn't print the hidden elements,
	// the ajax would not work!
	print $variables['elastic_indexing_form']['hidden'];
	print '</div>';

