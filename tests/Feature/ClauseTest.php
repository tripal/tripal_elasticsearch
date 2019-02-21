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

  /** @test */
  public function testAddonBuilders() {
    $clause = new Clause();

    $query = $clause->beginsWith('test')
      ->contains('test')
      ->endsWith('test')
      ->fuzzy('test')
      ->build();

    $this->assertEquals('test* AND *test* AND *test AND test~', $query);
  }

  /** @test */
  public function testAddonBuildersThatUseOr() {
    $clause = new Clause();

    $query = $clause->beginsWith('test')->orBeginsWith('test')->orContains(
        'test'
      )->orEndsWith('test')->orFuzzy('test')->build();

    $this->assertEquals('test* OR test* OR *test* OR *test OR test~', $query);
  }

  /** @test */
  public function testAddonsWithFields() {
    $clause = new Clause();

    $query = $clause->beginsWith('field', 'test')
      ->contains('field', 'test')
      ->endsWith('field', 'test')
      ->fuzzy('field', 'test')
      ->build();

    $this->assertEquals(
      'field:test* AND field:*test* AND field:*test AND field:test~',
      $query
    );
  }

  /** @test */
  public function testAddonsWithFieldsUsingOr() {
    $clause = new Clause();

    $query = $clause->beginsWith('field', 'test')
      ->orBeginsWith('field', 'test')
      ->orContains('field', 'test')
      ->orEndsWith('field', 'test')
      ->orFuzzy('field', 'test')
      ->build();

    $this->assertEquals(
      'field:test* OR field:test* OR field:*test* OR field:*test OR field:test~',
      $query
    );
  }

  /** @test */
  public function testRawQuery() {
    $clause = new Clause();

    $query = $clause->raw('test:value* AND test')->orRaw('test')->build();

    $this->assertEquals('test:value* AND test OR test', $query);
  }
}
