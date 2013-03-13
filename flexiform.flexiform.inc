<?php
/**
 * @file
 * All the flexiform hooks.
 */

/**
 * Implements hook_flexiform_group_info().
 */
function flexiform_flexiform_group_info() {
  return array(
    'general' => array(
      'label' => t('General'),
    ),
  );
}
