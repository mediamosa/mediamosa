<?php
/**
 * @file
 * Installer for mediamosa_acl_app_master_slave.
 */

require_once 'mediamosa_acl_app_master_slave_db.class.inc';

/**
 * Implements hook_ininstall().
 */
function _mediamosa_acl_app_master_slave_install() {
  mediamosa::drupal_install_schema('mediamosa_acl_app_master_slave');
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_acl_app_master_slave_uninstall() {
  mediamosa::drupal_uninstall_schema('mediamosa_acl_app_master_slave');
}

/**
 * Implements hook_schema().
 */
function mediamosa_acl_app_master_slave_schema() {
  $schema[mediamosa_acl_app_master_slave_db::TABLE_NAME] = array(
    'description' => 'This table stores master/slave links to media.',
    'fields' => array(
      mediamosa_acl_app_master_slave_db::ACL_OBJECT_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_ID_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The object ID, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE => array(
        'type' => "enum('" . implode("','", array(
          mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_MEDIAFILE,
          mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_ASSET,
          mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_COLLECTION,
        )) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(
          mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_MEDIAFILE,
          mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_ASSET,
          mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_COLLECTION,
        )) . "')",
        'default' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_MEDIAFILE,
        'description' => 'The type of object, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_acl_app_master_slave_db::APP_ID_SLAVE => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The slave application ID.',
      ),
      mediamosa_acl_app_master_slave_db::APP_ID_MASTER => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The master application ID.',
      ),
    ),
    'unique keys' => array(
      'uni_objtype_objid_masterapp_slaveapp' => array(
        mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE,
        mediamosa_acl_app_master_slave_db::ACL_OBJECT_ID,
        mediamosa_acl_app_master_slave_db::APP_ID_MASTER,
        mediamosa_acl_app_master_slave_db::APP_ID_SLAVE,
      ),
    ),
    'indexes' => array(
      'idx_masterapp_slaveapp' => array(
        mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE,
        mediamosa_acl_app_master_slave_db::APP_ID_MASTER,
        mediamosa_acl_app_master_slave_db::APP_ID_SLAVE,
      ),
    ),
  );

  return $schema;
}
