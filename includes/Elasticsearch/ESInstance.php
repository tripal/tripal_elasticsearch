<?php

/**
 * Class ESInstance
 * ================================================
 * Instantiates connections to an elasticsearch client.
 * Also Provides methods for building indices, searching,
 * deleting and indexing.
 */
class ESInstance {

  /**
   * Elasticsearch client.
   *
   * @var \Elasticsearch\Client
   */
  public $client;

  /**
   * Index Parameters.
   * Used to build a new index.
   *
   * @var array
   */
  protected $indexParams = [];

  /**
   * Search Parameters.
   * Used to build a search query.
   *
   * @var array
   */
  protected $searchParams = [];

  /**
   * ESInstance constructor.
   * Establishes a connection to a host.
   *
   * @param null $host
   *
   * @return void
   */
  public function __construct($host = NULL) {
    if ($host === NULL) {
      $host = variable_get('elasticsearch_host');
    }

    if (empty($host)) {
      throw new Exception('A host was not provided. Please set an Elasticsearch host through the admin interface.');
    }

    if (!is_array($host)) {
      $host = [$host];
    }

    // Load the elastic search library
    libraries_load('elasticsearch-php');

    $this->client = Elasticsearch\ClientBuilder::create()
                                               ->setHosts($host)
                                               ->build();
  }

  /**
   * Build a search query for the website and entities indices.
   *
   * @param string $search_terms
   * @param string $node_type
   * @param string $index
   * @param string $index_type
   * @param array $offset [int $from, int $to]
   *
   * @return $this
   */
  public function setWebsiteSearchParams($search_terms, $node_type = '', $index = 'website', $index_type = '', $offset = []) {
    $queries = [];

    $queries[] = [
      "query_string" => [
        "default_field" => "content",
        "query" => $search_terms,
        "default_operator" => "OR",
      ],
    ];

    if (!empty($node_type)) {
      $indices = $this->getIndices();

      if (in_array('website', $indices)) {
        $queries[1]['query_string'] = [
          "default_field" => "type",
          "query" => $node_type,
          "default_operator" => "OR",
        ];
      }

      if (in_array('entities', $indices)) {
        $queries[1]['query_string'] = [
          "default_field" => "bundle_label",
          "query" => $node_type,
          "default_operator" => "OR",
        ];
      }

      if (in_array('entities', $indices) && in_array('website', $indices)) {
        $queries[1]['query_string'] = [
          "fields" => ["type", "bundle_label"],
          "query" => $node_type, // Gene or mRNA (feature,Gene)
          "default_operator" => "OR",
        ];
      }
    }

    $query = [
      "bool" => [
        "must" => $queries,
      ],
    ];

    $highlight = [
      "pre_tags" => ["<em><b>"],
      "post_tags" => ["</b></em>"],
      "fields" => [
        "content" => [
          "fragment_size" => 150,
        ],
      ],
    ];

    $params = [];
    $params['index'] = $index;
    $params['type'] = $index_type;
    $params['body'] = [
      'query' => $query,
      'highlight' => $highlight,
    ];

    if (empty($offset)) {
      $offset = [0, 1000];
    }
    $params['from'] = $offset[0];
    $params['size'] = $offset[1];

    $this->searchParams = $params;

    return $this;
  }

  /**
   * Build table search params.
   * USe this method if not searching the website or entities indices.
   *
   * @param $index
   * @param $type
   * @param $query
   * @param array $offset
   *
   * @return $this
   */
  public function setTableSearchParams($index, $type, $query, $offset = []) {
    $params = [];
    $params['index'] = $index;
    $params['type'] = $type;

    // sort the table by the first field by default
    //$sort_field = array_keys($field_content_pairs)[0];

    $params['body'] = [
      'query' => $query,
    ];

    if (isset($_GET['order'])) {
      $sort_field = $_GET['order'];
      $sort_direction = isset($_GET['sort']) && $_GET['sort'] === 'desc' ? 'desc' : 'asc';
      $params['body']['sort'] = [
        $sort_field . ".raw" => $sort_direction,
      ];
    }

    if (empty($offset)) {
      $offset = [0, 1000];
    }
    $params['from'] = $offset[0];
    $params['size'] = $offset[1];

    $this->searchParams = $params;

    return $this;
  }

  /**
   * Build a new index parameters.
   *
   * @param $index_name
   * @param int $shards
   * @param int $replicas
   * @param string $tokenizer
   * @param array $token_filters
   * @param array $field_mapping_types
   *
   * @return $this
   */
  public function setIndexParams(
    $index_name, $shards = 5, $replicas = 0, $tokenizer = 'standard', $token_filters = [], $field_mapping_types = []
  ) {
    $analysis = [
      'analyzer' => [
        $index_name => [
          'type' => 'custom',
          'tokenizer' => $tokenizer,
          'filter' => array_keys($token_filters),
        ],
      ],
    ];

    $settings = [
      'number_of_shards' => $shards,
      'number_of_replicas' => $replicas,
      'analysis' => $analysis,
    ];

    $properties = [];
    foreach ($field_mapping_types as $field => $mapping_type) {
      $properties[$field] = [
        'type' => $mapping_type,
        'fields' => [
          'raw' => [
            'type' => $mapping_type,
            'index' => 'not_analyzed',
          ],
        ],
      ];
    }

    $mappings = [
      '_default_' => [
        'properties' => $properties,
      ],
    ];

    $this->indexParams = [
      'index' => $index_name,
      'body' => [
        'settings' => $settings,
        'mappings' => $mappings,
      ],
    ];

    return $this;
  }

  /**
   * Perform the actual search.
   * Use this function after setting the search params.
   *
   * @see \ESInstance::setTableSearchParams()
   * @see \ESInstance::setWebsiteSearchParams()
   *
   * @return array
   * @throws \Exception
   */
  public function search() {
    if (empty($this->searchParams)) {
      throw new Exception('Please build search parameters before attempting to search.');
    }

    $hits = $this->client->search($this->searchParams);
    $results = [];
    foreach ($hits['hits']['hits'] as $hit) {
      if (isset($hit['highlight'])) {
        $highlight = implode('......', $hit['highlight']['content']);
        $hit['_source']['highlight'] = $highlight;
      }
      $results[] = $hit['_source'];
    }

    return $results;
  }

  /**
   * Get the number of available results for a given search query.
   *
   * @return mixed
   * @throws \Exception
   */
  public function count() {
    if (empty($this->searchParams)) {
      throw new Exception('Please build search parameters before attempting to count results.');
    }

    // Get the search query
    $params = $this->searchParams;

    // Remove offset restrictions
    unset($params['from']);
    unset($params['size']);
    unset($params['body']['highlight']);

    return $this->client->count($params)['count'];
  }

  /**
   * Create a new index.
   * Use this function after building the index parameters.
   *
   * @see \ESInstance::setIndexParams()
   *
   * @param $params
   *
   * @return array
   */
  public function createIndex() {
    if (empty($this->indexParams)) {
      throw new Exception('Please set the index parameters before attempting to create a new index.');
    }

    return $this->client->indices()->create($this->indexParams);
  }

  /**
   * Delete an entire index.
   *
   * @param string $index Index name
   *
   * @return array
   */
  public function deleteIndex($index) {
    $params = ['index' => $index];
    return $this->client->indices()->delete($params);
  }

  /**
   * Delete an entry from an index.
   *
   * @param string $index Index name
   * @param string $index_type Index type
   * @param int $id Entry ID (node or entity id)
   */
  public function deleteEntry($index, $index_type, $id) {
    $params = [
      'index' => $index,
      'type' => $index_type,
      'id' => $id,
    ];

    return $this->client->delete($params);
  }

  /**
   * Create a new entry and add it to the provided index.
   *
   * @param string $index Index name
   * @param string $type Table name for table entries and index name for
   *                     website entries
   * @param int $id Entry ID (node or entity id). Set as FALSE if a table index
   *                entry.
   * @param array $body Array of record data to index. Must match index
   *                    structure.
   */
  public function createEntry($index, $type, $id, $body) {
    $params = [
      'index' => $index,
      'type' => $type,
      'body' => $body,
    ];

    if ($id !== FALSE) {
      $params['id'] = $id;
    }

    return $this->client->index($params);
  }

  /**
   * Populate an index with entries.
   * This methods pulls all the data from the database and create queue jobs
   * to get populated in the background.
   *
   * @param $type
   * @param string $index_table
   * @param string $index_name
   * @param string $index_type
   * @param array $field_mapping_types
   * @param int $queue_count
   */
  public function populateIndex(
    $type, $index_table, $index_name, $index_type, $field_mapping_types, $queue_count
  ) {
    // Get row count of selected table.
    $row_count = db_query("SELECT COUNT(*) FROM {$index_table}")->fetchAssoc()['count'];
    // Get total number of offsets (offset interval is 1000)
    $k = 1000;
    $total_offsets = intval($row_count / $k);
    // Separate table fields with comma
    $comma_separated_fields = implode(',', array_keys($field_mapping_types));
    $order_by_field = array_keys($field_mapping_types)[0];
    foreach (range(0, $total_offsets) as $offset) {
      $id = $offset % $queue_count + 1;
      $cron_queue_id = 'elasticsearch_queue_' . $id;
      $cron_queue = DrupalQueue::get($cron_queue_id);
      $OFFSET = $k * $offset;
      $item = new stdClass();

      // Use the first field to sort the table
      if ($type == 'website') {
        $sql = "SELECT nid, title, type FROM $index_table ORDER BY $order_by_field LIMIT $k OFFSET $OFFSET";
      }
      elseif ($type == 'entities') {
        $sql = "SELECT tripal_entity.id AS entity_id, title, label AS bundle_label
              FROM tripal_entity
              JOIN tripal_bundle ON tripal_entity.term_id = tripal_bundle.term_id
              ORDER BY title ASC LIMIT $k OFFSET $OFFSET";
      }
      else {
        $sql = "SELECT $comma_separated_fields FROM $index_table ORDER BY $order_by_field LIMIT $k OFFSET $OFFSET";
      }

      $item->index_name = $index_name;
      $item->index_type = $index_type;
      $item->type = $type;
      $item->field_mapping_types = $field_mapping_types;
      $item->sql = $sql;

      $cron_queue->createItem($item);
    }
  }

  /**
   * Paginate search results.
   *
   * @param $per_page
   *
   * @return array
   */
  public function paginate($per_page) {
    $total = $this->count();
    $current_page = pager_default_initialize($total, $per_page);

    // Set the offset.
    $this->searchParams['from'] = $per_page * $current_page;
    $this->searchParams['size'] = $per_page;

    $results = $this->search();

    return [
      'results' => $results,
      'total' => $total,
      'page' => $current_page,
      'pages' => ceil($total / $per_page),
      'pager' => theme('pager', ['quantity', $total])
    ];
  }

  /**
   * Get a flat list of indices.
   *
   * @return array
   */
  public function getIndices() {
    return array_keys($this->client->indices()->getMapping());
  }

  /**
   * Return settings for a particular index
   *
   * @return array
   */
  public function getIndexSettings($index) {
    $params = ['index' => $index];
    return ($this->client->indices()->getSettings($params));
  }

  /**
   * Update Index settings.
   * https://www.elastic.co/guide/en/elasticsearch/client/php-api/current/_index_management_operations.html#_put_settings_api
   * Note that the index to update is in the settings array.
   *
   * @param $settings
   *
   * @return mixed
   */

  /**Get the mappings for a particular index
   *
   * https://www.elastic.co/guide/en/elasticsearch/client/php-api/current/_index_management_operations.html#_get_mappings_api
   */

  public function getIndexMappings($index) {
    $params = ['index' => $index];
    return ($this->client->indices()->getMapping($params));
  }

  /*
     * Returns results from all indices.
     *
     * @param $terms
     * @param $size
     *
     * @return array
     */
  public function searchAllIndices($terms, $size, $category = NULL) {
    $index_name = [];

    $indices = $this->getIndices();

    if (in_array('website', $indices)) {
      $index_name[] = 'website';
    }

    if (in_array('entities', $indices)) {
      $index_name[] = 'entities';
    }

    $index = implode(',', $index_name);

    $this->setWebsiteSearchParams($terms, $category, $index, '', [0, $size]);
    $count = $this->count();
    $results = $this->search();

    return [
      'count' => $count,
      'results' => $results,
    ];
  }
}
