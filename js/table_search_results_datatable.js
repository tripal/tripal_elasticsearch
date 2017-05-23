(function ($) {
    Drupal.behaviors = Drupal.behaviors || {};

    Drupal.behaviors.indices_delete_confirm = {
        attach: function (context, settings) {
            $(document).ready(function () {
                $('#table_search_results_datatable .sticky-enabled').DataTable();
            })
        }
    }
})(jQuery)