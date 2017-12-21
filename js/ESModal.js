(function ($) {
  Drupal.behaviors = Drupal.behaviors || {};

  Drupal.behaviors.ESModal = {
    attach: function (context, settings) {
      var DATA_API = '[data-widget="modal"]';
      var DATA_KEY = 'es.modal';

      $(DATA_API).each(function () {
        var modal = $(this);
        var data = modal.data(DATA_KEY);

        if (!data) {
          var config = modal.data();
          modal.data(DATA_KEY, new ESModal(modal, config));
        }
      });
    }
  };

  /**
   * ESModal Constructor.
   *
   * @param modal
   * @param config
   * @constructor
   */
  function ESModal(modal, config) {
    // The modal element
    this.modal = modal;

    // The modal state
    this.state = {
      open: false,
      steps: [],
      activeStep: 0
    };

    // CSS selectors
    this.selectors = {
      backdrop: '.elastic-modal.show',
      close: '[data-modal="close"]',
      back: '[data-modal="back"]',
      wrapper: 'html, body',
      submit: '[type="submit"]',
      form: 'form',
      content: '.elastic-modal-card',
      foot: '.elastic-modal-card-foot',
      step: '.elastic-modal-step'
    };

    // Get the submit and back buttons from the form
    this.submitButton = this.modal.find(this.selectors.submit).first();
    this.backButton = this.modal.find(this.selectors.back).first();

    // Default config
    var defaults = {
      // The trigger to show the modal
      trigger: '#modal-trigger',
      wrapperOverflow: $(this.selectors.wrapper).css('overflow') || 'auto',
      submitText: this.submitButton.val(),
      cancelText: this.backButton.text()
    };

    // Merged config from data attributes
    this.config = $.extend({}, defaults, config);


    // Initiate the modal events
    this.attachEvents();
    this.prepareSubmitButton();
    this.fixBackButtonPosition();
  }

  /**
   * Toggle open state.
   */
  ESModal.prototype.toggle = function () {
    if (this.state.open) {
      this.hide();
      return;
    }

    this.show();
  };

  /**
   * Show the modal.
   */
  ESModal.prototype.show = function () {
    this.modal.addClass('show');
    $(this.selectors.wrapper).css('overflow', 'hidden');
    this.state.open = true;

    this.configureSteps();
  };

  /**
   * Hide the modal
   */
  ESModal.prototype.hide = function () {
    this.modal.removeClass('show');
    $(this.selectors.wrapper).css('overflow', this.config.wrapperOverflow);
    this.state.open = false;
  };

  ESModal.prototype.fixBackButtonPosition = function () {
    this.submitButton.after(this.backButton);
  };

  /**
   * Initiate events.
   */
  ESModal.prototype.attachEvents = function () {
    // Attach trigger button event to open the modal
    $(document).on('click', this.config.trigger, function (event) {
      if (event) {
        event.preventDefault();
      }

      this.show();
    }.bind(this));

    // Allow backdrop to close the modal
    $(document).on('click', this.selectors.backdrop, function (event) {
      if (!event) {
        return;
      }

      if ($(event.target).is(this.selectors.backdrop)) {
        this.hide();
      }
    }.bind(this));

    // Attach hide event
    this.modal.on('click', this.selectors.close, function (event) {
      if (event) {
        event.preventDefault();
      }

      this.hide();
    }.bind(this));

    // Attache back event
    this.modal.on('click', this.selectors.back, function (event) {
      if (event) {
        event.preventDefault();
      }

      if (this.state.activeStep > 0) {
        this.back();
      }
      else {
        this.hide();
      }
    }.bind(this));
  };

  /**
   * Go to next step.
   */
  ESModal.prototype.next = function () {
    if (this.state.activeStep === this.state.steps.length - 1) {
      return;
    }

    this.state.steps[this.state.activeStep].css('display', 'none');

    this.state.activeStep++;
    this.state.steps[this.state.activeStep].css('display', 'block');
    this.modal.find('.elastic-modal-card-body').first().animate({
      scrollTop: 0
    }, 0);

    if (this.state.activeStep === this.state.steps.length - 1) {
      this.submitButton.val(this.config.submitText);
    }

    if (this.state.activeStep > 0) {
      this.backButton.html('Previous Step');
    }
  };

  /**
   * Go to previous step.
   */
  ESModal.prototype.back = function () {
    if (this.state.activeStep === 0) {
      return;
    }

    this.state.steps[this.state.activeStep].css('display', 'none');

    this.state.activeStep--;
    this.state.steps[this.state.activeStep].css('display', 'block');

    if (this.state.activeStep < this.state.steps.length - 1) {
      this.submitButton.val('Next Step');
    }

    if (this.state.activeStep === 0) {
      this.backButton.html(this.config.cancelText);
    }
  };

  /**
   * Allow the modal submit button to submit the form.
   */
  ESModal.prototype.prepareSubmitButton = function () {
    // Find the form
    var form = this.modal.find(this.selectors.form).first();

    this.submitButton.on('click', function (e) {
      if (e) {
        e.preventDefault();
      }

      if (this.state.steps.length > 0) {
        if (this.state.steps.length - 1 === this.state.activeStep && form) {
          form.submit();
        }
        else {
          this.next();
        }
      }
      else {
        if (form) {
          form.submit();
        }
      }
    }.bind(this));

    /**
     * Adjust width and height of modal steps.
     */
    ESModal.prototype.configureSteps = function () {
      this.state.steps = [];
      this.state.activeStep = 0;

      var steps = this.modal.find(this.selectors.step);

      if (steps.length > 1) {
        this.modal.find(this.selectors.step).each(function (index, step) {
          this.state.steps.push($(step));
          if (index > 0) {
            $(step).css('display', 'none');
          }
          else {
            $(step).css('display', 'block');
          }
        }.bind(this));

        this.submitButton.val('Next Step');
        this.backButton.html(this.config.cancelText).attr('disabled');
      }
    };
  };
})(jQuery);