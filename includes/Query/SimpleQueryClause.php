<?php

namespace ES\Query;

class SimpleQueryClause extends BuilderContract{

  /**
   * The built query.
   *
   * @var array
   */
  protected $queries = [];

  /**
   * @var \ES\Query\ClauseSanitizer
   */
  protected $sanitizer;

  /**
   * Clause constructor.
   */
  public function __construct() {
    $this->sanitizer = new ClauseSanitizer();
  }

  /**
   * Recursively build the query.
   *
   * @param string $field
   * @param string $value
   *
   * @return array
   */
  protected function makeQuery($field, $value = NULL) {
    if ($field instanceof \Closure) {
      $clause = new SimpleQueryClause();
      $field($clause);
      return $clause->build();
    }

    if (is_null($value)) {
      return $this->multiMatch(['*'], $field);
    }

    $fields = is_array($field) ? $field : [$field];
    $value = $this->sanitizer->escape($value);

    return $this->multiMatch($fields, $value);
  }

  /**
   * Create a multi_match query.
   *
   * @param array $fields
   * @param string $value
   *
   * @return array
   */
  public function multiMatch(array $fields, $value) {
    return [
      'multi_match' => [
        'fields' => $fields,
        'query' => $value,
        'lenient' => TRUE,
        'analyzer' => 'synonym',
        //'type' => 'phrase',
        'fuzziness' => 'AUTO',
      ],
    ];
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
    if (count($query) > 1) {
      foreach ($query as $q) {
        $this->queries[] = $q;
      }
    }
    else {
      $this->queries[] = $query;
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
    $this->where($field, $data);

    return $this;
  }

  /**
   * @return array
   */
  public function build() {
    return $this->queries;
  }
}
