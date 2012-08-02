<?php
/**
 * @file flexiform.flexiform.inc
 * Helper function for embedding the fields into the flexiform in an arbitrary order
 */
 
/**
 * Prepare the entities needed for the form, either by creating or loading
 *
 * @param $flexiform
 *  The form definition
 * @param $flexiform_submission
 *  The Current Submission
 *
 * @return
 *  An array of entities keyed by namespace
 */
function flexiform_prepare_entities($flexiform, $submission) {
  $current_entities = $submission->entities;
  print_r($submission);
  $entities = array();
  dpm($flexiform);
  foreach($flexiform->entities as $namespace => $info) {
    // If the id is already set on the submission entities...
    if (isset($current_entities[$namespace])) {
      // ...load that entity and continue
      //$entities[$namespace] = entity_load($info['entity_type'], $current_entities[$namespace]);
      continue;
    }
        
    if (isset($info['getter'])) {
      $getter = flexiform_entity_getter_info($info['getter']);
      
      if (isset($getter['file']) && file_exists(drupal_get_path('module', $getter['module']) . '/' . $getter['file'])) {
        include_once drupal_get_path('module', $getter['module']) . '/' . $getter['file'];
      }      
      
      $callback = $getter['callback'];
      if (function_exists($callback)) {      
        // Todo: Sort out params
        $entities[$namespace] = $callback($flexiform, $submission, $entities, $flexiform->entities[$namespace]);
      }
    }
  }
  
  return $entities;
}
 
/**
 * Add fields from an entity to the form based on settings in the $fields array.
 *
 * @param $entity_namespace
 *  The namespace for this entity in this form
 * @param $entity_type
 *  The entity type e.g. node,user or taxonomy term
 * @param $entity
 *  The entity object
 * @param $field array
 *  An array of fields to be included in the form. Keyed by field_name. Each field has two sub-arrays
 *   - 'field' - settings to override the field config
 *   - 'instance' - settings that will overide the instance config
 * @param $form
 * @param $form_state
 */ 
function flexiform_embed_entity_fields($entity_namespace, $entity_type, $entity, $fields, &$form, &$form_state) {
  // Sniff out bundle
  $bundle = _flexiform_get_bundle($entity_type, $entity);
  
  // Set up our form bits
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  
  // Iterate over the fields
  foreach ($fields as $field_name => $options) {
    if (is_string($options)) {
      $field_name = $options;
    }
    elseif (isset($options['#markup'])) {
      $form[$field_name] = $options;
      continue;
    }
    
    // Get information
    $field = field_info_field($field_name);
    $instance = field_info_instance($entity_type, $field_name, $bundle);
    $items = field_get_items($entity_type, $entity, $field_name, LANGUAGE_NONE);
    
    if (!$field || !$instance) {
      drupal_set_message("Error in " . __FUNCTION__ .": Field <em>$field_name</em> does not exist or does not have an instance on " . $entity_type . ":" . $bundle . ".", "error");
      continue;
    }
    
    // Merge in our own settings
    if (is_array($options)) {
      if (!empty($options['field'])) {
        $field = array_replace_recursive($field, $options['field']);
      }
      
      if (!empty($options['instance'])) {
        $instance = array_replace_recursive($instance, $options['instance']);
        
        // Default value might be a numerically indexed array, so we'll overwrite
        if (isset($options['default_value'])) {
          $instance['default_value'] = $options['instance']['default_value'];
        }
      }
    }
    
    // Attach the form element
    $form_fields = field_default_form($entity_type, $entity, $field, $instance, LANGUAGE_NONE, $items, $form, $form_state);
    foreach ($form_fields as $form_key => $form_field) {
      $form_field['#parents'] = array($entity_namespace, $field_name);
    
      if (!empty($options['remove_none'])) {
        unset($form_field[LANGUAGE_NONE]['#options']['']);
        unset($form_field[LANGUAGE_NONE]['#options']['_none']);
      }
      
      if (isset($options['attributes']) && is_array($options['attributes'])) {
        if (isset($form_field['#attributes'])) {
          $form_field['#attributes'] = array_replace_recursive($form_field['#attributes'], $options['attributes']);
        }
        else {
          $form_field['#attributes'] = $options['attributes'];
        }
      }
      
      $form[$entity_namespace . '__' . $form_key] = $form_field;
    }    
  }
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
 * Getter callback for 'this submission'
 */
function flexiform_entity_getter_this_submission($flexiform, $submission, $namespace, $entities) {
  return $submission;
} 