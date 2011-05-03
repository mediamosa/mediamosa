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
  * Installer for mediamosa_acl_name.
  */

require_once 'mediamosa_acl_object_db.class.inc';

/**
 * Our sub install.
 */
function _mediamosa_acl_object_install() {
// Create our table.
  mediamosa::drupal_install_schema('mediamosa_acl_object');
}

/**
 * Our sub uninstall.
 */
function _mediamosa_acl_object_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema('mediamosa_acl_object');
}

/**
 * Implements hook_schema().
 */
function mediamosa_acl_object_schema() {
  $schema[mediamosa_acl_object_db::TABLE_NAME] = array(
    'description' => 'This table stores authorization links between named/group entities to objects.',
    'fields' => array(
      mediamosa_acl_object_db::ACL_OBJECT_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_acl_object_db::ACL_OBJECT_ID_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The object ID, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_acl_object_db::ACL_OBJECT_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_acl_object_db::ACL_OBJECT_TYPE_MEDIAFILE, mediamosa_acl_object_db::ACL_OBJECT_TYPE_ASSET, mediamosa_acl_object_db::ACL_OBJECT_TYPE_COLLECTION)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_acl_object_db::ACL_OBJECT_TYPE_MEDIAFILE, mediamosa_acl_object_db::ACL_OBJECT_TYPE_ASSET, mediamosa_acl_object_db::ACL_OBJECT_TYPE_COLLECTION)) . "')",
        'default' => mediamosa_acl_object_db::ACL_OBJECT_TYPE_MEDIAFILE,
        'description' => 'The type of object, links to mediamosa_asset_mediafile, mediamosa_asset or mediamosa_collection.',
      ),
      mediamosa_acl_object_db::ACL_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The entity ID, links to acl_name or acl_group.',
      ),
      mediamosa_acl_object_db::ACL_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_acl_object_db::ACL_TYPE_NAME, mediamosa_acl_object_db::ACL_TYPE_GROUP)) . "')",
        'not null' => FALSE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_acl_object_db::ACL_TYPE_NAME, mediamosa_acl_object_db::ACL_TYPE_GROUP)) . "')",
        'default' => NULL,
        'description' => 'The type of entity, links to acl_name or acl_group.',
      ),
    ),
    'unique keys' => array(
      'uni_link_to_obj' => array(mediamosa_acl_object_db::ACL_OBJECT_TYPE, mediamosa_acl_object_db::ACL_OBJECT_ID, mediamosa_acl_object_db::ACL_TYPE, mediamosa_acl_object_db::ACL_ID),
    ),
    'indexes' => array(
      'idx_type_id' => array(mediamosa_acl_object_db::ACL_TYPE, mediamosa_acl_object_db::ACL_ID),
      'idx_objid' => array(mediamosa_acl_object_db::ACL_OBJECT_ID)
    ),
  );

  return $schema;
}
