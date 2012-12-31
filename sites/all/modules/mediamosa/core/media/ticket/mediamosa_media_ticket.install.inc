<?php
/**
 * @file
 * Install, update, and uninstall functions for the MediaMosa module.
 */

require_once 'mediamosa_media_ticket.class.inc';
require_once 'mediamosa_media_ticket_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_media_ticket_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_media_ticket_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_media_ticket_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_media_ticket_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_media_ticket_schema() {

  $schema[mediamosa_media_ticket_db::TABLE_NAME] = array(
  'description' => 'The ticket table enables temporary events for allowing user actions.',
    'fields' => array(
      mediamosa_media_ticket_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_media_ticket_db::TICKET_TYPE => array(
        'description' => 'The ticket type.',
        'type' => "enum('" . implode("','", array(mediamosa_media_ticket_db::TICKET_TYPE_VIEW, mediamosa_media_ticket_db::TICKET_TYPE_DOWNLOAD, mediamosa_media_ticket_db::TICKET_TYPE_STILL, mediamosa_media_ticket_db::TICKET_TYPE_UPLOAD)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_media_ticket_db::TICKET_TYPE_VIEW, mediamosa_media_ticket_db::TICKET_TYPE_DOWNLOAD, mediamosa_media_ticket_db::TICKET_TYPE_STILL, mediamosa_media_ticket_db::TICKET_TYPE_UPLOAD)) . "')",
        'default' => mediamosa_media_ticket_db::TICKET_TYPE_VIEW,
      ),
      mediamosa_media_ticket_db::ISSUED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when media was issued.',
      ),
      mediamosa_media_ticket_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
      mediamosa_media_ticket_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_media_ticket_db::OWNER_ID_LENGTH,
        'default' => NULL,
        'description' => 'The owner ID.',
      ),
      mediamosa_media_ticket_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_media_ticket_db::GROUP_ID_LENGTH,
        'default' => NULL,
        'description' => 'The group ID.',
      ),
      mediamosa_media_ticket_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The mediafile ID.',
      ),
      mediamosa_media_ticket_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when media was created.',
      ),
      mediamosa_media_ticket_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when media was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_media_ticket_db::ID),
    'indexes' => array(
      'idx_type_issued' => array(mediamosa_media_ticket_db::TICKET_TYPE, mediamosa_media_ticket_db::ISSUED),
    ),
  );

  return $schema;
}

