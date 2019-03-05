<p>
    <span>
        <b><?php print number_format($total) ?> results found</b>
      <?php if (isset($time)): ?>
          (<?php print number_format($time, 2) ?> seconds)
      <?php endif; ?>
    </span>
    <span style="float: right">Page <?php print $page ?> of <?php print $pages ?></span>
</p>
