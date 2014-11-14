<?php
/**
 * @file
 * Installer for Metadata type Asset
 */

// Include the Metadata class.
require_once 'mediamosa_metadata_asset.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_metadata_asset_install() {

  // Create the property group.
  $property_group_id = mediamosa_metadata_asset::property_group_create(mediamosa_metadata_asset::METADATA_PROPERTY_GROUP_NAME);

  // The properties to create.
  $properties = array(
    array(
      mediamosa_asset_metadata_property_db::NAME => mediamosa_metadata_asset::TAG,
      mediamosa_asset_metadata_property_db::TYPE => mediamosa_asset_metadata_property_db::TYPE_CHAR,
      mediamosa_asset_metadata_property_db::IS_HIDDEN => TRUE,
    ),
    array(
      mediamosa_asset_metadata_property_db::NAME => mediamosa_metadata_asset::PUBLISHED,
      mediamosa_asset_metadata_property_db::TYPE => mediamosa_asset_metadata_property_db::TYPE_CHAR,
      mediamosa_asset_metadata_property_db::IS_HIDDEN => FALSE,
      mediamosa_asset_metadata_property_db::IS_PROTECTED => TRUE,
    ),
    array(
      mediamosa_asset_metadata_property_db::NAME => mediamosa_metadata_asset::TRASHCAN,
      mediamosa_asset_metadata_property_db::TYPE => mediamosa_asset_metadata_property_db::TYPE_CHAR,
      mediamosa_asset_metadata_property_db::IS_HIDDEN => FALSE,
      mediamosa_asset_metadata_property_db::IS_PROTECTED => TRUE,
    ),
  );

  // Create the properties.
  foreach ($properties as $property) {
    mediamosa_metadata_asset::property_create(
      $property_group_id,
      $property[mediamosa_asset_metadata_property_db::NAME],
      $property[mediamosa_asset_metadata_property_db::TYPE],
      array(
        mediamosa_asset_metadata_property_db::IS_HIDDEN => empty($property[mediamosa_asset_metadata_property_db::IS_HIDDEN]) ? mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE : mediamosa_asset_metadata_property_db::IS_HIDDEN_TRUE,
        mediamosa_asset_metadata_property_db::IS_PROTECTED => empty($property[mediamosa_asset_metadata_property_db::IS_PROTECTED]) ? mediamosa_asset_metadata_property_db::IS_PROTECTED_FALSE : mediamosa_asset_metadata_property_db::IS_PROTECTED_TRUE,
      )
    );
  }
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_metadata_asset_uninstall() {
  // Remove it and delete all properties.
  mediamosa_metadata_asset::property_group_delete(mediamosa_metadata_asset::METADATA_PROPERTY_GROUP_NAME);
}
