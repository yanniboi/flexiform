<?php

/**
 * @file
 * Contains \Drupal\flexiform\FlexiformEntityFormDisplay.
 */

namespace Drupal\flexiform;

use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Entity\Entity\EntityFormDisplay;
use Drupal\Core\Entity\FieldableEntityInterface;
use Drupal\Core\Field\FieldDefinitionInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\flexiform\FormEntity\FlexiformFormEntityManager;

/**
 * Defines a class to extend EntityFormDisplays to work with multiple entity
 * forms.
 */
class FlexiformEntityFormDisplay extends EntityFormDisplay implements FlexiformEntityFormDisplayInterface {

  /**
   * The flexiform form Entity Manager.
   *
   * @var \Drupal\flexiform\FormEntity\FlexiformFormEntityManager
   */
  protected $formEntityManager;

  /**
   * {@inheritdoc}
   */
  public function buildForm(FieldableEntityInterface $entity, array &$form, FormStateInterface $form_state) {
    $this->getFormEntityManager($entity);

    // Set #parents to 'top-level' by default.
    $form += array('#parents' => array());

    // Let each widget generate the form elements.
    foreach ($this->getComponents() as $name => $options) {
      if ($widget = $this->getRenderer($name)) {
        if (strpos($name, ':')) {
          list($namespace, $field_name) = explode(':', $name, 2);
          $items = $this->getFormEntityManager($entity)->getEntity($namespace)->get($field_name);
        }
        else {
          $items = $entity->get($name);
        }
        $items->filterEmptyItems();

        $form[$name] = $widget->form($items, $form, $form_state);
        $form[$name]['#access'] = $items->access('edit');

        // Assign the correct weight. This duplicates the reordering done in
        // processForm(), but is needed for other forms calling this method
        // directly.
        $form[$name]['#weight'] = $options['weight'];

        // Associate the cache tags for the field definition & field storage
        // definition.
        $field_definition = $this->getFieldDefinition($name);
        $this->renderer->addCacheableDependency($form[$name], $field_definition);
        $this->renderer->addCacheableDependency($form[$name], $field_definition->getFieldStorageDefinition());
      }
    }

    // Associate the cache tags for the form display.
    $this->renderer->addCacheableDependency($form, $this);

    // Add a process callback so we can assign weights and hide extra fields.
    $form['#process'][] = array($this, 'processForm');
  }

  /**
   * {@inheritdoc}
   */
  public function getFormEntityConfig() {
    return [
      'current_user' => [
        'plugin' => 'current_user',
        'map' => [],
        'entity_type' => 'user',
        'bundle' => 'user',
        'label' => 'Current User',
      ],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function getFieldDefinition($field_name) {
    if ($definition = parent::getFieldDefinition($field_name)) {
      return $definition;
    }

    // Find our own.
    if (strpos($field_name, ':')) {
      list($namespace, $form_entity_field_name) = explode(':', $field_name, 2);
      return $this->getFormEntityFieldDefinition($namespace, $form_entity_field_name);
    }
  }

  /**
   * Get the form entity manager.
   */
  public function getFormEntityManager(FieldableEntityInterface $entity = NULL) {
    if (empty($this->formEntityManager)) {
      $this->formEntityManager = new FlexiformFormEntityManager($this, $entity);
    }

    return $this->formEntityManager;
  }

  /**
   * Get the form entity field definitions.
   */
  public function getFormEntityFieldDefinitions() {
    $definitions = [];
    foreach ($this->getFormEntityManager()->getFormEntities() as $namespace => $form_entity) {
      // Ignore the base entity.
      if ($namespace == '') {
        continue;
      }

      $display_context = $this->displayContext;
      $definitions[$namespace] = array_filter(
        $this->entityManager()->getFieldDefinitions($form_entity->getEntityType(), $form_entity->getBundle()),
        function (FieldDefinitionInterface $field_definition) use ($display_context) {
          return $field_definition->isDisplayConfigurable($display_context);
        }
      );
    }
    return $definitions;
  }

  /**
   * Get a specific form entity field definition.
   */
  public function getFormEntityFieldDefinition($namespace, $field_name) {
    $definitions = $this->getFormEntityFieldDefinitions();
    if (isset($definitions[$namespace][$field_name])) {
      return $definitions[$namespace][$field_name];
    }
  }
}