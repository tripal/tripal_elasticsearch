<?php

namespace ES\Query;

class ClauseSanitizer{

  /**
   * @param $value
   *
   * @return string
   */
  public function escape($value) {
    return stripslashes($value);
  }

  /**
   * @param $field
   *
   * @return mixed
   */
  public function escapeField($field) {
    return str_replace('.*', '.\\*', $field);
  }
}
