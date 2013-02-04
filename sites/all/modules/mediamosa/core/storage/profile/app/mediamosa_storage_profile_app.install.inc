<?php
/**
 * @file
 * MediaMosa storage profile application.
 *
 * The storage profile application is the link between the client application
 * and the storage profile. It also stores specific data for this client
 * application.
 */

require_once 'mediamosa_storage_profile_app_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_storage_profile_app_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_storage_profile_app_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_profile_app_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_storage_profile_app_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_storage_profile_app_schema() {

  $schema[mediamosa_storage_profile_app_db::TABLE_NAME] = array(
  'description' => 'The storage mapping table.',
    'fields' => array(
      mediamosa_storage_profile_app_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_storage_profile_app_db::APP_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_storage_profile_app_db::PROFILE_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The storage profile ID for MediaFiles.',
      ),
      mediamosa_storage_profile_app_db::PATH => array(
        'type' => 'varchar',
        'length' => mediamosa_storage_profile_app_db::PATH_LENGTH,
        'description' => 'Storage path for MediaFiles.',
      ),
      mediamosa_storage_profile_app_db::PROFILE_ID_STILLS => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The storage profile ID for MediaFiles (stills).',
      ),
      mediamosa_storage_profile_app_db::PATH_STILLS => array(
        'type' => 'varchar',
        'length' => mediamosa_storage_profile_app_db::PATH_STILLS_LENGTH,
        'description' => 'Storage path for MediaFiles (stills).',
      ),
      mediamosa_storage_profile_app_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when storage mapping was created.',
      ),
      mediamosa_storage_profile_app_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when storage mapping was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_storage_profile_app_db::ID),
//    'unique keys' => array(
//      // Can only link to one app.
//      'uni_appid' => array(mediamosa_storage_profile_app_db::APP_ID),
//    ),
    'foreign keys' => array(
      mediamosa_storage_profile_app_db::APP_ID => array(mediamosa_app_db::TABLE_NAME => mediamosa_app_db::APP_ID),
    ),
  );

  return $schema;
}
