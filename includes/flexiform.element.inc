<?php
/**
 * @file
 * Controller class for flexiform elements.
 */

/**
 * Interface for all types of flexiform elements.
 */
interface FlexiformElementInterface {


}

/**
 * Base class for all FlexiformElements
 */
class FlexiformElement implements FlexiformElementInterface {

  /**
   * The flexiform entity this element is one.
   */
  protected $flexiform;

  /**
   * The namespace of this element.
   */
  protected $element_namespace;

  /**
   * The entity namespace of the entity this element is acting on.
   */
  protected $entity_namespace;

  /**
   * The entity type this field is on.
   */
  protected $entity_type;

  /**
   * The bundle this is on.
   */
  protected $bundle;

  /**
   * The weight of this element.
   */
  protected $weight;

  /**
   * The settings for this element.
   */
  protected $settings;

  /**
   * Construct the class.
   *
   * @param string $namespace
   *   The unique namespace of this form element.
   * @param array $settings
   *   An array of settings for the element. It must include at least the
   *   following keys:
   *     - 'entity_namespace' - The namespace of the entity this element is
   *     acting on. This is the key of the entity in the $flexiform->entities
   *     array.
   *     - 'entity_type' - The entity type of the entity for this element.
   */
  public function __construct($flexiform, $namespace, $settings = array()) {
    $this->element_namespace = $namespace;
    $this->settings = $settings;

    $this->entity_namespace = $settings['entity_namespace'];
    $this->entity_type = $flexiform->entities[$settings['entity_namespace']]['entity_type'];
    $this->bundle = $flexiform->entities[$settings['entity_namespace']]['bundle'];
  }

  /**
   * Get the label for this form element.
   */
  public function label() {
    return '';
  }

  /**
   * Get the name for this form element.
   */
  public function name() {
    return '';
  }

  /**
   * Get the element namespace for this form element.
   *
   * @return string
   *   The namespace of the element in the form.
   */
  public function getElementNamespace() {
    return $this->element_namespace;
  }

  /**
   * Get the entity namespace for this form element.
   *
   * @return string
   *   The namespace of the entity.
   */
  public function getEntityNamespace() {
    return $this->entity_namespace;
  }

  /**
   * Get the entity type for this element.
   */
  public function getEntityType() {
    return $this->entity_type;
  }

  /**
   * Get the weight of this form element.
   *
   * @return int
   *   The weight (ordering number) of the element.
   */
  public function getWeight() {
    return $this->weight;
  }

  /**
   * Set the weight of this form element.
   *
   * @param int $weight
   */
  public function setWeight($weight) {
    $this->weight = $weight;
  }

  /**
   * Get the form element.
   */
  public function form($form, &$form_state, $entity) {
    $form[$this->element_namespace] = array(
      '#weight' => $this->getWeight(),
    );

    return $form;
  }

  /**
   * Convert this object into a settings array.
   */
  public function toSettingsArray() {
    $settings = array(
      'type' => '',
      'entity_namespace' => $this->entity_namespace,
      'weight' => $this->weight,
    );

    return $settings;
  }

  /**
   * Get an element object.
   */
  public static function getElement($flexiform, $type, $namespace, $settings = array()) {
    $cached_elements = &drupal_static(__CLASS__, array());

    if (empty($cached_elements[$flexiform->form][$namespace])) {
      $class_name = 'FlexiformElement'.ucfirst($type);
      if (!class_exists($class_name)
        || !in_array('FlexiformElementInterface', class_implements($class_name))) {
        throw new Exception('Flexiform element type ' . $type . '  does not exist.');
      }

      $cached_elements[$flexiform->form][$namespace] = new $class_name($flexiform, $namespace, $settings);
    }

    return $cached_elements[$flexiform->form][$namespace];
  }

  /**
   * Get the namespace for an element.
   */
  public static function makeElementNamespace($type, $entity_namespace, $name) {
    return $entity_namespace . "__" . $type . "__" . $name;
  }

  /**
   * Split a namespace into pieces.
   */
  public static function splitElementNamespace($namespace) {
    $bits = explode('__', $namespace);
    if (count($bits) != 3) {
      drupal_set_message('Invalid Namspace Provided: ' . $namespace, 'Error');
      return FALSE;
    }

    $element['entity_namespace'] = $bits[0];
    $element['type'] = $bits[1];
    $element['element_name'] = $bits[2];

    return $element;
  }
}

/**
 * Class for Field API elemtents.
 */
class FlexiformElementField extends FlexiformElement{

  /**
   * The field name of the field.
   */
  protected $field_name;

  /**
   * The instance settings for the field
   */
  protected $instance;

  /**
   * The field settings for the field.
   */
  protected $field;

  /**
   * Overrides FlexiformElement::__construct().
   */
  public function __construct($flexiform, $namespace, $settings = array()) {
    parent::__construct($flexiform, $namespace, $settings);
    $this->field_name = $settings['field_name'];

    // Build the fake instance for this form (incase the widget has changed -
    // we would normally have to be careful of default_value but that doesn't
    // figure here, so we should be fine.
    $this->instance = !empty($settings['instance']) ? $settings['instance'] : array();
    $this->field = !empty($settings['field']) ? $settings['field'] : array();
    $this->weight = empty($settings['weight']) ? $settings['instance']['widget']['weight'] : $settings['weight'];
  }

  /**
   * Overrides FlexiformElement::label().
   *
   * @return string
   *   The field label.
   */
  public function label() {
    return check_plain($this->instance['label']);
  }

  /**
   * Overrides FlexiformElement::name().
   *
   * @return string
   *   A field name.
   */
  public function name() {
    return $this->field_name;
  }

  /**
   * Return the type.
   */
  public function type() {
    return 'Field';
  }

  /**
   * Get the widget type from the instance settings.
   *
   * @return string
   *   Machine name of the field widget in use.
   */
  public function getWidgetType() {
    $instance = $this->getInstance();
    return $instance['widget']['type'];
  }

  /**
   * Get the widget label from the instance settings.
   *
   * @return string
   *   Human readable name of the field widget.
   */
  public function getWidgetLabel() {
    $widget_types = field_info_widget_types();
    return $widget_types[$this->getWidgetType()]['label'];
  }

  /**
   * Get the instance array.
   *
   * At this point we merge the instance settings for the form into the normal
   * field instance settings.
   */
  public function getInstance() {
    if (empty($this->full_instance)) {
      $field_instance = field_info_instance($this->entity_type, $this->field_name, $this->bundle);
      $this->full_instance = array_replace_recursive($field_instance, $this->instance);

      // Default value might be a numerically indexed array, so we'll overwrite
      if (isset($this->instance['default_value'])) {
        $this->full_instance['default_value'] = $this->instance['default_value'];
      }
    }

    return $this->full_instance;
  }

  /**
   * Get the field array.
   *
   * At this point we merge the field settings for the form into the normal
   * field settings.
   */
  public function getField() {
    if (empty($this->full_field)) {
      $field = field_info_field($this->field_name);
      $this->full_field = array_replace_recursive($field, $this->field);
    }

    return $this->full_field;
  }

  /**
   * Return the form element for this FlexiformElement.
   */
  public function form($form, &$form_state, $entity, $language = LANGUAGE_NONE) {
    $form = parent::form($form, $form_state, $entity);

    $items = field_get_items($this->entity_type, $entity, $this->field_name, $language);

    if (!field_info_field($this->field_name)
      || !field_info_instance($this->entity_type, $this->field_name, $this->bundle)) {
      drupal_set_message("Error in " . __CLASS__ . "::" . __METHOD__ .": Field <em>{$this->field_name}</em> does not exist or does not have an instance on {$this->entity_type}:{$this->bundle}.", "error");
      return FALSE;
    }

    $instance = $this->getInstance();
    $form_fields = field_default_form($this->entity_type, $entity, $this->getField(), $instance, $language, $items, $form, $form_state);
    foreach ($form_fields as $form_key => $form_field) {
      $form_field['#parents'] = $form['#parents'];
      array_push($form_field['#parents'], $this->entity_namespace, $this->field_name);
      $form_field['#flexiform'] = $this->flexiform;
      $form_field['#flexiform_element'] = $this;

      if (!empty($instance['remove_none'])) {
        unset($form_field[$language]['#options']['']);
        unset($form_field[$language]['#options']['_none']);
      }

      if (isset($instance['attributes']) && is_array($instance['attributes'])) {
        if (isset($form_field['#attributes'])) {
          $form_field['#attributes'] = array_replace_recursive($form_field['#attributes'], $instance['attributes']);
        }
        else {
          $form_field['#attributes'] = $instance['attributes'];
        }
      }

      $form[$this->element_namespace] = $form_field;
    }

    return $form;
  }

  /**
   * Validate this element.
   */
  public function formValidate($form, &$form_state, $entity, $language = LANGUAGE_NONE) {
    // Prepare arrays.
    $items = isset($entity->{$this->field_name}[$language]) ? $entity->{$this->field_name}[$language] : array();
    $errors = array();
    $field = $this->getField();

    // Extract form values.
    $form_elements = $this->extractFormElements($form);
    field_default_extract_form_values($this->getEntityType(), $entity, $field, $this->getInstance(), $language, $items, $form_elements, $form_state);

    // Set items onto the entity.
    if ($items !== array() || isset($entity->{$this->field_name}[$language])) {
      $entity->{$this->field_name}[$language] = $items;
    }

    // Validate values
    field_default_validate($this->getEntityType(), $entity, $field, $this->getInstance(), $language, $items, $errors);
    $specific_func = $field['module'] . '_field_validate';
    if (function_exists($specific_func)) {
      $specific_func($this->getEntityType(), $entity, $field, $this->getInstance(), $language, $items, $errors);
    }

    // Let other modules validate the entity.
    // Avoid module_invoke_all() to let $errors be taken by reference.
    // @todo Work out a way of not calling this loads and loads of times.
    foreach (module_implements('field_attach_validate') as $module) {
      $function = $module . '_field_attach_validate';
      $function($entity_type, $entity, $errors);
    }

    // Pass field-level validation errors back to widgets for accurate error
    // flagging.
    foreach ($errors as $field_name => $field_errors) {
      foreach ($field_errors as $langcode => $errors) {
        $field_state = field_form_get_state($form['#parents'], $field_name, $language, $form_state);
        $field_state['errors'] = $errors;
        field_form_set_state($form['#parents'], $field_name, $language, $form_state, $field_state);
      }
    }

    field_default_form_errors($this->getEntityType(), $entity, $field, $this->getInstance(), $language, $items, $form, $form_state);
  }

  /**
   * Submit callback for this form.
   */
  function formSubmit($form, &$form_state, $entity, $language = LANGUAGE_NONE) {
    // Prepare arrays.
    $items = isset($entity->{$this->field_name}[$language]) ? $entity->{$this->field_name}[$language] : array();
    $field = $this->getField();

    // Extract form values.
    $form_elements = $this->extractFormElements($form);
    field_default_extract_form_values($this->getEntityType(), $entity, $field, $this->getInstance(), $language, $items, $form_elements, $form_state);
    field_default_submit($this->getEntityType(), $entity, $field, $this->getInstance(), $language, $items, $form_elements, $form_state);

    // Set items onto the entity.
    if ($items !== array() || isset($entity->{$this->field_name}[$language])) {
      $entity->{$this->field_name}[$language] = $items;
    }
  }

  /**
   * Extract the form element from $form and give it the correct key.
   */
  function extractFormElements($form) {
    $form_element = $form[$this->getElementNamespace()];
    return array(
      '#parents' => array_merge($form['#parents'], array($this->getEntityNamespace())),
      $this->field_name => $form_element,
    );
  }

  /**
   * Overrides FlexifromElement::toSettingsArray();
   */
  public function toSettingsArray() {
    $settings = parent::toSettingsArray();
    $settings['type'] = 'field';
    $settings['field_name'] = $this->field_name;
    $settings['instance'] = $this->instance;
    $settings['field'] = $this->field;

    return $settings;
  }
}

class FlexiformElementFlexiform extends FlexiformElement {

  /**
   * The form name of the form.
   */
  protected $form;

  /**
   * Overrides FlexiformElement::__construct().
   */
  public function __construct($flexiform, $namespace, $settings = array()) {
    parent::__construct($flexiform, $namespace, $settings);
    $this->form = $settings['form'];
    $this->label = $settings['label'];
    $this->weight = $settings['weight'];
  }

  /**
   * Overrires FlexiformElement::label().
   */
  public function label() {
    return $this->settings['label'];
  }

  /**
   * Overrires FlexiformElement::label().
   */
  public function name() {
    return $this->form;
  }

  /**
   * Return the type.
   */
  public function type() {
    return 'Flexiform';
  }

  /**
   * Get the flexiform object of the embedded form.
   */
  public function getFlexiform() {
    return flexiform_load($this->form);
  }

  /**
   * Overrides FlexiformElement::form().
   */
  public function form($form, &$form_state, $entity) {
    $form = parent::form($form, $form_state, $entity);

    $sub_form = array(
      '#parents' => $form['#parents'] + array($this->element_namespace),
    );

    $builder = $this->getFlexiform()->getBuilder($entity);
    $form[$this->element_namespace]['#flexiform_builder'] = $builder;
    $form[$this->element_namespace]['#flexiform_element'] = $this;
    $form[$this->element_namespace]['#type'] = 'fieldset';
    $form[$this->element_namespace]['#title'] = $this->label;
    $form[$this->element_namespace] += $builder->form($sub_form, $form_state, $entity); //flexiform($sub_form, $form_state, $this->getFlexiform(), $entity, FALSE);

    return $form;
  }

  /**
   * Overrides FlexiformElement::formValidate().
   */
  public function formValidate($form, &$form_state, $entity, $language = LANGUAGE_NONE) {
    $sub_form = $form[$this->element_namespace];

    $sub_form['#flexiform_builder']->formValidate($sub_form, $form_state);
  }

  /**
   * Overrides FlexiformElement::formSubmit().
   */
  public function formSubmit($form, &$form_state, $entity, $language = LANGUAGE_NONE) {
    $sub_form = $form[$this->element_namespace];

    $sub_form['#flexiform_builder']->formSubmit($sub_form, $form_state);
  }

  /**
   * Overrides FlexifromElement::toSettingsArray();
   */
  public function toSettingsArray() {
    $settings = parent::toSettingsArray();
    $settings['type'] = 'flexiform';
    $settings['form'] = $this->form;
    $settings['label'] = $this->label();

    return $settings;
  }
}
