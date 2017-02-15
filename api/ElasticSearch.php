<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 2/15/17
 * Time: 12:01 AM
 */
class ElasticSearch
{
    protected $client;

    public function __construct($client)
    {

        $this->client = $client;

    }

    public function build_search_query_from_field_content_pairs (array $field_content_pairs, $query_method = 'query_string') {
        $queries = [];
        foreach ($field_content_pairs as $field => $content) {
            if (!empty($content)) {
                $queries[] = [
                    $query_method => [
                        "default_field" => $field,
                        "query" => $content,
                        "default_operator" => "OR"
                    ]
                ];
            }
        }

        $query = [
            "bool" => [
                "must" => $queries
            ]
        ];

        return $query;
    }

    public function build_search_params ($index, $type, $query, $from=0, $size=1000) {
        $params = [];
        $params['index'] = $index;
        $params['type'] = $type;
        $params['body'] = [
            'query' => $query
        ];
        $params['from'] = $from;
        $params['size'] = $size;

        return $params;
    }

    public function search ($params) {
        $hits = $this->client->search($params);
        $search_res = [];
        foreach ($hits['hits']['hits'] as $hit) {
            $search_res[] = $hit['_source'];
        }

        return $search_res;
    }
}