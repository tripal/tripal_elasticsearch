<?php

namespace Test\Feature;

use ES\Common\Instance;
use StatonLab\TripalTestSuite\DBTransaction;
use Tests\TestCase;

class InstanceTest extends TestCase
{
    use DBTransaction;

   /** @test */
    public function testThatConnectionToAnInvalidHostFails()
    {
        variable_del('elasticsearch_host');

        $this->expectException(\Exception::class);

        new \ES\Common\Instance();
    }

    /** @test */
    public function testThatWeCanSuccessfullyConnectToNonSpecifiedHost()
    {
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
}
