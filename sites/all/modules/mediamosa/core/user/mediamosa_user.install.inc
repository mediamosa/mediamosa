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
  * The user installer.
  */

require_once 'mediamosa_user_db.class.inc';

require_once 'favorite/mediamosa_user_favorite.install.inc';
require_once 'group/mediamosa_user_group.install.inc';
require_once 'group/mediamosa_user_in_group.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_user_install() {

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_user_db::TABLE_NAME);

  // Installer user group.
  _mediamosa_user_group_install();

  // Installer user/group relation.
  _mediamosa_user_in_group_install();

  // Installer user group.
  _mediamosa_user_favorite_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_user_uninstall() {

  // Uninstaller user favorite.
  _mediamosa_user_favorite_uninstall();

  // Uninstaller user group.
  _mediamosa_user_group_uninstall();

  // Uninstaller user/group relation.
  _mediamosa_user_in_group_uninstall();

  // Remove our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_user_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_user_schema() {
  $schema[mediamosa_user_db::TABLE_NAME] = array(
  'description' => 'The user table.',
    'fields' => array(
      mediamosa_user_db::KEY => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_user_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the user.',
      ),
      mediamosa_user_db::NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_user_db::NAME_LENGTH,
        'not null' => TRUE,
        'description' => 'The name of user.',
      ),
      mediamosa_user_db::QUOTUM => array(
        'type' => 'int',
        'unsigned' => FALSE,
        'not null' => TRUE,
        'description' => 'The disk quota in MB.',
      ),
      mediamosa_user_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when user was created.',
      ),
      mediamosa_user_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when user was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_user_db::KEY),
    'unique keys' => array(
      'uni_id_name_and_app' => array(mediamosa_user_db::NAME, mediamosa_user_db::APP_ID),
    ),

  );

  return $schema;
}
