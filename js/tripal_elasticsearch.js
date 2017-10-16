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

      this.settings = settings;

      this.axios   = window.axios.create({
        baseURL: '/elasticsearch/api/v1',
        timeout: 20000,
        headers: {
          'Accept'          : 'application/json',
          'X-Requested-With': 'XMLHttpRequest'
        }
      });
      this.remotes = settings.remotes;
      this.state   = {
        resultsCount: 0,
        sitesCount  : 0,
        emptySites  : [],
        footerBlock : $('<div />')
      };

      this[settings.action]();
    },

    /**
     * Reset state
     */
    resetState: function () {
      this.state = {
        resultsCount: 0,
        sitesCount  : 0,
        emptySites  : [],
        footerBlock : $('<div />')
      };
    },

    /**
     * Gets the status of a remote server and updates the status table.
     */
    getStatus: function () {
      this.remotes.map(function (remote) {
        this.axios.get('/remote/status/' + remote.id).then(function (response) {
          var data = response.data.data;

          if (typeof data.status === 'undefined') {
            data.status = 'No Elasticsearch Instance found';
          }

          $('#remote-host-' + remote.id).html(data.status);
          $('#remote-host-' + remote.id + '-circle').addClass('is-success');
        }).catch(function (error) {
          var response = error.response;
          if (response) {
            if (response.status === 404) {
              $('#remote-host-' + remote.id).html('Host Not Found');
            }
            else if (response.status === 500) {
              $('#remote-host-' + remote.id).html('Server Error');
            }
            else {
              var data = response.data.data;
              $('#remote-host-' + remote.id).html(data.status);
            }
          }
          else {
            $('#remote-host-' + remote.id).html('Invalid Host');
          }

          $('#remote-host-' + remote.id + '-circle').addClass('is-danger');
        });
      }.bind(this));
    },

    /**
     * Respond to search events in search from.
     * This method is called by this[action]() in the constructor (attach
     * method).
     */
    setupSearchPage: function () {
      $('input[name="search_term"]').attr('maxlength', null);

      this.readHistory();

      $('#tripal-elasticsearch-search-button').click(function (e) {
        e.preventDefault();
        var form     = {};
        var category = $('#tripal-elasticsearch-search-category').val();

        form.terms    = $('#tripal-elasticsearch-search-field').val();
        form.category = category === 'Any Type' ? null : category;

        this.pushHistory(form);
        this.sendSearchRequest(form);
      }.bind(this));
    },

    /**
     * Update results stats.
     */
    updateStats: function () {
      var content = 'Found ' + this.state.resultsCount + ' results from ' + this.state.sitesCount + ' websites';
      $('.elastic-stats-block').html(content);

      var footer = $('<div />').css('margin-top', '20px');

      if (this.state.emptySites.length > 0) {
        footer.html('<h3>Sites that returned no results</h3>');
        this.state.emptySites.map(function (site) {
          site.block.find('.elastic-result-block-content').html('No results found');
          site.block.find('.elastic-result-block-count').html('<a href="' + site.remote.url + '">Visit Site</a>');
          footer.append(site.block);
        }.bind(this));
      }
      this.state.footerBlock.html(footer);
    },

    /**
     * Sends a cross site search request.
     *
     * @param form
     */
    sendSearchRequest: function (form) {
      this.resetState();
      var block        = $('#tripal-elasticsearch-results-block');
      var resultsBlock = $('<div />');
      var statsBlock   = $('<div />', {'class': 'elastic-stats-block'});
      block.html(statsBlock);
      block.append(resultsBlock);
      block.append(this.state.footerBlock);

      if (!form.terms && form.category) {
        form.terms = '*';
      }

      this.remotes.map(function (remote) {
        var block = this.createSiteBlock(remote);
        resultsBlock.append(block);

        this.axios.get('/search/' + remote.id, {
          params: {
            terms   : form.terms,
            category: form.category,
            size    : 2
          }
        }).then(function (response) {
          var data = response.data.data;

          if (data.count === 0 || data.count === null) {
            data.markup = 'No results found';
            if (remote.id !== 0) {
              this.state.emptySites.push({block: block, remote: remote});
              return;
            }
          }
          else {
            var footer = $('<div />', {
              'class': 'elastic-result-block-footer'
            }).append('<a href="' + data.url + '">See All Results</a>');
            block.append(footer);
          }

          this.state.resultsCount += data.count || 0;
          this.state.sitesCount++;
          block.find('.elastic-result-block-content').html(data.markup);
          block.find('.elastic-result-block-count').html((data.count || 0) + ' total results');

          var event = $.Event('elasticsearch.completed');
          $(document).trigger(event, {remote: remote});
        }.bind(this)).catch(function (error) {
          console.log(error);
          this.state.emptySites.push({block: block, remote: remote});
        }.bind(this)).then(this.updateStats.bind(this));
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
    },

    /**
     * Update browser history.
     *
     * @param state
     */
    pushHistory: function (state) {
      if (window.history) {
        window.history.pushState(state, window.document.title, this._url(state));
      }
    },

    /**
     * Get browser history state.
     */
    readHistory: function () {
      if (window.history) {
        var state = window.history.state;
        if (!state) {
          return;
        }

        if (state.terms) {
          $('#tripal-elasticsearch-search-field').val(state.terms);
        }

        if (state.category) {
          $('#tripal-elasticsearch-search-category').val(state.category);
        }

        if (state.category || state.terms) {
          var form = {
            category: state.category || null,
            terms   : state.terms
          };
          this.sendSearchRequest(form);
        }
      }
    },

    setupTableIndexPage: function () {
      // Get page settings
      var index        = this.settings.index;
      var formSelector = '#cross-site-search-form';
      var mapper       = this.settings.field_mapper || {};

      // Set global variables
      this.searchURL = index + '/search';

      // Set up listeners
      $(formSelector).on('submit', function (event) {
        if (event) {
          event.preventDefault();
        }

        var form     = $(formSelector);
        var formData = this.formToObject(form);
        var data     = this.mapForm(formData, mapper);
        this.tableSearch(data);
        this.pushHistory(formData);
      }.bind(this));
    },

    formToObject: function (form) {
      var formData = form.serializeArray();
      var data     = {};

      // Prepare form data
      Object.keys(formData).map(function (key) {
        var element        = formData[key];
        data[element.name] = element.value;
      });

      return data;
    },

    mapForm: function (formData, mapper) {
      var mapperKeys = Object.keys(mapper);

      if (mapperKeys.length === 0) {
        return formData;
      }

      var mappedData = {};
      mapperKeys.map(function (key) {
        mappedData[key] = formData[mapper[key]];
      });

      return mappedData;
    },

    tableSearch: function (data) {
      this.resetState();
      var block        = $('#tripal-elasticsearch-results-block');
      var resultsBlock = $('<div />');
      var statsBlock   = $('<div />', {'class': 'elastic-stats-block'});
      block.html(statsBlock);
      block.append(resultsBlock);
      block.append(this.state.footerBlock);

      this.remotes.map(function (remote) {
        var block = this.createSiteBlock(remote);
        resultsBlock.append(block);

        this.axios.get(this.searchURL + '/' + remote.id, {
          params: data
        }).then(function (response) {
          var data = response.data.data;

          if (data.count === 0 || data.count === null) {
            data.markup = 'No results found';
            if (remote.id !== 0) {
              this.state.emptySites.push({block: block, remote: remote});
              return;
            }
          }
          else {
            var footer = $('<div />', {
              'class': 'elastic-result-block-footer'
            }).append('<a href="' + data.url + '">See All Results</a>');
            block.append(footer);
          }

          this.state.resultsCount += data.count || 0;
          this.state.sitesCount++;
          block.find('.elastic-result-block-content').html(data.markup);
          block.find('.elastic-result-block-count').html((data.count || 0) + ' total results');

          var event = $.Event('elasticsearch.completed');
          $(document).trigger(event, {remote: remote});
        }.bind(this)).catch(function (error) {
          console.log(error);
          this.state.emptySites.push({block: block, remote: remote});
        }.bind(this)).then(this.updateStats.bind(this));
      }.bind(this));
    },

    _serialize: function (data) {
      var serialized = '';

      Object.keys(data).map(function (key, index) {
        if (index > 0) {
          serialized += '&';
        }

        serialized += key + '=' + data[key];
      });

      return serialized;
    },

    _url: function (state) {
      var url = '?';
      url += this._serialize(state);

      return window.location.pathname + url;
    }
  };
}(jQuery));