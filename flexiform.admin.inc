<?php

/**
 * @file
 * Model type editing UI.
 */

/**
 * UI controller.
 */
class FlexiformUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
		$items[$this->path]['description'] = 'Manage flexiforms, including adding
		and removing fields and the display of fields.';
    return $items;
  }
}

/**
 * Generates the model type editing form.
 */
function flexiform_form($form, &$form_state, $flexiform, $op = 'edit') {

  if ($op == 'clone') {
    $flexiform->label .= ' (cloned)';
    $flexiform->form = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $flexiform->label,
    '#description' => t('The human-readable name of this flexiform.'),
    '#required' => TRUE,
    '#size' => 30,
  );
  // Machine-readable type name.
  $form['form'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($flexiform->form) ? $flexiform->form : '',
    '#maxlength' => 32,
//    '#disabled' => $flexiform->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'flexiform_get_flexiforms',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this flexiform. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Flexiform'),
    '#weight' => 40,
  );

  //Locking not supported yet
  /*if (!$flexiform->isLocked() && $op != 'add') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete model type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('flexiform_form_submit_delete')
    );
  }*/
  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function flexiform_form_submit(&$form, &$form_state) {
  $flexiform = entity_ui_form_submit_build_entity($form, $form_state);
  
  $flexiform->entities = array(
    'this_submission' => array(
      'entity_type' => 'flexiform_submission',
      'bundle' => $flexiform->form,
      'create' => TRUE,
      'getter_callback' => 'flexiform_entity_getter_this_submission',
    ),
  );
  
  $flexiform->save();
  $form_state['redirect'] = 'admin/structure/flexiforms';
}

/**
 * Form API submit callback for the delete button.
 */
function flexiform_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/flexiforms/manage/' . $form_state['flexiform']->type . '/delete';
}
