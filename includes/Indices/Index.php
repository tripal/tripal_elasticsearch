<?php

namespace ES\Indices;

use ES\Common\Instance;
use ES\Exceptions\IndexExistsException;
use ES\Exceptions\IndexMissingException;
use ES\Models\Model;
use Exception;

/**
 * Class Index.
 *
 * @package ES\Indices
 */
class Index {

  /**
   * The ES instance.
   *
   * @var \ES\Common\Instance
   */
  private $instance;

  /**
   * The name of the index.
   *
   * @var string
   */
  protected $index = NULL;

  /**
   * Fields mappings.
   *
   * @var array
   */
  protected $fields = [];

  /**
   * Index constructor.
   *
   * @param array $data
   *   Fill the object with data.
   * @param string $id
   *   The ES given id for the object.
   * @param \ES\Common\Instance $instance
   *   The ES instance.
   *
   * @throws \Exception
   */
  public function __construct(Instance $instance = NULL) {
    $this->instance = $instance ?? new Instance();
  }

  /**
   * Creates an index from a given model.
   *
   * @param \ES\Models\Model $model
   *   The model to create an index for.
   *
   * @return array
   *
   * @throws \Exception
   * @throws \ES\Exceptions\IndexExistsException
   */
  public function createFromModel(Model $model) {
    $this->setName($model->getIndexName());
    $this->setFields($model->getFields());

    return $this->create();
  }

  /**
   * Create the index.
   *
   * @throws \Exception
   * @throws \ES\Exceptions\IndexExistsException
   */
  public function create() {
    if (!$this->index || empty($this->index)) {
      throw new \Exception('Index name must be specified to create the index.');
    }

    if (empty($this->fields)) {
      throw new \Exception(
        'Indices cannot be created if fields are not specified.'
      );
    }

    if ($this->instance->hasIndex($this->index)) {
      throw new IndexExistsException(
        'Index ' . $this->index . ' already exists.'
      );
    }

    $shards = 5;
    $replicas = 0;
    $tokenizer = 'standard';
    $filters = [];

    return $this->instance->setIndexParams(
      $this->index,
      $shards,
      $replicas,
      $tokenizer,
      $filters,
      $this->fields
    )->createIndex();
  }

  /**
   * Delete the index.
   *
   * @return array|bool
   *
   * @throws \ES\Exceptions\IndexMissingException
   * @throws \Exception
   */
  public function delete() {
    if (!$this->index || empty($this->index)) {
      throw new \Exception('Index name must be specified to create the index.');
    }

    db_query(
      'DELETE FROM tripal_elasticsearch_indices WHERE index_name = :name',
      [':name' => $this->index]
    );

    if (!$this->instance->hasIndex($this->index)) {
      throw new IndexMissingException(
        'The index ' . $this->index . ' cannot be deleted because it does not exist.'
      );
    }

    return $this->instance->deleteIndex($this->index);
  }

  /**
   * Set the index name.
   *
   * @param $index
   *
   * @return $this
   */
  public function setName($index) {
    $this->index = $index;

    return $this;
  }

  /**
   * Get the index name.
   *
   * @return string
   */
  public function getName() {
    return $this->index;
  }

  /**
   * @param array $fields
   *
   * @return $this
   */
  public function setFields(array $fields) {
    $this->fields = $fields;

    return $this;
  }

  /**
   * @return array
   * @throws \Exception
   */
  public function getFields($refresh = FALSE) {
    if (!$refresh && !empty($this->fields)) {
      return array_keys($this->fields);
    }

    if (empty($this->index)) {
      throw new Exception('Index name must be provided before getting fields');
    }

    $data = $this->instance->getIndexMappings($this->index);

    $data = $data[$this->index];
    $fields = $data['mappings']['_default_']['properties'] ?? [];

    $results = [];

    foreach ($fields as $field => $props) {
      $results[$field] = $props['type'];
    }

    return $results;
  }

}
