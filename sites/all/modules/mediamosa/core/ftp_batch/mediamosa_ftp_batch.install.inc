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
  *
  */

require_once 'mediamosa_ftp_batch_db.class.inc';
require_once 'asset/mediamosa_ftp_batch_asset.install.inc';

/**
 * Implements hook_ininstall().
 */
function _mediamosa_ftp_batch_install() {
// Create our table.
  mediamosa::drupal_install_schema(mediamosa_ftp_batch_db::TABLE_NAME);

  // Create our child table.
  _mediamosa_ftp_batch_asset_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_ftp_batch_uninstall() {
  // Uninstall our child table.
  _mediamosa_ftp_batch_asset_uninstall();

  // Uninstall our table.
  mediamosa::drupal_uninstall_schema(mediamosa_ftp_batch_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_ftp_batch_schema() {
  $schema[mediamosa_ftp_batch_db::TABLE_NAME] = array(
    'description' => 'This FTP Batch table.',
    'fields' => array(
      mediamosa_ftp_batch_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => 'Primary key.',
      ),
      mediamosa_ftp_batch_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_ftp_batch_db::OWNER_ID => array(
        'description' => 'The owner of the batch.',
        'type' => 'varchar',
        'length' => mediamosa_ftp_batch_db::OWNER_ID_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_ftp_batch_db::GROUP_ID => array(
        'description' => 'The group owner of the batch.',
        'type' => 'varchar',
        'length' => mediamosa_ftp_batch_db::GROUP_ID_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_ftp_batch_db::VUF => array(
        'description' => 'The VUF file location.',
        'type' => 'varchar',
        'length' => mediamosa_ftp_batch_db::VUF_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_ftp_batch_db::STARTED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => FALSE,
        'description' => 'The started datatime of batch.',
      ),
      mediamosa_ftp_batch_db::FINISHED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => FALSE,
        'description' => 'The finished datatime of batch.',
      ),
      mediamosa_ftp_batch_db::EMAIL_ADDRESS => array(
        'description' => 'The email address of creator.',
        'type' => 'varchar',
        'length' => mediamosa_ftp_batch_db::EMAIL_ADDRESS_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_ftp_batch_db::EMAIL_CONTENTS => array(
        'description' => 'The email address of creator.',
        'type' => 'text',
        'not null' => FALSE,
      ),
    ),
    'primary key' => array(mediamosa_ftp_batch_db::ID),
  );

  return $schema;
}
