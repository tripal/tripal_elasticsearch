(function ($) {
    Drupal.behaviors = Drupal.behaviors || {};

    Drupal.behaviors.datatable_search_results = {
        attach: function (context, settings) {
            $(document).ready(function () {
                $('#table_search_results_datatable .sticky-enabled').DataTable({
                    language:{
                        search: 'Filter current results'
                    },
                    retrieve: true,
                });
            })
        }
    }
})(jQuery)