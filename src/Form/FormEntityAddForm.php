<?php
namespace Drupal\flexiform\Form;

use Drupal\Core\Ajax\AjaxResponse;
use Drupal\Core\Ajax\CloseModalDialogCommand;
use Drupal\Core\Ajax\SetDialogTitleCommand;
use Drupal\Core\Ajax\ReplaceCommand;
use Drupal\Core\Ajax\RedirectCommand;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\flexiform\FlexiformEntityFormDisplayInterface;
use Drupal\flexiform\FormEntity\FlexiformFormEntityManager;

class FormEntityAddForm extends FormBase {

  /**
   * @var \Drupal\flexiform\FlexiformEntityFormDisplay
   */
  protected $formDisplay;

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'flexiform_form_entity_add';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state, FlexiformEntityFormDisplayInterface $form_display = NULL) {
    $form_entity_manager = new FlexiformFormEntityManager($form_display);
    $this->formDisplay = $form_display;
    $available_plugins = \Drupal::service('plugin.manager.flexiform_form_entity')->getDefinitionsForContexts($form_entity_manager->getContexts());

    // Add prefix and suffix for ajax purposes.
    $form['#prefix'] = '<div id="flexiform-form-entity-add-wrapper">';
    $form['#suffix'] = '</div>';

    $form['actions'] = [
      '#type' => 'actions',
      'submit' => [
        '#type' => 'submit',
        '#value' => $this->t('Continue'),
        '#submit' => [
          [$this, 'submitSelectPlugin'],
        ],
        '#ajax' => [
          'callback' => [$this, 'ajaxSubmit'],
          'event' => 'click',
        ],
      ],
    ];

    if ($plugin = $form_state->get('selected_form_entity')) {
      $form['label'] = [
        '#type' => 'textfield',
        '#title' => $this->t('Label'),
        '#description' => $this->t('A label for this entity. This is only used in the admin UI.'),
        '#required' => TRUE,
      ];
      $form['namespace'] = [
        '#type' => 'machine_name',
        '#title' => $this->t('Namespace'),
        '#description' => $this->t('Internal namespace for this entity and its fields.'),
        '#machine_name' => [
          'exists' => [$this, 'namespaceExists'],
          'label' => $this->t('Namespace'),
        ],
      ];

      $form['configuration'] = [
        '#type' => 'container',
        '#tree' => TRUE,
      ];

      $form['actions']['submit']['#value'] = $this->t('Add Form Entity');
      $form['actions']['submit']['#submit'] = [
        [$this, 'submitForm'],
      ];
      return $form;
    }

    // Prepare selector form.
    $plugin_options = [];
    foreach ($available_plugins as $plugin_id => $plugin_definition) {
      if (empty($plugin_definition['no_ui'])) {
        $plugin_options[$plugin_id] = $plugin_definition['label'];
      }
    }
    $form['form_entity'] = [
      '#type' => 'select',
      '#required' => TRUE,
      '#options' => $plugin_options,
      '#title' => $this->t('Form Entity'),
    ];

    return $form;
  }

  /**
   * Submit the plugin selection.
   */
  public function submitSelectPlugin(array $form, FormStateInterface $form_state) {
    $form_state->set('selected_form_entity', $form_state->getValue('form_entity'));
    $form_state->setRebuild();
  }

  /**
   * Ajax the plugin selection.
   */
  public function ajaxSubmit(array $form, FormStateInterface $form_state) {
    $response = new AjaxResponse();
    if ($form_state->get('selected_form_entity')) {
      $response->addCommand(new ReplaceCommand('#flexiform-form-entity-add-wrapper', $form));
      $response->addCommand(new SetDialogTitleCommand(NULL, $this->t('Configure form entity')));
    }
    else {
      $response->addCommand(new CloseModalDialogCommand());
    }
    return $response;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $plugin_name = $form_state->get('selected_form_entity');
    $namespace = $form_state->getValue('namespace');
    $configuration = [
      'label' => $form_state->getValue('label'),
      'plugin' => $plugin_name,
    ];
    if ($plugin_conf = $form_state->getValue('configuration')) {
      $configuration += $plugin_conf;
    }

    $this->formDisplay->addFormEntityConfig($namespace, $configuration);
    $this->formDisplay->save();
    $form_state->set('selected_form_entity', FALSE);
  }

  /**
   * Check whether the namespace already exists.
   */
  public function namespaceExists($namespace, $element, FormStateInterface $form_state) {
    $entities = $this->formDisplay->getFormEntityConfig();
    return !empty($entities[$namespace]);
  }
}