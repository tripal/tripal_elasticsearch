(function ($) {
  Drupal.behaviors.exampleModule = {
    /**
     * Constructor.
     *
     * @param context
     * @param settings
     */
    attach: function (context, settings) {
      if (typeof settings.remotes === 'undefined' || typeof settings.action === 'undefined') {
        return;
      }

      if (typeof this[settings.action] !== 'function') {
        throw new Error('Undefined action');
      }

      this.axios   = window.axios.create({
        baseURL: '/elasticsearch/api/v1',
        timeout: 7000,
        headers: {
          'Accept'          : 'application/json',
          'X-Requested-With': 'XMLHttpRequest'
        }
      });
      this.remotes = settings.remotes;

      this[settings.action]();
    },

    /**
     * Gets the status of a remote server and updates the status table.
     */
    getStatus: function () {
      this.remotes.map(function (remote) {
        this.axios.get('/status').then(function (response) {
          var data = response.data.data;
          $('#remote-host-' + remote.id).html(data.status);
          $('#remote-host-' + remote.id + '-circle').addClass('is-success');
        }).catch(function (error) {
          var response = error.response;
          if (response) {
            var data = response.data.data;
            $('#remote-host-' + remote.id).html(data.status);
          }
          else {
            $('#remote-host-' + remote.id).html('Inactive');
          }

          $('#remote-host-' + remote.id + '-circle').addClass('is-danger');
        });
      }.bind(this));
    }
  };
}(jQuery));