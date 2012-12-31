<?php
/**
 * @file
 * The storage profile.
 */
require_once 'mediamosa_storage_transition_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_storage_transition_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_storage_transition_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_transition_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_storage_transition_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_storage_transition_schema() {

  $schema[mediamosa_storage_transition_db::TABLE_NAME] = array(
  'description' => 'The storage profile table.',
    'fields' => array(
      mediamosa_storage_transition_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The mediafile ID of the file in transition.',
      ),
      mediamosa_storage_transition_db::MD5 => array(
        'type' => 'varbinary',
        'length' => mediamosa_storage_transition_db::MD5_LENGTH,
        'not null' => TRUE,
        'description' => 'The MD5 hash of the file in transition.',
      ),
      mediamosa_storage_transition_db::FILESIZE => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The filesize of the file in transition.',
      ),
      mediamosa_storage_transition_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when storage transition was created.',
      ),
      mediamosa_storage_transition_db::ACCESSED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when storage transition was accessed.',
      ),
    ),
    'primary key' => array(mediamosa_storage_transition_db::MEDIAFILE_ID, mediamosa_storage_transition_db::MD5),
    'indexes' => array(
      'idx_fs' => array(mediamosa_storage_transition_db::FILESIZE),
    ),
  );

  return $schema;
}
