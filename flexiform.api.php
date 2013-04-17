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

/**
 * Influence the access for a submission.
 *
 * Access to flexiform submissions may involve checks outside of a simple
 * permission permission check, so we allow modules to influence the result
 * of an access check. Implementations can allow, deny or leave unaltered the
 * access depending on what is returned.
 *
 * @param string $op
 *   The operation being performed.
 * @param NULL|string|FlexiformSubmission $flexiform_submission
 *   The flexiform submission entity we are checking against, the flexiform
 *   machine name or NULL if we are checking global permissions.
 * @param stdClass $account
 *   The user account we are checking for.
 *
 * @param return NULL|boolean
 *   Return a boolean TRUE to allow access, FALSE to deny access or NULL to
 *   leave access unaltered. One or more TRUE responses will allow access
 *   unless a FALSE is returned by another implementation.
 *
 * @see flexiform_submission_access()
 */
function hook_flexiform_submission_access($op, $flexiform_submission, $account) {
  // We don't want to influence global permissions.
  if (!$flexiform_submission) {
    return;
  }

  // Find our the form group this access check relates to.
  $form_name = is_object($flexiform_submission) ? $flexiform_submission->form: $flexiform_submission;
  $flexiform = flexiform_load($form_name);

  // Always allow access to any operation on the open_form form group and
  // prevent any access to the closed_form group for everyone except uid 1.
  if ($flexiform->form_group == 'open_form') {
    return TRUE;
  }
  elseif ($flexiform->form_group == 'closed_form' && $account->uid != 1) {
    return FALSE;
  }

  // All other access checks will be unaffected by this implementation.
}
