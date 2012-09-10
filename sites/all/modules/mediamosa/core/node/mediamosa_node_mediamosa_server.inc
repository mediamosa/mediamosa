<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * hook_load / -update / -delete / -insert hooks for node type mediamosa_server.
  *
  * There is no need to revision the tools and containers for transcode or
  * streaming server. These values are taken from one field in the main object.
  */

/**
 * Implements hook_load().
 */
function mediamosa_server_load(&$nodes) {
  mediamosa_node_revision::load($nodes);
  return $nodes;
}

/**
 * Helper to trim array.
 * @param string $item1
 */
function _mediamosa_server_trim(&$item1) {
  $item1 = trim($item1);
}

/**
 * Save the extra tools and containers.
 *
 * @param object $node
 */
function _process_server_node(&$node) {
  // URI.
  if ($node->{mediamosa_server_db::SERVER_URI} && drupal_substr($node->{mediamosa_server_db::SERVER_URI}, -1) != '/') {
    $node->{mediamosa_server_db::SERVER_URI} .= '/';
  }

  if (isset($node->{mediamosa_server_db::TOOLS})) {
    $tools = explode('|', $node->{mediamosa_server_db::TOOLS});
    array_walk($tools, '_mediamosa_server_trim');
    $tools = array_unique($tools);

    // to fix spaces, doubles etc.
    $node->{mediamosa_server_db::TOOLS} = implode('|', $tools);

    // Remove all first.
    mediamosa_db::db_delete(mediamosa_server_tool_db::TABLE_NAME)
      ->condition(mediamosa_server_tool_db::NID, $node->nid)
      ->execute();

    foreach ($tools as $tool) {
      mediamosa_db::db_insert(mediamosa_server_tool_db::TABLE_NAME)
        ->fields(array(
          mediamosa_server_tool_db::NID => $node->nid,
          mediamosa_server_tool_db::TOOL => mediamosa_unicode::substr($tool, 0, mediamosa_server_tool_db::TOOL_LENGTH)
        ))
        ->execute();
    }
  }

  if (isset($node->{mediamosa_server_db::CONTAINERS})) {
    $containers = explode('|', $node->{mediamosa_server_db::CONTAINERS});
    array_walk($containers, '_mediamosa_server_trim');
    $containers = array_unique($containers);

    // to fix spaces, doubles etc.
    $node->{mediamosa_server_db::CONTAINERS} = implode('|', $containers);

    // Remove all first.
    mediamosa_db::db_delete(mediamosa_server_streaming_container_db::TABLE_NAME)
      ->condition(mediamosa_server_streaming_container_db::NID, $node->nid)
      ->execute();

    foreach ($containers as $container) {
      $container = trim($container);
      mediamosa_db::db_insert(mediamosa_server_streaming_container_db::TABLE_NAME)
        ->fields(array(
          mediamosa_server_streaming_container_db::NID => $node->nid,
          mediamosa_server_streaming_container_db::CONTAINER => mediamosa_unicode::substr($container, 0, mediamosa_server_streaming_container_db::CONTAINER_LENGTH)
        ))
        ->execute();
    }
  }
}

/**
 * Implements hook_update().
 */
function mediamosa_server_update($node) {
  if ($node->is_new || !empty($node->revision)) {
    mediamosa_server_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_server_db::get_fields();

    // Copy title.
    $node->{mediamosa_server_db::SERVER_NAME} = $node->title;

    // Save tools, containers.
    _process_server_node($node);

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_server_db::TABLE_NAME, $names);
  }
}

/**
 * Implements hook_insert().
 */
function mediamosa_server_insert(&$node) {
  // Get the field names of the table.
  $names = mediamosa_server_db::get_fields();

  // Copy title.
  $node->{mediamosa_server_db::SERVER_NAME} = $node->title;

  // Save tools, containers.
  _process_server_node($node);

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_server_db::TABLE_NAME, $names);
}

/**
 * Implements hook_delete().
 */
function mediamosa_server_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete_ex($node, mediamosa_server_db::TABLE_NAME);

  // Remove streaming containers.
  db_delete(mediamosa_server_streaming_container_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();

  // Remove tools.
  db_delete(mediamosa_server_tool_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();
}

/**
 * Implements hook_node_revision_delete().
 */
function mediamosa_server_node_revision_delete($node) {
  mediamosa_node_revision::delete_revision($node);
}

/**
 * Implements hook_view().
 */
function mediamosa_server_view($node, $view_mode = 'full') {
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Server listing'), 'admin/mediamosa/config/server');
  drupal_set_breadcrumb($breadcrumb);

  // Default parameters
  $rows = array(
    array(t('Name'), check_plain($node->title)),
    array(t('Status'), ($node->server_status) ? t('On') : t('Off')),
    array(t('Server type'), $node->server_type),
  );

  // The fields we want to show.
  $fields = array(
    mediamosa_server_db::SERVER_URI => t('URI / URL'),
    mediamosa_server_db::SERVER_PATH => t('Path'),
    mediamosa_server_db::URI_UPLOAD_PROGRESS => t('URI upload progress (client)'),
    mediamosa_server_db::CONTAINERS => t('Containers'),
    mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE => t('URI upload progress server type'),
    mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER => t('URI upload progress server'),
    mediamosa_server_db::SLOTS => t('Slots'),
    mediamosa_server_db::TOOLS => t('Tools'),
    mediamosa_server_db::OBJECT_CODE => t('Object Code'),
  );

  switch ($node->{mediamosa_server_db::SERVER_TYPE}) {
    case mediamosa_server_db::SERVER_TYPE_DOWNLOAD:
    case mediamosa_server_db::SERVER_TYPE_STILL:
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS]);
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE]);
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER]);
      unset($fields[mediamosa_server_db::CONTAINERS]);
      unset($fields[mediamosa_server_db::OBJECT_CODE]);
      unset($fields[mediamosa_server_db::SLOTS]);
      unset($fields[mediamosa_server_db::TOOLS]);
      break;
    case mediamosa_server_db::SERVER_TYPE_STREAMING:
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS]);
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE]);
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER]);
      unset($fields[mediamosa_server_db::SLOTS]);
      unset($fields[mediamosa_server_db::TOOLS]);
      break;
    case mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR:
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS]);
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE]);
      unset($fields[mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER]);
      unset($fields[mediamosa_server_db::CONTAINERS]);
      unset($fields[mediamosa_server_db::OBJECT_CODE]);
      break;
    case mediamosa_server_db::SERVER_TYPE_UPLOAD:
      unset($fields[mediamosa_server_db::CONTAINERS]);
      unset($fields[mediamosa_server_db::OBJECT_CODE]);
      unset($fields[mediamosa_server_db::SLOTS]);
      unset($fields[mediamosa_server_db::TOOLS]);
      break;
    default:
      assert(0); // so we know.
  }

  foreach ($fields as $key => $title) {
    if (isset($node->{$key}) && !is_null($node->{$key})) {
      if ($key == mediamosa_server_db::OBJECT_CODE) {
        $rows[] = array($title, '<pre>' . check_plain($node->{$key}) . '</pre>');
      }
      else {
        $rows[] = array($title, check_plain($node->{$key}));
      }
    }
  }

  // Add timestamps.
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['app'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implements hook_form().
 */
function mediamosa_server_form($node, &$form_state) {
  // Take type from node if available.
  if (isset($node->{mediamosa_server_db::SERVER_TYPE})) {
    $server_type = $node->{mediamosa_server_db::SERVER_TYPE};
  }

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  $breadcrumb[] = l(t('Server listing'), 'admin/mediamosa/config/server');
  drupal_set_breadcrumb($breadcrumb);

  // Else take it from the uri.
  if (!isset($server_type)) {
    $server_type = arg(5);
  }

  $is_insert = empty($node->nid);

  // Default server status options.
  $server_status_options =  array(
    mediamosa_server_db::SERVER_STATUS_ON => t('On'),
    mediamosa_server_db::SERVER_STATUS_OFF => t('Off')
  );

  switch ($server_type) {
    default:
      drupal_goto('admin/mediamosa/config/server/add/download');

    case mediamosa_server_db::SERVER_TYPE_DOWNLOAD:
      $title = $is_insert ? t('New download server') : t("Editing download server '@server_name'.", array('@server_name' => $node->title));
      $description = t('A download server handles download requests of mediafiles.');
      break;

    case mediamosa_server_db::SERVER_TYPE_STILL:
      $title = $is_insert ? t('New still server') : t("Editing still server '@server_name'.", array('@server_name' => $node->title));
      $description = t('A still server serves and generates stills from mediafiles.');
      break;

    case mediamosa_server_db::SERVER_TYPE_STREAMING:
      $title = $is_insert ? t('New streaming server') : t("Editing streaming server '@server_name'.", array('@server_name' => $node->title));
      $description = t('A streaming server will stream mediafiles for you.');
      break;

    case mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR:
      // If its already OFF then the value is allowed as option, else its either On or Close.
      if (isset($node->{mediamosa_server_db::SERVER_STATUS}) && $node->{mediamosa_server_db::SERVER_STATUS} != mediamosa_server_db::SERVER_STATUS_OFF) {
        $server_status_options =  array(
          mediamosa_server_db::SERVER_STATUS_ON => t('On'),
          mediamosa_server_db::SERVER_STATUS_CLOSE => t('Close'), // Job servers are set to off by cron.
        );
      }

      $title = $is_insert ? t('New processor server') : t("Editing processor server '@server_name'.", array('@server_name' => $node->title));
      $description = t('A processor server processes jobs like still generation, transcoding of mediafiles.');
      break;

    case mediamosa_server_db::SERVER_TYPE_UPLOAD:
      $title = $is_insert ? t('New upload server') : t("Editing upload server '@server_name'.", array('@server_name' => $node->title));
      $description = t('A upload server allows users to upload files to MediaMosa.');
      break;
  }

  $form = array();

  $form[mediamosa_server_db::SERVER_TYPE] = array(
    '#type' => 'hidden',
    '#value' => mediamosa_node::default_value(mediamosa_server_db::SERVER_TYPE, $node, $server_type),
  );

  $form['mediamosa_server'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => $title,
    '#description' => $description,
  );

  $form['mediamosa_server']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Server name'),
    '#description' => t('The name for the server.'),
    '#required' => TRUE,
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_server_db::SERVER_NAME_LENGTH,
  );

  $form['mediamosa_server'][mediamosa_server_db::DESCRIPTION] = array(
    '#type' => 'textarea',
    '#required' => TRUE,
    '#title' => t('Description'),
    '#description' => t('Description of the server.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_server_db::DESCRIPTION, $node),
    '#maxlength' => mediamosa_server_db::DESCRIPTION_LENGTH,
  );

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_DOWNLOAD, mediamosa_server_db::SERVER_TYPE_UPLOAD, mediamosa_server_db::SERVER_TYPE_STILL, mediamosa_server_db::SERVER_TYPE_STREAMING))) {
    $form['mediamosa_server'][mediamosa_server_db::SERVER_URI] = array(
      '#type' => 'textfield',
      '#required' => TRUE,
      '#title' => t('URI'),
      '#description' => t('E.g. http://upload.mediamosa.local/'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SERVER_URI, $node),
      '#maxlength' => mediamosa_server_db::SERVER_URI_LENGTH,
    );

    $form['mediamosa_server'][mediamosa_server_db::SERVER_PATH] = array(
      '#type' => 'textfield',
      '#required' => TRUE,
      '#title' => t('Path'),
      '#description' => t('{base_uri} is the placeholder of base URI. {TICKET} is available for use in the URI. Add id={PROGRESS_ID} for automaticly assigning the upload ID to the new upload. The {PROGRESS_ID} value will be unqiue and generated by MediaMosa. E.g. {base_uri}mediafile/upload?upload_ticket={TICKET}&X-Progress-ID={PROGRESS_ID}'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SERVER_PATH, $node),
      '#maxlength' => mediamosa_server_db::SERVER_PATH_LENGTH,
    );

  }

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR))) {
    $form['mediamosa_server'][mediamosa_server_db::SERVER_URI] = array(
      '#type' => 'textfield',
      '#title' => t('Base URI'),
      '#description' => t('The base of the URI or URL of the server.'),
      '#required' => TRUE,
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SERVER_URI, $node),
      '#maxlength' => mediamosa_server_db::SERVER_URI_LENGTH,
    );

    $form['mediamosa_server'][mediamosa_server_db::SERVER_PATH] = array(
      '#type' => 'textfield',
      '#required' => TRUE,
      '#title' => t('Path (main)'),
      '#description' => t('{base_uri} is the placeholder of base URI. {TICKET} is available for use in the URI. Add id={PROGRESS_ID} for automaticly assigning the upload ID to the new upload. The {PROGRESS_ID} value will be unqiue and generated by MediaMosa. E.g. {base_uri}mediafile/upload?upload_ticket={TICKET}&X-Progress-ID={PROGRESS_ID}'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SERVER_PATH, $node),
      '#maxlength' => mediamosa_server_db::SERVER_PATH_LENGTH,
    );

    $form['mediamosa_server'][mediamosa_server_db::SLOTS] = array(
      '#type' => 'textfield',
      '#title' => t('Slots'),
      '#required' => TRUE,
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SLOTS, $node),
    );

    $form['mediamosa_server'][mediamosa_server_db::TOOLS] = array(
      '#type' => 'textfield',
      '#title' => t('Tools'),
      '#description' => t('Set tools for this server in the following format: ffmpeg|STILL|lin2win.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::TOOLS, $node),
    );
  }

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_UPLOAD))) {
    // Check if APC is loaded.
    $apc_detected = extension_loaded('apc');

    // Get a server upload.
    $server_upload = mediamosa_server::get_random_upload();

    $form['mediamosa_server'][mediamosa_server_db::URI_UPLOAD_PROGRESS] = array(
      '#type' => 'textfield',
      '#title' => t('URI upload progress'),
      '#required' => TRUE,
      '#description' => t('Optional URI for client side upload progress information. This URL should match the REST call /mediafile/uploadprogress, e.g. http://upload.mediamosa.local/uploadprogress?id={PROGRESS_ID}. When using multi upload servers, use the server_id={SERVER_ID} as extra option. Upload progress will not work when using multiple upload servers with load balancers, its impossible to know which upload server is chosen by the load balancer.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::URI_UPLOAD_PROGRESS, $node),
      '#maxlength' => mediamosa_server_db::URI_UPLOAD_PROGRESS_LENGTH,
    );

    $options = array(
      'apc' => t('Use APC extension (@detected)', array('@detected' => $apc_detected ? t('APC extension has been detected on this server') : t('APC extension has not been detected on this server'))),
      'proxy' => t('Use nginx / lighttpd upload progress'),
    );

    $form['mediamosa_server'][mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE] = array(
      '#type' => 'select',
      '#options' => $options,
      '#title' => t('Choose upload progress for mediafiles.'),
      '#description' => t(' The APC upload progress requires the APC extension. The nginx and lighttpd extensions require the URL setting below to indentify the progress proxy URL. This URL is used by MediaMosa to pull upload progress from and is returned by the mediamosa/uploadprogress REST call.'),
      '#required' => FALSE,
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE, $node),
    );

    $parse_url = mediamosa_http::parse_url($server_upload[mediamosa_server_db::SERVER_URI]);
    $form['mediamosa_server'][mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER] = array(
      '#type' => 'textfield',
      '#title' => t('Supply the url to query the progress on nginx / lighttpd'),
      '#description' => t("This url is only required for nginx / lighttpd upload progress. By default this should be the upload server URL plus '/progress?X-Progress-ID={PROGRESS_ID}' and will be queried by MediaMosa on the default upload server. (example; http://@host/progress?X-Progress-ID={PROGRESS_ID}). You should define the URL for progress in your httpd server configuration and make sure the URL is used before it handles the MediaMosa URL.", array('@host' => $parse_url['host'])),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER, $node),
      '#required' => FALSE,
    );
  }

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_STREAMING))) {
    $form['mediamosa_server'][mediamosa_server_db::CONTAINERS] = array(
      '#type' => 'textfield',
      '#title' => t('Containers'),
      '#description' => t('Set containers for this server in the following format: asf|flv|mpeg|avi|ogg|mov;mp4;m4a;3gp;3g2;mj2|mp3. Containers or mime types must be set.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::CONTAINERS, $node),
      '#maxlength' => mediamosa_server_db::CONTAINERS_LENGTH,
    );

    $form['mediamosa_server'][mediamosa_server_db::MIME_TYPE] = array(
      '#type' => 'textfield',
      '#title' => t('Mime types'),
      '#description' => t('Set mime types for this server in the following format: application/pdf|image/*. Containers or mime types must be set.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::MIME_TYPE, $node),
      '#maxlength' => mediamosa_server_db::MIME_TYPE_LENGTH,
    );

    $form['mediamosa_server'][mediamosa_server_db::OBJECT_CODE] = array(
      '#type' => 'textarea',
      '#title' => t('Object code'),
      '#description' => t('{WIDTH}, {HEIGHT}, {HEIGHT_PLUS_number}, {MEDIAFILE_ID}, {TICKET_URI}, {STILL_URI}, {AUTOPLAY} (true or false), {AUTOPLAY_NUM} (1 or 0), {AUTOPLAY_TEXT} (autostart), {IF_START}, {/IF_START}, {IF_EXTERNAL}, {/IF_EXTERNAL}, {IF_DURATION}, {/IF_DURATION} are available for use in this field.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::OBJECT_CODE, $node),
    );
  }

  $form['mediamosa_server'][mediamosa_server_db::SERVER_STATUS] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#options' => $server_status_options,
    '#title' => t('Status'),
    '#description' => t('Availability of server.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SERVER_STATUS, $node),
  );

  $form['#redirect'] = 'admin/mediamosa/config/server';

  return $form;
}

/**
 * Implements hook_validate().
 */
function mediamosa_server_validate($node) {
  // Take type from node if available.
  if (isset($node->{mediamosa_server_db::SERVER_TYPE})) {
    $server_type = $node->{mediamosa_server_db::SERVER_TYPE};
  }

  // Else take it from the uri.
  if (!isset($server_type)) {
    $server_type = arg(5);
  }

  // Containers and mime type.
  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_STREAMING))) {
    if (!$node->{mediamosa_server_db::CONTAINERS} && !$node->{mediamosa_server_db::MIME_TYPE}) {
      form_set_error(mediamosa_server_db::CONTAINERS, t('Containers or mime types must be set.'));
    }
  }
}
