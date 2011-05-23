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
  * Installer for ftp_batch_asset.
  */

require_once 'mediamosa_ftp_batch_asset_db.class.inc';

/**
 * Implements hook_ininstall().
 */
function _mediamosa_ftp_batch_asset_install() {
// Create our table.
  mediamosa::drupal_install_schema(mediamosa_ftp_batch_asset_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_ftp_batch_asset_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema(mediamosa_ftp_batch_asset_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_ftp_batch_asset_schema() {
  $schema[mediamosa_ftp_batch_asset_db::TABLE_NAME] = array(
    'description' => 'This FTP Batch asset link table.',
    'fields' => array(
      mediamosa_ftp_batch_asset_db::ASSET_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The asset ID that links to mediamosa_asset table.',
      ),
      mediamosa_ftp_batch_asset_db::BATCH_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The batch ID that links to mediamosa_ftp_batch table.',
      ),
    ),
    'primary key' => array(mediamosa_ftp_batch_asset_db::BATCH_ID, mediamosa_ftp_batch_asset_db::ASSET_ID),
    'indexes' => array(
      'idx_' . mediamosa_ftp_batch_asset_db::ASSET_ID => array(mediamosa_ftp_batch_asset_db::ASSET_ID),
    )
  );

  return $schema;
}
