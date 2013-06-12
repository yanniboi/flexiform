<?php
/**
 * @file
 * Page and page title callbacks for flexiform.
 */

/**
 * Title Callback for Flexiform Submission Pages.
 */
function flexiform_page_title($flexiform, $base_entity_id = FALSE) {
  if (!is_object($flexiform)) {
    $flexiform = flexiform_load($flexiform);
  }

  if (!$flexiform) {
    return FALSE;
  }

  return $flexiform->label;
}

/**
 * Page callback for flexiform submission pages.
 */
function flexiform_page_wrapper($flexiform, $base_entity_id = FALSE) {
  if (!is_object($flexiform)) {
    $flexiform = flexiform_load($flexiform);
  }

  if (!$flexiform) {
    return drupal_not_found();
  }

  module_load_include('inc', 'flexiform', 'includes/flexiform.flexiform');
  $base_entity = flexiform_get_base_entity($flexiform, $base_entity_id);
  return flexiform_wrapper($flexiform, $base_entity);
}
