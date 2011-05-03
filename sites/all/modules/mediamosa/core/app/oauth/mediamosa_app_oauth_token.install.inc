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
  * Installer for the App module.
  */

require_once 'mediamosa_app_oauth_token_db.class.inc';

/**
 * Not hook_install().
 */
function _mediamosa_app_oauth_token_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_app_oauth_token_db::TABLE_NAME);
}

/**
 * Not hook_uninstall().
 */
function _mediamosa_app_oauth_token_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_app_oauth_token_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 *
 * @return array
 */
function mediamosa_app_oauth_token_schema() {
  $schema[mediamosa_app_oauth_token_db::TABLE_NAME] = array(
    'description' => 'Tokens stored on behalf of providers or consumers for request and services accesses.',
    'fields' => array(
      mediamosa_app_oauth_token_db::TOKEN_KEY => array(
        'description' => 'Token key.',
        'type' => 'varchar',
        'length' => mediamosa_app_oauth_token_db::TOKEN_KEY_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_app_oauth_token_db::SECRET => array(
        'description' => 'Token secret.',
        'type' => 'varchar',
        'length' => mediamosa_app_oauth_token_db::SECRET_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_app_oauth_token_db::CREATED => array(
        'description' => 'The time that the token was created, as a Unix timestamp.',
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_oauth_token_db::CHANGED => array(
        'description' => 'The last time the token was edited, as a Unix timestamp.',
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_oauth_token_db::EXPIRES => array(
        'description' => 'The expiry time for the token, as a Unix timestamp.',
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_oauth_token_db::TYPE => array(
        'description' => 'Token type: request or access.',
        'type' => 'varchar',
        'length' => mediamosa_app_oauth_token_db::TYPE_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_app_oauth_token_db::APP_ID => array(
        'description' => 'The unique application ID.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      mediamosa_app_oauth_token_db::CONSUMER_KEY => array(
        'description' => 'Consumer key.',
        'type' => 'varchar',
        'length' => mediamosa_app_oauth_token_db::CONSUMER_KEY_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_app_oauth_token_db::AUTHORIZED => array(
        'description' => 'In case its a request token, it checks if the user already authorized the consumer to get an access token.',
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'default' => 0,
      ),
      mediamosa_app_oauth_token_db::CALLBACK_URL => array(
        'description' => 'Callback url.',
        'type'        => 'varchar',
        'length'      => mediamosa_app_oauth_token_db::CALLBACK_URL_LENGTH,
        'not null'    => TRUE,
      ),
    ),
    'primary key' => array(mediamosa_app_oauth_token_db::TOKEN_KEY),
    'indexes' => array(
      'idx_' . mediamosa_app_oauth_token_db::TOKEN_KEY . '_' . mediamosa_app_oauth_token_db::TYPE => array(
        mediamosa_app_oauth_token_db::TOKEN_KEY,
        mediamosa_app_oauth_token_db::TYPE,
        ),
      'idx_' . mediamosa_app_oauth_token_db::CONSUMER_KEY => array(mediamosa_app_oauth_token_db::CONSUMER_KEY),
    ),
  );

  return $schema;
}
