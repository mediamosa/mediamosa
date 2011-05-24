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
  * Installer for the App module.
  */

require_once 'mediamosa_app_oauth_nonce_db.class.inc';

/**
 * Not hook_install().
 */
function _mediamosa_app_oauth_nonce_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_app_oauth_nonce_db::TABLE_NAME);
}

/**
 * Not hook_uninstall().
 */
function _mediamosa_app_oauth_nonce_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_app_oauth_nonce_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 * @return array
 */
function mediamosa_app_oauth_nonce_schema() {
  $schema[mediamosa_app_oauth_nonce_db::TABLE_NAME] = array(
    'description' => 'Stores timestamp against nonce for repeat attacks.',
    'fields' => array(
      mediamosa_app_oauth_nonce_db::NONCE => array(
        'description' => 'The random 32 characters long string used on each request.',
        'type' => 'varchar',
        'length' => mediamosa_app_oauth_nonce_db::NONCE_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_app_oauth_nonce_db::TIMESTAMP => array(
        'description' => 'The timestamp of the request.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_app_oauth_nonce_db::TOKEN => array(
        'description' => 'Tokens for request and services accesses.',
        'type' => 'varchar',
        'length' => mediamosa_app_oauth_nonce_db::TOKEN_LENGTH,
      ),
    ),
    'primary key' => array(mediamosa_app_oauth_nonce_db::NONCE),
    'indexes' => array(
      'idx_' . mediamosa_app_oauth_nonce_db::TIMESTAMP => array(mediamosa_app_oauth_nonce_db::TIMESTAMP),
    ),
  );

  return $schema;
}
