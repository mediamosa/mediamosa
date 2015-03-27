<?php
/**
 * @file
 * hook_load / -update / -delete / -insert hooks for node type mediamosa_tool_params.
 */

/**
 * Internal prefix.
 */
define('MEDIAMOSA_TOOL_PARAMS_INTERNAL_PREFIX', 'internal_');

/**
 * Implements hook_load().
 */
function mediamosa_tool_params_load(&$nodes) {
  $nodes = mediamosa_node_revision::load($nodes);

  foreach ($nodes as $key => $node) {
    $nodes[$key]->{mediamosa_tool_params_db::ALLOWED_VALUE} = empty($node->{mediamosa_tool_params_db::ALLOWED_VALUE}) ? NULL : $node->{mediamosa_tool_params_db::ALLOWED_VALUE};
  }

  return $nodes;
}

/**
 * Store allowed_values string.
 *
 * @param $allowed_values
 */
function _mediamosa_tool_params_do_command(&$node) {
  if ($node->{mediamosa_tool_params_db::ALLOWED_VALUE}) {
    if (!mediamosa_lib::is_serialized($node->{mediamosa_tool_params_db::ALLOWED_VALUE})) {
      if (is_array($node->{mediamosa_tool_params_db::ALLOWED_VALUE})) {
        $node->{mediamosa_tool_params_db::ALLOWED_VALUE} = serialize($node->{mediamosa_tool_params_db::ALLOWED_VALUE});
      }
      else {
        $node->{mediamosa_tool_params_db::ALLOWED_VALUE} = serialize(explode(';', $node->{mediamosa_tool_params_db::ALLOWED_VALUE}));
      }
    }
  }
  else {
    $node->{mediamosa_tool_params_db::ALLOWED_VALUE} = NULL;
  }

  if (!$node->{mediamosa_tool_params_db::MIN_VALUE}) {
    $node->{mediamosa_tool_params_db::MIN_VALUE} = NULL;
  }

  if (!$node->{mediamosa_tool_params_db::MAX_VALUE}) {
    $node->{mediamosa_tool_params_db::MAX_VALUE} = NULL;
  }
}

/**
 * Implements hook_update().
 */
function mediamosa_tool_params_update($node) {

  if ($node->revision) {
    mediamosa_tool_params_insert($node);
  }
  else {
    // Store the commands.
    _mediamosa_tool_params_do_command($node);

    // Get the field names of the table.
    $names = mediamosa_tool_params_db::get_fields();

    // Exception is title.
    $node->{mediamosa_tool_params_db::NICE_PARAMETER} = $node->title;

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_tool_params_db::TABLE_NAME, $names);
  }
}

/**
 * Implements hook_insert().
 */
function mediamosa_tool_params_insert($node) {
  // Store the commands.
  _mediamosa_tool_params_do_command($node);

  // Get the field names of the table.
  $names = mediamosa_tool_params_db::get_fields();

  // Exception is title.
  $node->{mediamosa_tool_params_db::NICE_PARAMETER} = $node->title;

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_tool_params_db::TABLE_NAME, $names);
}

/**
 * Implements hook_delete().
 */
function mediamosa_tool_params_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_tool_params_db::TABLE_NAME);
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_tool_params_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * Implements hook_view().
 */
function mediamosa_tool_params_view($node, $view_mode = 'full') {
  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Tool parameters'), 'admin/mediamosa/config/tool_params');
  drupal_set_breadcrumb($breadcrumb);

  // Default parameters.
  $rows = array(
    array(t('Name'), check_plain($node->nice_parameter)),
    array(t('Tool'), check_plain($node->tool)),
    array(t('Tool params id'), check_plain($node->tool_params_id)),
    array(t('Tool parameter'), check_plain($node->tool_parameter)),
    array(t('Min value'), check_plain($node->min_value)),
    array(t('Max value'), check_plain($node->max_value)),
    array(t('Default value'), check_plain($node->default_value)),
    array(t('Required'), t(drupal_ucfirst(drupal_strtolower($node->required)))),
    array(t('Description'), check_plain($node->description)),
    array(t('Switch'), t(drupal_ucfirst(drupal_strtolower($node->type_parameter ? $node->type_parameter : mediamosa_tool_params_db::TYPE_PARAMETER_SELECT)))),
  );

  // Add timestamps.
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['tool_params'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implements hook_form().
 */
function mediamosa_tool_params_form($node, $form_state) {
  // If we're inserting a new node, set some defaults:
  if (!isset($node->nid)) {
    $node->tool = 'ffmpeg';
    $node->required = 'FALSE';
  }

  $form = array();

  if (isset($node->tool_params_id)) {
    $form['tool_params_id_markup'] = array(
      '#title' => t('Tool params id'),
      '#type' => 'item',
      '#markup' => $node->tool_params_id,
    );
    $form['tool_params_id'] = array(
      '#type' => 'hidden',
      '#value' => $node->tool_params_id,
    );
  }

  $form['title'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('The name of this parameter.'),
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_tool_params_db::NICE_PARAMETER_LENGTH,
  );

  $tools = mediamosa_server_tool::get_server_tools();
  $options = array();
  foreach ($tools as $tool_id => $tool) {
    $options[$tool_id] = $tool['name'];
  }

  $form[mediamosa_tool_params_db::TOOL] = array(
    '#title' => t('Tool'),
    '#type' => 'select',
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::TOOL, $node),
    '#options' => $options,
    '#required' => TRUE,
    '#description' => t('Select one of the available tools.'),
  );

  $form[mediamosa_tool_params_db::REQUIRED] = array(
    '#title' => t('Required'),
    '#type' => 'select',
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::REQUIRED, $node),
    '#options' => array(
      mediamosa_tool_params_db::REQUIRED_TRUE => mediamosa_tool_params_db::REQUIRED_TRUE,
      mediamosa_tool_params_db::REQUIRED_FALSE => mediamosa_tool_params_db::REQUIRED_FALSE,
    ),
    '#required' => TRUE,
    '#description' => t('The parameter is required for this tool.'),
  );

  $form[mediamosa_tool_params_db::TOOL_PARAMETER] = array(
    '#title' => t('Tool parameter'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('Enter the parameter like it is used with the tool, i.e. --foo=test, where tool parameter entered here is \'--foo\'.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::TOOL_PARAMETER, $node),
    '#maxlength' => mediamosa_tool_params_db::TOOL_PARAMETER_LENGTH,
  );

  $type_parameter = mediamosa_node::default_value(mediamosa_tool_params_db::TYPE_PARAMETER, $node);
  $form[mediamosa_tool_params_db::TYPE_PARAMETER] = array(
    '#title' => t('Checkbox'),
    '#type' => 'select',
    '#default_value' => ($type_parameter ? $type_parameter : mediamosa_tool_params_db::TYPE_PARAMETER_SELECT),
    '#options' => array(
      mediamosa_tool_params_db::TYPE_PARAMETER_CHECKBOX => mediamosa_tool_params_db::TYPE_PARAMETER_CHECKBOX,
      mediamosa_tool_params_db::TYPE_PARAMETER_SELECT => mediamosa_tool_params_db::TYPE_PARAMETER_SELECT,
    ),
    '#required' => TRUE,
    '#description' => t('Show the parameter as a checkbox.'),
  );

  $form[mediamosa_tool_params_db::MIN_VALUE] = array(
    '#title' => t('Min value'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#description' => t('Minimum value (numeric value only).'),
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::MIN_VALUE, $node),
  );

  $form[mediamosa_tool_params_db::MAX_VALUE] = array(
    '#title' => t('Max value'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#description' => t('Maximum value (numeric value only).'),
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::MAX_VALUE, $node),
  );

  $allowed_value = unserialize(mediamosa_node::default_value(mediamosa_tool_params_db::ALLOWED_VALUE, $node));
  $allowed_value = implode(';', $allowed_value ? $allowed_value : array());
  $form[mediamosa_tool_params_db::ALLOWED_VALUE] = array(
    '#title' => t('Allowed value'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#description' => t('Allowed value separated by ; to allow multiple allowed values.'),
    '#default_value' => $allowed_value,
    '#maxlength' => mediamosa_tool_params_db::ALLOWED_VALUE_LENGTH,
  );

  $form[mediamosa_tool_params_db::DEFAULT_VALUE] = array(
    '#title' => t('Default value'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#description' => t('Default value when not supplied.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::DEFAULT_VALUE, $node),
    '#maxlength' => mediamosa_tool_params_db::DEFAULT_VALUE_LENGTH,
  );

  $form[mediamosa_tool_params_db::DESCRIPTION] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#description' => t('Description of this parameter.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::DESCRIPTION, $node),
  );

  return $form;
}

/**
 * Implements hook_validate().
 * TODO: Tool params should validate the values, when the user save the node:
 * - Nice parameter is unique.
 * - Tool parameter is unique.
 * - Allowed value is a ";" separated string.
 * - Default value is exists in the allowed values.
 */
function mediamosa_tool_params_validate($node) {
  // Nice parameter.
  if (drupal_substr($node->title, 0, drupal_strlen(MEDIAMOSA_TOOL_PARAMS_INTERNAL_PREFIX)) == MEDIAMOSA_TOOL_PARAMS_INTERNAL_PREFIX) {
    form_set_error('title', t("The tool param's name can't start with \"@prefix\".", array('@prefix' => MEDIAMOSA_TOOL_PARAMS_INTERNAL_PREFIX)));
  }

  // Tool.
  $tools = mediamosa_server_tool::get_server_tools();
  if (empty($tools[$node->{mediamosa_tool_params_db::TOOL}])) {
    form_set_error(mediamosa_tool_params_db::TOOL, t('This tool is either not enabled or does not exists.'));
  }

  // Min value.
  if ($node->{mediamosa_tool_params_db::MIN_VALUE} && !is_numeric($node->{mediamosa_tool_params_db::MIN_VALUE})) {
    form_set_error(mediamosa_tool_params_db::MIN_VALUE, t('Please enter a numerical value.'));
  }

  // Max value.
  if ($node->{mediamosa_tool_params_db::MAX_VALUE} && !is_numeric($node->{mediamosa_tool_params_db::MAX_VALUE})) {
    form_set_error(mediamosa_tool_params_db::MAX_VALUE, t('Please enter a numerical value.'));
  }
}
