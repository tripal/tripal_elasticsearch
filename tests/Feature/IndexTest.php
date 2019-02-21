<?php

namespace Tests\Feature;

use ES\Indices\Index;
use Tests\TestCase;

class IndexTest extends TestCase{

  /** @test */
  public function testThatCreatingAndDeletingIndicesWork() {
    $instance = $this->makeInstance();
    $index = new Index($instance);

    $name = uniqid();

    $data = $index->setIndexName($name)
      ->setFields([
        'content' => 'dynamic',
        'test' => 'text'
      ])
      ->createIndex();

    $this->assertTrue($data['acknowledged']);
    $this->assertEquals($index->getIndexName(), $data['index']);

    var_dump($index->getFields(true));

    $data = $index->deleteIndex();
    $this->assertTrue($data['acknowledged']);
  }
}
