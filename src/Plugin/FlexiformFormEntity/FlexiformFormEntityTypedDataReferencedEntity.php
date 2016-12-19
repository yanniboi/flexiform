<?php

/**
 * @file
 * Contains \Drupal\flexiform\Plugin\FlexiformFormEntity\FlexiformFormEntityReferencedEntity.
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
 *   id = "referenced_entity",
 *   deriver = "\Drupal\flexiform\Plugin\Deriver\FormEntityTypedDataReferencedEntityDeriver"
 * )
 *
 */
class FlexiformFormEntityTypedDataReferencedEntity extends FlexiformFormEntityBase {

  /**
   * {@inheritdoc}
   */
  protected function getEntity() {
  }
}
