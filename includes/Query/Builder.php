<?php

namespace ES\Query;

class Builder extends BuilderContract{

  /**
   * @var string
   */
  protected $index;

  /**
   * @var int
   */
  protected $from = NULL;

  /**
   * @var int
   */
  protected $size = NULL;

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
   * @var string
   */
  protected $type;

  /**
   * Builder constructor.
   *
   * @param $index
   */
  public function __construct($index = NULL) {
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
   * @param string $index
   *
   * @return $this
   */
  public function setIndex($index) {
    $this->index = $index;

    return $this;
  }

  /**
   * @param string $type
   *
   * @return $this
   */
  public function setType($type) {
    $this->type = $type;

    return $this;
  }

  /**
   * Validate the query.
   *
   * @throws \Exception
   */
  public function validate() {
    if (is_null($this->index)) {
      throw new \Exception(
        'Index name must be set to build request parameters.'
      );
    }

    if (empty($this->query->build())) {
      throw new \Exception(
        'Query string must be provided in order to build request parameters.'
      );
    }
  }

  /**
   * Build and return the parameters.
   *
   * @param bool $with_range Whether to include the range.
   *
   * @return array
   *   The params array.
   *
   * @throws \Exception
   */
  public function build($with_range = TRUE) {
    $this->validate();

    // Initialize params
    $params = [
      'index' => $this->index,
      'body' => [
        'query' => [
          'bool' => [
            'must' => $this->query->build(),
          ],
        ],
      ],
    ];

    // Set range
    if (!is_null($this->from) && $with_range) {
      $params['from'] = $this->from;
    }
    if (!is_null($this->size) && $with_range) {
      $params['size'] = $this->size;
    }

    // Set the highlighted field
    if ($this->highlight) {
      $params['body']['highlight'] = [
        'fields' => $this->highlight,
      ];
    }

    // Set the id
    if ($this->id !== FALSE && $this->id !== NULL) {
      $params['id'] = $this->id;
    }

    // Set the index type
    if ($this->type) {
      $params['type'] = $this->type;
    }

    return $params;
  }

  /**
   * @param null $index
   */
  public function reset($index = NULL) {
    $this->from = NULL;
    $this->query = NULL;
    $this->size = NULL;
    $this->type = NULL;
    if (!is_null($index)) {
      $this->index = $index;
    }
  }
}
