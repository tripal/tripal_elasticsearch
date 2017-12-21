<div class="elastic-modal" data-widget="modal"
     data-trigger="<?php print $variables['trigger'] ?>"
     data-backdrop-click="<?php print $variables['backdrop_click'] ?>">
    <div class="elastic-modal-card">
        <div class="elastic-modal-card-head">
            <p class="elastic-modal-title"><?php print $variables['title']; ?></p>
            <button type="button" class="elastic-modal-close-btn"
                    data-modal="close"></button>
        </div>
      <?php if ($variables['content']): ?>
          <div class="elastic-modal-card-body">
            <?php print render($variables['content']) ?>
            <?php if ($variables['cancel']): ?>
                <a class="btn btn-default"
                   data-modal="back"
                   style="float: right;">
                  <?php print $variables['cancel'] ?>
                </a>
            <?php endif; ?>
          </div>
      <?php endif; ?>
    </div>
</div>