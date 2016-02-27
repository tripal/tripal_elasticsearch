<?php


    print '<div id="elastic_search_form">';
	print '<hr>';
    print $variables['elastic_search_form']['elastic_indexed'];
	print $variables['elastic_search_form']['elastic_search'];
	print $variables['elastic_search_form']['all_selected_fields'];
	print $variables['elastic_search_form']['build_search_boxes'];
    // Hidden elements are very important. If I didn't print the hidden elements,
    // the ajax would not work!
    print $variables['elastic_search_form']['hidden'];
//	print $variables['elastic_search_form']['table'];
    print '</div>';
	print '<hr>';
