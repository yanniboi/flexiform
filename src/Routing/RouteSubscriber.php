<?php

namespace Drupal\flexiform\Routing;

use Drupal\field_ui\Routing\RouteSubscriber as FieldUIRouteSubscriber;
use Symfony\Component\Routing\Route;
use Symfony\Component\Routing\RouteCollection;

/**
 * Subscriber for flexiform ui routes.
 */
class RouteSubscriber extends FieldUIRouteSubscriber {

  /**
   * {@inheritdoc}
   */
  protected function alterRoutes(RouteCollection $collection) {
    foreach ($this->manager->getDefinitions() as $entity_type_id => $entity_type) {
      if ($route_name = $entity_type->get('field_ui_base_route')) {
        // Try to get the route from the current collection.
        if (!$entity_route = $collection->get($route_name)) {
          continue;
        }
        $path = $entity_route->getPath();

        $options = $entity_route->getOptions();
        if ($bundle_entity_type = $entity_type->getBundleEntityType()) {
          $options['parameters'][$bundle_entity_type] = array(
            'type' => 'entity:' . $bundle_entity_type,
          );
        }
        // Special parameter used to easily recognize all Field UI routes.
        $options['_field_ui'] = TRUE;
        $options['_flexiform_form_entity'] = TRUE;

        $defaults = [
          'entity_type_id' => $entity_type_id,
        ];
        // If the entity type has no bundles and it doesn't use {bundle} in its
        // admin path, use the entity type.
        if (strpos($path, '{bundle}') === FALSE) {
          $defaults['bundle'] = !$entity_type->hasKey('bundle') ? $entity_type_id : '';
        }

        $route = new Route(
          "$path/form-display/{form_mode_name}/add-form-entity",
          array(
            '_form' => '\Drupal\flexiform\Form\FormEntityAddForm',
            '_title' => 'Add form entity',
          ) + $defaults,
          array('_field_ui_form_mode_access' => 'administer ' . $entity_type_id . ' form display'),
          $options
        );
        $collection->add("entity.entity_form_display.{$entity_type_id}.form_mode.form_entity_add", $route);

        $route = new Route(
          "$path/form-display/{form_mode_name}/edit-form-entity/{entity_namespace}",
          array(
            '_form' => '\Drupal\flexiform\Form\FormEntityEditForm',
            '_title' => 'Configure form entity',
          ) + $defaults,
          array('_field_ui_form_mode_access' => 'administer ' . $entity_type_id . ' form display'),
          $options
        );
        $collection->add("entity.entity_form_display.{$entity_type_id}.form_mode.form_entity_edit", $route);
      }
    }
  }
}