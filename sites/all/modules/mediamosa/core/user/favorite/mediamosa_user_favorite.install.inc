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
  * Installer for user favorite.
  */
require_once 'mediamosa_user_favorite_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_user_favorite_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_user_favorite_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_user_favorite_uninstall() {
  // Remove our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_user_favorite_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_user_favorite_schema() {
  $schema[mediamosa_user_favorite_db::TABLE_NAME] = array(
  'description' => 'The user group table.',
    'fields' => array(
      mediamosa_user_favorite_db::NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_user_favorite_db::NAME_LENGTH,
        'not null' => TRUE,
        'description' => 'The name of user.',
      ),
      mediamosa_user_favorite_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the user.',
      ),
      mediamosa_user_favorite_db::FAV_TYPE => array(
        'description' => 'If the collection has unappropieate contents.',
        'type' => "enum('" . implode("','", array(mediamosa_user_favorite_db::FAV_TYPE_ASSET, mediamosa_user_favorite_db::FAV_TYPE_COLLECTION)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_user_favorite_db::FAV_TYPE_ASSET, mediamosa_user_favorite_db::FAV_TYPE_COLLECTION)) . "')",
      ),
      mediamosa_user_favorite_db::FAV_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The ID to the favorite object.',
      ),
    ),
    'primary key' => array(mediamosa_user_favorite_db::NAME, mediamosa_user_favorite_db::APP_ID, mediamosa_user_favorite_db::FAV_TYPE, mediamosa_user_favorite_db::FAV_ID),
    'indexes' => array(
      'idx_key_favtype_favid' => array(mediamosa_user_favorite_db::FAV_TYPE, mediamosa_user_favorite_db::FAV_ID)
    )
  );

  return $schema;
}
