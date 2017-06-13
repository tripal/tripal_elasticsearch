(function ($) {
    Drupal.behaviors = Drupal.behaviors || {};

    Drupal.behaviors.datatable_search_results = {
        attach: function (context, settings) {
            $(document).ready(function () {
                var table = $('#table_search_results_datatable .sticky-enabled').DataTable({
                    language: {
                        search: 'Filter current results'
                    },
                    retrieve: true
                });

                table.on('page.dt', function () {
                    var info = table.page.info();
                    // Total number of pages = info.pages
                    // Current page = info.page
                    //$('#pageInfo').html('Showing page: ' + info.page + ' of ' + info.pages);
                });
            });
        }
    };
})(jQuery);