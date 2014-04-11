<?php
/**
 * @file
 * The installation file of the asset module.
 */

require_once 'mediamosa_asset.class.inc';
require_once 'mediamosa_asset_db.class.inc';

require_once 'collection/mediamosa_asset_collection.install.inc';
require_once 'delete/mediamosa_asset_delete.install.inc';
require_once 'mediafile/mediamosa_asset_mediafile.install.inc';
require_once 'metadata/mediamosa_asset_metadata.install.inc';
require_once 'supplement/mediamosa_asset_supplement.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_install() {

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_db::TABLE_NAME);

  // Call installer asset_mediafile.
  _mediamosa_asset_mediafile_install();

  // Call installer asset_metadata.
  _mediamosa_asset_metadata_install();

  // Call installer asset_collection.
  _mediamosa_asset_collection_install();

  // Call installer asset_delete.
  _mediamosa_asset_delete_install();

  // Call installer asset_supplement.
  _mediamosa_asset_supplement_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_uninstall() {
  // Call uninstaller asset_supplement.
  _mediamosa_asset_supplement_uninstall();

  // Call uninstaller asset_delete.
  _mediamosa_asset_delete_uninstall();

  // Call uninstaller asset_collection.
  _mediamosa_asset_collection_uninstall();

  // Call uninstaller asset_metadata.
  _mediamosa_asset_metadata_uninstall();

  // Call uninstaller asset_mediafile.
  _mediamosa_asset_mediafile_uninstall();

  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_schema() {

  $schema[mediamosa_asset_db::TABLE_NAME] = array(
  'description' => 'The asset table.',
    'fields' => array(
      mediamosa_asset_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_asset_db::PARENT_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => 'The parent asset ID.',
      ),
      mediamosa_asset_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
      mediamosa_asset_db::PROVIDER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::PROVIDER_ID_LENGTH,
        'default' => NULL,
        'description' => 'The provider ID.',
      ),
      mediamosa_asset_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::OWNER_ID_LENGTH,
        'default' => NULL,
        'description' => 'The owner ID.',
      ),
      mediamosa_asset_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::GROUP_ID_LENGTH,
        'default' => NULL,
        'description' => 'The group ID.',
      ),
      mediamosa_asset_db::VIDEOTIMESTAMP => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The time and date of the asset.',
      ),
      mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The modified time and date of the asset.',
      ),
      mediamosa_asset_db::PLAY_RESTRICTION_START => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The start time in an range of time to restrict.',
      ),
      mediamosa_asset_db::PLAY_RESTRICTION_END => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The end time in an range of time to restrict.',
      ),
      mediamosa_asset_db::REFERENCE_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::REFERENCE_ID_LENGTH,
        'default' => NULL,
        'description' => 'The reference ID.',
      ),
      mediamosa_asset_db::ISPRIVATE => array(
        'description' => 'The asset is private.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::ISPRIVATE_FALSE, mediamosa_asset_db::ISPRIVATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::ISPRIVATE_FALSE, mediamosa_asset_db::ISPRIVATE_TRUE)) . "')",
        'default' => mediamosa_asset_db::ISPRIVATE_FALSE,
      ),
      mediamosa_asset_db::MEDIAFILE_DURATION => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::MEDIAFILE_DURATION_LENGTH,
        'default' => NULL,
        'description' => 'The mediafile duration length.',
      ),
      mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE_LENGTH,
        'default' => NULL,
        'description' => 'The container type length.',
      ),
      mediamosa_asset_db::MEDIAFILE_MIME_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::MEDIAFILE_MIME_TYPE_LENGTH,
        'default' => NULL,
        'description' => 'The mime type normalized from original.',
      ),
      mediamosa_asset_db::MEDIAFILE_FILENAME => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::MEDIAFILE_FILENAME_LENGTH,
        'default' => NULL,
        'description' => 'The normalized filename of the original mediafile.',
      ),
      mediamosa_asset_db::ASSET_TYPE => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_db::ASSET_TYPE_LENGTH,
        'default' => NULL,
        'description' => 'Type of asset based on mime type from original mediafile.',
      ),
      mediamosa_asset_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when asset was created.',
      ),
      mediamosa_asset_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when asset was changed last.',
      ),
      mediamosa_asset_db::IS_INAPPROPRIATE => array(
        'description' => 'If the asset has unappropieate contents.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_INAPPROPRIATE_FALSE, mediamosa_asset_db::IS_INAPPROPRIATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_INAPPROPRIATE_FALSE, mediamosa_asset_db::IS_INAPPROPRIATE_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_INAPPROPRIATE_FALSE,
      ),
      mediamosa_asset_db::IS_EXTERNAL => array(
        'description' => 'If the asset has external contents.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EXTERNAL_FALSE, mediamosa_asset_db::IS_EXTERNAL_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EXTERNAL_FALSE, mediamosa_asset_db::IS_EXTERNAL_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_EXTERNAL_FALSE,
      ),
      mediamosa_asset_db::IS_EMPTY_ASSET => array(
        'description' => 'True if the asset has media.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EMPTY_ASSET_FALSE, mediamosa_asset_db::IS_EMPTY_ASSET_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_EMPTY_ASSET_FALSE, mediamosa_asset_db::IS_EMPTY_ASSET_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_EMPTY_ASSET_TRUE,
      ),
      mediamosa_asset_db::VIEWED => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
        'description' => 'Number of times the asset was viewed.',
      ),
      mediamosa_asset_db::PLAYED => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
        'description' => 'Number of times the asset was played.',
      ),
      mediamosa_asset_db::IS_PROTECTED => array(
        'description' => 'Set kind of protection on the asset.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_PROTECTED_FALSE, mediamosa_asset_db::IS_PROTECTED_USER_USERGROUP, mediamosa_asset_db::IS_PROTECTED_DOMAIN_REALM, mediamosa_asset_db::IS_PROTECTED_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_db::IS_PROTECTED_FALSE, mediamosa_asset_db::IS_PROTECTED_USER_USERGROUP, mediamosa_asset_db::IS_PROTECTED_DOMAIN_REALM, mediamosa_asset_db::IS_PROTECTED_TRUE)) . "')",
        'default' => mediamosa_asset_db::IS_PROTECTED_FALSE,
      ),
    ),
    'primary key' => array(mediamosa_asset_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_asset_db::PARENT_ID => array(mediamosa_asset_db::PARENT_ID),
      'idx_' . mediamosa_asset_db::PROVIDER_ID => array(mediamosa_asset_db::PROVIDER_ID),
      'idx_' . mediamosa_asset_db::OWNER_ID => array(mediamosa_asset_db::OWNER_ID),
      'idx_' . mediamosa_asset_db::GROUP_ID => array(mediamosa_asset_db::GROUP_ID),
      'idx_' . mediamosa_asset_db::CREATED => array(mediamosa_asset_db::CREATED),
      'idx_' . mediamosa_asset_db::CHANGED => array(mediamosa_asset_db::CHANGED),
      'idx_' . mediamosa_asset_db::IS_PROTECTED => array(mediamosa_asset_db::IS_PROTECTED),
      'idx_app_priv_unapp_emptyasset' => array( // Main search index.
        mediamosa_asset_db::APP_ID,
        mediamosa_asset_db::ISPRIVATE,
        mediamosa_asset_db::IS_INAPPROPRIATE,
        mediamosa_asset_db::IS_EMPTY_ASSET,
      ),
      'idx_' . mediamosa_asset_db::VIDEOTIMESTAMP => array(mediamosa_asset_db::VIDEOTIMESTAMP),
    ),
  );

  return $schema;
}
