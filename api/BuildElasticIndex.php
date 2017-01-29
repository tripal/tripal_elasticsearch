<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/24/17
 * Time: 1:10 AM
 */
class BuildElasticIndex
{

    protected $client;

    protected $index;

    protected $number_of_shards;

    protected $number_of_replicas;

    protected $analyzer_name;

    protected $character_filters;

    protected $tokenizer;

    protected $token_filters;

    protected $field_mapping_types;

    public function __construct($client, $index = 'table_name', $shards, $replicas, $character_filters = [], $tokenizer = 'standard', $token_filters = [], $field_mapping_types = [])
    {
        $this->client = $client;

        $this->index = $index;

        $this->number_of_shards = $shards;

        $this->number_of_replicas = $replicas;

        $this->analyzer_name = $this->index;

        $this->character_filters = $character_filters;

        $this->tokenizer = $tokenizer;

        $this->token_filters = $token_filters;

        $this->field_mapping_types = $field_mapping_types;
    }


    protected function get_analyzer()
    {
        $analysis = [
            'analyzer' => [
                $this->analyzer_name => [
                    'type' => 'custom',
                    'tokenizer' => $this->tokenizer
                ]
            ],

            'char_filter' => $this->character_filters,

            'filter' => $this->token_filters,
        ];

        return $analysis;
    }


    protected function get_settings()
    {
        return $settings = [
            'number_of_shards' => $this->number_of_shards,
            'number_of_replicas' => $this->number_of_replicas,
            'analysis' => $this->get_analyzer()
        ];
    }

    protected function get_mappings()
    {
        $properties = [];
        foreach ($this->field_mapping_types as $field=>$mapping_type)
        {
            $properties[$field] = [
                'type' => $mapping_type,
                'analyzer' => $this->analyzer_name
            ];
        }

        $mappings = [
            '_default_' => [
                'properties' => $properties
            ]
        ];

        return $mappings;
    }


    public function get_index_params()
    {
        $params = [
            'index' => $this->index,
            'body' => [
                'settings' => $this->get_settings(),
                'mappings' => $this->get_mappings()
            ]
        ];

        return $params;
    }


    public function create_index()
    {
        $params = $this->get_index_params();

        try
        {
            $this->client->indices()->create($params);
        }
        catch (Exception $e)
        {
            drupal_set_message($e->getMessage(), 'error');
        }

    }
}