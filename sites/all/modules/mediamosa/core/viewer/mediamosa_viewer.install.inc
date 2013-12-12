<?php
/**
 * @file
 * Install, update, and uninstall functions for the mediamosa server module.
 */

require_once 'mediamosa_viewer_db.class.inc';
require_once 'mediamosa_viewer_app_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_viewer_install() {
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_viewer_uninstall() {
}

/**
 * Implements hook_schema().
 */
function mediamosa_viewer_schema() {

  $schema = array();
  $schema[mediamosa_viewer_db::TABLE_NAME] = array(
    'description' => 'The viewer table contains viewer specific settings.',
    'fields' => array(
      mediamosa_viewer_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_viewer_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_viewer_db::NAME => array(
        'description' => 'The unique name of the viewer.',
        'type' => 'varchar',
        'length' => mediamosa_viewer_db::NAME_LENGTH,
        'not null' => FALSE,
        'default' => '',
      ),
      mediamosa_viewer_db::DESCRIPTION => array(
        'description' => 'The description of the viewer.',
        'type' => 'varchar',
        'length' => mediamosa_viewer_db::DESCRIPTION_LENGTH,
        'not null' => FALSE,
        'default' => '',
      ),
      mediamosa_viewer_db::OBJECT_CODE => array(
        'description' => 'The object code for the viewer for generating the viewer code (in HTML or other markup language).',
        'type' => 'text',
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_viewer_db::NID),
    'indexes' => array(
      'idx_nid_vid' => array(mediamosa_viewer_db::NID, mediamosa_viewer_db::VID),
    ),
    'unique keys' => array(
      'uni_name' => array(mediamosa_viewer_db::NAME),
    ),
  );

  $schema[mediamosa_viewer_app_db::TABLE_NAME] = array(
    'description' => 'The viewer app links the choices of the app which viewers it will use.',
    'fields' => array(
      mediamosa_viewer_app_db::APP_ID => array(
        'description' => 'The client application ID.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_viewer_app_db::NID_SERVER => array(
        'description' => 'The view (streaming) server node ID.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_viewer_app_db::NID_VIEWER => array(
        'description' => 'The viewer ID.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_viewer_app_db::APP_ID, mediamosa_viewer_app_db::NID_SERVER, mediamosa_viewer_app_db::NID_VIEWER),
  );

  return $schema;
}
