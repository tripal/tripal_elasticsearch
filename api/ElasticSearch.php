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
                        "query" => _remove_special_chars($content),
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

    public function build_table_search_params ($index, $type, $query, $from=0, $size=1000, $sort_field, $sort_direction = 'asc' ) {
        $params = [];
        $params['index'] = $index;
        $params['type'] = $type;
        $params['body'] = [
            'query' => $query,
            'sort' => [
                $sort_field . ".raw" => $sort_direction,
            ]
        ];
        $params['from'] = $from;
        $params['size'] = $size;
        return $params;
    }

    public function build_website_search_params ($index='website', $type='website', $search_content='', $from=0, $size=1000) {
        $query = [
            "query_string" => [
                "default_field" => "body",
                "query" => $search_content,
                "default_operator" => "OR"
            ]
        ];

        $highlight = [
            "pre_tags" => ["<em><b>"],
            "post_tags" => ["</b></em>"],
            "fields" => [
                "body" =>  [
                    "fragment_size" => 150
                ]
            ]
        ];

        $params = [];
        $params['index'] = $index;
        $params['type'] = $type;
        $params['body'] = [
            'query' => $query,
            'highlight' => $highlight
        ];
        $params['from'] = $from;
        $params['size'] = $size;


        return $params;

    }

    public function table_search ($params) {
        $hits = $this->client->search($params);
        $search_res = [];
        foreach ($hits['hits']['hits'] as $hit) {
            $search_res[] = $hit['_source'];
        }

        return $search_res;
    }

    public function search_count ($params) {
        unset($params['from']);
        unset($params['size']);
        unset($params['scroll']);
        $count = $this->client->count($params);
        $count = $count['count'];

        return $count;
    }

    public function website_search ($params) {
        $hits = $this->client->search($params);
        $search_res = [];
        foreach ($hits['hits']['hits'] as $hit) {
            $highlight = implode('......', $hit['highlight']['body']);
            $hit['_source']['highlight'] = $highlight;
            $search_res[] = $hit['_source'];
        }

        return $search_res;
    }


}