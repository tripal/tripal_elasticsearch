<?php

namespace ES\Indices;

trait HandlesIndices{

  /**
   * @return string
   */
  private function getIndexName() {
    return $this->index ?? strtolower(get_class($this));
  }

  /**
   * @return mixed
   */
  private function getIndexType() {
    return $this->type;
  }
}
