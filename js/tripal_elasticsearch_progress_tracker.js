(function ($) {
  Drupal.behaviors.exampleModule = {
    /**
     * Constructor.
     *
     * @param context
     * @param settings
     */
    attach: function (context, settings) {
      var busy = false;
      setInterval(function () {
        if (!busy) {
          busy = true;
        }
        $.get('/admin/tripal/extension/tripal_elasticsearch/progress/all', function (response) {
          $('#progress-report-page').html(response);
          busy = false;
        }).error(function () {
          busy = false;
        });
      }, 1000);
    }
  };
}(jQuery));