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
 * Installer for mediamosa_server_streaming_container module.
 */

require_once 'mediamosa_server_streaming_container_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_streaming_container_install() {

  // Download server.
  mediamosa::drupal_install_schema(mediamosa_server_streaming_container_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_streaming_container_uninstall() {

  // Download server.
  mediamosa::drupal_uninstall_schema(mediamosa_server_streaming_container_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_streaming_container_schema() {

  $schema[mediamosa_server_streaming_container_db::TABLE_NAME] = array(
    'description' => 'The streaming server container table.',
    'fields' => array(
      mediamosa_server_streaming_container_db::ID => array(
        'description' => 'Primary key.',
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_container_db::NID => array(
        'description' => 'The parent server node ID.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_container_db::CONTAINER => array(
        'description' => 'The container.',
        'type' => 'varchar',
        'length' => mediamosa_server_streaming_container_db::CONTAINER_LENGTH,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_server_streaming_container_db::ID),
  );

  return $schema;
}
