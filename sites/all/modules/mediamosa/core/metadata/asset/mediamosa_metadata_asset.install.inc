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
      'name' => 'tag',
      'type' => mediamosa_asset_metadata_property_db::TYPE_CHAR,
      'is_hidden' => TRUE,
    ),
  );

  // Create the properties.
  foreach ($properties as $property) {
    mediamosa_metadata_asset::property_create(
      $property_group_id, $property['name'],
      $property['type'],
      array('is_hidden' => empty($property['is_hidden']) ? FALSE : $property['is_hidden'])
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
