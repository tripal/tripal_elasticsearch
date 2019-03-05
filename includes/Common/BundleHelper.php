<?php

namespace ES\Common;

class BundleHelper{

  /**
   * Gets a list of bundles with their associated cv terms and accessions.
   *
   * @return array
   *   An array of bundles.
   */
  public function getBundles() {
    $query = db_query(
      'SELECT tb.name AS name,
            tb.label AS label,
            tb.type AS type,
            tb.term_id AS term_id,
            tv.vocabulary AS cv_name,
            tt.accession AS accession
      FROM tripal_bundle tb
      INNER JOIN tripal_term tt ON tb.term_id = tt.id
      INNER JOIN tripal_vocab tv ON tt.vocab_id = tv.id'
    );
    return $query->fetchAll();
  }

  /**
   * @param $name
   *
   * @return mixed
   */
  public function getBundleByName($name) {
    $query = db_query(
      'SELECT tb.name AS name,
            tb.label AS label,
            tb.type AS type,
            tb.term_id AS term_id,
            tv.vocabulary AS cv_name,
            tt.accession AS accession
      FROM tripal_bundle tb
      INNER JOIN tripal_term tt ON tb.term_id = tt.id
      INNER JOIN tripal_vocab tv ON tt.vocab_id = tv.id
      WHERE tb.name = :name',
      [
        ':name' => $name,
      ]
    );
    return $query->fetchObject();
  }

  /**
   * Given a term and accession, get all bundles.
   *
   * @param string $cv_name The vocabulary name such as NCIT.
   * @param string $accession The accession such as 0000044.
   *
   * @return array|null
   *    An array of bundles. Null if no bundles exist.
   */
  public function getBundleByTerm($cv_name, $accession) {
    // Get the bundle
    $bundles = db_query(
      'SELECT tb.name AS name,
              tb.label AS label,
              tb.type AS type,
              tb.term_id AS term_id,
              tv.vocabulary AS cv_name,
              tt.accession AS accession
      FROM tripal_bundle tb
      INNER JOIN tripal_term tt ON tb.term_id = tt.id
      INNER JOIN tripal_vocab tv ON tt.vocab_id = tv.id
      WHERE tv.vocabulary = :cv_name 
            AND tt.accession = :accession',
      [
        ':cv_name' => $cv_name,
        ':accession' => $accession,
      ]
    )->fetchAll();

    if (count($bundles) === 0) {
      return NULL;
    }

    return $bundles;
  }

  /**
   * Given a bundle, get all available fields.
   *
   * @param object $bundle The bundle object as returned from the db
   *   (tripal_bundle table).
   *
   * @return array|mixed
   */
  public function getFieldsByBundle($bundle) {
    $fields = field_info_instances($bundle->type, $bundle->name);

    // Format the results into a simple low-memory array
    $formatted = [];
    foreach ($fields as $field => $data) {
      $formatted[] = (object) [
        'id' => $data['id'],
        'name' => $field,
        'label' => $data['label'],
      ];
    }

    return $formatted;
  }

  /**
   * Get a list of fields for all bundles that match the given term.
   *
   * @param string $cv_name The vocabulary name.
   * @param string $accession The term accesstion.
   *
   * @return array|null
   *   The list of array as ['id', 'name', 'label']. Null if no bundle exists.
   */
  public function getFieldsByBundleTerm($cv_name, $accession) {
    $bundles = $this->getBundleByTerm($cv_name, $accession);

    if (is_null($bundles)) {
      return NULL;
    }

    // It is likely that this loop will run only once since the possibility of
    // having multiple bundles matching a single term is extra rare. We have to
    // do this since there is no DB constraint on the aforementioned assertion.
    $fields = [];
    foreach ($bundles as $bundle) {
      $fields = $fields + $this->getFieldsByBundle($bundle);
    }

    return $fields;
  }

  /**
   * Gets node types in the same format as bundles.
   *
   * @return array
   *    A list of node types.
   */
  public function getNodeTypes() {
    $node_types = db_select('node_type', 'nt')
      ->fields('nt', ['type', 'name'])
      ->execute()
      ->fetchAll();

    $categories = [];

    foreach ($node_types as $node_type) {
      $categories[] = [
        'name' => $node_type->type,
        'label' => $node_type->name,
        'type' => 'node',
        'term_id' => NULL,
        'cv_term' => NULL,
        'accession' => NULL,
        'fields' => [],
      ];
    }

    return $categories;
  }

  /**
   * Get a list of bundles and attach fields.
   *
   * @return array
   *   A list of bundles with fields attached.
   */
  public function getBundlesWithFields() {
    $bundles = $this->getBundles();

    $categories = [];
    foreach ($bundles as $bundle) {
      $fields = $this->getFieldsByBundle($bundle);
      $bundle->fields = $fields;
      $categories[] = $bundle;
    }

    return $categories;
  }
}
