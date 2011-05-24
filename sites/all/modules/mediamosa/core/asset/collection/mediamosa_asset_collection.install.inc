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
  * The installation file of the asset module.
  */

require_once 'mediamosa_asset_collection.class.inc';
require_once 'mediamosa_asset_collection_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_collection_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_collection_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_collection_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_collection_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_collection_schema() {

  $schema[mediamosa_asset_collection_db::TABLE_NAME] = array(
    'description' => 'The asset collection link table.',
    'fields' => array(
      mediamosa_asset_collection_db::ASSET_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Link to asset table.',
      ),
      mediamosa_asset_collection_db::COLL_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Link to collection table.',
      ),
      mediamosa_asset_collection_db::IS_EMPTY_ASSET => array(
        'description' => 'True if the asset in this collection has media.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_collection_db::IS_EMPTY_ASSET_FALSE, mediamosa_asset_collection_db::IS_EMPTY_ASSET_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_collection_db::IS_EMPTY_ASSET_FALSE, mediamosa_asset_collection_db::IS_EMPTY_ASSET_TRUE)) . "')",
        'default' => mediamosa_asset_collection_db::IS_EMPTY_ASSET_FALSE,
      ),
      mediamosa_asset_collection_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when asset collection link was created.',
      ),
      mediamosa_asset_collection_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when asset collection link was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_asset_collection_db::ASSET_ID, mediamosa_asset_collection_db::COLL_ID),
    'foreign keys' => array(
      mediamosa_asset_collection_db::ASSET_ID => array(mediamosa_asset_db::TABLE_NAME => mediamosa_asset_db::ID),
      mediamosa_asset_collection_db::COLL_ID => array(mediamosa_collection_db::TABLE_NAME => mediamosa_collection_db::ID),
    ),
    'indexes' => array(
      'idx_coll_isempty' => array(mediamosa_asset_collection_db::COLL_ID, mediamosa_asset_collection_db::IS_EMPTY_ASSET)
    ),
  );

  return $schema;
}
