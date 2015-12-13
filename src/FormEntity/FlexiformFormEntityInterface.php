<?php

/**
 * @file
 * Contains \Drupal\flexiform\FormEntity\FlexiformFormEntityBase.
 */

namespace Drupal\flexiform\FormEntity;

use Drupal\Core\Plugin\ContextAwarePluginInterface;

interface FlexiformFormEntityInterface extends ContextAwarePluginInterface {

  /**
   * Get the context.
   *
   * @return \Drupal\Core\Plugin\Context\ContextInterface
   */
  public function getFormEntityContext();

  /**
   * Get the context definition.
   */
  public function getFormEntityContextDefinition();

  /**
   * Get the label for this plugin.
   */
  public function getLabel();

  /**
   * Get the entity type.
   */
  public function getEntityType();

  /**
   * Get the bundle.
   */
  public function getBundle();

}