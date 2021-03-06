<?php

/**
 * @class
 * Purpose:
 *
 * Display:
 * Configuration:
 */
class local__feature_search_formatter extends ChadoFieldFormatter {

  // The default label for this field.
  public static $default_label = 'Feature Search';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = ['local__feature_search'];

  // The list of default settings for this formatter.
  public static $default_settings = [];

  /**
   * Provides the field's setting form.
   *
   * This function corresponds to the hook_field_formatter_settings_form()
   * function of the Drupal Field API.
   *
   * The settings form appears on the 'Manage Display' page of the content
   * type administration page. This function provides the form that will
   * appear on that page.
   *
   * To add a validate function, please create a static function in the
   * implementing class, and indicate that this function should be used
   * in the form array that is returned by this function.
   *
   * This form will not be displayed if the formatter_settings_summary()
   * function does not return anything.
   *
   * param $field
   *   The field structure being configured.
   * param $instance
   *   The instance structure being configured.
   * param $view_mode
   *   The view mode being configured.
   * param $form
   *   The (entire) configuration form array, which will usually have no use
   *   here.  Typically for reference only.
   * param $form_state
   *   The form state of the (entire) configuration form.
   *
   * @return
   *   A Drupal Form array containing the settings form for this field.
   */
  public function settingsForm($view_mode, $form, &$form_state) {
  }

  /**
   *  Provides the display for a field
   *
   * This function corresponds to the hook_field_formatter_view()
   * function of the Drupal Field API.
   *
   *  This function provides the display for a field when it is viewed on
   *  the web page.  The content returned by the formatter should only include
   *  what is present in the $items[$delta]['values] array. This way, the
   *  contents that are displayed on the page, via webservices and downloaded
   *  into a CSV file will always be identical.  The view need not show all
   *  of the data in the 'values' array.
   *
   * @param $element
   * @param $entity_type
   * @param $entity
   * @param $langcode
   * @param $items
   * @param $display
   *
   * @return
   *    An element array compatible with that returned by the
   *    hook_field_formatter_view() function.
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    $organism = $entity->chado_record;

    try {
      $es = new ESInstance();
      $query = tripal_elasticsearch_gene_search_index_query_mapper([
        'organism' => "$organism->genus $organism->species",
        "search_term" => '*',
      ]);
      $count = $es->setTableSearchParams('gene_search_index', '', $query)
        ->count();
      if ($count < 1) {
        return NULL;
      }
    } catch (Exception $exception) {
      $element[] = [
        '#type' => 'markup',
        '#markup' => t('<p>Search service is not available at this time. Please 
                        contact us to fix this issue.</p>'),
      ];
      $message = "Feature search in organism {$organism->genus}
                  {$organism->species} has failed with the following 
                  message. " . $exception->getMessage();
      tripal_report_error('tripal_elasticsearch', TRIPAL_ERROR, $message);
      return;
    }

    $organism_name = "$organism->genus $organism->species";
    $form = drupal_get_form('tripal_elasticsearch_gene_search_form', TRUE, $organism_name, [
      'tripal_pane' => 'group_feature_search',
      'organism' => $organism_name,
    ]);

    $content = '<p>You can search this organism’s features in the database by entering search terms in the box. You can search by feature name or annotation.</p>';
    $content .= drupal_render($form);
    $element[] = [
      '#type' => 'markup',
      '#markup' => $content,
    ];

    return;
  }

  /**
   * Provides a summary of the formatter settings.
   *
   * This function corresponds to the hook_field_formatter_settings_summary()
   * function of the Drupal Field API.
   *
   * On the 'Manage Display' page of the content type administration page,
   * fields are allowed to provide a settings form.  This settings form can
   * be used to allow the site admin to define how the field should be
   * formatted.  The settings are then available for the formatter()
   * function of this class.  This function provides a text-based description
   * of the settings for the site developer to see.  It appears on the manage
   * display page inline with the field.  A field must always return a
   * value in this function if the settings form gear button is to appear.
   *
   * See the hook_field_formatter_settings_summary() function for more
   * information.
   *
   * @param $field
   * @param $instance
   * @param $view_mode
   *
   * @return string
   *   A string that provides a very brief summary of the field settings
   *   to the user.
   *
   */
  public function settingsSummary($view_mode) {
    return '';
  }
}