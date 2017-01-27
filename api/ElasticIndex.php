<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/23/17
 * Time: 5:12 PM
 */
class ElasticIndex
{

    protected $client;

    public function __construct($client)
    {

        $this->client = $client;

    }

    public function GetIndexHealth()
    {

        $param['v'] = True;

        $client_health = $this->client->cat()->health($params);
        $client_health = preg_split('/\s+/', $client_health);
        foreach (range(0, 13) as $i) {
            $client_health_arr[$client_health[$i]] = $client_health[$i + 14];
        }

        $output = "<h2>Elasticsearch cluster health information:</h2>\n";
        $output .= '<ul>';
        foreach ($client_health_arr as $key => $value) {
            $output .= "<li><b>$key:</b> $value</li>";
        }

        $output .= '</ul>';

        return $output;
    }


    public function BuildIndex($param)
    {
        return $this->client->index($param);
    }
}