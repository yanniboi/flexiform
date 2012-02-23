<?php
/**
 * @file flexiform.flexiform.inc
 * Helper function for embedding the fields into the flexiform in an arbitrary order
 */
 
/**
 * Add fields from an entity to the form based on settings in the $fields array.
 *
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
function flexiform_embed_entity_fields($entity_type, $entity, $fields, &$form, &$form_state) {
  // Sniff out bundle
  $info = entity_get_info($entity_type);
  $bundle_key = empty($info['entity keys']['bundle']) ? FALSE : $info['entity keys']['bundle'];
  if (!$bundle_key) {
    $bundle = $entity_type;
  }
  else {
    $bundle = $entity->{$bundle_key};
  }
  
  // Set up our form bits
  $entity_namespace = flexiform_get_entity_namespace($entity_type, $entity);
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