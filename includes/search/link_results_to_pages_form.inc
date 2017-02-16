<?php

function link_results_to_pages_form ($form, &$form_state) {
    $connection = (new ElasticConnection(["localhost:9201"]))->make();
    $elastic_index = (new ElasticIndex($connection));
    $indices_options = $elastic_index->GetIndices();

    $form['indices'] = array(
        '#type' => 'select',
        '#title' => t('Select an index'),
        '#options' => ['none' => 'Select an index'] + $indices_options,
        '#ajax' => [
            'callback' => 'display_form_fields_ajax',
            'wrapper' => 'display_form_fields'
        ]
    );

    $index_name = !empty($form_state['values']['indices']) ? $form_state['values']['indices'] : 'Select an index';

    $form['form_fields'] = [
        '#type' => 'fieldset',
        '#title' => t('Form Fields'),
        '#tree' => true,
        '#prefix' => '<div id="display_form_fields">',
        '#suffix' => '</div>',
    ];

    try{
        // get fields for a select index
        $index_mappings = $elastic_index->GetMappings($index_name);
        foreach ($index_mappings as $field) {
            $form['form_fields'][$field] = [
                '#type' => 'textfield',
                '#title' => t($field),
                '#collapsible' => true,
                '#collapsed' => true,
            ];
        }
    } catch (Exception $exception) {
        //drupal_set_message($exception->getMessage(), 'warning');
    }

    return $form;
}


/*
 * display_form_fields_ajax callback
 */
function display_form_fields_ajax ($form, &$form_state) {
    return $form['form_fields'];
}


/*
 * link_results_to_pages_form submit
 */
function link_results_to_pages_form_submit ($form, &$form_state) {

    $index_name = $form_state['values']['indices'];
    $table_name = preg_replace('/^chado_/', 'chado.', $index_name);

    // Delete the table and its fields in the database if that table name already exists
    db_delete('tripal_elasticsearch_search_forms')
        ->condition('table_name', $table_name)
        ->execute();

    $table_fields = array_keys($form_state['values']['form_fields']);
    foreach ($table_fields as $field) {
        $record['table_name'] = $table_name;
        $record['table_field'] = $field;
        $record['page_links'] = $form_state['values']['form_fields'][$field];

        drupal_write_record('tripal_elasticsearch_add_links', $record);
    }

    drupal_set_message(t('A search form block has been created for table '. $table_name));
}


/*
 * instruction form for link results to pages
 */
function link_results_to_pages_example_form($form, &$form_state){
    $form['add_links'] = array(
        '#type' => 'fieldset',
        '#title' => 'Example: add page links to search results',
        '#collapsible' => true,
        '#collapsed' => true,
    );
    $description = '<h2>Add page links to search results</h2>';
    $description .= '<p>This page allows you to add dynamic URL to search results. ';
    $description .= 'Here dynamic URL means that the URL dependens on the seach result values. ';
    $description .= 'For example, the search form for organism has four fields: <b>abbreviation</b>, <b>common_name</b>, <b>genus</b>, <b>species</b>. ';
    $description .= 'We want to link some search results to corresponding oranism pages. ';
    $description .= 'Assuming that all organism pages have the same pattern: <b>organism/genus/species</b> ';
    $description .= '(all URLs have the same part "organism/" but are different at "genus" and "species"),';
    $description .= 'and the different parts in URLs only depend on the values of search results, ';
    $description .= 'in this case, we can link search results to their corresponding organism pages by adding dynamic links to the fields. </p>';
    $description .= '<p><b>A dynamic link is created by replacing variable values in URL with corresponding field names within "[]".</b></p>';
    $description .= '<p>The example below shows how to add links to search results in fields <b>abbreviation</b> and <b>species</b>.</p>';
    $form['add_links']['description'] = array(
        '#type' => 'item',
        '#markup' => $description,
    );
    $form['add_links']['abbreviation'] = array(
        '#type' => 'textfield',
        '#title' => t('abbreviation'),
        '#title_display' => 'after',
        '#default_value' => t('organism/[genus]/[species]'),
    );
    $form['add_links']['common_name'] = array(
        '#type' => 'textfield',
        '#title' => t('common_name'),
        '#title_display' => 'after',
    );
    $form['add_links']['genus'] = array(
        '#type' => 'textfield',
        '#title' => t('genus'),
        '#title_display' => 'after',
    );
    $form['add_links']['species'] = array(
        '#type' => 'textfield',
        '#title' => t('species'),
        '#title_display' => 'after',
        '#default_value' => t('organism/[genus]/[species]'),
    );
    return $form;
}