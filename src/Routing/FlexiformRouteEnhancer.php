<?php

namespace Drupal\flexiform\Routing;

use Drupal\Core\Entity\Entity\EntityFormDisplay;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Routing\Enhancer\RouteEnhancerInterface;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\Routing\Route;

/**
 * Enhances Flexiform routes to get the form display object from request attributes.
 */
class FlexiformRouteEnhancer implements RouteEnhancerInterface {

  /**
   * The entity manager.
   *
   * @var \Drupal\Core\Entity\EntityManagerInterface
   */
  protected $entityManager;

  /**
   * Constructs a FlexiformRouteEnhancer object.
   *
   * @param \Drupal\Core\Entity\EntityManagerInterface $entity_manager
   *   The entity manager.
   */
  public function __construct(EntityManagerInterface $entity_manager) {
    $this->entityManager = $entity_manager;
  }

  /**
   * {@inheritdoc}
   */
  public function enhance(array $defaults, Request $request) {
    $defaults['form_display'] = EntityFormDisplay::load($defaults['entity_type_id'].'.'.$defaults['bundle'].'.'.$defaults['form_mode_name']);

    return $defaults;
  }

  /**
   * {@inheritdoc}
   */
  public function applies(Route $route) {
    return ($route->hasOption('_flexiform_form_entity'));
  }

}
