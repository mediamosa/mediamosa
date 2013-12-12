<?php
/**
 * @file
 * Install, update, and uninstall functions for the mediamosa server module.
 */

require_once 'mediamosa_server_viewer_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_viewer_install() {
  // Install our schema.
  mediamosa::drupal_install_schema(mediamosa_server_viewer_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_viewer_uninstall() {
  // Uninstall our schema.
  mediamosa::drupal_uninstall_schema(mediamosa_server_viewer_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_viewer_schema() {

  $viewer_status = array(
    mediamosa_server_viewer_db::STATUS_ENABLED,
    mediamosa_server_viewer_db::STATUS_DISABLED,
  );

  $schema = array();
  $schema[mediamosa_server_viewer_db::TABLE_NAME] = array(
    'description' => '.',
    'fields' => array(
      mediamosa_server_viewer_db::NID_VIEWER => array(
        'description' => 'Viewer node ID.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_viewer_db::NID_SERVER => array(
        'description' => 'Server node ID.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_viewer_db::STATUS => array(
        'description' => 'The status.',
        'type' => "enum('" . implode("','", $viewer_status) . "')",
        'mysql_type' => "enum('" . implode("','", $viewer_status) . "')",
        'not null' => FALSE,
      ),
      mediamosa_server_viewer_db::WEIGHT => array(
        'type' => 'int',
        'description' => 'The weight of the viewer for usage in order.',
        'default' => 0,
      ),
    ),
    'primary key' => array(mediamosa_server_viewer_db::NID_SERVER, mediamosa_server_viewer_db::NID_VIEWER),
    'indexes' => array(
    )
  );

  return $schema;
}
