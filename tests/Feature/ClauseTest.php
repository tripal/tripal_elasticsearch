<?php

namespace Tests\Feature;

use ES\Query\BuilderContract;
use ES\Query\Clause;
use Tests\TestCase;

class ClauseTest extends TestCase{

  /** @test */
  public function testThatWeCanBuildWhereQueriesWithoutUsingClosures() {
    $clause = new Clause();
    $query = $clause->where('field', 'value')->where('value')->orWhere('value');
    $this->assertEquals('field:value AND value OR value', $query->build());
  }

  /** @test */
  public function testThatClosuresGenerateEnclosedParameters() {
    $clause = new Clause();

    $query = $clause->where(
      function (BuilderContract $query) {
        $query->where('f', 'v');
        $query->where('f', 'v');
      }
    )->orWhere(
      function (BuilderContract $query) {
        $query->where('f', 'v');
        $query->orWhere('v');
      }
    )->build();

    $this->assertEquals('(f:v AND f:v) OR (f:v OR v)', $query);
  }
}
