<?php
/**
 * @file
 * Specify builder classes for flexiforms.
 */

/**
 * Interface for all Flexiform Builders.
 */
interface FlexiformBuilderInterface {

  /**
   * Build the form for this flexiform.
   */
  public function form($form, &$form_state);

  /**
   * Validate the input for the form.
   */
  public function formValidate($form, &$form_state);

  /**
   * Submit the form.
   */
  public function formSubmit($form, &$form_state);
}

/**
 * Base class for flexiform builders.
 */
class FlexiformBuilder implements FlexiformBuilderInterface {

  /**
   * The flexiform object.
   */
  protected $flexiform;

  /**
   * The base entity.
   */
  protected $base_entity;

  /**
   * The base entity type.
   */
  protected $base_entity_type;

  /**
   * Construct the Flexiform Builder.
   */
  public function __construct($flexiform, $base_entity) {
    $this->flexiform = $flexiform;
    $this->base_entity = $base_entity;
    $this->base_entity_type = $flexiform->base_entity;
  }

  /**
   * Build the form for this flexiform.
   */
  public function form($form, &$form_state) {
    $form['#flexiform'] = $this->flexiform;
    $form['#flexiform_base_entity'] = $this->base_entity;
    $form['#flexiform_builder'] = $this;

    return $form;
  }

  /**
   * Validate the input for the form.
   */
  public function formValidate($form, &$form_state) {}

  /**
   * Submit the form.
   */
  public function formSubmit($form, &$form_state) {}
}

/**
 * Flexiform Builder Class for the complex flexiforms.
 */
class FlexiformBuilderFlexiform extends FlexiformBuilder {

  /**
   * Build the form for this flexiform.
   */
  public function form($form, &$form_state) {
    $form = parent::form($form, &$form_state);

    $form['#flexiform_entities'] = flexiform_prepare_entities($this->flexiform, $this->base_entity);

    foreach($form['#flexiform_entities'] as $namespace => $entity) {
      $elements = isset($this->flexiform->elements[$namespace]) ? $this->flexiform->elements[$namespace] : FALSE;

      // If there are no elements for this namespace skip it.
      if (!$elements) {
        continue;
      }

      flexiform_embed_entity_elements($namespace, $this->flexiform, $entity, $elements, $form, $form_state);
    }

    return $form;
  }

  /**
   * Validate the input for the form.
   */
  public function formValidate($form, &$form_state) {
    foreach (element_children($form) as $element_namespace) {
      if (empty($form[$element_namespace]['#flexiform_element'])) {
        continue;
      }

      $element = $form[$element_namespace]['#flexiform_element'];
      $entity = $form['#flexiform_entities'][$element->getEntityNamespace()];
      $element->formValidate($form, $form_state, $entity);
    }
  }

  /**
   * Submit the form.
   */
  public function formSubmit($form, &$form_state) {
    $flexiform = $form['#flexiform'];

    foreach (element_children($form) as $element_namespace) {
      if (empty($form[$element_namespace]['#flexiform_element'])) {
        continue;
      }

      $element = $form[$element_namespace]['#flexiform_element'];
      $entity = $form['#flexiform_entities'][$element->getEntityNamespace()];
      $element->formSubmit($form, $form_state, $entity);
    }

    foreach ($flexiform->entities as $entity_namespace => $settings) {
      if (isset($settings['getter'])) {
        $getter = flexiform_entity_getter_info($settings['getter']);

        if (isset($getter['file']) && file_exists(drupal_get_path('module', $getter['module']) . '/' . $getter['file'])) {
          include_once drupal_get_path('module', $getter['module']) . '/' . $getter['file'];
        }

        $callback = $getter['setter callback'];
        if (function_exists($callback)) {
          $base_entity = $form['#flexiform_base_entity'];
          $entity = $form['#flexiform_entities'][$entity_namespace];
          $entities = $form['#flexiform_entities'];
          $callback($flexiform, $base_entity, $entity, $settings['entity_type'], $entities, $entity_namespace, $getter);
        }
      }
    }

    if (module_exists('rules')) {
      rules_invoke_event_by_args('flexiform_submit_'.$this->flexiform->form, $form['#flexiform_entities']);
    }
  }
}

/**
 * Flexiform Builder Class for stock entity forms.
 */
class FlexiformBuilderEntityForm extends FlexiformBuilder {

  /**
   * Attach all fields for this entity to the form.
   */
  public function form($form, &$form_state) {
    $form = parent::form($form, &$form_state);
    $form['#tree'] = TRUE;
    field_attach_form($this->base_entity_type, $this->base_entity, $form, $form_state);

    return $form;
  }

  /**
   * Validate the Fields
   */
  public function formValidate($form, &$form_state) {
    field_attach_form_validate($this->base_entity_type, $this->base_entity, $form, $form_state);
  }

  /**
   * Submit the form.
   */
  public function formSubmit($form, &$form_state) {
    field_attach_submit($this->base_entity_type, $this->base_entity, $form, $form_state);
    entity_save($this->base_entity_type, $this->base_entity);

    if (module_exists('rules')) {
      rules_invoke_event_by_args('flexiform_submit_'.$this->flexiform->form, array('base_entity' => $this->base_entity));
    }
  }
}