<?php
/**
 * @file
 * hook_load / -update / -delete / -insert hooks for node type mediamosa_transcode_profile.
 */

/**
 * Implements hook_load().
 */
function mediamosa_transcode_profile_load(&$nodes) {
  $nodes = mediamosa_node_revision::load($nodes);

  foreach ($nodes as $key => $node) {
    $nodes[$key]->commands = empty($node->{mediamosa_transcode_profile_db::COMMAND}) ? array() : mediamosa_transcode_profile::commandToArray($node->{mediamosa_transcode_profile_db::COMMAND});
  }

  return $nodes;
}

/**
 * Store command string.
 *
 * @param object $node
 *   The node to process the input.
 */
function _mediamosa_transcode_profile_do_command($node) {

  $commands = array();
  foreach ($node as $key => $value) {
    if (strpos($key, 'options_') === 0) {

      // Get the tmid and strip the value
      $command = drupal_substr($key, drupal_strlen('options_'));
      $value = trim($value);

      // Don't store switch value in case of checkbox wasn't checked.
      if (!$value && isset($node->{'switch_options_' . $command}) && $node->{'switch_options_' . $command} == mediamosa_tool_params_db::ALLOWED_VALUE_FOR_SWITCH) {
        continue;
      }

      // Gather all set options
      if ($value != '') {
        $commands[$command] = $value;
      }
    }
  }

  // If no options where found and we have already command in our node, then
  // use the found command instead.
  if (empty($commands) && !empty($node->{mediamosa_transcode_profile_db::COMMAND})) {
    return $node->{mediamosa_transcode_profile_db::COMMAND};
  }

  // Convert to string.
  return mediamosa_transcode_profile::arrayToCommand($commands);
}

/**
 * Implements hook_update().
 */
function mediamosa_transcode_profile_update($node) {
  if (!empty($node->revision)) {
    mediamosa_transcode_profile_insert($node);
  }
  else {
    // If this profile is set als default, then reset the rest to FALSE.
    _mediamosa_transcode_profile_update_isdefault($node);

    // Store the commands.
    $node->{mediamosa_transcode_profile_db::COMMAND} = _mediamosa_transcode_profile_do_command($node);

    // Get the field names of the table.
    $names = mediamosa_transcode_profile_db::get_fields();

    // Exception is title.
    $node->{mediamosa_transcode_profile_db::PROFILE} = $node->title;

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_transcode_profile_db::TABLE_NAME, $names);
  }
}

/**
 * Implements hook_insert().
 */
function mediamosa_transcode_profile_insert($node) {
  // If this profile is set als default, then reset the rest to FALSE.
  _mediamosa_transcode_profile_update_isdefault($node);

  // Store the commands.
  $node->{mediamosa_transcode_profile_db::COMMAND} = _mediamosa_transcode_profile_do_command($node);

  // Get the field names of the table.
  $names = mediamosa_transcode_profile_db::get_fields();

  // Exception is title.
  $node->{mediamosa_transcode_profile_db::PROFILE} = $node->title;

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_transcode_profile_db::TABLE_NAME, $names);
}

/**
 * Implements hook_delete().
 */
function mediamosa_transcode_profile_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_transcode_profile_db::TABLE_NAME);
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_transcode_profile_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * If this profile is set als default, then reset the rest to FALSE
 *
 * @param object $node
 */
function _mediamosa_transcode_profile_update_isdefault($node) {
  if ($node->{mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE} == mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE) {
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_transcode_profile} SET #is_default_profile = :is_default_profile_false',
      array(
        '#mediamosa_transcode_profile' => mediamosa_transcode_profile_db::TABLE_NAME,
        '#is_default_profile' => mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE,
        ':is_default_profile_false' => mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE,
      )
    );
  }
}

/**
 * Implements hook_view().
 */
function mediamosa_transcode_profile_view($node, $view_mode = 'full') {
  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Transcode profiles'), 'admin/mediamosa/config/transcode_profile');
  drupal_set_breadcrumb($breadcrumb);

  // Get the apps.
  $apps = mediamosa_app::collect_allowed_apps();

  // Default parameters.
  $rows = array(
    array(t('Title'), check_plain($node->title)),
    array(t('Transcode profile id'), check_plain($node->transcode_profile_id)),
    array(t('Application'), isset($apps[$node->app_id]) ? $apps[$node->app_id] : 'Unknown or deleted application'),
    array(t('Tool'), $node->tool),
    array(t('File extension'), $node->file_extension),
    array(t('Is default profile'), t(drupal_ucfirst(drupal_strtolower($node->is_default_profile)))),
  );

  // Optional parameters.
  $all_options = mediamosa_transcode_profile::get_by_tool(NULL, $node->tool);
  if (!empty($node->options)) {
    foreach ($node->options as $key => $value) {
      $rows[] = array($all_options[$key]['field_title'], check_plain($value));
    }
  }

  // Add timestamps.
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['transcode_profile'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implements hook_form().
 */
function mediamosa_transcode_profile_form($node, $form_state) {
  // Get the apps.
  //$apps = array('0' => '');
  $apps = mediamosa_app::collect_allowed_apps();
  $apps[0] = '[global profile]';

  // If we're inserting a new node, set some defaults:
  if (!isset($node->nid)) {
    $node->app_id = 0;
    $node->is_default_profile = 'FALSE';
    $node->file_extension = '';

    // Only administrators may add global profiles.
    if (!user_access(mediamosa_permission::ADMIN_MEDIAMOSA)) {
      unset($apps[0]);
    }
  }

  $form = array();

  $form['required'] = array(
    '#type' => 'fieldset',
    '#title' => t('Transcode profile'),
    '#description' => t('Required parameters for a transcoding profile.'),
  );


  if (isset($node->transcode_profile_id)) {
    $form['required']['transcode_profile_id_markup'] = array(
      '#title' => t('Transcode profile id'),
      '#type' => 'item',
      '#markup' => $node->transcode_profile_id,
    );
    $form['required']['transcode_profile_id'] = array(
      '#type' => 'hidden',
      '#value' => $node->transcode_profile_id,
    );
  }

  $form['required'][mediamosa_transcode_profile_db::APP_ID] = array(
    '#title' => t('Application'),
    '#type' => 'select',
    '#default_value' => $node->{mediamosa_transcode_profile_db::APP_ID},
    '#required' => TRUE,
    '#options' => $apps,
    '#description' => t('The application that owns this profile.'),
  );

  $form['required']['title'] = array(
    '#title' => t('Profile name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('The name of this transcode profile.'),
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_transcode_profile_db::PROFILE_LENGTH,
  );

  $form['required'][mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE] = array(
    '#title' => t('Is this the default profile?'),
    '#type' => 'select',
    '#default_value' => mediamosa_node::default_value(mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE, $node),
    '#options' => array(mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE => t('Yes'), mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE => t('No')),
    '#required' => TRUE,
    '#description' => t('If a transcoding profile is ommited from a transcoding request, the default profile will be choosen.'),
  );

  $tools = mediamosa_server_tool::get_server_tools();
  $options = array();
  foreach ($tools as $tool_id => $tool) {
    $options[$tool_id] = $tool['name'];
  }

  $form['required'][mediamosa_transcode_profile_db::TOOL] = array(
    '#title' => t('Transcoding Tool'),
    '#type' => 'select',
    '#default_value' => mediamosa_node::default_value(mediamosa_tool_params_db::TOOL, $node),
    '#options' => $options,
    '#required' => TRUE,
    '#description' => t('The transcoding tool used for this profile.'),
  );

  $form['required'][mediamosa_transcode_profile_db::FILE_EXTENSION] = array(
    '#title' => t('File extension'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('This file extension is only used for the generated filename, which will be the original filename (minus original extension) plus this extension.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_transcode_profile_db::FILE_EXTENSION, $node),
    '#maxlength' => mediamosa_transcode_profile_db::FILE_EXTENSION_LENGTH,
  );

  if (isset($node->tool) && $node->tool) {
    $tools = mediamosa_transcode_profile::get_by_tool(NULL, $node->tool)->fetchAll();
  }
  else {
    $tools = mediamosa_tool_params::search()->fetchAll();
  }

  // Skip these, not changable or internal.
  $tools_to_skip = array(
    'internal_previous_job',
    'internal_file_extension',
    'startposition',
  );

  // Show all parameters for profile.
  foreach ($tools as $key => $tool) {

    if (!isset($form['tool_options_' . $tool[mediamosa_tool_params_db::TOOL]])) {
      $form['tool_options_' . $tool[mediamosa_tool_params_db::TOOL]] = array(
        '#type' => 'fieldset',
        '#title' => t('@tool parameters', array('@tool' => drupal_ucfirst($tool[mediamosa_tool_params_db::TOOL]))),
        '#collapsed' => !isset($node->tool),
        '#collapsible' => TRUE,
      );
    }

    // get the name of the parameter.
    $name = $tool[mediamosa_tool_params_db::NICE_PARAMETER];

    if (in_array($name, $tools_to_skip)) {
      continue;
    }

    // Convert the allowed value to array.
    $tool[mediamosa_tool_params_db::ALLOWED_VALUE] = trim($tool[mediamosa_tool_params_db::ALLOWED_VALUE]) == '' ? array() : unserialize($tool[mediamosa_tool_params_db::ALLOWED_VALUE]);

    // Tool required?
    $is_required = $tool[mediamosa_tool_params_db::REQUIRED] == mediamosa_tool_params_db::REQUIRED_TRUE;

    // Put required higher.
    $weight = $is_required ? 0 : 15;

    // Get the value.
    $value = isset($node->commands{$name}) ? $node->commands{$name} : ($is_required ? $tool[mediamosa_tool_params_db::DEFAULT_VALUE] : '');

    $descriptions = array();
    $description = mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'tp')
      ->condition('tp.' . mediamosa_tool_params_db::NICE_PARAMETER, $name)
      ->fields('tp', array(mediamosa_tool_params_db::DESCRIPTION))
      ->execute()
      ->fetchField();

    if (!empty($description) && isset($tool[mediamosa_tool_params_db::DEFAULT_VALUE])) {
      $description = strtr($description, array('@default' => ' (default = @value).'));
      $description = strtr($description, array('@value' => $tool[mediamosa_tool_params_db::DEFAULT_VALUE]));
      $descriptions[] = $description;
    }
    elseif (!empty($description)) {
      $descriptions[] = strtr($description, array('@default' => '.'));
    }

    // Get the tool.
    $tool_name = drupal_ucfirst(str_replace('_', ' ', $name));

    if ($tool[mediamosa_tool_params_db::TYPE_PARAMETER] == mediamosa_tool_params_db::TYPE_PARAMETER_CHECKBOX) {
      $form['tool_options_' . $tool[mediamosa_tool_params_db::TOOL]]['options_' . $name] = array(
        '#type' => 'checkbox',
        '#title' => t($tool_name),
        '#description' => implode(' ', $descriptions),
        '#required' => $is_required,
        '#default_value' => $value,
        '#weight' => $weight,
        '#return_value' => mediamosa_tool_params_db::ALLOWED_VALUE_FOR_SWITCH,
      );
      $form['tool_options_' . $tool[mediamosa_tool_params_db::TOOL]]['switch_options_' . $name] = array(
        '#type' => 'value',
        '#value' => mediamosa_tool_params_db::ALLOWED_VALUE_FOR_SWITCH,
      );
    }
    elseif (!empty($tool[mediamosa_tool_params_db::ALLOWED_VALUE])) {
      $options = $is_required ? array() : array('' => '');
      foreach ($tool[mediamosa_tool_params_db::ALLOWED_VALUE] as $allowed_value) {
        $options[$allowed_value] = $allowed_value;
      }

      $descriptions[] = strtr('Possible values: @options.', array('@options' => implode(', ', $options)));

      $form['tool_options_' . $tool[mediamosa_tool_params_db::TOOL]]['options_' . $name] = array(
        '#description' => implode(' ', $descriptions),
        '#type' => 'select',
        '#title' => t($tool_name),
        '#required' => $is_required,
        '#default_value' => $value,
        '#options' => $options,
        '#weight' => $weight,
      );
    }
    elseif (!empty($tool[mediamosa_tool_params_db::MIN_VALUE]) && !empty($tool[mediamosa_tool_params_db::MAX_VALUE]) && $tool[mediamosa_tool_params_db::MIN_VALUE] && $tool[mediamosa_tool_params_db::MAX_VALUE]) {// Select box.
      // Add allowed value to description.
      $descriptions[] = t('The allowed value is between @min_value and @max_value.', array('@min_value' => $tool[mediamosa_tool_params_db::MIN_VALUE], '@max_value' => $tool[mediamosa_tool_params_db::MAX_VALUE]));

      $form['tool_options_' . $tool[mediamosa_tool_params_db::TOOL]]['options_' . $name] = array(
        '#description' => implode(' ', $descriptions),
        '#type' => 'textfield',
        '#title' => t($tool_name),
        '#required' => $is_required,
        '#default_value' => $value,
        '#size' => 25,
        '#weight' => $weight
      );
    }
    else {
      $form['tool_options_' . $tool[mediamosa_tool_params_db::TOOL]]['options_' . $name] = array(
        '#description' => implode(' ', $descriptions),
        '#type' => 'textfield',
        '#title' => t($tool_name),
        '#required' => $is_required,
        '#default_value' => $value,
        '#size' => 25,
        '#weight' => $weight
      );
    }
  }

  return $form;
}
