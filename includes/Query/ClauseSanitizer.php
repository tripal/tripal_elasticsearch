<?php

namespace ES\Query;

class ClauseSanitizer{

  public function escape($value) {
    $value = stripslashes($value);
    $value = str_replace('\\', ' ', $value);
    $value = str_replace('+', ' ', $value);
    $value = str_replace('-', ' ', $value);
    $value = str_replace('^', '', $value);
    return str_replace(':', '\\:', $value);
  }
}
