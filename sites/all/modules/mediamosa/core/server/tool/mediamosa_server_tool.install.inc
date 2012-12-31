<?php
/**
 * @file
 * Installer for mediamosa_server_transcoding module.
 */

require_once 'mediamosa_server_tool_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_tool_install() {

  // Download server.
  mediamosa::drupal_install_schema(mediamosa_server_tool_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_tool_uninstall() {

  // Download server.
  mediamosa::drupal_uninstall_schema(mediamosa_server_tool_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_tool_schema() {

  $schema[mediamosa_server_tool_db::TABLE_NAME] = array(
    'description' => 'Contains the tools for server subtable.',
    'fields' => array(
      mediamosa_server_tool_db::ID => array(
        'description' => 'Primary key.',
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_server_tool_db::NID => array(
        'description' => 'The parent server node ID.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_server_tool_db::TOOL => array(
        'description' => 'The tool used for job process.',
        'type' => 'varchar',
        'length' => mediamosa_server_tool_db::TOOL_LENGTH,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_tool_db::ID),
    'indexes' => array(
      'idx_tool_serverid' => array(mediamosa_server_tool_db::TOOL, mediamosa_server_tool_db::NID),
    ),
  );

  return $schema;
}
