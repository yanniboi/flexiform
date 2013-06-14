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
   * Build the configure form for the element.
   */
  public function configureForm($form, &$form_state, $flexiform) {
    $form['display_options'] = array(
      '#tree' => TRUE,
      '#type' => 'fieldset',
      '#title' => t('Display Options'),
    );

    $form['display_options']['css_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Additional CSS Classes'),
      '#default_value' => !empty($this->settings['display_options']['css']) ? $this->settings['display_options']['css'] : '',
    );

    $form['display_options']['hide_label'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide the Element Label'),
      '#default_value' => !empty($this->settings['display_options']['hide_label']),
    );

    return $form;
  }

  /**
   * Validate the configure form for the element.
   */
  public function configureFormValidate($form, &$form_state, $flexiform) {}

  /**
   * Submit the configure form for the element.
   */
  public function configureFormSubmit($form, &$form_state, $flexiform) {
    dpm($form_state);
    $this->settings['display_options']['css'] = $form_state['values']['display_options']['css_classes'];
    $this->settings['display_options']['hide_label'] = $form_state['values']['display_options']['hide_label'];

    dpm($this->settings);
    dpm($this->toSettingsArray());
    $flexiform->updateElement($this);
    $flexiform->save();

    $form_state['redirect'] = 'admin/structure/flexiforms/manage/' . $flexiform->form . '/form-fields';
  }

  /**
   * Build the remove form for the element.
   */
  public function removeForm($form, &$form_state, $flexiform) {
    $form['confirm'] = array(
      '#type' => 'hidden',
      '#value' => 1,
    );

    $question = t('Are you sure you want to remove @element?', array('@element' => $this->label()));
    drupal_set_title($question, PASS_THROUGH);

    $form['#attributes']['class'][] = 'confirmation';
    $form['description'] = array('#markup' => t('This action cannot be undone.'));

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Confirm'),
    );
    $form['actions']['cancel'] = array(
      '#type' => 'link',
      '#title' => t('Cancel'),
      '#href' => 'admin/structure/flexiforms/manage/' . $flexiform->form . '/form-fields',
    );

    $form['#theme'] = 'confirm_form';

    return $form;
  }

  /**
   * Validate the remove form for the element.
   */
  public function removeFormValidate($form, &$form_state, $flexiform) {}

  /**
   * Submit the remove form for the element.
   */
  public function removeFormSubmit($form, &$form_state, $flexiform) {
    $flexiform->removeElement($this);
    $flexiform->save();

    $form_state['redirect'] = 'admin/structure/flexiforms/manage/' . $flexiform->form . '/form-fields';
  }

  /**
   * Get the form element.
   */
  public function form($form, &$form_state, $entity) {
    $form[$this->element_namespace]['#weight'] = $this->getWeight();

    if (!empty($form[$this->element_namespace]['#title'])
      && !empty($this->settings['display_options']['hide_title'])) {
      $form[$this->element_namespace]['#title'] = FALSE;
    }

    if (!empty($this->settings['display_options']['css'])) {
      foreach (explode(' ', $this->settings['display_options']['css']) as $class) {
        $form[$this->element_namespace]['#attributes']['class'][] = $class;
      }
    }

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
      'display_options' => $this->settings['display_options'],
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
    $instance = $this->getInstance();
    return check_plain($instance['label']);
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

      // Here we check if a default value function is used.
      if (isset($this->instance['use_default_value_function'])) {
        if (empty($this->instance['use_default_value_function'])) {
          unset($this->full_instance['default_value_function']);
        }
        else {
          unset($this->full_instance['default_value']);
        }
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
   * Overrides FlexiformElement::configureForm().
   */
  public function configureForm($form, &$form_state, $flexiform) {
    form_load_include($form_state, 'inc', 'field_ui', 'field_ui.admin');
    $instance = $this->getInstance();
    $form = field_ui_field_edit_form($form, $form_state, $instance);

    // Allow changes to the default values regardless of anything else thats going on.
    $form['instance']['use_default_value_function'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use a Default Value Function'),
      '#default_value' => !(field_behaviors_widget('default value', $instance) == FIELD_BEHAVIOR_DEFAULT && empty($instance['default_value_function'])),
    );
    $form['instance']['default_value_function'] = array(
      '#type' => 'textfield',
      '#title' => t('Default Value Function'),
      '#description' => t('If you wish to use a custom default value function, enter the name of it here. Reccommended for advanced users only.'),
      '#default_value' => $instance['default_value_function'],
      '#states' => array('invisible' => array(':input[name="instance[use_default_value_function]"]' => array('checked' => FALSE))),
    );
    $form['instance']['default_value_widget'] = field_ui_default_value_widget($this->getField(), $instance, $form, $form_state);
    $form['instance']['default_value_widget']['#states'] = array(
      'visible' => array(':input[name="instance[use_default_value_function]"]' => array('checked' => FALSE)),
    );

    $form = parent::configureForm($form, $form_state, $flexiform);

    return $form;
  }

  /**
   * Overrides FlexiformElement::configureFormValidate().
   */
  public function configureFormValidate($form, &$form_state, $flexiform) {
    if ($form_state['values']['instance']['use_default_value_function']) {
      unset($form['instance']['default_value_widget']);
    }

    field_ui_field_edit_form_validate($form, $form_state);
  }

  /**
   * Overrides FlexiformElement::configureFormSubmit().
   */
  public function configureFormSubmit($form, &$form_state, $flexiform) {
    $instance = $form_state['values']['instance'];
    $field = $form_state['values']['field'];

    // Update any field settings that have changed.
    $element_field = $this->getField();
    $field = array_merge($element_field, $field);
    $original_field = field_info_field($instance['field_name']);
    $this->field = array_diff_assoc($field, $original_field);

    // Work out element instance settings. This gets a little complicated as
    // we want to make sure the default value settings work.
    $element_instance = $this->getInstance();
    $instance = array_merge($element_instance, $instance);
    $original_instance = field_read_instance($instance['entity_type'], $instance['field_name'], $instance['bundle']);

    if (!$instance['use_default_value_function']) {
      $element = $form['instance']['default_value_widget'];

      // Extract field values.
      $items = array();
      field_default_extract_form_values(NULL, NULL, $element_field, $element_instance, LANGUAGE_NONE, $items, $element, $form_state);
      field_default_submit(NULL, NULL, $element_field, $element_instance, LANGUAGE_NONE, $items, $element, $form_state);

      $instance['default_value'] = $items ? $items : NULL;
    }
    else {
      unset($instance['default_value']);
    }

    // Cast Required to a boolean so it doesn't get erroneously stored.
    $instance['required'] = (bool) $instance['required'];

    $this->instance = array_diff_assoc($instance, $original_instance);

    parent::configureFormSubmit($form, $form_state, $flexiform);
  }

  /**
   * Return the form element for this FlexiformElement.
   */
  public function form($form, &$form_state, $entity, $language = LANGUAGE_NONE) {
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

      // Honour the hide title settings.
      if (!empty($this->settings['display_options']['hide_title'])) {
        $form_field[LANGUAGE_NONE]['#title'] = FALSE;
      }

      $form[$this->element_namespace] = $form_field;
    }

    $form = parent::form($form, $form_state, $entity);
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
   * Build the configure form for the element.
   */
  public function configureForm($form, &$form_state, $flexiform) {
    $link = l(t('Edit @flexiform', array('@flexiform' => $this->label())), 'admin/structure/flexiform/manage/' . $this->name());
    $form['description']['#markup'] = t('There are no settings for @flexiform as it is a nested form. You can edit the flexiform by clicking on the link below, however these changes will effect the form everywhere it is used.<br />!link', array('@flexiform' => $this->label(), '!link' => $link));
    return $form;
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
