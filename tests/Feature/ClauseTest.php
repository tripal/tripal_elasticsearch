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
    $this->assertEquals(3, count($query->build()));
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

    $this->assertEquals(4, count($query));
  }

  /** @test */
  public function testAddonBuilders() {
    $clause = new Clause();

    $query = $clause->beginsWith('test')
      ->contains('test')
      ->endsWith('test')
      ->fuzzy('test')
      ->build();

    $this->assertEquals(4, count($query));
  }

  /** @test */
  public function testAddonBuildersThatUseOr() {
    $clause = new Clause();

    $query = $clause->orBeginsWith('test')->orContains(
      'test'
    )->orEndsWith('test')->orFuzzy('test')->build();

    $this->assertEquals(4, count($query));
  }

  /** @test */
  public function testAddonsWithFields() {
    $clause = new Clause();

    $query = $clause->beginsWith('field', 'test')
      ->contains('field', 'test')
      ->endsWith('field', 'test')
      ->fuzzy('field', 'test')
      ->build();

    $this->assertEquals(4, count($query));
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

    $this->assertEquals(5, count($query));
  }
}
