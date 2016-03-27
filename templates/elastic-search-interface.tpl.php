<?php
/**
	<h3>Elasticsearch: transcripts</h3>
	<div style="padding:10px"><?php print $organism; ?></div>
	<div style="padding:10px"><?php print $uniquename; ?></div>
	<div style="padding:10px"><?php print $seqtype; ?></div>
	<div style="padding:10px"><?php print $blast_hit_description; ?></div>
	<div><?php print $search_transcripts_submit; ?></div>
	
	<br/>
	<br/>
	<h3>Elasticsearch: webpages</h3>
	<div style="padding:10px"><?php print $search_webpages; ?></div>
	<div><?php print $search_webpages_submit; ?></div>
*/
?>


	<div class="elastic_search_interface_form"><?php print $children; ?></div>
	
	<?php if(!empty($from_nth_entry_nth)) {
		print '<div class="results_info">';
					print $from_nth_entry_nth;
					print $from_nth_entry_submit;
		print '</div>';
		print '<p id="records-found">';
		print '<span style="color:#ff0000">'.$search_record_count. '</span> records were found'; 
		print '</p>';
		print '<hr/>';
	} ?>
