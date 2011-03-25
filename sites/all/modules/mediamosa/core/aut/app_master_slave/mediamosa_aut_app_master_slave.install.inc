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
  * Installer for mediamosa_aut_app_master_slave.
  */

require_once 'mediamosa_aut_app_master_slave_db.inc';

/**
 * Implements hook_ininstall().
 */
function _mediamosa_aut_app_master_slave_install() {
// Create our table.
  mediamosa::drupal_install_schema('mediamosa_aut_app_master_slave');
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_aut_app_master_slave_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema('mediamosa_aut_app_master_slave');
}

/**
 * Implements hook_schema().
 */
function mediamosa_aut_app_master_slave_schema() {
  $schema[mediamosa_aut_app_master_slave_db::TABLE_NAME] = array(
    'description' => 'This table stores master/slave links to media.',
    'fields' => array(
      mediamosa_aut_app_master_slave_db::AUT_OBJECT_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_ID_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The object ID, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_MEDIAFILE, mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_ASSET, mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_COLLECTION)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_MEDIAFILE, mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_ASSET, mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_COLLECTION)) . "')",
        'default' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_MEDIAFILE,
        'description' => 'The type of object, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_aut_app_master_slave_db::APP_ID_SLAVE => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The slave application ID.',
      ),
      mediamosa_aut_app_master_slave_db::APP_ID_MASTER => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The master application ID.',
      ),
    ),
    'unique keys' => array(
      'uni_objtype_objid_masterapp_slaveapp' => array(mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE, mediamosa_aut_app_master_slave_db::AUT_OBJECT_ID, mediamosa_aut_app_master_slave_db::APP_ID_MASTER, mediamosa_aut_app_master_slave_db::APP_ID_SLAVE),
    ),
    'indexes' => array(
      'idx_masterapp_slaveapp' => array(mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE, mediamosa_aut_app_master_slave_db::APP_ID_MASTER, mediamosa_aut_app_master_slave_db::APP_ID_SLAVE),
    ),
  );

  return $schema;
}
