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
  * Installer for mediamosa_aut_group.
  */

require_once 'mediamosa_aut_group_db.inc';

/**
 * Our sub install.
 */
function _mediamosa_aut_group_install() {
// Create our table.
  mediamosa::drupal_install_schema('mediamosa_aut_group');
}

/**
 * Our sub uninstall.
 */
function _mediamosa_aut_group_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema('mediamosa_aut_group');
}

/**
 * Implements hook_schema().
 */
function mediamosa_aut_group_schema() {
  $schema[mediamosa_aut_group_db::TABLE_NAME] = array(
    'description' => 'This table stores grouped named entities on which autorization can be linked.',
    'fields' => array(
      mediamosa_aut_group_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_aut_group_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_aut_group_db::AUT_GROUP_NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_aut_group_db::AUT_GROUP_NAME_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The group name.',
      ),
      mediamosa_aut_group_db::AUT_GROUP_TYPE => array(
        'type' => "enum('" . implode("','", array(mediamosa_aut_group_db::AUT_GROUP_TYPE_REALM, mediamosa_aut_group_db::AUT_GROUP_TYPE_DOMAIN)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_aut_group_db::AUT_GROUP_TYPE_REALM, mediamosa_aut_group_db::AUT_GROUP_TYPE_DOMAIN)) . "')",
        'default' => mediamosa_aut_group_db::AUT_GROUP_TYPE_REALM,
        'description' => 'The type of group.',
      ),
    ),
    'primary key' => array(mediamosa_aut_group_db::ID),
    'unique keys' => array(
      'uni_app_type_name' => array(mediamosa_aut_group_db::APP_ID, mediamosa_aut_group_db::AUT_GROUP_TYPE, mediamosa_aut_group_db::AUT_GROUP_NAME),
    ),
  );

  return $schema;
}
