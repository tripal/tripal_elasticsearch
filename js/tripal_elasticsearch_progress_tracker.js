(function ($) {
  Drupal.behaviors.exampleModule = {
    /**
     * Constructor.
     *
     * @param context
     * @param settings
     */
    attach: function (context, settings) {
      setInterval(function () {
        $.get('/admin/tripal/extension/tripal_elasticsearch/progress/all', function (response) {
          $('#progress-report-page').html(response);
        });
      }, 1000);
    }
  };
}(jQuery));