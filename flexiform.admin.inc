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
  
  // Temp till a UI gets build
  $flexiform->fields = array(
    'this_submission' => array(
      'field_test_field' => array(
        'instance' => array(
          'label' => 'Proof',
          'widget' => array(
            'weight' => 400,
          ),
        ),
      ),
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

/**
 * flexiform_manage_form_fields_form
 *
 * This form configures the fields on an flexiform. You can choose them
 * rearrange them and override verious instance settings through this form. 
 */
function flexiform_manage_form_fields_form($form, &$form_state, $flexiform) {
  // Get list of entities 
  $entities = $flexiform->entities;
   
  // Get the fields in the flexiform deff
  $formfields = array();
  foreach ($flexiform->fields as $entity_namespace => $entity_fields) {
    foreach ($entity_fields as $field_name => $settings) {
      $settings['field_name'] = $field_name;
      $settings['entity_namespace'] = $entity_namespace;
      $settings['entity_type'] = $entities[$entity_namespace]['entity_type'];
      $settings['bundle'] = $entities[$entity_namespace]['bundle'];
      $formfields[$entity_namespace . "__" . $field_name] = $settings;
    }
  };
  
  $form += array(
    '#flexiform' => $flexiform,
    '#formfields' => array_keys($formfields),
  );
  
  $form_state += array(
    '#flexiform' => $flexiform,
    '#formfields' => $formfields,
  );
  
  $table = array(
    '#type' => 'flexiform_field_table',
    '#tree' => TRUE,
    '#header' => array(
      t('Label'),
      t('Weight'),
      t('Parent'),
      t('Entity'),
      t('Field'),
      t('Widget'),
      array('data' => t('Operations'), 'colspan' => 2),
    ),
    '#parent_options' => array(),
    '#regions' => array(
      'main' => array('message' => t('No fields are present yet')),
      'add_new' => array('title' => '&nbsp;'),
    ),
    '#attributes' => array(
      'class' => array('field-ui-overview'),
      'id' => 'field-overview',
    ),
  );
  
  $widget_types = field_info_widget_types();

  foreach($formfields as $field_namespace => $formfield) {
    $field = field_info_field($formfield['field_name']);
    $instance = field_info_instance($formfield['entity_type'], $formfield['field_name'], $formfield['bundle']);
    
    // Build the fake instance for this form (incase the widget has changed - 
    // we would normally have to be careful of default_value but that doesn't
    // figure here, so we should be fine.
    $forminstance = array_replace_recursive($instance, $formfield['instance']);
    
    $field_path = '';
    
    $table[$field_namespace] = array(
      '#attributes' => array('class' => array('draggable', 'tabledrag-leaf')),
      '#row_type' => 'field',
      '#region_callback' => 'flexiform_field_form_row_region',
      'label' => array(
        '#markup' => check_plain($forminstance['label']),
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#title' => t('Weight for @title', array('@title' => $forminstance['label'])),
        '#title_display' => 'invisible',
        '#default_value' => $forminstance['widget']['weight'],
        '#size' => 3,
        '#attributes' => array('class' => array('field-weight')),
      ),
      'parent_wrapper' => array(
        'parent' => array(
          '#type' => 'select',
          '#title' => t('Parent for @title', array('@title' => $forminstance['label'])),
          '#title_display' => 'invisible',
          '#options' => $table['#parent_options'],
          '#empty_value' => '',
          '#attributes' => array('class' => array('field-parent')),
          '#parents' => array('fields', $field_namespace, 'parent'),
        ),
        'hidden_name' => array(
          '#type' => 'hidden',
          '#default_value' => $field_namespace,
          '#attributes' => array('class' => array('field-name')),
        ),
      ),
      'entity' => array(
        '#markup' => $formfield['entity_namespace'],
      ),
      'field' => array(
        '#markup' => $formfield['field_name'],
      ),
      'widget_type' => array(
        '#type' => 'link',
        '#title' => t($widget_types[$forminstance['widget']['type']]['label']),
        '#href' => $field_path . '/widget-type',
        '#options' => array('attributes' => array('title' => t('Change widget type.'))),
      ),
      'configure' => array(
        '#type' => 'link',
        '#title' => t('configure'),
        '#href' => $field_path,
        '#options' => array('attributes' => array('title' => t('Configure field instance settings for this form'))),
      ),
      'remove' => array(
        '#type' => 'link',
        '#title' => t('remove'),
        '#href' => $field_path,
        '#options' => array('attributes' => array('title' => t('Remove this field.'))),
      ),
    ); 
  }
  
  // Additional row: add new field.
  $max_weight = flexiform_field_max_weight($flexiform);
  $entity_options = array();
  foreach($entities as $entity_namespace => $info) {
    $entity_options[$entity_namespace] = $entity_namespace;
  }
  
  if (isset($form_state['values']['fields']['_add_field']['entity'])) {
    $entity_namespace = $form_state['values']['fields']['_add_field']['entity'];
  }
  else { 
    $entity_namespace = NULL;
  }
  $formfield_options = flexiform_field_field_options($flexiform, $entity_namespace);
  
  if ($entity_options) {
    $name = '_add_field';
    $table[$name] = array(
      '#attributes' => array('class' => array('draggable', 'tabledrag-leaf', 'add-new')),
      '#row_type' => 'add_field',
      '#region_callback' => 'flexiform_field_form_row_region',
      'label' => array(
        '#type' => 'textfield',
        '#title' => 'Lable',
        '#title_display' => 'invisible',
        '#size' => 15,
        '#description' => t('Label'),
        '#prefix' => '<div class="label-input"><div class="add-new-placeholder">' . t('Add new field') .'</div>',
        '#suffix' => '</div>',
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $max_weight + 1,
        '#size' => 3,
        '#title_display' => 'invisible',
        '#title' => t('Weight for new field'),
        '#attributes' => array('class' => array('field-weight')),
        '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
      ),
      'parent_wrapper' => array(
        'parent' => array(
          '#type' => 'select',
          '#title' => t('Parent for new field'),
          '#title_display' => 'invisible',
          '#options' => $table['#parent_options'],
          '#empty_value' => '',
          '#attributes' => array('class' => array('field-parent')),
          '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
          '#parents' => array('fields', $name, 'parent'),
        ),
        'hidden_name' => array(
          '#type' => 'hidden',
          '#default_value' => $name,
          '#attributes' => array('class' => array('field-name')),
        ),
      ),
      'entity' => array(
        '#type' => 'select',
        '#title' => t('The Enitity'),
        '#title_display' => 'invisible',
        '#options' => $entity_options,
        '#empty_option' => t('- Select an Entity -'),
        '#description' => t('Entity from which to take the field'),
        '#attributes' => array('class' => array('entity-select')),
        '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
        '#ajax' => array(
          'event' => 'change',
          'method' => 'replace',
          'wrapper' => 'flexiform-field-selector',
          'callback' => 'flexiform_manage_form_fields_fields',
        ),
      ),
      'field' => array(
        '#type' => 'select',
        '#title' => t('The Field'),
        '#title_display' => 'invisible',
        '#options' => $formfield_options,
        '#empty_option' => t('- Select an Field -'),
        '#description' => t('Pick a Field'),
        '#attributes' => array('class' => array('entity-select')),
        '#prefix' => '<div id="flexiform-field-selector"><div class="add-new-placeholder">&nbsp;</div>',
        '#suffix' => '</div>',
      ),
    );
  }
  
  $form['fields'] = $table;
  
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  
  $form['#attached']['css'][] = drupal_get_path('module', 'flexiform') . '/flexiform_ui.css';
  $form['#attached']['js'][] = drupal_get_path('module', 'flexiform') . '/flexiform_ui.js';
  
  // Add tabledrag behavior.
  $form['#attached']['drupal_add_tabledrag'][] = array('field-overview', 'order', 'sibling', 'field-weight');
  $form['#attached']['drupal_add_tabledrag'][] = array('field-overview', 'match', 'parent', 'field-parent', 'field-parent', 'field-name');
  
  return $form;
}

/**
 * Validate handler for the flexiform_manage_form_fields_form
 */
function flexiform_manage_form_fields_form_validate($form, &$form_state) {
  $field = $form_state['values']['fields']['_add_field'];
  
  if (array_filter(array($field['label'], $field['entity'], $field['field']))) {
    // Missing Label
    if (!$field['label']) {
      form_set_error('fields][_add_field][label', t('Add field: you must provide a label'));
    }
    
    if (!$field['entity']) {
      form_set_error('fields][_add_field][entity', t('Add field: you must choose an entity'));
    }
    
    if (!$field['field']) {
      form_set_error('fields][_add_field][label', t('Add field: you must choose a field'));
    }
  }
}

/**
 * Submit handler for the flexiform_manage_form_fields_form
 */
function flexiform_manage_form_fields_form_submit($form, &$form_state) {
  $form_values = $form_state['values']['fields'];
  $flexiform = $form_state['#flexiform'];
  $formfields = $form_state['#formfields'];
  
  $formfield_path = '';
  
  foreach($form_values as $formfield_namespace => $values) {
    if (in_array($formfield_namespace, $form['#formfields'])) {
      $formfield = $formfields[$formfield_namespace];
      $flexiform->fields[$formfield['entity_namespace']][$formfield['field_name']]['instance']['widget']['weight'] = $values['weight'];
    }    
  }
  
  // Add a field
  $formfield = array();
  if (!empty($form_values['_add_field']['field'])) {
    $values = $form_values['_add_field'];
    
    $formfield['field_name'] = $values['field'];
    $formfield['entity_namespace'] = $values['entity'];
    $formfield['instance']['label'] = $values['label'];
    $formfield['instance']['widget']['weight'] = $values['weight'];
    
    $flexiform->fields[$values['entity']][$values['field']] = $formfield;
    
    // Send the User to the Instance Settings page
  }
  
  $flexiform->save();
  
  drupal_set_message(t('Your settings have been saved'));
}
 
/**
 * Ajax callback to update the fields available
 */
function flexiform_manage_form_fields_fields($form, $form_state) {
  return $form['fields']['_add_field']['field'];
}

/**
 * flexiform_manage_form_entities_form
 *
 * This form configures the entities involved in a flexiform. 
 */
function flexiform_manage_form_entities_form($form, &$form_state, $flexiform) {
  $form['temp'] = array(
    '#markup' => t('Temporary'),
  );
  
  return $form;
}

/**
 * Get the max weight of fields in the flexiform
 */
function flexiform_field_max_weight($flexiform) {
  $max_weight = 0;
  foreach($flexiform->fields as $entity_namespace => $formfields) {
    foreach($formfields as $field_namespace => $field) {
      $weight = $field['instance']['widget']['weight'];
      if ($weight > $max_weight) {
        $max_weight = $weight;
      }
    }
  }
  
  return $max_weight;
}

/**
 * Get all available fields for a given entity
 */
function flexiform_field_field_options($flexiform, $entity_namespace = NULL) {
  if (!isset($entity_namespace) || !isset($flexiform->entities[$entity_namespace])) {
    return array();
  }
  
  $entity_info = $flexiform->entities[$entity_namespace];
  $instances = field_info_instances($entity_info['entity_type'], $entity_info['bundle']);
  $options = array();
  foreach ($instances as $instance) {
    $options[$instance['field_name']] = $instance['label'];
  }
  
  return $options;
}

/**
 * Pre-render callback for flexiform_field_table elements.
 */
function flexiform_field_table_pre_render($elements) {
  $js_settings = array();

  // For each region, build the tree structure from the weight and parenting
  // data contained in the flat form structure, to determine row order and
  // indentation.
  $regions = $elements['#regions'];
  $tree = array('' => array('name' => '', 'children' => array()));
  $trees = array_fill_keys(array_keys($regions), $tree);

  $parents = array();
  $list = drupal_map_assoc(element_children($elements));

  // Iterate on rows until we can build a known tree path for all of them.
  while ($list) {
    foreach ($list as $name) {
      $row = &$elements[$name];
      $parent = $row['parent_wrapper']['parent']['#value'];
      // Proceed if parent is known.
      if (empty($parent) || isset($parents[$parent])) {
        // Grab parent, and remove the row from the next iteration.
        $parents[$name] = $parent ? array_merge($parents[$parent], array($parent)) : array();
        unset($list[$name]);

        // Determine the region for the row.
        $function = $row['#region_callback'];
        $region_name = $function($row);

        // Add the element in the tree.
        $target = &$trees[$region_name][''];
        foreach ($parents[$name] as $key) {
          $target = &$target['children'][$key];
        }
        $target['children'][$name] = array('name' => $name, 'weight' => $row['weight']['#value']);

        // Add tabledrag indentation to the first row cell.
        if ($depth = count($parents[$name])) {
          $cell = current(element_children($row));
          $row[$cell]['#prefix'] = theme('indentation', array('size' => $depth)) . (isset($row[$cell]['#prefix']) ? $row[$cell]['#prefix'] : '');
        }

        // Add row id and associate JS settings.
        $id = drupal_html_class($name);
        $row['#attributes']['id'] = $id;
        if (isset($row['#js_settings'])) {
          $row['#js_settings'] += array(
            'rowHandler' => $row['#row_type'],
            'name' => $name,
            'region' => $region_name,
          );
          $js_settings[$id] = $row['#js_settings'];
        }
      }
    }
  }
  // Determine rendering order from the tree structure.
  foreach ($regions as $region_name => $region) {
    $elements['#regions'][$region_name]['rows_order'] = array_reduce($trees[$region_name], '_flexiform_field_form_reduce_order');
  }

  $elements['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array('fieldUIRowsData' => $js_settings),
  );
  return $elements;
}

/**
 * Returns HTML for Flexiform Field tables.
 *
 * @param $variables
 *   An associative array containing:
 *   - elements: An associative array containing a Form API structure to be
 *     rendered as a table.
 *
 * @ingroup themeable
 */
function theme_flexiform_field_table($variables) {
  $elements = $variables['elements'];
  $table = array();
  $js_settings = array();

  // Add table headers and attributes.
  foreach (array('header', 'attributes') as $key) {
    if (isset($elements["#$key"])) {
      $table[$key] = $elements["#$key"];
    }
  }

  // Determine the colspan to use for region rows, by checking the number of
  // columns in the headers.
  $colums_count = 0;
  foreach ($table['header'] as $header) {
    $colums_count += (is_array($header) && isset($header['colspan']) ? $header['colspan'] : 1);
  }

  // Render rows, region by region.
  foreach ($elements['#regions'] as $region_name => $region) {
    $region_name_class = drupal_html_class($region_name);
    // Add region rows.
    if (isset($region['title'])) {
      $table['rows'][] = array(
        'class' => array('region-title', 'region-' . $region_name_class . '-title'),
        'no_striping' => TRUE,
        'data' => array(
          array('data' => $region['title'], 'colspan' => $colums_count),
        ),
      );
    }
    if (isset($region['message'])) {
      $class = (empty($region['rows_order']) ? 'region-empty' : 'region-populated');
      $table['rows'][] = array(
        'class' => array('region-message', 'region-' . $region_name_class . '-message', $class),
        'no_striping' => TRUE,
        'data' => array(
          array('data' => $region['message'], 'colspan' => $colums_count),
        ),
      );
    }

    // Add form rows, in the order determined at pre-render time.
    foreach ($region['rows_order'] as $name) {
      $element = $elements[$name];

      $row = array('data' => array());
      if (isset($element['#attributes'])) {
        $row += $element['#attributes'];
      }

      // Render children as table cells.
      foreach (element_children($element) as $cell_key) {
        $child = &$element[$cell_key];
        // Do not render a cell for children of #type 'value'.
        if (!(isset($child['#type']) && $child['#type'] == 'value')) {
          $cell = array('data' => drupal_render($child));
          if (isset($child['#cell_attributes'])) {
            $cell += $child['#cell_attributes'];
          }
          $row['data'][] = $cell;
        }
      }
      $table['rows'][] = $row;
    }
  }
  return theme('table', $table);
}

/**
 * Returns the region to which a row in the 'Manage fields' screen belongs.
 *
 * This function is used as a #row_callback in field_ui_field_overview_form(),
 * and is called during field_ui_table_pre_render().
 */
function flexiform_field_form_row_region($row) {
  switch ($row['#row_type']) {
    case 'field':
    case 'extra_field':
      return 'main';
    case 'add_field':
      // If no input in 'label', assume the row has not been dragged out of the
      // 'add new' section.
      return (!empty($row['label']['#value']) ? 'main' : 'add_new');
  }
}

/**
 * Helper function: determines the rendering order of a tree array.
 *
 * This is intended as a callback for array_reduce().
 */
function _flexiform_field_form_reduce_order($array, $a) {
  $array = !isset($array) ? array() : $array;
  if ($a['name']) {
    $array[] = $a['name'];
  }
  if (!empty($a['children'])) {
    uasort($a['children'], 'drupal_sort_weight');
    $array = array_merge($array, array_reduce($a['children'], '_flexiform_field_form_reduce_order'));
  }
  return $array;
}