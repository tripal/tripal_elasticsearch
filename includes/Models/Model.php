<?php

namespace ES\Models;

use ES\Common\Instance;
use ES\Query\Builder;

/**
 *
 */
class Model{

  /**
   * @var array
   */
  protected $attributes = [];

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
   * Optional index type.
   *
   * @var string
   */
  protected $type;

  /**
   * Fields mappings.
   *
   * The format must be ['field' => 'mapping type']
   *
   * @var array
   */
  protected $fields = [];

  /**
   * The id of the entry.
   *
   * @var int|bool
   */
  protected $id = FALSE;

  /**
   * Query Builder.
   *
   * @var \ES\Query\Builder
   */
  protected $builder;

  /**
   * Whether the entry exists in the index.
   *
   * @var bool
   */
  protected $exists = FALSE;

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
    $this->builder = new Builder($this->index);
  }

  /**
   * Add a where clause.
   *
   * @param string $field
   *   The field.
   * @param string $value
   *   The value.
   *
   * @return $this
   *   The object.
   *
   * @see \ES\Query\Clause::where()
   */
  public function where($field, $value = NULL) {
    $this->builder->where($field, $value);

    return $this;
  }

  /**
   * Add an or clause.
   *
   * @param string $field
   *   The field.
   * @param string $value
   *   The value.
   *
   * @return $this
   *   The object.
   *
   * @see \ES\Query\Clause::orWhere()
   */
  public function orWhere($field, $value = NULL) {
    $this->builder->orWhere($field, $value);

    return $this;
  }

  /**
   * Fills the object attributes with values.
   *
   * @param array $data
   */
  public function fill(array $data = []) {
    foreach ($data as $key => $value) {
      if (isset($this->fields[$key])) {
        $this->attributes[$key] = $value;
      }
    }
  }

  public function find($id) {
    $record = $this->instance->getRecord(
      $this->getIndexName(),
      $this->getIndexType(),
      $id
    );
  }

  /**
   * Save the data.
   *
   * @return $this
   *   The object.
   *
   * @throws \Exception
   */
  public function save() {
    $exists = FALSE;
    if ($this->id !== FALSE) {
      $record = $this->find($this->id);
      if ($record) {
        $this->id = $record->id;
        $exists = TRUE;
      }
    }
    else {
      $exists = $this->exists();
    }

    if ($exists) {
      $data = $this->instance->update(
        $this->getIndexName(),
        $this->getIndexType(),
        $this->id,
        $this->attributes
      );

      $this->id = $data['_id'];

      return $this;
    }

    // There data does not exist so.
    $data = $this->instance->createEntry(
      $this->getIndexName(),
      $this->getIndexType(),
      $this->id ?? FALSE,
      $this->attributes
    );

    $this->id = $data['_id'];

    return $this;
  }

  /**
   * Check whether the record exists.
   *
   * @return bool
   *   Whether the record exists.
   *
   * @throws \Exception
   */
  public function exists() {
    $this->exists = $this->count() > 0;

    return $this->exists;
  }

  /**
   * Count the number of documents in the index.
   *
   * @return int
   *   The number of documents in the index.
   *
   * @throws \Exception
   */
  public function count() {
    return (int) $this->instance->client->count($this->builder->build(FALSE));
  }

  /**
   * @param $data
   *
   * @throws \Exception
   * @return \ES\Models\Model
   */
  public function createOrUpdate($data, $id = FALSE) {
    $this->fill($data);
    $this->setID($id);
    $this->save();

    return $this;
  }

  /**
   * @param $id
   *
   * @return $this
   */
  public function setID($id) {
    $this->id = $id;

    $this->builder->setID($id);

    return $this;
  }

  /**
   * Get the ES given id.
   *
   * @return bool|int
   */
  public function getID() {
    return $this->id;
  }

  /**
   * @param array $data
   * @param bool $id
   *
   * @return \ES\Models\Model
   * @throws \Exception
   */
  public function create(array $data) {
    $this->fill($data);
    $this->setID(FALSE);
    $this->save();

    return $this;
  }

  /**
   * Set the index name.
   *
   * @param $index
   *
   * @return $this
   */
  public function setIndexName($index) {
    $this->index = $index;

    // Update the builder as well
    $this->builder->setIndex($index);

    return $this;
  }

  /**
   * Get the index name.
   *
   * @return string
   */
  public function getIndexName() {
    return $this->index;
  }

  /**
   * Set the type of the index.
   *
   * @param string $type
   *
   * @return $this
   */
  public function setIndexType($type) {
    $this->type = $type;

    // Update the builder.
    $this->builder->setType($type);

    return $this;
  }

  /**
   * Get the index type.
   *
   * @return string
   */
  public function getIndexType() {
    return $this->type;
  }

  /**
   * @param array fields
   *
   * @return $this
   */
  public function setFields($fields) {
    $this->fields = $fields;

    return $this;
  }

  /**
   * Get the fields for the model.
   *
   * @return array
   */
  public function getFields() {
    return $this->fields;
  }

  /**
   * @return array
   * @throws \Exception
   */
  public function search() {
    $params = $this->builder->build();
    $results = $this->instance->client->search($params);

    $this->builder->reset($this->index);

    return $results;
  }

  /**
   * Get query paramaters.
   *
   * @return array
   * @throws \Exception
   */
  public function getQuery() {
    return $this->builder->build();
  }

  /**
   * Get an attribute.
   *
   * @param string $name
   *   The name of attribute.
   *
   * @return mixed
   *   The value of the attribute if it exists.
   */
  public function __get($name) {
    if (array_key_exists($name, $this->attributes)) {
      return $this->attributes[$name];
    }

    if (method_exists(static::class, $name)) {
      return;
    }

    return $this->{$name};
  }

  /**
   * Check if an attribute isset.
   *
   * @param string $name
   *   The name of the attribute.
   *
   * @return bool
   *   Whether the attribute has been set.
   */
  public function __isset($name) {
    return isset($this->attributes[$name]);
  }
}
