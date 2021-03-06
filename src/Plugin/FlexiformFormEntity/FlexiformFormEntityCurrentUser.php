<?php

/**
 * @file
 * Contains \Drupal\flexiform\Plugin\FlexiformFormEntity\FlexiformFormEntityCurrentUser.
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
 *   id = "current_user",
 *   label = @Translation("Current User"),
 *   entity_type = "user",
 *   bundle = "user"
 * )
 *
 */
class FlexiformFormEntityCurrentUser extends FlexiformFormEntityBase {

  /**
   * {@inheritdoc}
   */
  protected function getEntity() {
    $uid = \Drupal::currentUser()->id();
    return entity_load('user', $uid);
  }
}