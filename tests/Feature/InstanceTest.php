<?php

namespace Test\Feature;

use ES\Common\Instance;
use StatonLab\TripalTestSuite\DBTransaction;
use Tests\TestCase;

class InstanceTest extends TestCase{

  use DBTransaction;

  /** @test */
  public function testThatConnectionToAnInvalidHostFails() {
    variable_del('elasticsearch_host');

    $this->expectException(\Exception::class);

    new \ES\Common\Instance();
  }

  /** @test */
  public function testThatWeCanSuccessfullyConnectToNonSpecifiedHost() {
    variable_set('elasticsearch_host', getenv('ES_HOST'));

    $es = new \ES\Common\Instance();

    $this->assertInstanceOf(Instance::class, $es);
  }

  /** @test */
  public function testThatCreatingAnIndexSucceeds() {
    $name = uniqid();
    $index = $this->makeIndex($name);

    $this->assertTrue($index['acknowledged']);
    $this->assertEquals($name, $index['index']);
  }

  /** @test */
  public function testCreatingAndUpdatingDocuments() {
    $name = uniqid();

    $this->makeIndex($name, ['content' => 'text']);

    $es = new Instance();
    $data = $es->createEntry(
      $name,
      $name,
      FALSE,
      [
        'content' => 'some text',
      ]
    );

    $this->assertTrue(is_array($data));
    $this->assertEquals('created', $data['result']);

    $id = $data['_id'];

    $data = $es->update(
      $name,
      $name,
      $id,
      [
        'content' => 'updated text!',
      ]
    );

    $this->assertTrue(is_array($data));
    $this->assertEquals('updated', $data['result']);

    // Verify that the record got updated
    $data = $es->getRecord($name, $name, $id);
    $this->assertTrue(is_array($data));
    $this->assertTrue($data['found']);
    $this->assertEquals('updated text!', $data['_source']['content']);
  }
}
