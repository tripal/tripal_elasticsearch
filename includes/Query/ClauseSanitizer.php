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
}
