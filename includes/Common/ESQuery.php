<?php

class ESQuery{

  /**
   * The elasticsearch instance.
   *
   * @var \ES\Common\Instance
   */
  protected $es;

  /**
   * The fields to search.
   *
   * @var string
   */
  protected $field;

  /**
   * The index to search.
   *
   * @var string
   */
  protected $index;

  /**
   * The type. See ES docs for more info.
   *
   * @var string
   */
  protected $type;

  /**
   * The category.
   *
   * For site-wide searching only!
   *
   * @var string
   */
  protected $category;

  /**
   * The range of the query.
   *
   * Used for pagination.
   *
   * @var array
   */
  protected $range;

  /**
   * The built queries.
   *
   * @var array
   */
  protected $queries = [];

  /**
   * ESQuery constructor.
   *
   * @param \ES\Common\Instance $es
   */
  public function __construct(Instance $es, $index = 'entities') {
    $this->es = $es;
    $this->index = $index;
  }

  /**
   * Set the category.
   *
   * @param string $category
   *
   * @return $this
   */
  public function setCategory($category) {
    $this->category = $category;

    return $this;
  }

  /**
   * Set the type.
   *
   * @param string $type
   *
   * @return $this
   */
  public function setType($type) {
    $this->type = $type;
    return $this;
  }

  /**
   * Set the index.
   *
   * @param string $index
   *
   * @return $this
   */
  public function setIndex($index) {
    $this->index = $index;
    return $this;
  }

  /**
   * Get the field as an array.
   *
   * @return array
   */
  public function getField() {
    return is_array($this->field) ? $this->field : [$this->field];
  }

  /**
   * Specify which field to search,
   *
   * @param array|string $field The name of the field or an array of field
   *   names.
   *
   * @return $this
   */
  public function setField($field) {
    $this->field = $field;

    return $this;
  }

  /**
   * Get the fields to search when specifying a category.
   *
   * @return array
   */
  protected function getCategoryFields() {
    $fields = [];
    if ($this->es->hasIndex('website')) {
      $fields[] = 'type';
    }

    if ($this->es->hasIndex('entities')) {
      $fields[] = 'bundle_label';
    }

    return $fields;
  }

  /**
   * Validate a query.
   *
   * @throws \Exception
   */
  protected function validateQuery() {
    if (empty($this->index)) {
      throw new Exception('Please provide an index name');
    }

    if (!$this->es->hasIndex($this->index)) {
      throw new Exception(
        'Index ' . $this->index . ' does not exist. Please provide a valid index name.'
      );
    }

    if (!empty($this->category) && !in_array(
        $this->index,
        ['website', 'entities']
      )) {
      throw new Exception(
        'When specifying a category, the index name must be either website or entities. Currently, it is set to ' . $this->index . '.'
      );
    }
  }

  /**
   * Set the range for the query.
   *
   * @param int $from
   * @param int $size
   *
   * @return  $this
   */
  public function range($from, $size) {
    $this->range = [$from, $size];

    return $this;
  }

  /**
   * @param $terms
   * @param $per_page
   *
   * @return array
   * @throws \Exception
   */
  public function paginate($terms, $per_page) {
    $count = $this->count($terms);
    $total = min($count, 1000000);
    $current_page = pager_default_initialize($total, $per_page);

    // Set the offset.
    $this->range($per_page * $current_page, $per_page);

    $results = $this->search($terms);

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
   * @param string $terms Query string
   *
   * @return int
   */
  public function count($terms) {
    $params = $this->buildQuery($terms, FALSE);
    return (int) $this->es->client->count($params)['count'];
  }

  /**
   * @param $terms
   * @param bool $with_range
   *
   * @return array
   */
  protected function buildQuery($terms, $with_range = TRUE) {
    // Initialize the query
    $query = [];
    $query[] = [
      'simple_query_string' => [
        'fields' => ['content.*'],
        'query' => $terms,
      ],
    ];

    // Set searchable fields
    $fields = $this->getField();
    if (!empty($fields)) {
      $query[0]['simple_query_string']['fields'] = $fields;
    }

    // Add query
    if (!empty($this->category)) {
      $query[] = [
        'query_string' => [
          'fields' => $this->getCategoryFields(),
          'query' => '"' . $this->category . '"',
        ],
      ];
    }

    $params = [
      'index' => $this->index,
      'type' => $this->type,
      'body' => ['query' => $query],
    ];

    if ($with_range && !empty($this->range)) {
      $params['from'] = $this->range[0];
      $params['size'] = $this->range[1];
    }

    return $params;
  }

  /**
   * @param $terms
   *
   * @throws \Exception
   * @return array
   */
  public function search($terms) {
    $this->validateQuery();

    $hits = $this->es->client->search($this->buildQuery($terms));

    return $this->es->formatHits($hits);
  }
}
