	<div ><?php print $children; ?></div>


	<div><br/><br/><br/></div>
	<div>
		<span style="color:#ff0000"><?php print $search_record_count. '</span> records were found'; ?>
		<?php 
			if(!empty($from_nth_entry_nth)){
				print $from_nth_entry_nth;
				print $from_nth_entry_submit;
			}
		?>
	</div>
	<hr/>
