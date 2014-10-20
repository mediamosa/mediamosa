<?php
/**
 * @file
 * The installation file of the asset metadata group module.
 */

require_once 'mediamosa_asset_metadata_property_group.class.inc';
require_once 'mediamosa_asset_metadata_property_group_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_metadata_property_group_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_metadata_property_group_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_metadata_property_group_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_metadata_property_group_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_metadata_property_group_schema() {

  $schema[mediamosa_asset_metadata_property_group_db::TABLE_NAME] = array(
  'description' => 'The asset metadata property group table.',
    'fields' => array(
      mediamosa_asset_metadata_property_group_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_asset_metadata_property_group_db::NAME => array(
        'type' => 'varchar',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
        'length' => mediamosa_asset_metadata_property_group_db::NAME_LENGTH,
      ),
      mediamosa_asset_metadata_property_group_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when asset metadata property was created.',
      ),
      mediamosa_asset_metadata_property_group_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when asset metadata property was changed.',
      ),
    ),
    'primary key' => array(mediamosa_asset_metadata_property_group_db::ID),
    'unique keys' => array(
      'uni_propgroupname' => array(mediamosa_asset_metadata_property_group_db::NAME),
    ),
  );

  return $schema;
}
