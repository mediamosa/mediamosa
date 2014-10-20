<?php
/**
 * @file
 * Install, update, and uninstall functions for the asset delete module.
 */

require_once 'mediamosa_asset_delete_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_delete_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_delete_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_delete_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_delete_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_delete_schema() {

  $schema = array();
  $schema[mediamosa_asset_delete_db::TABLE_NAME] = array(
    'description' => 'The asset collection link table.',
    'fields' => array(
      mediamosa_asset_delete_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Link to asset table.',
      ),
      mediamosa_asset_delete_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
      mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The modified time and date of the asset.',
      ),
      mediamosa_asset_delete_db::COLL_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'description' => 'Link to collection table.',
        'default' => NULL,
      ),
    ),
    'indexes' => array(
      'idx_app_videotimestamp' => array(mediamosa_asset_delete_db::APP_ID, mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED),
    ),
    // Defined as unique and not as primary because drupal will undo the NULL
    // option on coll_id.
    'unique keys' => array(
      'uni_id_collid' => array(mediamosa_asset_delete_db::ID, mediamosa_asset_delete_db::COLL_ID),
    ),
  );

  return $schema;
}
