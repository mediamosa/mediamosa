<?php
/**
 * @file
 * The storage profile installer.
 */
require_once 'mediamosa_storage_profile_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_storage_profile_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_storage_profile_db::TABLE_NAME);

  // Create our node type.
  $storage_profile_node_type = array(
    'type' => mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE,
    'name' => st('MediaMosa storage profile'),
    'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE,
    'description' => st('The storage profile contains general information about the storage and the storage type.'),
    'custom' => 1,
    'modified' => 1,
    'locked' => 0,
    'has_body' => 0,
  );

  $storage_profile_node_type = node_type_set_defaults($storage_profile_node_type);
  node_type_save($storage_profile_node_type);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_storage_profile_uninstall() {
  // Delete node type
  node_type_delete(mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE);

  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_storage_profile_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_storage_profile_schema() {

  $schema[mediamosa_storage_profile_db::TABLE_NAME] = array(
  'description' => 'The storage profile table.',
    'fields' => array(
      mediamosa_storage_profile_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_storage_profile_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_storage_profile_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_storage_profile_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The owning application ID.',
      ),
      mediamosa_storage_profile_db::VERSION => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'default' => NULL,
        'description' => 'Version profile.',
      ),
      mediamosa_storage_profile_db::PROFILE => array(
        'type' => 'varchar',
        'length' => mediamosa_storage_profile_db::PROFILE_LENGTH,
        'description' => 'Profile name.',
      ),
      mediamosa_storage_profile_db::TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_storage_profile_db::TYPE_LENGTH,
        'description' => 'Profile type.',
      ),
      mediamosa_storage_profile_db::IS_DEFAULT_PROFILE => array(
        'description' => 'Indicates if the profile is the default profile.',
        'type' => "enum('" . implode("','", array(mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_FALSE, mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_FALSE, mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_TRUE)) . "')",
        'default' => mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_FALSE,
      ),
      mediamosa_storage_profile_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when storage profile was created.',
      ),
      mediamosa_storage_profile_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when storage profile was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_storage_profile_db::ID),
    'unique keys' => array(
      'uni_appid_profile' => array(mediamosa_storage_profile_db::APP_ID, mediamosa_storage_profile_db::PROFILE),
      'uni_nid' => array(mediamosa_storage_profile_db::NID),
    ),
    'indexes' => array(
      'idx_app_isdefault' => array(mediamosa_storage_profile_db::APP_ID, mediamosa_storage_profile_db::IS_DEFAULT_PROFILE),
    ),
  );

  return $schema;
}
