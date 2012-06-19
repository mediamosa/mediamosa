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
  * The installation file of the asset module.
  */

require_once 'mediamosa_collection.class.inc';
require_once 'mediamosa_collection_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_collection_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_collection_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_collection_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_collection_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_collection_schema() {

  $schema[mediamosa_collection_db::TABLE_NAME] = array(
  'description' => 'The collection table.',
    'fields' => array(
      mediamosa_collection_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_collection_db::TITLE => array(
        'type' => 'varchar',
        'length' => mediamosa_collection_db::TITLE_LENGTH,
        'default' => NULL,
        'description' => 'The collection title.',
      ),
      mediamosa_collection_db::DESCRIPTION => array(
        'type' => 'varchar',
        'length' => mediamosa_collection_db::DESCRIPTION_LENGTH,
        'default' => NULL,
        'description' => 'The collection description.',
      ),
      mediamosa_collection_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the asset.',
      ),
      mediamosa_collection_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_collection_db::OWNER_ID_LENGTH,
        'default' => NULL,
        'description' => 'The owner ID.',
      ),
      mediamosa_collection_db::GROUP_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_collection_db::GROUP_ID_LENGTH,
        'default' => NULL,
        'description' => 'The group ID.',
      ),
      mediamosa_collection_db::ISPRIVATE => array(
        'description' => 'The collection is private.',
        'type' => "enum('" . implode("','", array(mediamosa_collection_db::ISPRIVATE_FALSE, mediamosa_collection_db::ISPRIVATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_collection_db::ISPRIVATE_FALSE, mediamosa_collection_db::ISPRIVATE_TRUE)) . "')",
        'default' => mediamosa_collection_db::ISPRIVATE_FALSE,
      ),
      mediamosa_collection_db::PRIVATE_ => array(
        'description' => 'The collection is private.',
        'type' => "enum('" . implode("','", array(mediamosa_collection_db::PRIVATE_FALSE, mediamosa_collection_db::PRIVATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_collection_db::PRIVATE_FALSE, mediamosa_collection_db::PRIVATE_TRUE)) . "')",
        'default' => mediamosa_collection_db::PRIVATE_FALSE,
      ),
      mediamosa_collection_db::_PUBLIC => array(
        'description' => 'The collection is public.',
        'type' => "enum('" . implode("','", array(mediamosa_collection_db::PUBLIC_FALSE, mediamosa_collection_db::PUBLIC_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_collection_db::PUBLIC_FALSE, mediamosa_collection_db::PUBLIC_TRUE)) . "')",
        'default' => mediamosa_collection_db::PUBLIC_TRUE,
      ),
      mediamosa_collection_db::CATEGORY => array(
        'description' => 'The collection\'s category.',
        'type' => "enum('" . implode("','", array(mediamosa_collection_db::CATEGORY_FALSE, mediamosa_collection_db::CATEGORY_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_collection_db::CATEGORY_FALSE, mediamosa_collection_db::CATEGORY_TRUE)) . "')",
        'default' => mediamosa_collection_db::CATEGORY_FALSE,
      ),
      mediamosa_collection_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when asset was created.',
      ),
      mediamosa_collection_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when asset was changed last.',
      ),
      mediamosa_collection_db::IS_UNAPPROPRIATE => array(
        'description' => 'If the collection has unappropieate contents.',
        'type' => "enum('" . implode("','", array(mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE, mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE, mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE)) . "')",
        'default' => mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE,
      ),
      mediamosa_collection_db::PUBLIC_ASSIGN => array(
        'description' => 'If the collection public assigned.',
        'type' => "enum('" . implode("','", array(mediamosa_collection_db::PUBLIC_ASSIGN_FALSE, mediamosa_collection_db::PUBLIC_ASSIGN_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_collection_db::PUBLIC_ASSIGN_FALSE, mediamosa_collection_db::PUBLIC_ASSIGN_TRUE)) . "')",
        'default' => mediamosa_collection_db::PUBLIC_ASSIGN_FALSE,
      ),
    ),
    'primary key' => array(mediamosa_collection_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_collection_db::TITLE => array(mediamosa_collection_db::TITLE),
      'idx_' . mediamosa_collection_db::APP_ID => array(mediamosa_collection_db::APP_ID),
      'idx_' . mediamosa_collection_db::OWNER_ID => array(mediamosa_collection_db::OWNER_ID),
      'idx_' . mediamosa_collection_db::GROUP_ID => array(mediamosa_collection_db::GROUP_ID),
      'idx_' . mediamosa_collection_db::CREATED => array(mediamosa_collection_db::CREATED),
      'idx_' . mediamosa_collection_db::CHANGED => array(mediamosa_collection_db::CHANGED),
      'idx_' . mediamosa_collection_db::IS_UNAPPROPRIATE => array(mediamosa_collection_db::IS_UNAPPROPRIATE),
    ),
  );

  return $schema;
}
