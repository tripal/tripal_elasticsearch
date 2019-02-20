<?php

namespace ES\Query;

class Builder{

  /**
   * @var string
   */
  protected $index;

  /**
   * @var int
   */
  protected $from;

  /**
   * @var int
   */
  protected $size;

  /**
   * @var array
   */
  protected $queries = [];

  /**
   * @var array
   */
  protected $highlight;

  /**
   * Builder constructor.
   *
   * @param $index
   */
  public function __construct($index) {
    $this->index = $index;
  }

  /**
   * Range setter.
   *
   * @param int $from The offset parameter.
   * @param int $size The limit parameter. Defaults to 10.
   *
   * @return $this
   *   An Instance of this object.
   */
  public function range($from, $size = 10) {
    $this->from = $from;
    $this->size = $size;

    return $this;
  }

  /**
   * Add a clause.
   *
   * @param $query
   * @param null $fields
   */
  public function query($query, array $fields = NULL) {
    $query = [
      'simple_query_string' => [
        'query' => $query,
      ],
    ];

    if (!is_null($fields)) {
      if (!is_array($fields)) {
        $fields = [$fields];
      }

      $query['simple_query_string']['fields'] = $fields;
    }

    $this->queries[] = $query;
  }

  /**
   *
   * @param string|array $fields
   *
   * @return $this The current object.
   */
  public function highlight($fields) {
    if (is_array($fields)) {
      foreach ($fields as $field) {
        $this->addHighlightField($field);
      }
    }
    else {
      $this->addHighlightField($fields);
    }

    return $this;
  }

  /**
   * Highlight a field.
   *
   * @param string $field The name of the field
   */
  private function addHighlightField($field) {
    $this->highlight[$field] = ['fragment_size' => 150];
  }

  /**
   * Build and return the parameters.
   *
   * @return array
   *   The params array.
   */
  public function build() {
    $params = [];

    if ($this->size) {
      $params['size'] = $this->size;
    }

    if ($this['from']) {
      $params['from'] = $this->from;
    }

    $params['body'] = [
      'query' => $this->queries,
    ];

    if ($this->highlight) {
      $params['body']['highlight'] = [
        'fields' => $this->highlight,
      ];
    }

    return $params;
  }
}
