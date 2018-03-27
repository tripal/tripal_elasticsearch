<?php foreach ($rows as $row): ?>
    <p>
      <?php // Deal with urls to outside sites (we can't use l() here) ?>
      <?php if ($base_url): ?>
          <a href="<?php print url($row->url, ['base_url' => $base_url]) ?>">
              <strong><?php print $row->title ?></strong>
          </a>
          <br/>
      <?php else: ?>
          <strong><?php print l($row->title, $row->url) ?></strong><br/>
      <?php endif ?>

        <span>Content type: <em><?php print $row->type ?></em></span><br/>

      <?php if (!empty($row->content)): ?>
          <span><?php print $row->content ?></span><br/>
      <?php endif; ?>

      <?php if ($base_url): ?>
          <small>
              <a href="<?php print $row->url ?>" class="text-muted">
                <?php print substr($row->url, 0, 40) . (strlen($row->url) > 40 ? '...' : '') ?>
              </a>
          </small>
      <?php else: ?>
          <small>
            <?php
            $url = $GLOBALS['base_url'] . url(ltrim($row->url, '/'));
            $url = substr($url, 0, 40) . (strlen($url) > 40 ? '...' : '')
            ?>
            <?php print l($url, $row->url, [
              'attributes' => [
                'class' => ['text-muted'],
              ],
            ]) ?>
          </small>
      <?php endif; ?>
    </p>
<?php endforeach;
