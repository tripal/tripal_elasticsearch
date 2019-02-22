<?php

namespace Tests\Feature;

use ES\Indices\Index;
use ES\Models\Model;
use Tests\TestCase;

class ModelTest extends TestCase{

  /**
   * @test
   * @throws \Exception
   */
  public function testAbilityToQueryAnIndex() {
    $instance = $this->makeInstance();

    $name = uniqid();
    $this->makeIndex($name);

    // Insert data into the index
    $this->createRecord($name, [
      'content' => 'data'
    ]);

    $model = new Model($instance);
    $model->setIndexName($name);
    $model->setIndexType($name);

    $data = $model->where('*')->search();
    $this->assertEquals($data['hits']['total'], 1);
  }
}
