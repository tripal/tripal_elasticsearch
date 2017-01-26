<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/23/17
 * Time: 11:42 PM
 */
class FormElementsForIndex
{
    protected $CharacterFiltersOptions = [
        'html_strip' => 'html_strip',
        'mapping' => 'mapping',
        'pattern_replace' => 'pattern_replace'
    ];

    protected $TokenizerOptions = [
        'standard' => 'standard',
        'letter' => 'letter',
        'lowercase' => 'lowercase',
        'whitespace' => 'whitespace',
        'uax_url_email' => 'uax_url_email',
        'classic' => 'classic',
        //'thai' => 'thao',
        'ngram' => 'ngram',
        'edge_ngram' => 'edge_ngram',
        'keyword' => 'keyword',
        'pattern' => 'pattern',
        'path_hierarchy' => 'path_hierarchy',
    ];

    protected $TokenFiltersOptions = [
        'standard' => 'standard',
        'asciifolding' => 'asciifolding',
        'length' => 'length',
        'lowercase' => 'lowercase',
        'uppercase' => 'uppercase',
    ];

    public function CharacterFiltersElements()
    {

        $form['CharacterFiltersElements'] = array(
            '#type' => 'checkboxes',
            '#title' => t('Character Filters'),
            '#options' => $this->CharacterFiltersOptions,
        );

        return $form;
    }

    public function TokenizerElements()
    {
        $form['TokenizerElements'] = array(
            '#type' => 'select',
            '#title' => t('Tokenizer'),
            '#options' => $this->TokenizerOptions,
        );

        return $form;

    }

    public function TokenFiltersElements()
    {
        $form['TokenFiltersElements'] = array(
            '#type' => 'checkboxes',
            '#title' => t('Token Filters'),
            '#options' => $this->TokenFiltersOptions,
        );

        return $form;

    }
}