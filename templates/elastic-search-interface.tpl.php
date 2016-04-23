<?php
/**
	print '<div class="elastic_search_interface_form">';
	foreach(array_keys($elastic_search_interface) as $key){
		if($key != 'download_table' and $key != 'download_fasta'){
			print $elastic_search_interface[$key];
		}
	}
	print '</div>';

	print '<div style="display:none">';
	print $children;
	print '</div>';

	if(!empty($from_nth_entry_nth)) {
		print '<div class="results_info">';
		print $from_nth_entry_nth;
		print $from_nth_entry_submit;
		print '</div>';

		print '<p id="records-found">';
		print '<span style="color:#ff0000">';
		print $search_record_count;
		print '</span> records were found'; 
		print '</p>';
		
		print '<div class="download-table">';
		print $elastic_search_interface['download_table'];
		print $elastic_search_interface['download_fasta'];
		print '<div class="download-fasta">';

		print '<br/><br/>';
	}
*/
?>

<?php 
	print '<div class="elastic_search_interface_form">';
		foreach(array_keys($elastic_search_interface) as $key){ 
			if($key != 'download_table' and $key != 'download_fasta'){
				print $elastic_search_interface[$key]; 
			}
		}

	print '</div>';
?>

	<div style="display:none">
		<?php print $children; ?>
	</div>


	<?php if(!empty($from_nth_entry_nth)): ?>

		<div class="results_info">
			<?php print $from_nth_entry_nth; ?>
			<?php print $from_nth_entry_submit; ?>
		</div>

		<div class="download-table">
			<?php print $elastic_search_interface['download_table']; ?>
		</div>

		<br/><br/>
	<?php endif; ?>




