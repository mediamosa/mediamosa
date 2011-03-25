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

require_once 'mediamosa_asset_delete.inc';
require_once 'mediamosa_asset_delete_db.inc';

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
    //'primary key' => array(mediamosa_asset_delete_db::ID, mediamosa_asset_delete_db::COLL_ID),
    'indexes' => array(
      'idx_app_videotimestamp' => array(mediamosa_asset_delete_db::APP_ID, mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED),
    ),
    // Defined as unique and not as primary because drupal will undo the NULL option on coll_id.
    'unique keys' => array(
      'uni_id_collid' => array(mediamosa_asset_delete_db::ID, mediamosa_asset_delete_db::COLL_ID),
    )
  );

  return $schema;
}
