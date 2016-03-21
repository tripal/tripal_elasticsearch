

	<h2>Elastic search: transcripts</h2>
	<div style="padding:10px; left:0"><?php print $organism; ?></div>
	<div style="padding:10px; left:0"><?php print $uniquename; ?></div>
	<div style="padding:10px; left:0"><?php print $seqtype; ?></div>
	<div style="padding:10px; left:0"><?php print $seqlen; ?></div>
	<div style="padding:10px; right:0"><?php print $blast_hit_description; ?></div>
	<div style="padding:10px; right:0"><?php print $blast_hit_score; ?></div>
	<div style="padding:10px; right:0"><?php print $blast_hit_eval; ?></div>
	<div style="float:left"><?php print $submit; ?></div>
	<div style="display:none"><?php print $children; ?></div>


	<div><br/><br/><br/></div>
	<div>
		<span style="color:#ff0000"><?php print $search_record_count. '</span> records were found'; ?>
		<?php 
			if(!empty($from_nth_entry)){
				print $from_nth_entry;
			}
		?>
	</div>
	<hr/>
