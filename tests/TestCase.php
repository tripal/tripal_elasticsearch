<?php

namespace Tests;

use Faker\Factory;
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
   * @return \Faker\Generator
   */
  public function makeFaker() {
    return Factory::create();
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

    $index = $es->setIndexParams($name, 5, 0, 'standard', [], $fields)
      ->createIndex();

    $this->_indices[] = $index;

    return $index;
  }

  /**
   * Creating records is asynchronous in ES. This method adds the wait time
   * required to make sure the record was created before making assertions.
   *
   * @param string $index Index name (and type!)
   * @param array $data
   *
   * @throws \Exception
   */
  public function createRecord($index, $data) {
    $instance = $this->makeInstance();
    $record = $instance->createEntry($index, $index, FALSE, $data);
    sleep(1);
    return $record;
  }
}
