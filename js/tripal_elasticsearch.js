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
        timeout: 20000,
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
        this.axios.defaults.baseURL = '';
        this.axios.get(remote.url + '/elasticsearch/api/v1/status').then(function (response) {
          var data = response.data.data;

          if(typeof data.status === 'undefined') {
            data.status = 'No Elasticsearch Instance found'
          }

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
    },

    /**
     * Respond to search events in search from.
     */
    setupSearchPage: function () {
      $('#tripal-elasticsearch-search-button').click(function (e) {
        e.preventDefault();
        var terms = $('#tripal-elasticsearch-search-field').val();
        this.sendSearchRequest(terms);
      }.bind(this));
    },

    /**
     * Sends a cross site search request.
     *
     * @param terms
     */
    sendSearchRequest: function (terms) {
      var resultsBlock = $('#tripal-elasticsearch-results-block');

      resultsBlock.html('');

      this.remotes.map(function (remote) {
        var block = this.createSiteBlock(remote);
        resultsBlock.append(block);

        this.axios.get('/search/' + remote.id, {
          params: {
            terms: terms,
            size : 2
          }
        }).then(function (response) {
          var data = response.data.data;
          block.find('.elastic-result-block-count').html(data.count + ' total results');

          if (data.count === 0 || data.count === null) {
            data.markup = 'No results found';

            if (remote.id !== 0) {
              block.slideUp();
              return;
            }
          }
          else {
            var footer = $('<div />', {
              'class': 'elastic-result-block-footer'
            }).append('<a href="' + data.url + '">See All Results</a>');
            block.append(footer);
          }

          block.find('.elastic-result-block-content').html(data.markup);
        }).catch(function (error) {
          console.log(error);
          block.slideUp();
        });
      }.bind(this));
    },

    /**
     * Create a result block.
     *
     * @param remote
     * @returns {*|HTMLElement}
     */
    createSiteBlock: function (remote) {
      var block = $('<div />', {
        'class': 'elastic-result-block'
      });

      var title = $('<h3 />', {
        'class': 'elastic-result-block-title'
      }).append(remote.label);
      block.append(title);

      var count = $('<div />', {
        'class': 'elastic-result-block-count'
      });
      block.append(count);

      var content = $('<div />', {
        'class': 'elastic-result-block-content'
      }).append('Searching <div class="ajax-progress ajax-progress-throbber"><div class="throbber">&nbsp;</div></div>');
      block.append(content);

      return block;
    }
  };
}(jQuery));