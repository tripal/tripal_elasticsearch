<?php

namespace ES\Query;

class Clause{

  /**
   * The built query.
   *
   * @var string
   */
  protected $query = '';

  /**
   * @param $field
   * @param null $value
   *
   * @return string
   */
  protected function makeQuery($field, $value = NULL) {
    if ($field instanceof \Closure) {
      $query = '(';
      $clause = new Clause();
      $field($clause);
      $query .= $clause->build();
      $query .= ')';
      return $query;
    }

    if (is_null($value)) {
      return $field;
    }

    return $field . ':' . $value;
  }

  /**
   * Add a where clause.
   *
   * @param string $field The value to query if a field is not specified or the
   *   field name.
   * @param string $data The value to query if a field is specified.
   *
   * @return $this
   *   The object.
   */
  public function where($field, $data = NULL) {
    $query = $this->makeQuery($field, $data);
    if (!empty($this->query)) {
      $this->query .= " AND $query";
    }
    else {
      $this->query = $query;
    }

    return $this;
  }

  /**
   * Add an or where clause.
   *
   * @param string $field The value to query if a field is not specified or the
   *   field name.
   * @param string $data The value to query if a field is specified.
   *
   * @return $this
   *   The object.
   */
  public function orWhere($field, $data = NULL) {
    $query = $this->makeQuery($field, $data);
    if (!empty($this->query)) {
      $this->query .= " OR $query";
    }
    else {
      $this->query = $query;
    }

    return $this;
  }

  /**
   * @return string
   */
  public function build() {
    return $this->query;
  }
}
