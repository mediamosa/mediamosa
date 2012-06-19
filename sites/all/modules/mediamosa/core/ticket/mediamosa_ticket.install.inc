<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

 /**
  * @file
  * The ticket DB class.
  */

require_once 'mediamosa_ticket.class.inc';
require_once 'mediamosa_ticket_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_ticket_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_ticket_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_ticket_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_ticket_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_ticket_schema() {

  $schema[mediamosa_ticket_db::TABLE_NAME] = array(
  'description' => 'The ticket table enables temporary events for allowing user actions.',
    'fields' => array(
      mediamosa_ticket_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_ticket_db::TICKET_TYPE => array(
        'description' => 'The ticket type.',
        'type' => "enum('" . implode("','", array(mediamosa_ticket_db::TICKET_TYPE_PLAY, mediamosa_ticket_db::TICKET_TYPE_DOWNLOAD, mediamosa_ticket_db::TICKET_TYPE_STILL, mediamosa_ticket_db::TICKET_TYPE_UPLOAD)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_ticket_db::TICKET_TYPE_PLAY, mediamosa_ticket_db::TICKET_TYPE_DOWNLOAD, mediamosa_ticket_db::TICKET_TYPE_STILL, mediamosa_ticket_db::TICKET_TYPE_UPLOAD)) . "')",
        'default' => mediamosa_ticket_db::TICKET_TYPE_PLAY,
      ),
      mediamosa_ticket_db::ISSUED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when media was issued.',
      ),
      mediamosa_ticket_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
      mediamosa_ticket_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_ticket_db::OWNER_ID_LENGTH,
        'default' => NULL,
        'description' => 'The owner ID.',
      ),
      mediamosa_ticket_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_ticket_db::GROUP_ID_LENGTH,
        'default' => NULL,
        'description' => 'The group ID.',
      ),
      mediamosa_ticket_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The mediafile ID.',
      ),
      mediamosa_ticket_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when media was created.',
      ),
      mediamosa_ticket_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when media was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_ticket_db::ID),
    'indexes' => array(
      'idx_type_issued' => array(mediamosa_ticket_db::TICKET_TYPE, mediamosa_ticket_db::ISSUED),
    ),
  );

  return $schema;
}

