<?php

namespace Test\Feature;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class ConnectToElasticSearchServerTest extends TripalTestCase
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

        new \ESInstance();
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

        $es = new \ESInstance();

        $this->assertInstanceOf('\\ESInstance', $es);
    }
}
