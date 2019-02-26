<?php

namespace Tests\Feature;

use ES\Common\BundleHelper;
use StatonLab\TripalTestSuite\DBTransaction;
use Tests\TestCase;

class BundleHelperTest extends TestCase{

  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Get a term.
   *
   * @return mixed
   */
  public function getExistingTerm() {
    // Get an existing bundle
    $bundle = db_query('SELECT * FROM tripal_bundle LIMIT 1')->fetchObject();

    // Get a term from tripal_term
    return db_query(
      'SELECT tt.accession, tv.vocabulary AS cv_name
                      FROM tripal_term tt
                      INNER JOIN tripal_vocab tv ON tv.id = tt.vocab_id 
                      WHERE tt.id = :id LIMIT 1',
      [
        ':id' => $bundle->term_id,
      ]
    )->fetchObject();
  }

  /** @test */
  public function testGettingBundles() {
    $helper = new BundleHelper();

    $bundles = $helper->getBundles();
    $this->assertNotEmpty($bundles);

    $bundle = $bundles[0];
    $this->assertObjectHasAttribute('name', $bundle);
    $this->assertObjectHasAttribute('label', $bundle);
    $this->assertObjectHasAttribute('type', $bundle);
    $this->assertObjectHasAttribute('term_id', $bundle);
    $this->assertObjectHasAttribute('cv_name', $bundle);
    $this->assertObjectHasAttribute('accession', $bundle);
  }

  /** @test */
  public function testGettingBundlesOfAGivenTerm() {
    $term = $this->getExistingTerm();
    $helper = new BundleHelper();
    $bundles = $helper->getBundleByTerm($term->cv_name, $term->accession);
    $this->assertGreaterThanOrEqual(1, count($bundles));
  }

  /** @test */
  public function testGettingFieldsByTerm() {
    $term = $this->getExistingTerm();
    $helper = new BundleHelper();
    $fields = $helper->getFieldsByBundleTerm($term->cv_name, $term->accession);

    $this->assertNotEmpty($fields);
    $field = $fields[0];
    $this->assertObjectHasAttribute('id', $field);
    $this->assertObjectHasAttribute('name', $field);
    $this->assertObjectHasAttribute('label', $field);
  }
}
