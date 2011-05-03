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

require_once 'mediamosa_acl_name_db.class.inc';

/**
 * Our sub install.
 */
function _mediamosa_acl_name_install() {
// Create our table.
  mediamosa::drupal_install_schema('mediamosa_acl_name');
}

/**
 * Our sub uninstall.
 */
function _mediamosa_acl_name_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema('mediamosa_acl_name');
}

/**
 * Implements hook_schema().
 */
function mediamosa_acl_name_schema() {
  $schema[mediamosa_acl_name_db::TABLE_NAME] = array(
    'description' => 'This table stores named entities on which autorization can be linked.',
    'fields' => array(
      mediamosa_acl_name_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_acl_name_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_acl_name_db::ACL_GROUP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => FALSE,
        'description' => 'The optional group ID if name is part of group.',
      ),
      mediamosa_acl_name_db::ACL_NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_acl_name_db::ACL_NAME_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The name of the entity.',
      ),
      mediamosa_acl_name_db::ACL_PREFIX => array(
        'type' => 'varchar',
        'length' => mediamosa_acl_name_db::ACL_PREFIX_LENGTH,
        'not null' => FALSE,
        'default' => NULL,
        'description' => 'The optional prefix of the entity, only used with REALM type.',
      ),
      mediamosa_acl_name_db::ACL_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_acl_name_db::ACL_TYPE_DOMAIN, mediamosa_acl_name_db::ACL_TYPE_REALM, mediamosa_acl_name_db::ACL_TYPE_USER, mediamosa_acl_name_db::ACL_TYPE_USER_GROUP)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_acl_name_db::ACL_TYPE_DOMAIN, mediamosa_acl_name_db::ACL_TYPE_REALM, mediamosa_acl_name_db::ACL_TYPE_USER, mediamosa_acl_name_db::ACL_TYPE_USER_GROUP)) . "')",
        'default' => mediamosa_acl_name_db::ACL_TYPE_DOMAIN,
        'description' => 'The type of name entity.',
      ),
    ),
    'primary key' => array(mediamosa_acl_name_db::ID),
    'unique keys' => array(
      'uni_app_type_name_prefix' => array(mediamosa_acl_name_db::APP_ID, mediamosa_acl_name_db::ACL_TYPE, mediamosa_acl_name_db::ACL_NAME, mediamosa_acl_name_db::ACL_PREFIX),
    ),
    'indexes' => array(
      'idx_groupid' => array(mediamosa_acl_name_db::ACL_GROUP_ID),
      'idx_app_type_groupid_name_prefix' => array(mediamosa_acl_name_db::APP_ID, mediamosa_acl_name_db::ACL_TYPE, mediamosa_acl_name_db::ACL_GROUP_ID, mediamosa_acl_name_db::ACL_NAME, mediamosa_acl_name_db::ACL_PREFIX),
    ),
  );

  return $schema;
}
