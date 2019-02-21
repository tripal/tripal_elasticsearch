<?php

namespace ES\Models;

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
   * @var array
   */
  protected $fields = [];

  /**
   * The id of the entry.
   *
   * @var int
   */
  protected $id;

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
   * @param array $data Fill the object with data.
   * @param string $id The ES given id for the object.
   * @param Instance $instance The ES instance.
   *
   * @throws \Exception
   */
  public function __construct($data = [], $id = FALSE, Instance $instance = NULL) {
    $this->instance = $instance ?? new Instance();
    $this->fill($data);

    $this->id = $id;

    $this->builder = new Builder($this->index);
  }

  /**
   * Add a where clause.
   *
   * @param string $field The field.
   * @param string $value The value
   *
   * @return $this
   *   The object.
   * @see \ES\Query\Clause::where()
   */
  public function where($field, $value = NULL) {
    $this->builder->where($field, $value);

    return $this;
  }

  /**
   * Add an or clause.
   *
   * @param string $field The field.
   * @param string $value The value
   *
   * @return $this
   *   The object.
   * @see \ES\Query\Clause::orWhere()
   */
  public function orWhere($field, $value = NULL) {
    $this->builder->where($field, $value);

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

  /**
   * Save the data.
   *
   * @return $this
   *    The object.
   * @throws  \Exception
   */
  public function save() {
    if ($this->exists()) {
      $data = $this->instance->update(
        $this->getIndexName(),
        $this->getIndexType(),
        $this->id ?? FALSE,
        $this->attributes
      );

      $this->id = $data['_id'];

      return $this;
    }

    // There data does not exist so
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
   *    Whether the record exists.
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
   *    The number of documents in the index.
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
  public static function createOrUpdate($data, $id = FALSE) {
    $record = new static($data);

    $record->setID($id);
    $record->save();

    return $record;
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
   * @param array $data
   * @param bool $id
   *
   * @return \ES\Models\Model
   * @throws \Exception
   */
  public static function create(array $data, $id = FALSE) {
    $index = new static($data);

    $index->setID($id);
    $index->save();

    return $index;
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
   * Get the fields for the model.
   *
   * @return array
   */
  public function getFields() {
    return $this->fields;
  }

  /**
   * Get an attribute.
   *
   * @param string $name The name of attribute.
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
   * @param string $name The name of the attribute.
   *
   * @return bool
   *   Whether the attribute has been set.
   */
  public function __isset($name) {
    return isset($this->attributes[$name]);
  }
}
