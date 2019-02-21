<?php

namespace ES\Query;

abstract class BuilderContract{

  /**
   * Add a match condition joined with ANDs.
   *
   * @param string|\Closure $field The value to match against or the name of
   *   the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  abstract public function where($field, $value = NULL);

  /**
   * Add a match condition joined with ORs.
   *
   * @param string|\Closure $field The value to match against or the name of
   *   the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  abstract public function orWhere($field, $value = NULL);

  /**
   * Add a begins with condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function beginsWith($field, $value = NULL) {
    if (is_null($value)) {
      return $this->where("{$field}*");
    }

    return $this->where($field, "{$value}*");
  }

  /**
   * Add a ends with condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function endsWith($field, $value = NULL) {
    if (is_null($value)) {
      return $this->where("*{$field}");
    }

    return $this->where($field, "*{$value}");
  }

  /**
   * Add a contains condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function contains($field, $value = NULL) {
    if (is_null($value)) {
      return $this->where("*{$field}*");
    }

    return $this->where($field, "*{$value}*");
  }

  /**
   * Add a fuzzy condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function fuzzy($field, $value = NULL) {
    if (is_null($value)) {
      return $this->where("{$field}~");
    }

    return $this->where($field, "{$value}~");
  }

  /**
   * Add a begins with condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function orBeginsWith($field, $value = NULL) {
    if (is_null($value)) {
      return $this->orWhere("{$field}*");
    }

    return $this->orWhere($field, "{$value}*");
  }

  /**
   * Add a ends with condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function orEndsWith($field, $value = NULL) {
    if (is_null($value)) {
      return $this->orWhere("*{$field}");
    }

    return $this->orWhere($field, "*{$value}");
  }

  /**
   * Add a contains condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function orContains($field, $value = NULL) {
    if (is_null($value)) {
      return $this->orWhere("*{$field}*");
    }

    return $this->orWhere($field, "*{$value}*");
  }

  /**
   * Add a fuzzy condition.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return $this
   */
  public function orFuzzy($field, $value = NULL) {
    if (is_null($value)) {
      return $this->orWhere("{$field}~");
    }

    return $this->orWhere($field, "{$value}~");
  }
}
