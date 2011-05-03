<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Install for statistics used diskspace.
 */

require_once 'mediamosa_statistics_stream_request_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_stream_request_install() {

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_statistics_stream_request_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_stream_request_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_statistics_stream_request_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_statistics_stream_request_schema() {

  $schema[mediamosa_statistics_stream_request_db::TABLE_NAME] = array(
  'description' => 'The statistics stream request table.',
    'fields' => array(
      mediamosa_statistics_stream_request_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_statistics_stream_request_db::TID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The TID (mediamosa_streaming_ticket primary key).',
      ),
      mediamosa_statistics_stream_request_db::TIME => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time, when the streaming request happened.',
      ),
      mediamosa_statistics_stream_request_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_statistics_stream_request_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The parent mediafile ID.',
      ),
      mediamosa_statistics_stream_request_db::DURATION => array(
        'type' => 'int',
        'size' => 'big',
        'not null' => FALSE,
        'unsigned' => TRUE,
        'description' => 'Length of time that a client played content before a client event.',
      ),
      mediamosa_statistics_stream_request_db::BYTES => array(
        'type' => 'int',
        'size' => 'big',
        'not null' => FALSE,
        'unsigned' => TRUE,
        'description' => 'Bytes sent by the server to the client.',
      ),
    ),
    'primary key' => array(mediamosa_statistics_stream_request_db::ID),
    'unique keys' => array(
      'uni_' . mediamosa_statistics_stream_request_db::TID . '_' . mediamosa_statistics_stream_request_db::TIME => array(
        mediamosa_statistics_stream_request_db::TID,
        mediamosa_statistics_stream_request_db::TIME,
      ),
    ),
    'indexes' => array(
      'idx_tid'  => array(mediamosa_statistics_stream_request_db::TID),
      'idx_app_id'  => array(mediamosa_statistics_stream_request_db::APP_ID),
      'idx_mediafile_id'  => array(mediamosa_statistics_stream_request_db::MEDIAFILE_ID),
    ),
  );

  return $schema;
}
