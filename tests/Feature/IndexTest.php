<?php

namespace Tests\Feature;

use ES\Indices\Index;
use ES\Models\Model;
use Tests\TestCase;

/**
 *
 */
class IndexTest extends TestCase {

  /**
   * @test
   * @throws \Exception
   */
  public function testThatCreatingAndDeletingIndicesWork() {
    $instance = $this->makeInstance();
    $index = new Index($instance);

    $name = uniqid();

    $fields = [
      'content' => 'object',
      'test' => 'text',
    ];

    $data = $index->setName($name)->setFields($fields)->create();

    $this->assertTrue($data['acknowledged']);
    $this->assertEquals($index->getName(), $data['index']);

    $this->assertEquals($fields, $index->getFields(TRUE));

    $data = $index->delete();
    $this->assertTrue($data['acknowledged']);
  }

  /**
   * @test
   * @throws \Exception
   */
  public function testCreatingFromModel() {
    $instance = $this->makeInstance();
    $index = new Index($instance);
    $model = new Model();
    $name = uniqid();
    $model->setIndexName($name);
    $model->setFields(
      [
        'content' => 'object',
      ]
    );

    $data = $index->createFromModel($model);
    $this->assertTrue($data['acknowledged']);
    $this->assertEquals($name, $data['index']);

    $index->delete();
  }

}
