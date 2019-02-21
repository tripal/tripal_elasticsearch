<?php

namespace Tests\Feature;

use ES\Query\Builder;
use Tests\TestCase;

class QueryBuilderTest extends TestCase{

  /** @test */
  public function testThatParamsBuildCorrectly() {
    $builder = new Builder('entities');

    $builder->setID('some_id')->where('field', 'value')->orWhere(
        'other',
        'value'
      )->setType('type')->highlight('highlighted_field')->range(0, 100);

    // Build with range
    $params = $builder->build();

    $this->assertArrayHasKey('body', $params);
    $this->assertArrayHasKey('from', $params);
    $this->assertArrayHasKey('size', $params);
    $this->assertArrayHasKey('type', $params);
    $this->assertArrayHasKey('index', $params);
    $this->assertArrayHasKey('query', $params['body']);
    $this->assertArrayHasKey(
      'query',
      $params['body']['query']['simple_query_string']
    );
    $this->assertArrayHasKey('highlight', $params['body']);
    $this->assertArrayHasKey('fields', $params['body']['highlight']);
    $this->assertArrayHasKey(
      'highlighted_field',
      $params['body']['highlight']['fields']
    );

    // Check query value
    $this->assertEquals(
      'field:value OR other:value',
      $params['body']['query']['simple_query_string']['query']
    );

    // Build without range
    $params = $builder->build(FALSE);

    // Verify range doesn't exist
    $this->assertArrayNotHasKey('from', $params);
    $this->assertArrayNotHasKey('size', $params);
  }

  /** @test */
  public function testThatAnExceptionIsThrownWhenIndexIsNotProvided() {
    $builder = new Builder();

    $this->expectException(\Exception::class);
    $builder->build();
  }

  /** @test */
  public function testThatAnExceptionIsThrownWhenQueryIsNotProvided() {
    $builder = new Builder('test');

    $this->expectException(\Exception::class);
    $builder->build();
  }
}
