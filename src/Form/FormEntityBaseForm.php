<?php

namespace Drupal\flexiform\Form;

use Drupal\Core\Ajax\AjaxResponse;
use Drupal\Core\Ajax\CloseModalDialogCommand;
use Drupal\Core\Entity\Display\EntityFormDisplayInterface;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\flexiform\FlexiformEntityFormDisplayInterface;
use Drupal\flexiform\FlexiformFormEntityPluginManager;
use Drupal\flexiform\FormEntity\FlexiformFormEntityInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

abstract class FormEntityBaseForm extends FormBase implements FormEntityBaseFormInterface {

  /**
   * @var \Drupal\flexiform\FormEntity\FlexiformFormEntityInterface
   */
  protected $formEntity;

  /**
   * @var \Drupal\flexiform\FlexiformFormEntityPluginManager
   */
  protected $pluginManager;

  /**
   * Constructor.
   */
  public function __construct(FlexiformFormEntityPluginManager $plugin_manager) {
    $this->pluginManager = $plugin_manager;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('plugin.manager.flexiform_form_entity')
    );
  }

  /**
   * Get the form entity manager.
   *
   * @return \Drupal\flexiform\FormEntity\FlexiformFormEntityManager
   */
  protected function getFormEntityManager($form_state) {
    return $this->getFormDisplay($form_state)->getFormEntityManager();
  }

  /**
   * {@inheritdoc}
   */
  public function getFormDisplay(FormStateInterface $form_state) {
    return $form_state->get('form_display');
  }

  /**
   * {@inheritdoc}
   */
  public function setFormDisplay(EntityFormDisplayInterface $form_display, FormStateInterface $form_state) {
    $form_state->set('form_display', $form_display);
    return $this;
  }


  /**
   * Build the plugin configuration form.
   */
  protected function buildConfigurationForm(array $form, FormStateInterface $form_state, FlexiformFormEntityInterface $form_entity = NULL, $namespace = '') {
    $this->formEntity = $form_entity;
    $form['label'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Label'),
      '#description' => $this->t('A label for this entity. This is only used in the admin UI.'),
      '#required' => TRUE,
      '#default_value' => $form_entity->getLabel(),
    ];

    if (empty($namespace)) {
      $form['namespace'] = [
        '#type' => 'machine_name',
        '#title' => $this->t('Namespace'),
        '#description' => $this->t('Internal namespace for this entity and its fields.'),
        '#machine_name' => [
          'exists' => [$this, 'namespaceExists'],
          'label' => $this->t('Namespace'),
        ],
      ];
    }
    else {
      $form['namespace'] = [
        '#type' => 'value',
        '#value' => $namespace,
      ];
    }

    $form['configuration'] = [
      '#type' => 'container',
      '#parents' => ['configuration'],
      '#tree' => TRUE,
    ];
    $form['configuration'] += $form_entity->configurationForm($form['configuration'], $form_state);

    $form['actions'] = [
      '#type' => 'actions',
      'submit' => [
        '#type' => 'submit',
        '#value' => $this->t('Save Configuration'),
        '#validate' => [[$this, 'validateForm']],
        '#submit' => [[$this, 'submitForm']],
      ],
      '#ajax' => [
        'callback' => [$this, 'ajaxSubmit'],
        'event' => 'click',
      ],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, FlexiformEntityFormDisplayInterface $form_display = NULL) {
    $this->setFormDisplay($form_display, $form_state);
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    $this->formEntity->configurationFormValidate($form['configuration'], $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $namespace = $form_state->getValue('namespace');
    $configuration = [
      'label' => $form_state->getValue('label'),
      'plugin' => $this->formEntity->getPluginId(),
    ];
    $this->formEntity->configurationFormSubmit($form['configuration'], $form_state);
    if ($plugin_conf = $form_state->getValue('configuration')) {
      $configuration += $plugin_conf;
    }
    $form_display = $this->getFormDisplay($form_state);
    $form_display->addFormEntityConfig($namespace, $configuration);
    $form_display->save();
  }

  /**
   * Ajax the plugin selection.
   */
  public function ajaxSubmit(array $form, FormStateInterface $form_state) {
    $response = new AjaxResponse();
    $response->addCommand(new CloseModalDialogCommand());
    return $response;
  }

  /**
   * Check whether the namespace already exists.
   */
  public function namespaceExists($namespace, $element, FormStateInterface $form_state) {
    $entities = $this->getFormDisplay($form_state)->getFormEntityConfig();
    return !empty($entities[$namespace]);
  }
}