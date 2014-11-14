<?php
/**
 * @file
 * Installer for mediamosa_server_streaming_container module.
 */

require_once 'mediamosa_server_streaming_container_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_streaming_container_install() {

  // Download server.
  mediamosa::drupal_install_schema(mediamosa_server_streaming_container_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_streaming_container_uninstall() {

  // Download server.
  mediamosa::drupal_uninstall_schema(mediamosa_server_streaming_container_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_streaming_container_schema() {

  $schema[mediamosa_server_streaming_container_db::TABLE_NAME] = array(
    'description' => 'The streaming server container table.',
    'fields' => array(
      mediamosa_server_streaming_container_db::ID => array(
        'description' => 'Primary key.',
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_container_db::NID => array(
        'description' => 'The parent server node ID.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_container_db::CONTAINER => array(
        'description' => 'The container.',
        'type' => 'varchar',
        'length' => mediamosa_server_streaming_container_db::CONTAINER_LENGTH,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_streaming_container_db::ID),
  );

  return $schema;
}
