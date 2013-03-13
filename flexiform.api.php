<?php
/**
 * @file
 * API documentation for Flexiform.
 */

/**
 * Inform flexiform of a group of forms.
 *
 * Flexiforms are assigned to a group to allow additional logic to be performed
 * by other modules. For example, a module may define a group which it then
 * adds additional fields to that are relevant for that form group.
 *
 * @return
 *   An array whose keys are the value for the group and whose values are an
 *   an array with the following:
 *   - label: The human-readable name of the group.
 *   - locked: Set to TRUE to prevent forms being created in this group through
 *     the UI.
 */
function hook_flexiform_group_info() {
  return array(
    'application' => array(
      'label' => t('Application'),
      // We want site builders to use the UI for this group.
      'locked' => FALSE,
    ),
  );
}
