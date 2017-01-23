<?php

/**
 * analyzer components
 *   1. character filters
 *   2. tokenizer
 *   3. token filters
 */
$character_filters_reference = 'https://www.elastic.co/guide/en/elasticsearch/reference/current/analysis-charfilters.html';
$character_filters = array(
    'html_strip' => 'HTML Strip Character Filter',
    'mapping' => 'Mapping Character Filter',
    'pattern_replace' => 'Pattern Replace Character Filter',
);

$tokenizers_reference = 'https://www.elastic.co/guide/en/elasticsearch/reference/current/analysis-tokenizers.html';
$tokenizers = array(
    'standard' => 'Standard Tokenizer',
    'letter' => 'Letter Tokenizer',
    'lowercase' => 'Lowercase Tokenizer',
    'whitespace' => 'Whitespace Tokenizer',
    'uax_url_email' => 'UAX URL Email Tokenizer',
    'classic' => 'Classic Tokenizer',
    'ngram' => 'N-Gram Tokenizer',
    'edge_ngram' => 'edge_ngram',
    'keyword' => 'Keyword Tokenizer',
    'pattern' => 'Pattern Tokenizer',
    'path_hierarchy' => 'Path Tokenizer',
);

$token_filters_reference = 'https://www.elastic.co/guide/en/elasticsearch/reference/current/analysis-tokenfilters.html';
$token_filters = array(
    'standard' => 'Standard Token Filter',
    'asciifolding' => 'ASCII Folding Token Filter',
    'length' => 'Length Token Filter',
    'lowercase' => 'Lowercase Token Filter',
    'uppercase' => 'Uppercase Token Filter',
    'nGram' => 'NGram Token Filter',
    'edgeNGram' => 'Edge NGram Token Filter',
    'porter_stem' => 'Porter Stem Token Filter',
    'shingle' => 'Shingle Token Filter',
    'stop' => 'Stop Token Filter',
    'word_delimiter' => 'Word Delimiter Token Filter',
    'stemmer' => 'Stemmer Token Filter',
    'keyword_marker' => 'Keyword Marker Token Filter',
    'keyword_unique' => 'Keyword Repeat Token Filter',
    'kstem' => 'KStem Token Filter',
    'snowball' => 'Snowball Token Filter',
    'reverse' => 'Reverse Token Filter',
    'elision' => 'Elision Token Filter',
    'truncate' => 'Truncate Token Filter',
    'unique' => 'Unique Token Filter',
    'pattern_capture' => 'Pattern Capture Token Filter',
    'pattern_replace' => 'Pattern Replace Token Filter',
    'trim' => 'Trim Token Filter',
    'limit' => 'Limit Token Count Token Filter',
    'delimited_playload_filter' => 'Delimited Payload Token Filter',
    'keep' => 'Keep Words Token Filter',
    'keep_types' => 'Keep Types Token Filter',
    'apostrophe' => 'Apostrophe Token Filter',
    'decimal_digit' => 'Decimal Digit Token Filter',
    'fingerprint' => 'Fingerprint Token Filter',
    'min_has' => 'Minhash Token Filter',
);

$builtin_analyzers_reference = 'https://www.elastic.co/guide/en/elasticsearch/reference/current/analysis-analyzers.html';
$builtin_analyzers = array(
    'standard' => 'Standard Analyzer',
    'simple' => 'Simple Analyzer',
    'whitespace' => 'Whitespace Analyzer',
    'stop' => 'Stop Analyzer',
    'keyword' => 'Keyword Analyzer',
    'pattern' => 'Pattern Analyzer',
    'fingerprint' => 'Fingerprint Analyzer',
);

/**
 * functions to generate form elements for advanced elasticsearch index settings
 */

function generate_form_elements_for_advanced_index_setting()
{
    $form = array();
    $form['advanced_index'] = array(
        '#type' => 'fieldset',
        '#title' => t('Advanced Index Setting'),
    );

    $form['advanced_index']['setting_options'] = array(
        '#type' => 'checkbox',
        '#title' => t('Select how you want to set up your index'),
        '#options' => array('default' => 'default', 'custom' => 'custome'),
        '#ajax' => array(
            'callback' => 'advanced_setting_form_elements_ajax',
            'wrapper' => 'advanced_setting_form_elements'
        )
    );

    /**
     * analyzer components
     *   1. character filters
     *   2. tokenizer
     *   3. token filters
     */
    $form['advanced_index']['advanced_setting_form_elements'] = array(
        '#type' => 'fieldset',
        '#title' => t('Analyzer Components')
    '#prefix' => '<div id="advanced_setting_form_elements">',
    '#suffix' => '<div>',
  );

  // character filters
  $form['advanced_index']['advanced_setting_form_elements']['character_filters'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Character Filters'),
      '#description' => t('Character filters are used to preprocess the stream of 
                         characters before it is passed to the tokenizer.'),
      '#options' =>
  );  

}


/**
 * advanced_index_setting_form_elements_ajax callback
 */
function advanced_index_setting_form_elements_ajax($form, &$form_state)
{

    return $form['advanced_index']['setting_options'];
}
