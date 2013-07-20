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
 * Alter a flexiform as it gets built.
 *
 * @param array $form
 *   The form array that has been built by the flexiform builder.
 * @param array $form_state
 *   The form_state of the form.
 * @param Flexiform $flexiform
 *   The flexiform object.
 *
 * @see FlexiformBuilder::invoke()
 * @see FlexiformBuilderFlexiform::form()
 */
function hook_flexiform_build_alter(&$form, &$form_state, $flexiform) {

}

/**
 * Alter a flexiform as it gets built by a particular builder.
 *
 * @param array $form
 *   The form array that has been built by the flexiform builder.
 * @param array $form_state
 *   The form_state of the form.
 * @param Flexiform $flexiform
 *   The flexiform object.
 *
 * @see FlexiformBuilder::invoke()
 * @see FlexiformBuilderFlexiform::form()
 * @see flexiform_get_builder_info()
 */
function hook_flexiform_build_FLEXIFORM_BUILDER_alter(&$form, &$form_state, $flexiform) {

}

/**
 * Act on the validation of a flexiform.
 *
 * @param array $form
 *   The form array that has been built by the flexiform builder.
 * @param array $form_state
 *   The form_state of the form.
 * @param Flexiform $flexiform
 *   The flexiform object.
 *
 * @see FlexiformBuilder::invoke()
 * @see FlexiformBuilderFlexiform::formValidate()
 */
function hook_flexiform_build_validate_alter(&$form, &$form_state, $flexiform) {

}

/**
 * Act on the validation of a flexiform built by a particular builder.
 *
 * @param array $form
 *   The form array that has been built by the flexiform builder.
 * @param array $form_state
 *   The form_state of the form.
 * @param Flexiform $flexiform
 *   The flexiform object.
 *
 * @see FlexiformBuilder::invoke()
 * @see FlexiformBuilderFlexiform::formValidate()
 */
function hook_flexiform_build_FLEXIFORM_BUILDER_validate_alter(&$form, &$form_state, $flexiform) {

}

/**
 * Act on the submission of a flexiform.
 *
 * @param array $form
 *   The form array that has been built by the flexiform builder.
 * @param array $form_state
 *   The form_state of the form.
 * @param Flexiform $flexiform
 *   The flexiform object.
 *
 * @see FlexiformBuilder::invoke()
 * @see FlexiformBuilderFlexiform::formSubmit()
 */
function hook_flexiform_build_submit_alter(&$form, &$form_state, $flexiform) {

}

/**
 * Act on the submission of a flexiform built by a particular builder.
 *
 * @param array $form
 *   The form array that has been built by the flexiform builder.
 * @param array $form_state
 *   The form_state of the form.
 * @param Flexiform $flexiform
 *   The flexiform object.
 *
 * @see FlexiformBuilder::invoke()
 * @see FlexiformBuilderFlexiform::formSubmit()
 */
function hook_flexiform_build_FLEXIFORM_BUILDER_submit_alter(&$form, &$form_state, $flexiform) {

}
