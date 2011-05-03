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
  * The user group installer.
  *
  * Although users are not part of a user group, I still decided to put the
  * file hierargy for future logic.
  */

require_once 'mediamosa_user_in_group_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_user_in_group_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_user_in_group_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_user_in_group_uninstall() {
  // Remove our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_user_in_group_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_user_in_group_schema() {
  $schema[mediamosa_user_in_group_db::TABLE_NAME] = array(
  'description' => 'The user/group relation table.',
    'fields' => array(
      mediamosa_user_in_group_db::ID => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_user_in_group_db::USER_KEY => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'User ID.',
      ),
      mediamosa_user_in_group_db::GROUP_KEY => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'Group ID.',
      ),
    ),
    'primary key' => array(mediamosa_user_in_group_db::ID),
    'unique keys' => array(
      'uni_relation' => array(mediamosa_user_in_group_db::USER_KEY, mediamosa_user_in_group_db::GROUP_KEY),
    ),
    'foreign keys' => array(
      mediamosa_user_in_group_db::USER_KEY => array(mediamosa_user_db::TABLE_NAME => mediamosa_user_db::KEY),
      mediamosa_user_in_group_db::GROUP_KEY => array(mediamosa_user_group_db::TABLE_NAME => mediamosa_user_group_db::KEY),
    ),
  );

  return $schema;
}
