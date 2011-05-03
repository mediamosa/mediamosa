<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * The node type client application.
 * hook_load / -update / -delete / -insert / -view / -form / -validate hooks for
 * node type mediamosa_app.
 */

/**
 * Static functions used by node type mediamosa_app.
 *
 */
class mediamosa_node_mediamosa_app {
  /**
   * Retrieve the transcoding profiles for dropdown list.
   *
   * @return array
   *  An array with all the profile Ids
   */
  static public function app_get_transcode_profiles() {
    $result = mediamosa_transcode_profile::get_all(array(mediamosa_transcode_profile_db::ID, mediamosa_transcode_profile_db::PROFILE));

    $output = array();
    foreach ($result as $transcode_profile) {
      $output[$transcode_profile['transcode_profile_id']] = $transcode_profile['transcode_profile_id'] . ' - ' . $transcode_profile['profile'];
    }

    return $output;
  }
}

/**
 * Insert a random app shared key in the text block.
 */
function _mediamosa_app_ajax_generate_shared_key() {

  // Load the form based upon the $_POST data sent via the ajax call.
  list($form, $form_state) = ajax_get_form();

  $commands = array();

  // Build and render the new select element, then return it in JSON format.
  drupal_process_form($form['form_id'], $form, $form_state);
  $form['client_application'][mediamosa_app_db::SHARED_KEY]['#value'] = mediamosa_db::uuid(rand(1, 9999));

  $commands[] = ajax_command_replace(NULL, drupal_render($form['client_application'][mediamosa_app_db::SHARED_KEY]));

  return array('#type' => 'ajax', '#commands' => $commands, '#header' => FALSE);
}

/**
 * Implements hook_load().
 */
function mediamosa_app_load(&$nodes) {
  mediamosa_node_revision::load($nodes);

  $app_ids = array();

  // Loop through the nodes and unserialize the master slave apps.
  foreach ($nodes as $key => $node) {
    $apps = array();
    if (!empty($nodes[$key]->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS})) {
      $apps = unserialize($nodes[$key]->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS});
    }

    // Copy apps.
    $nodes[$key]->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS} = $apps;

    // Make sure timezone is set.
    $nodes[$key]->{mediamosa_app_db::TIMEZONE} = empty($nodes[$key]->{mediamosa_app_db::TIMEZONE}) ? mediamosa_settings::MEDIAMOSA_DEFAULT_TIMEZONE : $nodes[$key]->{mediamosa_app_db::TIMEZONE};
  }
}

/**
 * Implements hook_update().
 */
function mediamosa_app_update($node) {
  if ($node->revision) {
    mediamosa_app_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_app_db::get_fields();

    // Exception is title.
    $node->app_name = $node->title;

    // Master/Slave Apps is an array, serialize it.
    $node->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS} = isset($node->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS}) ? serialize($node->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS}) : serialize(array());

    // Tool analyse is an array, serialize it.
    $node->{mediamosa_app_db::TOOL_ANALYSE} = isset($node->{mediamosa_app_db::TOOL_ANALYSE}) ? serialize($node->{mediamosa_app_db::TOOL_ANALYSE}) : serialize(array());

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_app_db::TABLE_NAME, $names);

    // Update webservice.
    _mediamosa_app_update_webservice($node);
  }
}

/**
 * Implements hook_insert().
 */
function mediamosa_app_insert($node) {
  // Get the field names of the table.
  $names = mediamosa_app_db::get_fields();

  // Exception is title.
  $node->app_name = $node->title;

  // Master/Slave Apps is an array, serialize it.
  $node->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS} = isset($node->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS}) ? serialize($node->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS}) : serialize(array());

  // Tool analyse is an array, serialize it.
  $node->{mediamosa_app_db::TOOL_ANALYSE} = isset($node->{mediamosa_app_db::TOOL_ANALYSE}) ? serialize($node->{mediamosa_app_db::TOOL_ANALYSE}) : serialize(array());

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_app_db::TABLE_NAME, $names);

  // Update webservice.
  _mediamosa_app_update_webservice($node);
}

/**
 * Update the webservice link.
 *
 * @param object $node
 */
function _mediamosa_app_update_webservice($node) {

  // Get all handles.
  $handles = mediamosa_webservice::get_all_handles();

  // Now set any.
  foreach ($handles as $handle) {
    if (isset($node->{'webservice_' . $handle})) {
      mediamosa_webservice_app::webservice_status($node->app_id, $handle, $node->{'webservice_' . $handle} == mediamosa_webservice_app_db::STATUS_TRUE);
    }
    else {
      mediamosa_webservice_app::webservice_status($node->app_id, $handle, TRUE);
    }
  }
}

/**
 * Implements hook_delete().
 */
function mediamosa_app_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_app_db::TABLE_NAME);
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_app_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * Implements hook_view().
 */
function mediamosa_app_view($node, $view_mode = 'full') {

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Client applications'), 'admin/mediamosa/config/app');
  drupal_set_breadcrumb($breadcrumb);

  $status = array(
    mediamosa_app_db::ACTIVE_TRUE => t('Enabled'),
    mediamosa_app_db::ACTIVE_FALSE => t('Disabled'),
  );

  // Get all apps.
  $apps = mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME, mediamosa_app_db::NID), mediamosa_app_db::APP_NAME);

  $allow_masterslave_apps = array();
  foreach ($apps as $app) {
    if (!in_array($app[mediamosa_app_db::APP_ID], $node->{mediamosa_app_db::ALLOW_MASTERSLAVE_APPS})) {
      continue;
    }

    $allow_masterslave_apps[] = strtr('@app_name (@app_id)', array('@app_name' => $app[mediamosa_app_db::APP_NAME], '@app_id' => $app[mediamosa_app_db::APP_ID]));
  }

  // Default parameters
  $rows = array(
    array(t('Name'), check_plain($node->title)),
    array(t('Application ID'), check_plain($node->{mediamosa_app_db::APP_ID})),
    array(t('Quota'), check_plain($node->{mediamosa_app_db::QUOTA})),
    array(t('Shared Key'), node_access('update', $app[mediamosa_app_db::NID]) ? check_plain($node->{mediamosa_app_db::SHARED_KEY}) : str_repeat('*', drupal_strlen($node->{mediamosa_app_db::SHARED_KEY}))),
    array(t('Default time zone'), check_plain($node->{mediamosa_app_db::TIMEZONE})),
    array(t('Description'), check_plain($node->{mediamosa_app_db::DESCRIPTION})),
    array(t('Status'), $status[$node->{mediamosa_app_db::ACTIVE}]),
    array(t('REST call version'), check_plain($node->{mediamosa_app_db::ACTIVE_VERSION})),
    array(t('Owner'), check_plain($node->{mediamosa_app_db::OWNER})),
    array(t('Allowed Master/Slave Apps'), empty($allow_masterslave_apps) ? t('No applications allowed.') : implode(', ', $allow_masterslave_apps)),
  );

  // Add timestamps.
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $rows_contact = array(
    array(t('Technical name'), check_plain($node->technical_name)),
    array(t('Technical phone'), check_plain($node->technical_phone)),
    array(t('Technical email'), check_plain($node->technical_email)),
    array(t('Administrative name'), check_plain($node->administrative_name)),
    array(t('Administrative phone'), check_plain($node->administrative_phone)),
    array(t('Administrative email'), check_plain($node->administrative_email)),
  );

  $node->content = array(
    'app_title' => array(
      '#markup' => '<h1>Client application data</h1>',
    ),
    'app' => array(
      '#markup' => theme('table', array('header' => array(t('Item'), t('Value')), 'rows' => $rows))
    ),
    'contact_title' => array(
      '#markup' => '<h1>Client application technical and administrative contact information</h1>',
    ),
    'contact' => array(
      '#markup' => theme('table', array('header' => array(t('Item'), t('Value')), 'rows' => $rows_contact))
    ),
  );

  return $node;
}

/**
 * Form validate for image style.
 */
function mediamosa_node_mediamosa_app_image_style_validate($form, &$form_state) {
  $app_id = arg(5);

  if (!is_numeric($app_id)) {
    $pos = strpos($app_id, mediamosa_settings::IMAGE_STYLE_SEPARATOR);
    if ($pos === FALSE) {
      return;
    }

    $app_id = drupal_substr($app_id, 0, $pos);
  }

  $form_state['values']['name'] = $app_id . mediamosa_settings::IMAGE_STYLE_SEPARATOR . $form_state['values']['name'];
}

/**
 * Implements hook_form().
 */
function mediamosa_app_form($node, &$form_state) {
  global $user;

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Client applications'), 'admin/mediamosa/config/app');
  drupal_set_breadcrumb($breadcrumb);

  // Get all apps. app_id -> name
  $apps_all = mediamosa_app::get_all_apps(array(mediamosa_app_db::NID, mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME), mediamosa_app_db::APP_NAME);

  $apps = array();
  foreach ($apps_all as $app) {
    // Skip yourself.
    if (isset($node->nid) && $app[mediamosa_app_db::NID] == $node->nid) {
      continue;
    }

    $apps[$app[mediamosa_app_db::APP_ID]] = strtr('@app_name (@app_id)', array('@app_name' => $app[mediamosa_app_db::APP_NAME], '@app_id' => $app[mediamosa_app_db::APP_ID]));
  }

  // Use drupal to get the timezones.
  $timezones = system_time_zones();

  $form = array();

  $form['client_application'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Client application'),
    '#description' => t('Settings for client application.')
  );

  $form['client_application']['title'] = array( // app_name.
    '#type' => 'textfield',
    '#title' => t('Client application name'),
    '#description' => t('Full name for client application.'),
    '#required' => TRUE,
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_app_db::APP_NAME_LENGTH,
  );

  $form['client_application'][mediamosa_app_db::APP_ID] = array(
    '#type' => 'textfield',
    '#title' => t('App ID'),
    '#description' => t('Unique application ID.'),
    '#required' => TRUE,
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::APP_ID, $node),
  );

  $form['client_application'][mediamosa_app_db::SHARED_KEY] = array(
    '#type' => 'textfield',
    '#title' => t('Shared key'),
    '#required' => TRUE,
    '#prefix' => '<div id="replace_shared_key_div">',
    '#suffix' => '</div>',
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::SHARED_KEY, $node),
    '#maxlength' => mediamosa_app_db::SHARED_KEY_LENGTH,
    '#description' => t('The shared key is the password that the application must use during the application verification.'),
  );

  $form['client_application']['generate_shared_key'] = array(
    '#type' => 'button',
    '#value' => t('Paste generated key'),
    '#ajax' => array(
      'event' => 'click',
      'wrapper' => 'replace_shared_key_div',
      'path' => 'admin/mediamosa/config/app/js/form',
    ),
  );

  $form['client_application'][mediamosa_app_db::QUOTA] = array(
    '#type' => 'textfield',
    '#title' => t('Quota'),
    '#required' => TRUE,
    '#description' => t('Quota for disk space usage in GB.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::QUOTA, $node),
  );

  $form['client_application'][mediamosa_app_db::TIMEZONE] = array(
    '#type' => 'select',
    '#title' => t('Default time zone'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::TIMEZONE, $node, mediamosa_settings::MEDIAMOSA_DEFAULT_TIMEZONE),
    '#options' => $timezones,
    '#required' => TRUE,
  );

  $form['client_application'][mediamosa_app_db::DESCRIPTION] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Description of client application'),
    '#required' => FALSE,
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::DESCRIPTION, $node),
    '#maxlength' => mediamosa_app_db::DESCRIPTION_LENGTH,
  );

  $form['client_application'][mediamosa_app_db::ACTIVE] = array(
    '#type' => 'select',
    '#title' => t('Status'),
    '#description' => t('Availability of client application.'),
    '#options' => array(mediamosa_app_db::ACTIVE_TRUE => t('Enabled'), mediamosa_app_db::ACTIVE_FALSE => t('Disabled')),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ACTIVE, $node, mediamosa_app_db::ACTIVE_TRUE),
  );

  $form['client_application'][mediamosa_app_db::ACTIVE_VERSION] = array(
    '#type' => 'select',
    '#title' => t('Select MediaMosa version'),
    '#description' => t('Select the version this client application can use. If your client application can only support version 2.3 and lower, then select 2.3. This setting will override the global setting.'),
    '#options' => mediamosa_rest::get_all_versions(t('Use global setting')),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ACTIVE_VERSION, $node, mediamosa_version::LATEST),
  );

  $form['masterslave'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Master/slave settings'),
    '#description' => t('Control which other clientapplications can slave asset to this application'),
  );

  $form['masterslave'][mediamosa_app_db::ALLOW_MASTERSLAVE_APPS] = array(
    '#type' => 'select',
    '#title' => t('Allow master/slave access'),
    '#description' => t('Select other client applications that are allowed to master/slave their  assets and mediafiles to this application. Use Use SHIFT or CTRL to select more than one application. If no applications are selected, then no application can master/slave their assets/mediafiles to this application (default). Existing master/slave rules will continue to function.'),
    '#options' => $apps,
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ALLOW_MASTERSLAVE_APPS, $node, array()),
    '#multiple' => TRUE,
    '#size' => 20,
  );

  $form['oai'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('OAI Interface settings'),
    '#description' => t('')
  );

  $form['oai'][mediamosa_app_db::IS_METADATA_PUBLIC] = array(
    '#type' => 'select',
    '#title' => t('OAI visibility metadata'),
    '#description' => t('Is metadata of this client application public visible in OAI?'),
    '#options' => array(mediamosa_app_db::IS_METADATA_PUBLIC_TRUE => t('Public'), mediamosa_app_db::IS_METADATA_PUBLIC_FALSE => t('Hidden')),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::IS_METADATA_PUBLIC, $node, mediamosa_app_db::IS_METADATA_PUBLIC_TRUE),
  );
  $form['oai'][mediamosa_app_db::SHOW_EXTERNAL_ASSETS] = array(
    '#type' => 'select',
    '#title' => t('OAI external references'),
    '#description' => t('Show external references in OAI (in field referenceId)?'),
    '#options' => array('TRUE' => t('show'), 'FALSE' => t('hide')),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::SHOW_EXTERNAL_ASSETS, $node, mediamosa_app_db::SHOW_EXTERNAL_ASSETS_TRUE),
  );

  $form['oauth'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('oAuth settings'),
    '#description' => t('')
  );
  $form['oauth'][mediamosa_app_db::OAUTH_CONSUMER_KEY] = array(
    '#type' => 'textfield',
    '#title' => t('oAuth consumer key'),
    '#description' => t('Unique oAuth consumer key.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::OAUTH_CONSUMER_KEY, $node),
  );
  $form['oauth'][mediamosa_app_db::OAUTH_CONSUMER_SECRET] = array(
    '#type' => 'textfield',
    '#title' => t('oAuth consumer secret'),
    '#description' => t('oAuth consumer secret. Required, if oAuth consumer key is given.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::OAUTH_CONSUMER_SECRET, $node),
  );

  /*
  // This is for later development / test.
  // See: hook_menu() of sites/all/modules/mediamosa/modules/app/oauth/mediamosa_app_oauth.module
  if (isset($node->nid)) {
    $form['oauth']['oauth_client'] = array(
      '#type' => 'item',
      '#title' => l(t('oAuth client test'), 'admin/oauth_test/' . $node->nid),
      '#description' => t('Available oAuth client'),
    );
  }
  */

  $tool_analyse_checkboxes = module_invoke_all('mediamosa_tool_options');
  $options = array();
  if (isset($tool_analyse_checkboxes['name']) && is_array($tool_analyse_checkboxes['name'])) {
    foreach ($tool_analyse_checkboxes['name'] as $tool_analyse_key => $tool_analyse_checkbox) {
      if (isset($tool_analyse_checkboxes['tool'][$tool_analyse_key])) {
        $options[$tool_analyse_checkboxes['tool'][$tool_analyse_key]] = $tool_analyse_checkbox;
      }
    }
  }

  $form['tool'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Tool selection'),
    '#description' => t('')
  );
  $default_value = mediamosa_node::default_value(mediamosa_app_db::TOOL_ANALYSE, $node, '');
  $form['tool'][mediamosa_app_db::TOOL_ANALYSE] = array(
    '#type' => 'checkboxes',
    '#title' => t('Choose the tools that may copy technical metadata to asset metadata.'),
    '#default_value' => ($default_value ? unserialize($default_value) : array()),
    '#options' => $options,
    '#description' => t('The tools can analyse their mediafiles and they can copy the data (f.e. author) to the asset metadata. You can allow or disallow these functions here.'),
  );

  $form['still'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Still'),
    '#description' => t('')
  );

  $tool = mediamosa_transcode_profile::get_by_tool(NULL, 'ffmpeg', 'size')->fetchAll();
  $tool = empty($tool) ? NULL : reset($tool);
  $allowed_values = empty($tool) ? array() : (trim($tool[mediamosa_tool_params_db::ALLOWED_VALUE]) == '' ? array() : unserialize($tool[mediamosa_tool_params_db::ALLOWED_VALUE]));
  $options = array('' => '');
  foreach ($allowed_values as $allowed_value) {
    $options[$allowed_value] = $allowed_value;
  }
  $form['still'][mediamosa_app_db::STILL_DEFAULT_SIZE] = array(
    '#type' => 'select',
    '#title' => t('Still default size'),
    '#description' => t('Set frame size using format WIDTHxHEIGHT (default = @size). Possible values: @options.', array('@size' => mediamosa_settings::STILL_DEFAULT_SIZE, '@options' => implode(', ', $options))),
    '#options' => $options,
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::STILL_DEFAULT_SIZE, $node, mediamosa_settings::STILL_DEFAULT_SIZE),
  );

  $form['still'][mediamosa_app_db::STILL_PADDING] = array(
    '#type' => 'select',
    '#title' => t('Padding'),
    '#description' => t('Force padding, if aspect ratio is maintained (default = yes). Possible values: , yes, no.'),
    '#options' => array('YES' => t('Yes'), 'NO' => t('No')),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::STILL_PADDING, $node, mediamosa_app_db::STILL_PADDING_YES),
  );

  if (isset($node->nid) && $node->nid) {
    $nid = $node->nid;

    $query = 'admin/mediamosa/config/app/' . $nid . '/edit';
    require_once('modules/image/image.admin.inc');
    $app_id = mediamosa_node::default_value(mediamosa_app_db::APP_ID, $node);
    $styles = image_styles();

    // Unset styles not created by this app.
    foreach ($styles as $style_key => $style) {
      if (drupal_substr($style_key, 0, drupal_strlen($app_id . mediamosa_settings::IMAGE_STYLE_SEPARATOR)) != $app_id . mediamosa_settings::IMAGE_STYLE_SEPARATOR) {
        unset($styles[$style_key]);
      }
    }

    // Change the links.
    $output_image_style_list = theme('image_style_list', array('styles' => $styles));
    $output_image_style_list = str_replace('admin/config/media/image-styles/add', 'admin/config/media/image-styles/add/'. $app_id . '?destination=' . $query, $output_image_style_list);
    foreach ($styles as $style_key => $style) {
      //$output_image_style_list = str_replace('admin/config/media/image-styles/edit/' . $style_key, 'admin/config/media/image-styles/edit/' . $style_key . '?destination=' . $query, $output_image_style_list);
      $output_image_style_list = str_replace('admin/config/media/image-styles/delete/' . $style_key, 'admin/config/media/image-styles/delete/' . $style_key . '?destination=' . $query, $output_image_style_list);
    }

    // Create the form element.
    $form['still']['image_style_list'] = array(
      '#markup' => t('Styles - !add_style', array('!add_style' => l(t('Add style'), 'admin/config/media/image-styles/add/'. $app_id, array('query' => array('destination' => $query))),)) . $output_image_style_list,
      '#attached' => array(
        'css' => array(drupal_get_path('module', 'image') . '/image.admin.css' => array()),
      ),
    );
  }

  $form['transcoding'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Transcoding'),
    '#description' => t('Transcoding settings for this client application.')
  );

  $transcoding_pos = array();
  for ($i = -10; $i <= 10; $i++) {
    $transcoding_pos[$i] = $i;
  }

  $form['transcoding'][mediamosa_app_db::TRANSCODING_POSITION_FROM] = array(
    '#type' => 'select',
    '#title' => t('The transcoding position from'),
    '#options' => $transcoding_pos,
    '#description' => t('Give server priority.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::TRANSCODING_POSITION_FROM, $node),
  );

  $form['transcoding'][mediamosa_app_db::TRANSCODING_POSITION_TO] = array(
    '#type' => 'select',
    '#title' => t('The transcoding position to'),
    '#options' => $transcoding_pos,
    '#description' => t('Give server priority.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::TRANSCODING_POSITION_TO, $node),
  );

  $form['transcoding'][mediamosa_app_db::TRANSCODING_MAX_SLOTS] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum transcoding slots allowed'),
    '#description' => t('Set the maximum number of transcoding jobs allowed for this application.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::TRANSCODING_MAX_SLOTS, $node, mediamosa_settings::JOB_MAX_SLOTS),
  );

  $form['transcoding'][mediamosa_app_db::ALWAYS_HINT_MP4] = array(
    '#type' => 'select',
    '#title' => t('Always hint (make streamable) mp4 files'),
    '#options' => array(mediamosa_app_db::ALWAYS_HINT_MP4_FALSE => t('Never'), mediamosa_app_db::ALWAYS_HINT_MP4_TRUE => t('Always')),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ALWAYS_HINT_MP4, $node, mediamosa_app_db::ALWAYS_HINT_MP4_TRUE),
  );

  $form['transcoding'][mediamosa_app_db::ALWAYS_INSERT_MD] = array(
    '#type' => 'select',
    '#title' => t('Always insert extra metadata flv files'),
    '#options' => array(mediamosa_app_db::ALWAYS_INSERT_MD_FALSE => t('Never'), mediamosa_app_db::ALWAYS_INSERT_MD_TRUE => t('Always')),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ALWAYS_INSERT_MD, $node, mediamosa_app_db::ALWAYS_INSERT_MD_TRUE),
  );

  $form['transcoding'][mediamosa_app_db::PREVIEW_PROFILE_ID] = array(
    '#type' => 'select',
    '#title' => t('Preview transcode profile id'),
    '#description' => t('Transcoding profile used for previewing.'),
    '#options' => mediamosa_node_mediamosa_app::app_get_transcode_profiles(),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::PREVIEW_PROFILE_ID, $node),
  );

  $form['urls'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('URLs'),
    '#description' => t('URLs shown in the asset settings.')
  );

  $form['urls'][mediamosa_app_db::DOWNLOAD_URL] = array(
    '#type' => 'textfield',
    '#title' => t('Download url'),
    '#description' => t('Shown in the asset details rest calls. Available tags: {asset_id}, {mediafile_id} and {mediafile_filename}'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::DOWNLOAD_URL, $node),
    '#maxlength' => mediamosa_app_db::DOWNLOAD_URL_LENGTH,
  );
  $form['urls'][mediamosa_app_db::STREAM_URL] = array(
    '#type' => 'textfield',
    '#title' => t('Stream url'),
    '#description' => t('Shown in the asset details rest calls. Available tags: {asset_id}, {mediafile_id} and {mediafile_filename}.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::STREAM_URL, $node),
    '#maxlength' => mediamosa_app_db::STREAM_URL_LENGTH,
  );
  $form['urls'][mediamosa_app_db::PLAY_PROXY_URL] = array(
    '#type' => 'textfield',
    '#title' => t('Play proxy url'),
    '#description' => t('Shown in the result of rest calls and shown by OAI. Available tags: {asset_id}, {mediafile_id}, {mediafile_filename} and {preview_profile_id}.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::PLAY_PROXY_URL, $node),
    '#maxlength' => mediamosa_app_db::PLAY_PROXY_URL_LENGTH,
  );
  $form['urls'][mediamosa_app_db::VIEW_ASSET_URL] = array(
    '#type' => 'textfield',
    '#title' => t('View asset url'),
    '#description' => t('Shown by OAI, field \'Resource Identifier\'. Available tags: {asset_id}.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::VIEW_ASSET_URL, $node),
    '#maxlength' => mediamosa_app_db::VIEW_ASSET_URL_LENGTH,
  );
  $form['urls']['still_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Still url'),
    '#description' => t('Shown by OAI for still images. Available tags: {asset_id}.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::STILL_URL, $node),
    '#maxlength' => mediamosa_app_db::STILL_URL_LENGTH,
  );

  $form['client_application'][mediamosa_app_db::OWNER] = array(
    '#type' => 'hidden',
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::OWNER, $node, ($user->uid > 1 ? $user->name : '')),
  );

  $form['webservice'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Webservices'),
    '#description' => t('Availibility per web service for this client application.')
  );

  // Dynamicly add fields for webservice handles
  if (!empty($node->app_id)) {
    $result = mediamosa_webservice::get_all($node->app_id);
  }
  else {
    $result = mediamosa_webservice::get_all();
  }

  // Get the stored one.
  $stored = $result->fetchAllAssoc(mediamosa_webservice_db::HANDLE);

  // any skipped hanlde?
  $handles = mediamosa_webservice::get_all_handles();

  foreach ($handles as $handle) {
    if (!isset($stored[$handle])) {
      $form['webservice']['webservice_' . $handle] = array(
        '#type' => 'select',
        '#title' => $handle,
        '#description' => t('Availibility of web service \'' . $handle . '\' for this client application.'),
        '#default_value' => 'TRUE',
        '#options' => array(mediamosa_webservice_app_db::STATUS_TRUE => t('Enabled'), mediamosa_webservice_app_db::STATUS_FALSE => t('Disabled')),
      );
    }
    else {
      $form['webservice']['webservice_' . $handle] = array(
        '#type' => 'select',
        '#title' => $stored[$handle]['handle'],
        '#description' => t('Availibility of web service \'' . $stored[$handle]['handle'] . '\' for this client application.'),
        '#default_value' => $stored[$handle]['status'],
        '#options' => array(mediamosa_webservice_app_db::STATUS_TRUE => t('Enabled'), mediamosa_webservice_app_db::STATUS_FALSE => t('Disabled')),
    );
    }
  }

  $form['technical'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Technical Contact'),
    '#description' => t('Technical contact person for this client application.')
  );

  $form['technical'][mediamosa_app_db::TECHNICAL_NAME] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of technical contact person for this client application.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::TECHNICAL_NAME, $node),
    '#maxlength' => mediamosa_app_db::TECHNICAL_NAME_LENGTH,
  );

  $form['technical'][mediamosa_app_db::TECHNICAL_PHONE] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#description' => t('Phone number of technical contact person for this client application.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::TECHNICAL_PHONE, $node),
    '#maxlength' => mediamosa_app_db::TECHNICAL_PHONE_LENGTH,
  );

  $form['technical'][mediamosa_app_db::TECHNICAL_EMAIL] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail'),
    '#description' => t('Email-address of technical contact person for this client application.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::TECHNICAL_PHONE, $node),
    '#maxlength' => mediamosa_app_db::TECHNICAL_EMAIL_LENGTH,
  );

  $form['administrative'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Administrative Contact'),
    '#description' => t('Administrative contact person for this client application.')
  );

  $form['administrative'][mediamosa_app_db::ADMINISTRATIVE_NAME] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of administrative contact person for this client application.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ADMINISTRATIVE_NAME, $node),
    '#maxlength' => mediamosa_app_db::ADMINISTRATIVE_NAME_LENGTH,
  );

  $form['administrative'][mediamosa_app_db::ADMINISTRATIVE_PHONE] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#description' => t('Phone number of administrative contact person for this client application.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ADMINISTRATIVE_PHONE, $node),
    '#maxlength' => mediamosa_app_db::ADMINISTRATIVE_PHONE_LENGTH,
  );

  $form['administrative'][mediamosa_app_db::ADMINISTRATIVE_EMAIL] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail'),
    '#description' => t('Email-address of administrative contact person for this client application.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_app_db::ADMINISTRATIVE_EMAIL, $node),
    '#maxlength' => mediamosa_app_db::ADMINISTRATIVE_EMAIL_LENGTH,
  );

  $form['#redirect'] = 'admin/mediamosa/config/app';

  return $form;
}

/**
 * Implements hook_validate().
 */
function mediamosa_app_validate($node) {

  // We need to know if its an update.
  $is_insert = empty($node->nid);

  // App Id checks.
  if (!is_numeric($node->{mediamosa_app_db::APP_ID})) {
    form_set_error(mediamosa_app_db::APP_ID, t('The Application ID must be between @app_id_min and @app_id_max', array('@app_id_min' => mediamosa_settings::APP_ID_MIN, '@app_id_max' => mediamosa_settings::APP_ID_MAX)));
  }
  elseif ((int) $node->{mediamosa_app_db::APP_ID} < mediamosa_settings::APP_ID_MIN || $node->{mediamosa_app_db::APP_ID} > mediamosa_settings::APP_ID_MAX) {
    form_set_error(mediamosa_app_db::APP_ID, t('The Application ID must be between @app_id_min and @app_id_max', array('@app_id_min' => mediamosa_settings::APP_ID_MIN, '@app_id_max' => mediamosa_settings::APP_ID_MAX)));
  }
  else {
    // Get all apps to know if app id is already taken.
    if ($is_insert && mediamosa_app::get_by_appid($node->{mediamosa_app_db::APP_ID})) {
      form_set_error(mediamosa_app_db::APP_ID, t('The Application ID is already taken, please choose another Application ID.'));
    }

    // If we change the app ID, we need to know if its already taken.
    if (!$is_insert) {
      $org_node = node_load($node->nid);
      if ($org_node->{mediamosa_app_db::APP_ID} != $node->{mediamosa_app_db::APP_ID}) {
        if (mediamosa_app::get_by_appid($node->{mediamosa_app_db::APP_ID})) {
          form_set_error(mediamosa_app_db::APP_ID, t('The Application ID is already taken, please choose another Application ID.'));
        }
      }
    }
  }

  // Transcoding position.
  if (!is_numeric($node->{mediamosa_app_db::TRANSCODING_POSITION_FROM})) {
    form_set_error(mediamosa_app_db::TRANSCODING_POSITION_FROM, t('Please enter a numerical value.'));
  }

  // Transcoding position.
  if (!is_numeric($node->{mediamosa_app_db::TRANSCODING_POSITION_TO})) {
    form_set_error(mediamosa_app_db::TRANSCODING_POSITION_TO, t('Please enter a numerical value.'));
  }

  // Transcoding position.
  if ($node->{mediamosa_app_db::TRANSCODING_POSITION_FROM} > $node->{mediamosa_app_db::TRANSCODING_POSITION_TO}) {
    form_set_error(mediamosa_app_db::TRANSCODING_POSITION_FROM, t('Transcoding position FROM value is bigger than transcoding position TO.'));
  }

  // Quota.
  if (!is_numeric($node->{mediamosa_app_db::QUOTA})) {
    form_set_error(mediamosa_app_db::QUOTA, t('Please enter a numerical value.'));
  }

  // oAuth.
  if (
    ( empty($node->{mediamosa_app_db::OAUTH_CONSUMER_KEY}) && !empty($node->{mediamosa_app_db::OAUTH_CONSUMER_SECRET})) ||
    ( !empty($node->{mediamosa_app_db::OAUTH_CONSUMER_KEY}) && empty($node->{mediamosa_app_db::OAUTH_CONSUMER_SECRET}))
  ) {
    form_set_error(mediamosa_app_db::OAUTH_CONSUMER_KEY, t('You must fill both oAuth key and secret or non of them.'));
  }
  // The consumer key must be unique.
  if (!empty($node->{mediamosa_app_db::OAUTH_CONSUMER_KEY}) && !empty($node->{mediamosa_app_db::OAUTH_CONSUMER_SECRET})) {
    $app = mediamosa_app::get_by_oauth_consumer_key($node->{mediamosa_app_db::OAUTH_CONSUMER_KEY});

    if ($app && $app['nid'] != $node->nid) {
      form_set_error(mediamosa_app_db::OAUTH_CONSUMER_KEY, t('This oAuth consumer key is already exists.'));
    }
  }
}

/**
 * Implements hook_node_access().
 */
function mediamosa_app_node_access($node, $op, $account) {

  // A string is given when the node is created.
  if (is_string($node)) {
    return NODE_ACCESS_IGNORE;
  }

  // We only support Client Apps.
  if ($node->type != mediamosa_node::MEDIAMOSA_NODE_TYPE_APP) {
    return NODE_ACCESS_IGNORE;
  }

  // Just for debugging.
  assert($node->{mediamosa_app_db::APP_ID});

  // Build the role we need.
  $role_needed = mediamosa_permission::ADMIN_MEDIAMOSA . '|app|' . $node->{mediamosa_app_db::APP_ID} . '|' . $op;

  // If we have to role, then we allow the action.
  return _mediamosa_user_access($role_needed) ? NODE_ACCESS_ALLOW : NODE_ACCESS_IGNORE;
}
