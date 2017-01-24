<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/24/17
 * Time: 1:10 AM
 */
class BuildElasticIndex
{

    protected $index;

    protected $number_of_shards;

    protected $number_of_replicas;

    public function __construct($index, $shards, $replicas)
    {
        $this->index = $index;

        $this->number_of_shards = $shards;

        $this->number_of_replicas = $replicas;
    }


    private function get_analyzer($analyzer_name = 'table_name', array $character_filters = [], $tokenizer = 'standard', array $token_filters = [])
    {
        $analysis = [
            'analyzer' => [
                $analyzer_name => [
                    'type' => 'custom',
                    'tokenizer' => $tokenizer
                ]
            ],

            'char_filter' => $character_filters,

            'filter' => $token_filters,
        ];

        return $analysis;
    }

    private function get_mappings(array $field_mapping_types, $analyzer = 'table_name')
    {
        $properties = [];
        foreach ($field_mapping_types as $field=>$mapping_type) {
            $properties[] = [
                $field => [
                    'type' => $mapping_type,
                    'analyzer' => $analyzer
                ]
            ];
        }

        $mappings = [
            '_default_' => [
                'properties' => $properties
            ]
        ];

        return $mappings;
    }


}