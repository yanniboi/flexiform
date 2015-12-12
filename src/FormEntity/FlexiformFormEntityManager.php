<?php

/**
 * @file
 * Contains \Drupal\flexiform\FormEntity\FlexiformFormEntityManager.
 */

namespace Drupal\flexiform\FormEntity;

use Drupal\flexiform\FlexiformEntityFormDisplayInterface;
use Drupal\Core\Entity\FieldableEntityInterface;
use Drupal\Core\Plugin\Context\Context;
use Drupal\Core\Plugin\Context\ContextDefinition;
use Drupal\Core\StringTranslation\StringTranslationTrait;

class FlexiformFormEntityManager {

  use StringTranslationTrait;

  /**
   * The form display config entity.
   *
   * @var \Drupal\flexiform\FlexiformEntityFormDisplayInterface
   */
  protected $formDisplay;

  /**
   * The Form entity plugins.
   *
   * @var \Drupal\flexiform\FlexiformFormEntityInterface[]
   */
   protected $formEntities;

  /**
   * Construct a new FlexiformFormEntityManager.
   *
   * @param \Drupal\flexiform\FlexiformEntityFormDisplayInterface $form_display
   *   The form display to manage the entities for.
   * @param \Drupal\Core\Entity\FieldableEntityInterface $base_entity
   *   The base entity of the form.
   */
  public function __construct(FlexiformEntityFormDisplayInterface $form_display, FieldableEntityInterface $entity = NULL) {
    $this->formDisplay = $form_display;

    $context_definition = new ContextDefinition(
      $this->formDisplay->getTargetEntityTypeId(),
      $this->t(
        'Base :entity_type',
        [
          ':entity_type' => \Drupal::service('entity_type.manager')
            ->getDefinition($this->formDisplay->getTargetEntityTypeId())->getLabel(),
        ]
      )
    );
    $context_definition->addConstraint('Bundle', $this->formDisplay->getTargetBundle());
    $this->formEntities[''] = new Context($context_definition, $entity);
  }

  /**
   * Get the context definitions from the form entity plugins.
   */
  public function getContextDefinitions() {
    $context_definitions = [];
    foreach ($this->formEntities as $namespace => $form_entity) {
      $context_definitions[$namespace] = $form_entity->getContextDefinition();
    }
    return $context_definitions;
  }

}