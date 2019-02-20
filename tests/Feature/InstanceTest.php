<?php

namespace Test\Feature;

use ES\Common\Instance;
use StatonLab\TripalTestSuite\DBTransaction;
use Tests\TestCase;

class ConnectToElasticSearchServerTest extends TestCase
{
    use DBTransaction;

    /**
     * Tests that an exception is thrown when no host is specified.
     *
     * @throws \Exception
     * @test
     */
    public function should_fail_to_connect_to_server_before_specifying_host()
    {
        variable_del('elasticsearch_host');

        $this->expectException('Exception');

        new \ES\Common\Instance();
    }

    /**
     * Tests whether connecting to ES server is possible.
     *
     * @throws \Exception
     * @test
     */
    public function should_successfully_connect_to_server_after_specifying_host()
    {
        variable_set('elasticsearch_host', 'http://localhost:9200');

        $es = new \ES\Common\Instance();

        $this->assertInstanceOf(Instance::class, $es);
    }
}
