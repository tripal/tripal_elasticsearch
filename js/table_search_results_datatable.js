(function ($) {
  Drupal.behaviors = Drupal.behaviors || {};
  Drupal.behaviors.datatable_search_results = {
    attach: function (context, settings) {
      $(document).ready(function () {
        // var table = $('#table_search_results_datatable
        // .sticky-enabled').DataTable({ language: { search: 'Filter current
        // results' }, retrieve: true });  table.on('page.dt', function () {
        // var info = table.page.info(); // Total number of pages = info.pages
        // // Current page = info.page //$('#pageInfo').html('Showing page: ' +
        // info.page + ' of ' + info.pages); });
        console.log('ran');

        $('#table_search_results_datatable tr td').each(function () {
          var text = $(this).html();
          var array = text.split('<br>');

          if (array.length > 2) {
            var div = $('<div />', {});
            div.append(array.shift() + '<br>' + array.shift());
            var hidden = $('<div />', {'class': 'hidden-hit'}).css('display', 'none');
            hidden.append(array.join('<br>'));
            div.append(hidden);
            div.append('<br>');
            var btn = $('<button />', {
              'type': 'button',
              'class': 'btn btn-secondary btn-sm'
            }).html('Show More')
                .click(function (e) {
                  e.preventDefault();

                  var hidden_hit = hidden;
                  if (hidden_hit.hasClass('is_open')) {
                    hidden_hit.removeClass('is_open');
                    hidden_hit.slideUp();
                    btn.html('Show More');
                  }
                  else {
                    hidden_hit.slideDown();
                    hidden_hit.addClass('is_open');
                    btn.html('Show Less');
                  }
                });
            div.append(btn);
            $(this).html(div);
          }
        });
      });
    }
  };
})(jQuery);