<?php
/**
 * @file
 *
 * theme implementation for displaying website search results.
 *
 * Available variables:
 *
 * $website_search_results: an array with two keys (title and highlight).
 */
?>


<?php foreach ($variables['website_search_results'] as $item): ?>
	<h3><?php print l($item['title'], "node/".$item['nid']) ;?></h3>
	<p><?php print $item["highlight"]; ?></p>
<?php endforeach; ?>
