<?php

namespace Test\Feature;

use ES\Common\Instance;
use StatonLab\TripalTestSuite\DBTransaction;
use Tests\TestCase;

class InstanceTest extends TestCase{

  use DBTransaction;

  protected $old_host = NULL;

  /**
   * @throws \StatonLab\TripalTestSuite\Exceptions\TripalTestSuiteException
   */
  public function setUp() {
    parent::setUp();

    $this->old_host = NULL;
  }

  /**
   * @throws \Exception
   */
  public function tearDown() {
    parent::tearDown();

    if (!is_null($this->old_host)) {
      putenv("ES_HOST=$this->old_host");
    }
  }

  /** @test */
  public function testThatConnectionToAnInvalidHostFails() {
    variable_del('elasticsearch_host');
    $this->old_host = getenv('ES_HOST');
    putenv('ES_HOST');

    $this->expectException(\Exception::class);

    // Set the host to false to simulate a non-existent host
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

    $es = $this->makeInstance();
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
