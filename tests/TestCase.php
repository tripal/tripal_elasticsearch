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
   * @return \ES\Common\Instance
   * @throws \Exception
   */
  public function makeInstance() {
    return new \ES\Common\Instance(getenv('ES_HOST'));
  }

  /**
   * @param string $name
   *
   * @throws \Exception
   */
  public function deleteIndex($name) {
    $es = $this->makeInstance();

    $es->deleteIndex($name);
  }

  /**
   * @param string $name
   *
   * @throws \Exception
   */
  public function makeIndex($name = NULL, $fields = []) {
    $es = $this->makeInstance();

    if (is_null($name)) {
      $name = uniqid();
    }

    $index = $es->setIndexParams($name, 1, 0, 'standard', [], $fields)
      ->createIndex();

    $this->_indices[] = $index;

    return $index;
  }
}
