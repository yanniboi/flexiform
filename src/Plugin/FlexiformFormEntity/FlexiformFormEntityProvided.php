<?php

/**
 * @file
 * Contains \Drupal\flexiform\Plugin\FlexiformFormEntity\FlexiformFormEntityProvided.
 */

namespace Drupal\flexiform\Plugin\FlexiformFormEntity;

use Drupal\Core\Entity\EntityInterface;
use Drupal\flexiform\Annotation\FlexiformFormEntity;
use Drupal\flexiform\FormEntity\FlexiformFormEntityBase;

/**
 * Form Entity plugin for entities that are passed in through the configuration
 * like the base entity.
 *
 * @FlexiformFormEntity(
 *   id = "provided",
 *   label = @Translation("Provided Entity"),
 *   no_ui = true
 * )
 *
 */
class FlexiformFormEntityProvided extends FlexiformFormEntityBase {

  /**
   * {@inheritdoc}
   */
  protected function getEntity() {
    if (isset($this->configuration['entity'])
        && ($this->configuration['entity'] instanceof EntityInterface)) {
      return $this->configuration['entity'];
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getEntityType() {
    return $this->configuration['entity_type'];
  }

  /**
   * {@inheritdoc}
   */
  public function getBundle() {
    return $this->configuration['bundle'];
  }
}