<?php
/**
 * @file flexiform.flexiform.inc
 * Helper function for embedding the fields into the flexiform in an arbitrary order
 */

/**
 * Get the base entity for a flexible form.
 *
 * If an id is passed, load the entity and return it. Otherwise create a new
 * entity with the correct type and bundle.
 *
 * @param Flexiform $flexiform
 *   A flexiform object.
 * @param int $base_entity_id
 *   (optional) The id of the base entity if it already exists.
 *
 * @return stdClass|Entity
 *   The base entity for the form.
 */
function flexiform_get_base_entity($flexiform, $base_entity_id = FALSE) {
  if ($base_entity_id) {
    $base_entity = entity_load_single($flexiform->base_entity, $base_entity_id);

    //@todo: Check the entity is the right bundle.
    return $base_entity;
  }

  $info = entity_get_info($flexiform->base_entity);
  $values = array();

  if (!empty($info['entity keys']['bundle'])) {
    $values[$info['entity keys']['bundle']] = $flexiform->base_entity_bundle;
  }

  $base_entity = entity_create($flexiform->base_entity, $values);
  return $base_entity;
}

/**
 * Form callback wrapper: fill out a flexiform.
 *
 * @param $flexiform
 *   The flexiform being submitted by this form.
 * @param $base_entity
 *   The base entity for this form.
 *
 * @see flexiform()
 * @see flexiform_simple()
 */
function flexiform_wrapper($flexiform, $base_entity) {
  if ($flexiform->advanced) {
    return drupal_get_form('flexiform', $flexiform, $base_entity);
  }
  else {
    return drupal_get_form('flexiform', $flexiform, $base_entity);
  }
}

/**
 * Build a flexiform.
 *
 * @param array $form
 * @param array $form_state
 * @param Flexiform $flexiform
 * @param Entity|stdClass $base_entity
 * @param bool $include_actions
 *   Boolean flag for whether the actions element should be included at the
 *   bottom of the form.
 *
 * @return
 *   Constructed form array.
 */
function flexiform($form, &$form_state, $flexiform, $base_entity, $include_actions = TRUE) {
  global $user;

  $form['#flexiform'] = $flexiform;
  $form['#flexiform_base_entity'] = $base_entity;
  $form['#flexiform_entities'] = flexiform_prepare_entities($flexiform, $base_entity);

  foreach($form['#flexiform_entities'] as $namespace => $entity) {
    $elements = isset($flexiform->elements[$namespace]) ? $flexiform->elements[$namespace] : FALSE;

    // If there are no elements for this namespace skip it.
    if (!$elements) {
      continue;
    }

    flexiform_embed_entity_elements($namespace, $flexiform, $entity, $elements, $form, $form_state);
  }

  if ($include_actions) {
    $form['actions'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('form-actions')),
      '#weight' => 400,
    );

    // We add the form's #submit array to this button along with the actual submit
    // handler to preserve any submit handlers added by a form callback_wrapper.
    $submit = array();

    if (!empty($form['#submit'])) {
      $submit += $form['#submit'];
    }

    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => $submit + array('flexiform_submit'),
    );

    // We append the validate handler to #validate in case a form callback_wrapper
    // is used to add validate handlers earlier.
    $form['#validate'][] = 'flexiform_validate';
  }

  return $form;
}

/**
 * Validation callback for a flexiform stuff.
 */
function flexiform_validate($form, &$form_state) {
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
 * Submit callback for a flexiform.
 */
function flexiform_submit($form, &$form_state) {
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
    rules_invoke_event_by_args('flexiform_submit_'.$flexiform->form, $form['#flexiform_entities']);
  }
}

/**
 * Add elements for an entity to the form based on settings in the $elements array.
 *
 * @param $entity_namespace
 *  The namespace for this entity in this form
 * @param $flexiform
 *  The flexiform object.
 * @param $entity
 *  The entity object
 * @param $elements array
 *  An array of elementss to be included in the form. Keyed by element_name.
 * @param $form
 * @param $form_state
 */
function flexiform_embed_entity_elements($entity_namespace, $flexiform, $entity, $elements, &$form, &$form_state) {
  // Set up our form bits
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }

  // Iterate over the fields
  foreach ($elements as $element_name => $settings) {
    $element_namespace = FlexiformElement::makeElementNamespace($settings['type'], $entity_namespace, $element_name);
    $element = FlexiformElement::getElement($flexiform, $settings['type'], $element_namespace, $settings);

    $form += $element->form($form, $form_state, $entity);
  }
}

/**
 * Prepare the entities needed for the form, either by creating or loading
 *
 * @param $flexiform
 *  The form definition
 * @param $base_entity
 *  The base entity of the form.
 *
 * @return
 *  An array of entities keyed by namespace
 */
function flexiform_prepare_entities($flexiform, $base_entity) {
  $entities = array();

  foreach($flexiform->entities as $namespace => $info) {
    if (isset($info['getter'])) {
      $getter = flexiform_entity_getter_info($info['getter']);

      if (isset($getter['file']) && file_exists(drupal_get_path('module', $getter['module']) . '/' . $getter['file'])) {
        include_once drupal_get_path('module', $getter['module']) . '/' . $getter['file'];
      }

      $callback = $getter['getter callback'];
      if (function_exists($callback)) {
        // Todo: Sort out params
        $entities[$namespace] = $callback($flexiform, $base_entity, $entities, $namespace, $getter);
      }
    }
  }

  return $entities;
}

/**
 * Get the namespace we use for the entity
 *
 * @param $entity_type
 *  The entity type e.g. node,user or taxonomy term
 * @param $entity
 *  The entity object
 *
 * @return
 *  A Unique namespace for the entity in this form
 */
function flexiform_get_entity_namespace($entity_type, $entity) {
  $entity_info = entity_get_info($entity_type);
  return $entity_type . '__' . $entity->{$entity_info['entity keys']['id']};
}

/**
 * Get the fields on a form from a particular entity namespace
 *
 * @param $entity_namespace
 *  The namespace as give by flexiform_get_entity_namespace
 * @param $form
 *  Fully Built form object
 *
 * @return
 *  A form array with only the fields from that entity included without the namespace prefixed
 */
function flexiform_get_entity_namespace_fields($entity_namespace, $form) {
  $entity_form = array();
  $namespace_length = strlen($entity_namespace);

  foreach ($form as $key => $field) {
    if (substr($key, 0, 1) == '#') {
      $entity_form[$key] = $field;
    }
    elseif (substr($key, 0, $namespace_length) == $entity_namespace) {
      $entity_form[substr($key, $namespace_length + 2)] = $field;
    }
  }

  $entity_form['#parents'][] = $entity_namespace;

  return $entity_form;
}

/**
 * Get the bundle key for an entity type
 */
function _flexiform_get_bundle_key($entity_type) {
  $info = entity_get_info($entity_type);
  return empty($info['entity keys']['bundle']) ? FALSE : $info['entity keys']['bundle'];
}

/**
 * Get the bundle for an entity type and entity
 */
function _flexiform_get_bundle($entity_type, $entity) {
  $bundle_key = _flexiform_get_bundle_key($entity_type);
  if (!$bundle_key) {
    return $entity_type;
  }
  else {
    return $entity->{$bundle_key};
  }
}

/**
 * Getter callback for 'base entity'
 *
 * @param Flexiform $flexiform
 *   The flexiform object.
 * @param Entity|stdClass $base_entity
 *   The base entity for the form.
 * @param array $entities
 *   An array of entities already embedded in the form.
 * @param string $namespace
 *   The namespace of the entity being got for this form.
 * @param array $config
 *   The settings for this getter, as defined in flexiform_getter_info.
 */
function flexiform_entity_getter_base_entity($flexiform, $base_entity, $entities, $namespace, $config) {
  return $base_entity;
}

/**
 * Getter callback for 'base entity'
 */
function flexiform_entity_setter_base_entity($flexiform, $base_entity, $entity, $entity_type, $entities, $namespace, $config) {
  entity_save($entity_type, $entity);
}