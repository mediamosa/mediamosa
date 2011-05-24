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
  * FTP user installer.
  */

require_once 'mediamosa_ftp_user_db.class.inc';

/**
 * Implements hook_ininstall().
 */
function _mediamosa_ftp_user_install() {
// Create our table.
  mediamosa::drupal_install_schema(mediamosa_ftp_user_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_ftp_user_uninstall() {
  // Uninstall our table.
  mediamosa::drupal_uninstall_schema(mediamosa_ftp_user_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_ftp_user_schema() {
  $schema[mediamosa_ftp_user_db::TABLE_NAME] = array(
    'description' => 'This FTP User table.',
    'fields' => array(
      mediamosa_ftp_user_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => 'Primary key.',
      ),
      mediamosa_ftp_user_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID of the FTP user.',
      ),
      mediamosa_ftp_user_db::USERID => array(
        'type' => 'varbinary',
        'length' => mediamosa_ftp_user_db::USERID_LENGTH,
        'description' => 'The User ID.',
        'not null' => TRUE,
      ),
      mediamosa_ftp_user_db::PASSWD => array(
        'type' => 'varbinary',
        'length' => mediamosa_ftp_user_db::PASSWD_LENGTH,
        'description' => 'The Password.',
        'not null' => TRUE,
      ),
      mediamosa_ftp_user_db::UID => array(
        'type' => 'int',
        'size' => 'small',
        'not null' => TRUE,
        'description' => 'The linux user ID of the FTP user.',
        'default' => '200',
      ),
      mediamosa_ftp_user_db::GID => array(
        'type' => 'int',
        'size' => 'small',
        'not null' => TRUE,
        'description' => 'The linux group ID of the FTP user.',
        'default' => '200',
      ),
      mediamosa_ftp_user_db::HOMEDIR => array(
        'type' => 'varchar',
        'length' => mediamosa_ftp_user_db::HOMEDIR_LENGTH,
        'description' => 'The home directory path.',
        'not null' => TRUE,
      ),
      mediamosa_ftp_user_db::SHELL => array(
        'type' => 'varchar',
        'length' => mediamosa_ftp_user_db::SHELL_LENGTH,
        'description' => 'The home directory path.',
        'not null' => TRUE,
        'default' => '/bin/false',
      ),
      mediamosa_ftp_user_db::ACTIVE => array(
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'description' => 'Active setting.',
        'default' => '0',
      ),
      mediamosa_ftp_user_db::COUNT => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => 'The login count.',
        'default' => '0',
      ),
      mediamosa_ftp_user_db::ACCESSED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'description' => 'The access time.',
      ),
      mediamosa_ftp_user_db::MODIFIED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'description' => 'The modified time.',
      ),
    ),
    'primary key' => array(mediamosa_ftp_user_db::ID),
    'indexes' => array(
      'idx_userid' => array(mediamosa_ftp_user_db::USERID),
    ),
  );

  return $schema;
}
