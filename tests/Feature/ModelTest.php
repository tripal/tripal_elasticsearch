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

    $faker = $this->makeFaker();

    $name = uniqid();
    $this->makeIndex(
      $name,
      [
        'content' => 'object',
      ]
    );

    $name1 = $faker->name;

    // Insert data into the index
    $this->createRecord(
      $name,
      [
        'content' => [
          'field1' => $name1,
          'field2' => $faker->name,
        ],
      ]
    );

    $model = new Model($instance);

    $model->setIndexName($name);
    $model->setIndexType($name);
    $model->where($name1)->orWhere('content.*', $name1)->orWhere(
      'content.field1',
      $name1
    );

    $data = $model->search();

    $this->assertEquals($data['hits']['total'], 1);
  }
}
