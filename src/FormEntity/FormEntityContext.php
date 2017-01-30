<?php

namespace Drupal\flexiform\FormEntity;

use Drupal\Core\Plugin\Context\Context;

/**
 * Class for form entity contexts.
 */
class FormEntityContext extends Context implements FormEntityContextInterface {

  /**
   * The entity namespace.
   *
   * @var string
   */
  protected $entityNamespace;

  /**
   * {@inheritdoc}
   */
  public function setEntityNamespace($namespace) {
    $this->entityNamespace = $namespace;
  }

  /**
   * {@inheritdoc}
   */
  public function getEntityNamespace() {
    return $this->entityNamespace;
  }
}