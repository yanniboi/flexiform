<?php
/**
 * @file flexiform_submission.entity.inc
 */


/**
 * The class used for flexiform_submission entities
 */
class FlexiformSubmission extends Entity {

  public function __construct($values = array()) {
    parent::__construct($values, 'flexiform_submission');
  }

  protected function defaultLabel() {
    $flexiform = flexiform_load($this->form);
    return $flexiform->label . ' ' . $this->flexiform_submission_id;
  }

  protected function defaultUri() {
    return array('path' => 'flexiform_submission/' . $this->flexiform_submission_id);
  } 
}

/**
 * The Controller for FlexiformSubmission entities
 */
class FlexiformSubmissionController extends EntityAPIController {
  public function __construct($entityType) {
    parent::__construct($entityType);
  }

  /**
   * Create a flexiform_submission - we first set up the values that are specific
   * to our flexiform_submission schema but then also go through the EntityAPIController
   * function.
   * 
   * @param $type
   *   The machine-readable type of the flexiform_submission.
   *
   * @return
   *   A flexiform_submission object with all default fields initialized.
   */
  public function create(array $values = array()) {
    // Add values that are specific to our FlexiformSubmission
    $values += array( 
      'flexiform_submission_id' => '',
      'is_new' => TRUE,
      'entities' => array(),
      'created' => '',
      'changed' => '',
    );

    $flexiform_submission = parent::create($values);
    return $flexiform_submission;
  }
  
  /**
   * Overriding the buldContent function to add entity specific fields
   *
   * This eventually needs to use the Fields stored in the form UI
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $content = parent::buildContent($entity, $view_mode, $langcode, $content);
    
    return $content;
  }
}
