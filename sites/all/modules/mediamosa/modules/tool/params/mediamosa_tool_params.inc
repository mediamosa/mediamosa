<?php

/**
 * @file
 * MediaMosa tool params function.
 */

/**
 * Returns HTML for a sync listing form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @see bamopen_plugin_list()
 */
function theme_mediamosa_tool_params_list_form($variables) {
  $form = $variables['form'];

  $rows = array();
  foreach ($form['params'] as $tool => &$params) {
    if (drupal_substr($tool, 0, 1) === '#') {
      continue;
    }
    $rows[] = array(
      'data' => array(array(
        'colspan' => 10,
        'data' => '<strong>' . $tool . '</strong>',
      )),
      'class' => array('params-tool', 'params-tool-' . $tool),
    );

    foreach ($params as $name => &$param) {
      if (drupal_substr($name, 0, 1) === '#') {
        continue;
      }

      $row = array();
      $row[] = drupal_render($param[mediamosa_tool_params_db::TOOL]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::WEIGHT]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::NICE_PARAMETER]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::TOOL_PARAMETER]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::MIN_VALUE]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::MAX_VALUE]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::DEFAULT_VALUE]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::REQUIRED]);
      $row[] = drupal_render($param[mediamosa_tool_params_db::TYPE_PARAMETER]);
      $row[] = drupal_render($param['edit']);
      $rows[] = array(
        'data' => $row,
        'class' => array('draggable'),
      );
    }
  }

  $header = array(t('Tool'), t('Weight'), t('Name'), t('Tool parameter'), t('Min value'), t('Max value'), t('Default value'), t('Required'), t('Type'), t('Operations'));
  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'tool-params'), 'empty' => t('No params for tool found.')));
  $output .= drupal_render($form['actions']);
  $output = drupal_render_children($form) . $output;
  drupal_add_tabledrag('tool-params', 'order', 'sibling', 'param-weight');
  return $output;
}

/**
 * Show the listing of the current tool params.
 */
function mediamosa_tool_params_list_form($form, &$form_state) {

  $result = mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'tp')
    ->fields('tp')
    ->orderBy(mediamosa_tool_params_db::TOOL)
    ->orderBy(mediamosa_tool_params_db::WEIGHT)
    ->execute()
    ->fetchAll();

  $form['params'] = array(
    '#tree' => TRUE,
    '#weight' => -20,
  );

  $tools = array();
  foreach ($result as $param) {
    $tools[$param[mediamosa_tool_params_db::TOOL]] = $param[mediamosa_tool_params_db::TOOL];
  }

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  foreach ($result as $param) {
    $tool = $param[mediamosa_tool_params_db::TOOL];
    $name = $param[mediamosa_tool_params_db::NICE_PARAMETER];

    $actions = array();
    if (node_access('update', $param[mediamosa_tool_params_db::NID])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/tool_params/' . $param[mediamosa_tool_params_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $param[mediamosa_tool_params_db::NID])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/tool_params/' . $param[mediamosa_tool_params_db::NID] . '/delete', $l_options);
    }

    $form['params'][$tool][$name][mediamosa_tool_params_db::TOOL] = array(
      '#type' => 'select',
      '#title' => t('Tool'),
      '#options' => $tools,
      '#default_value' => $tool,
      '#attributes' => array('class' => array('param-tool-select')),
      '#access' => FALSE,
    );
    $form['params'][$tool][$name][mediamosa_tool_params_db::WEIGHT] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#delta' => 10000,
      '#default_value' => $param[mediamosa_tool_params_db::WEIGHT],
      '#attributes' => array('class' => array('param-weight')),
    );
    $form['params'][$tool][$name][mediamosa_tool_params_db::NICE_PARAMETER]['#markup'] = check_plain($name);
    $form['params'][$tool][$name][mediamosa_tool_params_db::TOOL_PARAMETER]['#markup'] = check_plain($param[mediamosa_tool_params_db::TOOL_PARAMETER]);
    $form['params'][$tool][$name][mediamosa_tool_params_db::MIN_VALUE]['#markup'] = check_plain($param[mediamosa_tool_params_db::MIN_VALUE]);
    $form['params'][$tool][$name][mediamosa_tool_params_db::MAX_VALUE]['#markup'] = check_plain($param[mediamosa_tool_params_db::MAX_VALUE]);
    $form['params'][$tool][$name][mediamosa_tool_params_db::DEFAULT_VALUE]['#markup'] = check_plain($param[mediamosa_tool_params_db::DEFAULT_VALUE]);
    $form['params'][$tool][$name][mediamosa_tool_params_db::REQUIRED]['#markup'] = check_plain($param[mediamosa_tool_params_db::REQUIRED]);
    $form['params'][$tool][$name][mediamosa_tool_params_db::TYPE_PARAMETER]['#markup'] = check_plain($param[mediamosa_tool_params_db::TYPE_PARAMETER]);

    $form['params'][$tool][$name]['edit']['#markup'] = empty($actions) ? '-' : implode(' | ', $actions);
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#access' => !empty($tools),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );

  return $form;
}

/**
 * hook_submit()
 */
function mediamosa_tool_params_list_form_submit($form, &$form_state) {
  // Store the weights as they come in.
  if (!empty($form_state['values']['params'])) {
    foreach ($form_state['values']['params'] as $tool_name => $tool) {
      foreach ($tool as $nice_parameter => $params) {
        db_update(mediamosa_tool_params_db::TABLE_NAME)
          ->fields(array(
            mediamosa_tool_params_db::WEIGHT => $params['weight'],
          ))
        ->condition(mediamosa_tool_params_db::NICE_PARAMETER, $nice_parameter)
        ->condition(mediamosa_tool_params_db::TOOL, $tool_name)
        ->execute();
      }
    }
  }
}
