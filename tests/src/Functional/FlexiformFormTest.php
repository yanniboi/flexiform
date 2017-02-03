<?php

namespace Drupal\Tests\flexiform\Functional;

use Drupal\Tests\BrowserTestBase;

/**
 * Tests CRM fields and views.
 *
 * @group flexiform
 */
class FlexiformFormTest extends BrowserTestBase {

  /**
   * {@inheritdoc}
   */
  public static $modules = ['user', 'flexiform', 'flexiform_test'];

  /**
   * Testing admin user.
   *
   * @var \Drupal\user\Entity\User
   */
  protected $adminUser;

  /**
   * {@inheritdoc}
   */
  protected function setUp() {
    parent::setUp();

    // @todo Remove this when we fix https://www.drupal.org/node/2849440.
    $this->updateConfig();

    $this->adminUser = $this->createUser([], NULL, TRUE);
    $this->adminUser->field_name = 'test_name';
    $this->adminUser->save();
  }

  protected function updateConfig() {
    $name = 'core.entity_form_display.user.user.compare';

    $storage = \Drupal::service('config.storage');
    $config = $storage->read($name);

    $form_entities = [
      'current_user_0' => [
        'label' => 'Current User',
        'plugin' => 'current_user',
        'context_mapping' => [
          'base' => '',
        ],
        'create' => 1,
      ],
    ];

    $config['third_party_settings']['flexiform']['form_entities'] = $form_entities;
    $storage->write($name, $config);
  }

  public function testFormValues() {
    // Check the site has installed successfully.
    $this->drupalGet('');
    $this->assertSession()->statusCodeEquals(200);

    $this->drupalGet('test/user-compare');
    $session = $this->assertSession();
    $session->statusCodeEquals(200);
    $session->elementTextContains('css', '.page-title', 'Compare users');
    $session->fieldExists('field_name[0][value]');
    $session->fieldValueEquals('field_name[0][value]', '');
    $session->fieldExists('current_user_0[field_name][0][value]');
    $session->fieldValueEquals('current_user_0[field_name][0][value]', '');

    $this->drupalLogin($this->adminUser);

    $this->drupalGet('test/user-compare');
    $session = $this->assertSession();
    $session->statusCodeEquals(200);
    $session->fieldValueEquals('field_name[0][value]', '');
    $session->fieldValueEquals('current_user_0[field_name][0][value]', 'test_name');
  }

}
