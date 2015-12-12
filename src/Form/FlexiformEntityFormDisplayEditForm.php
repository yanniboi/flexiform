<?php

/**
 * @file
 * Contains \Drupal\flexiform\Form\FlexiformEntityFormDisplayEditForm.
 */

namespace Drupal\flexiform\Form;

use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Plugin\Context\ContextDefinition;
use Drupal\ctools\Form\AjaxFormTrait;
use Drupal\field_ui\Form\EntityFormDisplayEditForm;
use Drupal\flexiform\FormEntity\FlexiformFormEntityManager;

class FlexiformEntityFormDisplayEditForm extends EntityFormDisplayEditForm {

  use AjaxFormTrait;

  /**
   * The form entity manager object.
   *
   * @var \Drupal\flexiform\FormEntity\FlexiformFormEntityManager.
   */
  protected $formEntityManager;

  /**
   * {@inheritdoc}
   */
  public function form(array $form, FormStateInterface $form_state) {
    $form = parent::form($form, $form_state);

    $form['entities_section'] = [
      '#type' => 'container',
      '#tree' => TRUE,
    ];
    $form['entities_section']['add_entity'] = [
      '#type' => 'link',
      '#title' => $this->t('Add Entity'),
      '#url' => 'admin/structure',
      '#attributes' => $this->getAjaxButtonAttributes(),
      '#attached' => [
        'library' => [
          'core/drupal.ajax',
        ],
      ],
    ];
    $form['entities_section']['entities'] = [
      '#type' => 'table',
      '#header' => [
        $this->t('Entity'),
        $this->t('Plugin'),
        $this->t('Operations'),
      ],
      '#title' => t('Entities'),
      '#empty' => t('This form display has no entities yet.'),
    ];

    foreach ($this->getFormEntityManager()->getContextDefinitions() as $namespace => $context_definition) {
      $form['entities_section']['entities'][$namespace] = [
        'human_name' => [
          '#plain_text' => $context_definition->getLabel(),
        ],
        'plugin' => [
          '#plain_text' => $namespace,
        ],
        'operations' => [
          '#plain_text' => 'Operations',
        ],
      ];
    }
    dpm($form);

    return $form;
  }

  /**
   * Get the form entity manager.
   *
   * @return Drupal\flexiform\FormEntity\FlexiformFormEntityManager
   */
  public function getFormEntityManager() {
    if (empty($this->formEntityManager)) {
      $this->initFormEntityManager();
    }

    return $this->formEntityManager;
  }

  /**
   * Initialize the form entity manager.
   */
  protected function initFormEntityManager() {
    $this->formEntityManager = new FlexiformFormEntityManager($this->entity);
  }
}