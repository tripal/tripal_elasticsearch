<?php

namespace ES\Indices;

abstract class Index{

  use HandlesIndices;

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
  protected $index;

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
   * Clause.
   *
   * @var \ES\Query\Clause
   */
  protected $clause;

  /**
   * Index constructor.
   *
   * @param array $data Fill the object with data.
   *
   * @throws \Exception
   */
  public function __construct($data = []) {
    $this->instance = new \ES\Common\Instance();
    $this->fill($data);
  }

  /**
   * @param $data
   * @param bool $id
   */
  public function create($data, $id = FALSE) {
    $this->instance->createEntry(
      $this->getIndexName(),
      $this->getIndexType(),
      $id,
      $data
    );
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

  public function save() {

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
   * @return bool
   *   Whether the attribute has been set.
   */
  public function __isset($name) {
    return isset($this->attributes[$name]);
  }
}
