<?php

namespace ES\Common;

use Elasticsearch\ClientBuilder;
use Exception;

/**
 * Class Instance
 * ================================================
 * Instantiates connections to an elasticsearch client.
 * Also Provides methods for building indices, searching,
 * deleting and indexing.
 */
class Instance{

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
   * @throws \Exception
   * @return void
   */
  public function __construct($host = NULL) {
    if ($host === NULL) {
      $host = variable_get('elasticsearch_host');
    }

    if (empty($host)) {
      throw new Exception(
        'A host was not provided. Please set an Elasticsearch host through the admin interface.',
        100
      );
    }

    if (!is_array($host)) {
      $host = [$host];
    }

    // Load the elastic search library
    libraries_load('elasticsearch-php');

    $exists = class_exists(ClientBuilder::class);
    if ($exists === FALSE) {
      throw new \Exception(
        'The elasticsearch-php library is not available. Please refer to the prerequisites section of the online documentation.'
      );
    }

    $this->client = ClientBuilder::create()
      ->setHosts($host)
      ->build();
  }

  /**
   * Remove special characters from string.
   *
   * @param $query
   *
   * @return mixed
   */
  public function sanitizeQuery($query) {
    $query = stripslashes($query);
    $query = str_replace('\\', ' ', $query);
    $query = str_replace('+', ' ', $query);
    $query = str_replace('-', ' ', $query);
    $query = str_replace('^', '', $query);
    return str_replace(':', '\\:', $query);
  }

  /**
   * Build a search query for the website and entities indices.
   *
   * @param string $search_terms
   * @param string $node_type
   * @param string $index
   * @param string $index_type
   * @param array $offset [int $from, int $to]
   * @param bool $force_entities_only force search of entities only
   *
   * @return $this
   */
  public function setWebsiteSearchParams($search_terms, $node_type = '', $index = 'website', $index_type = '', $offset = [], $force_entities_only = FALSE) {
    $queries = [];

    $queries[] = [
      'query_string' => [
        //'default_field' => 'content.taxrank__genus',
        'query' => $this->sanitizeQuery($search_terms),
        'default_operator' => 'AND',
      ],
    ];

    if (!empty($node_type)) {
      $indices = $this->getIndices();

      if (in_array('website', $indices) && !$force_entities_only) {
        $queries[1]['query_string'] = [
          'default_field' => 'type',
          'query' => '"' . $node_type . '"',
          'default_operator' => 'AND',
        ];
      }

      if (in_array('entities', $indices)) {
        $queries[1]['query_string'] = [
          'default_field' => 'bundle_label',
          'query' => '"' . $node_type . '"',
          'default_operator' => 'AND',
        ];
      }

      if (in_array('entities', $indices) && in_array(
          'website',
          $indices
        ) && !$force_entities_only) {
        $queries[1]['query_string'] = [
          'fields' => ['type', 'bundle_label'],
          'query' => '"' . $node_type . '"', // Gene or mRNA (feature,Gene)
          'default_operator' => 'AND',
        ];
      }
    }

    $query = [
      'bool' => [
        'must' => $queries,
      ],
    ];

    $highlight = [
      'pre_tags' => ['<em><b>'],
      'post_tags' => ['</b></em>'],
      'fields' => [
        'content' => [
          'fragment_size' => 150,
        ],
      ],
    ];

    $params = [];
    $params['index'] = $force_entities_only ? 'entities' : $index;
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
   * @param $index
   *
   * @return bool
   */
  public function hasIndex($index) {
    $indices = $this->getIndices();

    return in_array($index, $indices);
  }

  /**
   * Build table search params.
   * USe this method if not searching the website or entities indices.
   *
   * @param string $index Index name
   * @param string $type Index type
   * @param array $query ES query array
   * @param array $offset [int from, int size]
   * @param boolean $highlight Whether to highlight fields
   *
   * @return $this
   */
  public function setTableSearchParams($index, $type, $query, $offset = [], $highlight = FALSE) {
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

    if ($highlight) {
      $params['body']['highlight'] = [
        "fields" => [],
      ];

      $fields = $this->getIndexFields($index);
      foreach ($fields as $field) {
        $params['body']['highlight']['fields'][$field] = [
          'pre_tags' => ['<em>', '<strong>'],
          'post_tags' => ['</em>', '<strong>'],
        ];
      }
    }

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
  public function setIndexParams($index_name, $shards = 5, $replicas = 0, $tokenizer = 'standard', $token_filters = [], $field_mapping_types = []) {
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
      'max_result_window' => 1000000,
    ];

    $properties = [];
    foreach ($field_mapping_types as $field => $mapping_type) {
      $properties[$field] = [
        //'type' => $mapping_type,
        //'fields' => [
        //  'raw' => [
        //    'type' => $mapping_type,
        //    //'index' => 'not_analyzed',
        //  ],
        //],
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
   * @param bool $return_source whether to format the results or not.
   *
   * @see \ES\Common\Instance::setTableSearchParams()
   * @see \ES\Common\Instance::setWebsiteSearchParams()
   *
   * @return array
   * @throws \Exception
   */
  public function search($return_source = FALSE) {
    if (empty($this->searchParams)) {
      throw new Exception(
        'Please build search parameters before attempting to search.'
      );
    }

    $hits = $this->client->search($this->searchParams);

    if ($return_source) {
      return $hits;
    }

    return $this->formatHits($hits);
  }

  /**
   * Format hits.
   *
   * @param array $hits The hits returned from the search operation.
   *
   * @return array
   */
  public function formatHits($hits) {
    $results = [];
    foreach ($hits['hits']['hits'] as $hit) {
      if (isset($hit['highlight'])) {
        $highlight = '';
        foreach ($hit['highlight'] as $content) {
          $highlight .= implode('...', $content);
        }
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
      throw new Exception(
        'Please build search parameters before attempting to count results.'
      );
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
   * @see \ES\Common\Instance::setIndexParams()
   *
   * @return array
   * @throws \Exception
   */
  public function createIndex() {
    if (empty($this->indexParams)) {
      throw new Exception(
        'Please set the index parameters before attempting to create a new index.'
      );
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
   *
   * @return array
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
   * Index multiple entries at once.
   *
   * @param string $index Index name
   * @param array $entries Array of entries
   * @param string $type Index type
   * @param string $id_key The object key to get the id value
   *
   * @return array
   */
  public function bulkIndex($index, $entries, $type = NULL, $id_key = NULL) {
    return $this->bulk('index', $index, $entries, $type, $id_key);
  }

  /**
   * @param string $index Index name
   * @param array $entries Array of entries
   * @param string $type Index type
   * @param string $id_key The object key to get the id value
   *
   * @return array
   */
  public function bulkUpdate($index, $entries, $type = NULL, $id_key = NULL) {
    return $this->bulk('update', $index, $entries, $type, $id_key);
  }

  /**
   * @param string $operation
   * @param string $index
   * @param array $entries Array of entries of the form
   *              [
   *                [ // Start of entry 1
   *                  'field1' => 'value for field1',
   *                  'field2' => 'value for field 2'
   *                ],
   *                [ // Start of entry 2
   *                  'field1' => 'another value',
   *                  'field2' => 'some other value'
   *                ]
   *              ]
   * @param string $type
   * @param string $id_key
   *
   * @return array
   */
  public function bulk($operation, $index, $entries, $type = NULL, $id_key = NULL) {
    if (count($entries) === 0) {
      return [];
    }

    $params = ['body' => []];

    if ($type === NULL) {
      $type = $index;
    }

    foreach ($entries as $entry) {
      $request = [
        '_index' => $index,
        '_type' => $type,
      ];

      if ($id_key !== NULL) {
        $request['_id'] = $entry->{$id_key};
      }

      $params['body'][] = [
        $operation => $request,
      ];

      switch ($operation) {
        case 'index':
          $params['body'][] = $entry;
          break;
        case 'update':
          $params['body'][] = ['doc' => $entry];
          break;
      }
    }

    return $this->client->bulk($params);
  }

  /**
   * Paginate search results.
   *
   * @param $per_page
   *
   * @throws \Exception
   * @return array
   */
  public function paginate($per_page) {
    $count = $this->count();
    $total = min($count, 1000000);
    $current_page = pager_default_initialize($total, $per_page);

    // Set the offset.
    $this->searchParams['from'] = $per_page * $current_page;
    $this->searchParams['size'] = $per_page;

    $results = $this->search();

    return [
      'results' => $results,
      'total' => $total,
      'count' => $count,
      'page' => $current_page + 1,
      'pages' => ceil($total / $per_page),
      'pager' => theme('pager', ['quantity', $total]),
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
   * Get all available categories.
   *
   * @param int $version Get specific tripal version categories.
   * @param boolean $get_count Add count to the list. If set to TRUE, elements
   *                            with 0 count won't be removed.
   * @param string $keyword Count keyword.
   *
   * @throws \Exception
   * @return array
   *          If count is requested, 2 arrays will be returned.
   *          Otherwise, the structure is $array[$type_label] = $type_label
   */
  public function getAllCategories($version = NULL, $get_count = FALSE, $keyword = '*') {
    $types = [];
    $indices = $this->getIndices();
    $search_index = [];
    if (in_array(
        'website',
        $indices
      ) && ($version === NULL || $version === 2)) {
      // Get all node types from the node table.
      $node_types = db_query("SELECT name, type FROM {node_type}")->fetchAll();
      foreach ($node_types as $type) {
        $types[$type->type] = $type->name;
      }

      $search_index[] = 'website';
    }

    if (in_array(
        'entities',
        $indices
      ) && ($version === NULL || $version === 3)) {
      // Get all tripal entity types from the tripal_bundle table.
      $entity_types = db_query(
        "SELECT name, label FROM {tripal_bundle}"
      )->fetchAll();
      foreach ($entity_types as $type) {
        $types[$type->name] = $type->label;
      }

      $search_index[] = 'entities';
    }

    // Prevent anonymous categories from showing up.
    $es = new static();
    $indices = implode(',', $search_index);
    $counts = [];
    foreach ($types as $key => $type) {
      $count = $es->setWebsiteSearchParams($keyword, $key, $indices)->count();
      if ($count < 1 && !$get_count) {
        unset($types[$key]);
      }
      $counts[$key] = $count;
    }

    if (!$get_count) {
      return $types;
    }

    asort($types);

    return [
      'types' => $types,
      'count' => $counts,
    ];
  }

  /**
   * Return settings for a particular index.
   *
   * @param $index
   *
   * @return array
   */
  public function getIndexSettings($index) {
    $params = ['index' => $index];

    return $this->client->indices()->getSettings($params);
  }

  /**
   * Get the mappings for a particular index.
   *
   * @param $index
   *
   * @return array
   */
  public function getIndexMappings($index) {
    $params = ['index' => $index];

    return $this->client->indices()->getMapping($params);
  }

  /**
   * Returns results from all indices.
   *
   * @param string $terms
   * @param int $size
   * @param string|null $category
   *
   * @throws \Exception
   * @return array
   */
  public function searchWebIndices($terms, $size, $category = NULL) {
    $index_name = [];

    $indices = $this->getIndices();

    if (in_array('website', $indices)) {
      $index_name[] = 'website';
    }

    if (in_array('entities', $indices)) {
      $index_name[] = 'entities';
    }

    $index = implode(',', $index_name);

    $categories = $this->getAllCategories(NULL, FALSE, $terms);
    $category = trim($category);
    $category_index = array_search($category, $categories);
    if ($category_index !== FALSE) {
      $category = $category_index;
    }

    $this->setWebsiteSearchParams($terms, $category, $index);
    $results = $this->paginate($size);

    return [
      'count' => $results['total'],
      'results' => $results['results'],
    ];
  }

  /**
   * Get index fields.
   *
   * @param $index
   *
   * @return array
   */
  public function getIndexFields($index) {
    $mapping = $this->client->indices()->getMapping();
    $fields = isset($mapping[$index]) ? $mapping[$index]['mappings']['_default_']['properties'] : [];

    return array_keys($fields);
  }

  /**
   * Delete all records in an index.
   *
   * @param string $index_name
   * @param null|string $type
   *
   * @throws \Exception
   */
  public function deleteAllRecords($index_name, $type = NULL) {
    if (empty($index_name)) {
      throw new Exception(
        'Please provide an index name when deleting records from an index'
      );
    }

    if ($type === NULL) {
      $type = $index_name;
    }

    $this->client->deleteByQuery(
      [
        'index' => $index_name,
        'type' => $type,
        'body' => [
          'query' => [
            'match_all' => (object) [],
          ],
        ],
      ]
    );
  }

  /**
   * Get a single record.
   *
   * @param string $index
   * @param string $type
   * @param int $id
   *
   * @return array
   */
  public function getRecord($index, $type, $id) {
    try {
      return $this->client->get(
        [
          'index' => $index,
          'type' => $type,
          'id' => $id,
        ]
      );
    } catch (Exception $exception) {
      return ['found' => FALSE];
    }
  }

  /**
   * Update field mappings of an index.
   *
   * @param string $index_name Index name
   * @param string $field_name Field name
   * @param string $field_type Mapping type. E.g, text, integer, etc.
   *
   * @throws \Exception
   * @return array
   */
  public function putMapping($index_name, $field_name, $field_type, $index_type = NULL) {
    if ($index_type === NULL) {
      $index_type = $index_name;
    }

    $properties = [
      $field_name => [
        'type' => $field_type,
        'fields' => [
          'raw' => [
            'type' => $field_type,
            //'index' => 'not_analyzed',
          ],
        ],
      ],
    ];

    return $this->client->indices()->putMapping(
      [
        'index' => $index_name,
        'type' => $index_type,
        'body' => [
          'properties' => $properties,
        ],
      ]
    );
  }

  /**
   * Create a new element if one does not exist. Update the
   * element if it already exists.
   *
   * @param string $index The index name
   * @param string $index_type The index type
   * @param mixed $id The document ID
   * @param array $item The fields to update or create.
   *
   * @return array
   */
  public function createOrUpdate($index, $index_type, $id, $item) {
    return $this->client->update(
      [
        'index' => $index,
        'type' => $index_type,
        'id' => $id,
        'body' => [
          'doc' => $item,
          'upsert' => $item,
        ],
      ]
    );
  }
}
