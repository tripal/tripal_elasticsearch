<?php


class CronQueueWorker
{

    protected $client;

    protected $queue_item;

    public function __construct($client, $queue_item)
    {
        $this->client = $client;

        $this->queue_item = $queue_item;
    }

    private function index_database_table()
    {
        $result = db_query($this->queue_item->sql)->fetchAll();

        foreach ($result as $record)
        {
            $params = [
                'index' => $this->queue_item->index,
                'type' => $this->queue_item->table,
                'body' => (array) $record
            ];

            $this->client->index($params);
            dpm($this->queue_item->table);

            watchdog("Tripal Elasticsearch", "Indexed 1 record from " . $this->queue_item->table . ' - ' . format_date(time()));
        }

    }


    private function get_node_content($nid, $node_title, $base_url)
    {
        $page_html = file_get_contents("$base_url/node/$nid");
        // remove raw sequences
        $pattern_1 = preg_quote('<pre class="tripal_feature-sequence">'). "(.|\n)*".preg_quote('</pre>');
        $page_html = preg_replace("!".$pattern_1."!U", ' ', $page_html);
        // remove query sequences
        $pattern_2 = preg_quote('<pre>Query')."(.|\n)*".preg_quote('</pre>');
        $page_html = preg_replace("!".$pattern_2."!U", ' ', $page_html);
        // add one space to html tags to avoid words catenation after stripping html tags
        $page_html = str_replace( '<', ' <', $page_html);
        // remove generated jQuery script
        $page_html =  preg_replace('/<script\b[^>]*>(.*?)<\/script>/is', "", $page_html);
        // make page title searchable
        $page_content = $node_title.' '.strip_tags($page_html);
        // merge multiple spaces into one
        return preg_replace('!\s+!', ' ', $page_content);

    }


    private function index_website()
    {
        $result = db_query($this->queue_item->sql)->fetchAll();

        foreach ($result as $record)
        {
            $nid = $this->queue_item->nid;
            $title = $this->queue_item->title;
            $type = $this->queue_item->type;
            $base_url = $this->queue_item->website_base_url;
            $params = [
                'index' => $this->queue_item->index,
                'type' => 'website',
                'body' => [
                    'nid' => $nid,
                    'title' => $title,
                    'type' => $type,
                    'content' => $this->get_node_content($nid, $title, $base_url)
                ]
            ];

            $this->client->index($params);
            watchdog("Tripal Elasticsearch", "Indexed 1 page" . ' - ' . format_date(time()));
        }

    }

    public function indexing()
    {
        if ($this->queue_item->is_website)
        {
            $this->index_website();
        } else
        {
            $this->index_database_table();
        }
    }

}