<?php

namespace Tests;

use StatonLab\TripalTestSuite\TripalTestCase;

class TestCase extends TripalTestCase{

  /**
   * Holds tmp indices.
   *
   * @var array
   */
  protected $_indices = [];

  /**
   * Remove all temporary indices.
   *
   * @throws \Exception
   */
  protected function tearDown() {
    foreach ($this->_indices as $index) {
      if (is_array($index) && isset($index['index'])) {
        $this->deleteIndex($index['index']);
      }
    }

    parent::tearDown();
  }

  /**
   * @param string $name
   *
   * @throws \Exception
   */
  public function deleteIndex($name) {
    $es = new \ES\Common\Instance(getenv('ES_HOST'));

    $es->deleteIndex($name);
  }

  /**
   * @param string $name
   *
   * @throws \Exception
   */
  public function makeIndex($name = NULL) {
    $es = new \ES\Common\Instance(getenv('ES_HOST'));

    if (is_null($name)) {
      return $name;
    }

    $index = $es->setIndexParams($name)->createIndex();

    $this->_indices[] = $index;

    return $index;
  }
}
