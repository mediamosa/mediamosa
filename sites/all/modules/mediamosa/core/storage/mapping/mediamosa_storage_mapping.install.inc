<?php
// $Id$

/**
 * @file
 *
 */
require_once 'mediamosa_storage_mapping_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_storage_mapping_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_storage_mapping_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_mapping_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_storage_mapping_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_storage_mapping_schema() {

  $schema[mediamosa_storage_mapping_db::TABLE_NAME] = array(
  'description' => 'The storage mapping table.',
    'fields' => array(
      mediamosa_storage_mapping_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_storage_mapping_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_storage_mapping_db::PROFILE_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The storage profile ID.',
      ),
      mediamosa_storage_mapping_db::PATH => array(
        'type' => 'varchar',
        'length' => mediamosa_storage_mapping_db::PATH_LENGTH,
        'description' => 'Storage path.',
      ),
      mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING => array(
        'description' => 'Indicates if the mapping is the default mapping.',
        'type' => "enum('" . implode("','", array(mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE, mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE, mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_TRUE)) . "')",
        'default' => mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE,
      ),
      mediamosa_storage_mapping_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when storage mapping was created.',
      ),
      mediamosa_storage_mapping_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when storage mapping was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_storage_mapping_db::ID),
    'unique keys' => array(
      'uni_appid_profile' => array(mediamosa_storage_mapping_db::APP_ID, mediamosa_storage_mapping_db::PROFILE_ID),
    ),
    'indexes' => array(
      'idx_app_isdefault' => array(mediamosa_storage_mapping_db::APP_ID, mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING),
    ),
  );

  return $schema;
}
