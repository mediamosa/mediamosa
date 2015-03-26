<?php
/**
 * @file
 * Include file for core node types. Add new node type includes here.
 * This file is included on several places to ensure including all node
 * creation code.
 */

/**
 * These are required, or else some hooks will not work for these node types.
 */
require_once 'mediamosa_node_mediamosa_app.class.inc';
require_once 'mediamosa_node_mediamosa_server.inc';
require_once 'mediamosa_node_mediamosa_viewer.inc';
require_once 'mediamosa_node_mediamosa_settings.class.inc';
require_once 'mediamosa_node_mediamosa_transcode_profile.inc';
require_once 'mediamosa_node_mediamosa_storage_profile.inc';
require_once 'mediamosa_node_mediamosa_tool_params.inc';
require_once 'mediamosa_node_mediamosa_tool_mapping.inc';

class mediamosa_node {
  // ------------------------------------------------------------------ Consts.
  // Node types.
  const MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE = 'mediamosa_storage_profile';
  const MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE = 'mediamosa_transcode_profile';
  const MEDIAMOSA_NODE_TYPE_TOOL_PARAMS = 'mediamosa_tool_params';
  const MEDIAMOSA_NODE_TYPE_TOOL_MAPPING = 'mediamosa_tool_mapping';
  const MEDIAMOSA_NODE_TYPE_SERVER = 'mediamosa_server';
  const MEDIAMOSA_NODE_TYPE_VIEWER = 'mediamosa_viewer';
  const MEDIAMOSA_NODE_TYPE_APP = 'mediamosa_app';
  const MEDIAMOSA_NODE_TYPE_SETTINGS = 'mediamosa_settings';

  // ------------------------------------------------------------------ Functions.
  /**
   * Get all node type of mediamosa.
   */
  public static function get_all_node_types() {
    return array_keys(mediamosa_node_info());
  }

  /**
   * Create basic MediaMosa node object to fill.
   *
   * @param string $type
   *   The node type.
   * @param string $title
   *   The title.
   * @param string $description
   *   (optional) The node description.
   *
   * @return object
   *   A node type object.
   */
  public static function create_basic_node($type, $title, $description = '') {

    $node = new stdClass();
    // Admin.
    $node->uid = 1;
    // Published.
    $node->status = 1;
    $node->type = $type;
    $node->locked = 0;
    $node->has_title = 1;
    $node->has_body = 0;
    $node->custom = 1;
    $node->language = LANGUAGE_NONE;
    $node->title = $title;
    $node->description = $description;
    return $node;
  }

  /**
   * Changes the form for mediamosa node type.
   *
   * @param array $form
   * @param array $form_state
   */
  public static function common_form(&$form, &$form_state) {
    $form['revision_information']['revision']['#default_value'] = '1';
    $form['revision_information']['log']['#default_value'] = ' ';

    // Remove the options block.
    unset($form['options']);

    // Add hidden values.
    $form['status'] = array(
      '#type' => 'hidden',
      '#value' => 1,
    );

    $form['sticky'] = array(
      '#type' => 'hidden',
      '#value' => 0,
    );

    $form['promote'] = array(
      '#type' => 'hidden',
      '#value' => 0,
    );

    // Make revision top item.
    $form['revision_information']['#weight'] = -100;

    // Remove the preview button for now (it doesn't work well atm).
    unset($form['actions']['preview']);
  }

  /**
   * Helper setting the default value.
   *
   * @param string $name
   * @param object $node
   */
  public static function default_value($name, $node, $default = '') {
    return isset($node->{$name}) ? $node->{$name} : $default;
  }

  /**
   * returns the field names used in this node type.
   *
   * @param $node
   */
  public static function get_fields($node) {

    switch ($node->type) {
      case mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE:
        $fields = mediamosa_storage_profile_db::get_fields();
        return $fields;

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE:
        $fields = mediamosa_transcode_profile_db::get_fields();
        $fields[] = 'commands';
        return $fields;

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS:
        $fields = mediamosa_tool_params_db::get_fields();
        return $fields;

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_MAPPING:
        $fields = mediamosa_tool_mapping_db::get_fields();
        return $fields;

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER:
        $fields = mediamosa_server_db::get_fields();

        switch ($node->{mediamosa_server_db::SERVER_TYPE}) {
          default:
            assert(0); // so we know.
          case mediamosa_server_db::SERVER_TYPE_DOWNLOAD:
          case mediamosa_server_db::SERVER_TYPE_STILL:
            $to_remove = array(
              mediamosa_server_db::URI_UPLOAD_PROGRESS,
              mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER,
              mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE,
              mediamosa_server_db::CONTAINERS,
              mediamosa_server_db::OBJECT_CODE,
              mediamosa_server_db::SLOTS,
              mediamosa_server_db::TOOLS
            );
            break;
          case mediamosa_server_db::SERVER_TYPE_STREAMING:
            $to_remove = array(
              mediamosa_server_db::URI_UPLOAD_PROGRESS,
              mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER,
              mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE,
              mediamosa_server_db::SLOTS,
              mediamosa_server_db::TOOLS,
            );
            break;
          case mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR:
            $to_remove = array(
              mediamosa_server_db::URI_UPLOAD_PROGRESS,
              mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER,
              mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE,
              mediamosa_server_db::CONTAINERS,
              mediamosa_server_db::OBJECT_CODE,
            );
            break;
          case mediamosa_server_db::SERVER_TYPE_UPLOAD:
            $to_remove = array(
              mediamosa_server_db::CONTAINERS,
              mediamosa_server_db::OBJECT_CODE,
              mediamosa_server_db::SLOTS,
              mediamosa_server_db::TOOLS,
            );
            break;
        }

        // Remove unused fields.
        foreach ($to_remove as $remove) {
          $key = array_search($remove, $fields);
          if ($key !== FALSE) {
            unset($fields[$key]);
          }
        }

        return $fields;

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_APP:
        return mediamosa_app_db::get_fields();

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS:
        return mediamosa_node_mediamosa_settings::get_fields();

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE:
        return mediamosa_storage_profile_db::get_fields();

      case mediamosa_node::MEDIAMOSA_NODE_TYPE_VIEWER:
        $fields = mediamosa_viewer_db::get_fields();
        return $fields;
    }

    return array();
  }
}

/**
 * Implements hook_node_info().
 */
function mediamosa_node_info() {
  return array(
    mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE => array(
      'name' => t('MediaMosa storage profile'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE,
      'has_body' => FALSE,
      'description' => t('The storage profile contains information about storage for media files'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE => array(
      'name' => t('MediaMosa transcode profile'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE,
      'has_body' => FALSE,
      'description' => t('The transcode profile contains information about the codec type and settings for a video transcoding.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS => array(
      'name' => t('MediaMosa tool_params'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS,
      'has_body' => FALSE,
      'description' => t('The tool params contains information about the tool (eg. ffmpeg, windows) parameters.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_MAPPING => array(
      'name' => t('MediaMosa tool_mapping'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_MAPPING,
      'has_body' => FALSE,
      'description' => t('The tool mapping contains information about how to map the tool (eg. pdf, tiff) analyse data to asset metadata.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER => array(
      'name' => t('MediaMosa server'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER,
      'has_body' => FALSE,
      'description' => t('MediaMosa servers execute jobs for transcoding, uploading and other related work.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_APP => array(
      'name' => t('MediaMosa client application'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_APP,
      'has_body' => FALSE,
      'description' => t('MediaMosa client applications are clients that can login onto MediaMosa and execute REST calls. All data stored is stored under its client application ID; giving each client application its own database of assets, mediafiles etc.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_VIEWER => array(
      'name' => t('MediaMosa viewer'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_VIEWER,
      'has_body' => FALSE,
      'description' => t('MediaMosa viewer object allow client application to set custom or other specific players (viewers) for their content.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS => array(
      'name' => t('MediaMosa settings'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS,
      'has_body' => FALSE,
      'description' => t('MediaMosa settings node type contains a copy of the configuration settings for revision usage.'),
    ),
  );
}

/**
 * Show the node revision information.
 *
 * @param node $nid
 */
function _mediamosa_node_revision_overview($node) {

  // Set breadcrum.
  switch ($node->type) {
    case mediamosa_node::MEDIAMOSA_NODE_TYPE_APP:
      // Reset the breadcrum.
      $breadcrumb = array();
      $breadcrumb[] = l(t('Home'), NULL);
      $breadcrumb[] = l(t('Administration'), 'admin');
      $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
      $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
      $breadcrumb[] = l(t('Client applications'), 'admin/mediamosa/config/app');
      drupal_set_breadcrumb($breadcrumb);
      break;

    case mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER:
      // Reset the breadcrum.
      $breadcrumb = array();
      $breadcrumb[] = l(t('Home'), NULL);
      $breadcrumb[] = l(t('Administration'), 'admin');
      $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
      $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
      $breadcrumb[] = l(t('Server listing'), 'admin/mediamosa/config/server');
      drupal_set_breadcrumb($breadcrumb);
      break;

    case mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE:
      // Reset the breadcrum.
      $breadcrumb = array();
      $breadcrumb[] = l(t('Home'), NULL);
      $breadcrumb[] = l(t('Administration'), 'admin');
      $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
      $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
      $breadcrumb[] = l(t('Storage profiles'), 'admin/mediamosa/config/storage_profile');
      $breadcrumb[] = l($node->title, 'admin/mediamosa/config/storage_profile/' . $node->nid);
      drupal_set_breadcrumb($breadcrumb);
      break;

    case mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE:
      // Reset the breadcrum.
      $breadcrumb = array();
      $breadcrumb[] = l(t('Home'), NULL);
      $breadcrumb[] = l(t('Administration'), 'admin');
      $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
      $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
      $breadcrumb[] = l(t('Transcode profiles'), 'admin/mediamosa/config/transcode_profile');
      drupal_set_breadcrumb($breadcrumb);
      break;

    case mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS:
      // Reset the breadcrum.
      $breadcrumb = array();
      $breadcrumb[] = l(t('Home'), NULL);
      $breadcrumb[] = l(t('Administration'), 'admin');
      $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
      $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
      $breadcrumb[] = l(t('Tool parameters'), 'admin/mediamosa/config/tool_params');
      drupal_set_breadcrumb($breadcrumb);
      break;

    case mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_MAPPING:
      // Reset the breadcrum.
      $breadcrumb = array();
      $breadcrumb[] = l(t('Home'), NULL);
      $breadcrumb[] = l(t('Administration'), 'admin');
      $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
      $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
      $breadcrumb[] = l(t('Tool metadata mapping'), 'admin/mediamosa/config/tool_mapping');
      drupal_set_breadcrumb($breadcrumb);
      break;

    case mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS:
      // Reset the breadcrum.
      $breadcrumb = array();
      $breadcrumb[] = l(t('Home'), NULL);
      $breadcrumb[] = l(t('Administration'), 'admin');
      $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
      $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
      drupal_set_breadcrumb($breadcrumb);
      break;
  }

  return node_revision_overview($node);
}

function _mediamosa_node_revision_overview_globalconfig() {
  // Only one node of global config type.
  $nid = db_select('node', 'n')
    ->fields('n', array('nid'))
    ->condition('type', mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS)
    ->range(0, 1)
    ->execute()
    ->fetchField();

  // Not found, no revisions.
  if (!$nid) {
    drupal_goto('admin/mediamosa/config/global');
  }

  // Get the node.
  $node = node_load($nid);

  // Now access on it.
  return _mediamosa_node_revision_overview($node);
}

/**
 * Access check for revisions on global config.
 */
function _mediamosa_node_revision_access_globalconfig() {

  // Only one node of global config type.
  $nid = db_select('node', 'n')
    ->fields('n', array('nid'))
    ->condition('type', mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS)
    ->range(0, 1)
    ->execute()
    ->fetchField();

  if (!$nid) {
    return FALSE;
  }

  // Get the node.
  $node = node_load($nid);

  // Now access on it.
  return _node_revision_access($node);
}

/**
 * Confirm node deletion.
 */
function _mediamosa_node_delete_confirm($form, &$form_state, $node) {

  // Call the node delete confirm form.
  return node_delete_confirm($form, $form_state, $node);
}

/**
 * Execute node deletion
 */
function _mediamosa_node_delete_confirm_submit($form, &$form_state) {

  // Call the node confirm submit function.
  node_delete_confirm_submit($form, $form_state);
}

/**
 * Implements hook_form_alter().
 */
function mediamosa_form_mediamosa_storage_profile_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/storage_profile';
}

/**
 * Implements hook_form_alter().
 */
function mediamosa_form_mediamosa_transcode_profile_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/transcode_profile';
}

/**
 * Implements hook_form_alter().
 */
function mediamosa_form_mediamosa_tool_params_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/tool_params';
}

/**
 * Implements hook_form_alter().
 */
function mediamosa_form_mediamosa_tool_mapping_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/tool_mapping';
}

/**
 * Implements hook_form_alter().
 */
function mediamosa_form_mediamosa_server_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/server';
}

/**
 * Implements hook_form_alter().
 */
function mediamosa_form_mediamosa_viewer_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/viewer';
}

/**
 * Implements hook_form_alter().
 */
function mediamosa_form_mediamosa_app_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/app';
}
