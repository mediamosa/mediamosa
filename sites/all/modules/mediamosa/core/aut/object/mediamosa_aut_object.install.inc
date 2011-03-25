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
  * Installer for mediamosa_aut_name.
  */

require_once 'mediamosa_aut_object_db.inc';

/**
 * Our sub install.
 */
function _mediamosa_aut_object_install() {
// Create our table.
  mediamosa::drupal_install_schema('mediamosa_aut_object');
}

/**
 * Our sub uninstall.
 */
function _mediamosa_aut_object_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema('mediamosa_aut_object');
}

/**
 * Implements hook_schema().
 */
function mediamosa_aut_object_schema() {
  $schema[mediamosa_aut_object_db::TABLE_NAME] = array(
    'description' => 'This table stores authorization links between named/group entities to objects.',
    'fields' => array(
      mediamosa_aut_object_db::AUT_OBJECT_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_aut_object_db::AUT_OBJECT_ID_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The object ID, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_aut_object_db::AUT_OBJECT_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_OBJECT_TYPE_MEDIAFILE, mediamosa_aut_object_db::AUT_OBJECT_TYPE_ASSET, mediamosa_aut_object_db::AUT_OBJECT_TYPE_COLLECTION)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_OBJECT_TYPE_MEDIAFILE, mediamosa_aut_object_db::AUT_OBJECT_TYPE_ASSET, mediamosa_aut_object_db::AUT_OBJECT_TYPE_COLLECTION)) . "')",
        'default' => mediamosa_aut_object_db::AUT_OBJECT_TYPE_MEDIAFILE,
        'description' => 'The type of object, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_aut_object_db::AUT_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The entity ID, links to aut_name or aut_group.',
      ),
      mediamosa_aut_object_db::AUT_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_TYPE_NAME, mediamosa_aut_object_db::AUT_TYPE_GROUP)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_object_db::AUT_TYPE_NAME, mediamosa_aut_object_db::AUT_TYPE_GROUP)) . "')",
        'default' => NULL,
        'description' => 'The type of entity, links to aut_name or aut_group.',
      ),
    ),
    'unique keys' => array(
      'uni_link_to_obj' => array(mediamosa_aut_object_db::AUT_OBJECT_TYPE, mediamosa_aut_object_db::AUT_OBJECT_ID, mediamosa_aut_object_db::AUT_TYPE, mediamosa_aut_object_db::AUT_ID),
    ),
    'indexes' => array(
      'idx_type_id' => array(mediamosa_aut_object_db::AUT_TYPE, mediamosa_aut_object_db::AUT_ID),
      'idx_objid' => array(mediamosa_aut_object_db::AUT_OBJECT_ID)
    ),
  );

  return $schema;
}
