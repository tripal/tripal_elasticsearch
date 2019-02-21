<?php

namespace ES\Query;

class Builder implements BuilderContract{

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
  protected $highlight;

  /**
   * The query clause builder.
   *
   * @var \ES\Query\Clause
   */
  protected $query;

  /**
   * ES generated id if exists.
   *
   * @var string
   */
  protected $id = FALSE;

  /**
   * Builder constructor.
   *
   * @param $index
   */
  public function __construct($index) {
    $this->index = $index;
    $this->query = new Clause();
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
   * Build a where clause.
   *
   * @param string $field
   * @param string $value
   *
   * @return $this
   */
  public function where($field, $value = NULL) {
    $this->query->where($field, $value);
    return $this;
  }

  /**
   * Build an or where clause.
   *
   * @param string $field
   * @param string $value
   *
   * @return $this
   */
  public function orWhere($field, $value = NULL) {
    $this->query->orWhere($field, $value);
    return $this;
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
   * @param $id
   *
   * @return $this
   */
  public function setID($id) {
    $this->id = $id;
    return $this;
  }

  /**
   * Build and return the parameters.
   *
   * @param bool $with_range Whether to include the range.
   *
   * @return array
   *   The params array.
   */
  public function build($with_range = TRUE) {
    $params = [];

    if ($this->size && $with_range) {
      $params['size'] = $this->size;
    }

    if ($this->from && $with_range) {
      $params['from'] = $this->from;
    }

    $params['body'] = [
      'query' => [
        'simple_query_string' => [
          'query' => $this->query->build(),
        ],
      ],
    ];

    if ($this->highlight) {
      $params['body']['highlight'] = [
        'fields' => $this->highlight,
      ];
    }

    if ($this->id !== FALSE) {
      $params['id'] = $this->id;
    }

    return $params;
  }
}
