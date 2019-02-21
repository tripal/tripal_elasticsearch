<?php

namespace ES\Query;

class ClauseSanitizer{

  public function escape($value) {
    return stripslashes($value);
  }
}
