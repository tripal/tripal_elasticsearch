/**
 * Created by mingchen on 1/31/17.
 */
(function ($) {
    Drupal.behaviors = Drupal.behaviors || {}


    Drupal.behaviors.indices_delete_confirmation = {
        attach: function (context, settings) {
            $(document).on('click', '#indices-delete-confirmation', function(e) {

                if (confirm("Are you sure? This step can not be undo.")) {
                }
                else {
                    e.preventDefault();
                }
            });
        }
    }

})(jQuery)