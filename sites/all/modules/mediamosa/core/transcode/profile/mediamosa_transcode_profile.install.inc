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
 *
 */
require_once 'mediamosa_transcode_profile_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_transcode_profile_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_transcode_profile_db::TABLE_NAME);

  // Create our node type.
  $transcode_profile_node_type = array(
    'type' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE,
    'name' => st('MediaMosa transcode profile'),
    'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE,
    'description' => st('The transcode profile contains information about the codec type and settings for a video transcoding.'),
    'custom' => 1,
    'modified' => 1,
    'locked' => 0,
    'has_body' => 0,
  );

  $transcode_profile_node_type = node_type_set_defaults($transcode_profile_node_type);
  node_type_save($transcode_profile_node_type);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_transcode_profile_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_transcode_profile_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_transcode_profile_schema() {

  $schema[mediamosa_transcode_profile_db::TABLE_NAME] = array(
  'description' => 'The transcode profile table.',
    'fields' => array(
      mediamosa_transcode_profile_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_transcode_profile_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_transcode_profile_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_transcode_profile_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_transcode_profile_db::VERSION => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'default' => NULL,
        'description' => 'Version profile.',
      ),
      mediamosa_transcode_profile_db::PROFILE => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_profile_db::PROFILE_LENGTH,
        'description' => 'Profile data.',
      ),
      mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE => array(
        'description' => 'The asset is private.',
        'type' => "enum('" . implode("','", array(mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE, mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE, mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_TRUE)) . "')",
        'default' => mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE_FALSE,
      ),
      mediamosa_transcode_profile_db::TOOL => array(
        'type' => 'varchar',
        'length' => mediamosa_transcode_profile_db::TOOL_LENGTH,
        'description' => 'Tool used.',
      ),
      mediamosa_transcode_profile_db::FILE_EXTENSION => array(
        'type' => 'char',
        'length' => mediamosa_transcode_profile_db::FILE_EXTENSION_LENGTH,
        'description' => 'file extension.',
      ),
      mediamosa_transcode_profile_db::COMMAND => array(
        'type' => 'text',
        'description' => 'Command.',
      ),
      mediamosa_transcode_profile_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when transcode profile was created.',
      ),
      mediamosa_transcode_profile_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when transcode profile was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_transcode_profile_db::ID),
    'unique keys' => array(
      'uni_appid_profile' => array(mediamosa_transcode_profile_db::APP_ID, mediamosa_transcode_profile_db::PROFILE),
      'uni_nid' => array(mediamosa_transcode_profile_db::NID),
    ),
    'indexes' => array(
      'idx_app_isdefault' => array(mediamosa_transcode_profile_db::APP_ID, mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE),
    ),
  );

  return $schema;
}
