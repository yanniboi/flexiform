<?php

/**
 * @file
 * Contains \Drupal\flexiform\FormEntity\FlexiformFormEntityBase.
 */

namespace Drupal\flexiform\FormEntity;

use Drupal\Core\Plugin\ContextAwarePluginBase;
use Drupal\Core\Plugin\Context\Context;
use Drupal\Core\Plugin\Context\ContextInterface;
use Drupal\Core\Plugin\Context\ContextDefinition;

abstract class FlexiformFormEntityBase extends ContextAwarePluginBase implements FlexiformFormEntityInterface {

  /**
   * The flexiform entity manager.
   *
   * @var \Drupal\flexiform\FormEntity\FlexiformFormEntityManager
   */
  protected $formEntityManager;

  /**
   * The actual context, wraps the entity item.
   *
   * @var \Drupal\Core\Plugin\Context\ContextInterface.
   */
  protected $formEntityContext;

  /**
   * Whether or not the form entity has been prepared.
   */
  protected $prepared;

  /**
   * {@inheritdoc}
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition) {
    if (!isset($configuration['manager'])) {
      throw new \Exception('No Form Entity Manager Supplied');
    }

    // Set the form entity manager.
    $this->formEntityManager = $configuration['manager'];

    // Initialise the form entity context object.
    $this->initFormEntityContext($configuration);

    // Map the parameters for this form entity plugin so that
    // ContextAwarePluginBase can use them.
    $configuration['context'] = $this->mapFormEntityToContexts($configuration['context_map']);

    parent::__construct($configuration, $plugin_id, $plugin_definition);
  }

  /**
   * Map the parameters in the configuration to the entities using the entity
   * manager.
   *
   * @param array $map
   *   A map from the entity namespace to the context required.
   *
   * @return array
   *   Array of entities keyed by context key.
   */
  protected function mapFormEntityToContexts(array $map) {
    $contexts = [];
    foreach ($map as $key => $namespace) {
      $contexts[$key] = $this->formEntityManager->getEntity($namespace);
    }
    return $contexts;
  }

  /**
   * Initialize a context object for the formEntity.
   *
   * If the 'entity' key is set in the configuration then set the value
   * immediately and set this form entity as executed.
   */
  protected function initFormEntityContext($configuration) {
    $context_definition = new ContextDefinition('entity:'.$configuration['entity_type'], $configuration['label']);
    $context_definition->addConstraint('Bundle', $configuration['bundle']);
    $this->formEntityContext = new Context($context_definition);
  }

  /**
   * Get the context.
   */
  public function getFormEntityContext() {
    $this->prepareFormEntityContext();
    return $this->formEntityContext;
  }

  /**
   * Get the context definition.
   */
  public function getFormEntityContextDefinition() {
    return $this->formEntityContext->getContextDefinition();
  }

  /**
   * {@inheritdoc}
   */
  public function getLabel() {
    return $this->pluginDefinition['label'];
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

  /**
   * Prepare the form entity context.
   *
   * Attempt to find a value for the form entity context.
   */
  protected function prepareFormEntityContext() {
    if ($this->prepared) {
      return;
    }

    if ($entity = $this->getEntity()) {
      $this->formEntityContext = Context::createFromContext($this->formEntityContext, $entity);
    }

    $this->prepared = TRUE;
  }

  /**
   * Get the Entity.
   */
  abstract protected function getEntity();

}