<?php
/**
 * @file
 * The install file for asset media still style.
 */

require_once 'mediamosa_asset_mediafile_still_style.class.inc';
require_once 'mediamosa_asset_mediafile_still_style_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_mediafile_still_style_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_mediafile_still_style_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_mediafile_still_style_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_mediafile_still_style_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_mediafile_still_style_schema() {

  $schema[mediamosa_asset_mediafile_still_style_db::TABLE_NAME] = array(
    'description' => 'The asset mediafile still style table.',
    'fields' => array(
      mediamosa_asset_mediafile_still_style_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_asset_mediafile_still_style_db::ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The name of image style.',
      ),
      mediamosa_asset_mediafile_still_style_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
    ),
    'primary key' => array(mediamosa_asset_mediafile_still_style_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_asset_mediafile_still_style_db::APP_ID => array(mediamosa_asset_mediafile_still_style_db::APP_ID),
    ),
  );

  return $schema;
}
