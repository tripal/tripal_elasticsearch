<?php

namespace ES\Query;

interface BuilderContract{

  /**
   * Add a match condition joined with ANDs.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return mixed
   */
  public function where($field, $value = NULL);

  /**
   * Add a match condition joined with ORs.
   *
   * @param string $field The value to match against or the name of the field.
   * @param string $value The value if the field is specified.
   *
   * @return mixed
   */
  public function orWhere($field, $value = NULL);
}
