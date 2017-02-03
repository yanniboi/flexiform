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
use Drupal\flexiform\FlexiformFormEntityPluginManager;
use Drupal\flexiform\FormEntity\FlexiformFormEntityManager;
use Symfony\Component\DependencyInjection\ContainerInterface;

class FormEntityAddForm extends FormEntityBaseForm {

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
    parent::buildForm($form, $form_state, $form_display);

    $available_plugins = $this->pluginManager->getDefinitionsForContexts($this->formEntityManager()->getContexts());

    // Add prefix and suffix for ajax purposes.
    $form['#prefix'] = '<div id="flexiform-form-entity-add-wrapper">';
    $form['#suffix'] = '</div>';

    if ($plugin_id = $form_state->get('selected_form_entity')) {
      $configuration = ['manager' => $form_entity_manager];
      $plugin = $this->pluginManager->createInstance($plugin_id, $configuration);
      $i = 0;
      while ($this->namespaceExists($plugin_id . '_' . $i, $form, $form_state)) {
        $i++;
      }
      $namespace = $plugin_id . '_' . $i;
      return $this->buildConfigurationForm($form, $form_state, $plugin, $namespace);
    }
    else {
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
    }

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
   * {@inheritdoc}
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
    parent::submitForm($form, $form_state);
    $form_state->set('selected_form_entity', FALSE);
  }
}