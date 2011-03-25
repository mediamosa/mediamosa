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
  * Installer for asset supplement.
  */
require_once 'mediamosa_asset_supplement_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_supplement_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_supplement_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_supplement_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_supplement_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_supplement_schema() {

  $schema[mediamosa_asset_supplement_db::TABLE_NAME] = array(
  'description' => 'The asset supplement table.',
    'fields' => array(
      mediamosa_asset_supplement_db::ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_asset_supplement_db::ASSET_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_asset_supplement_db::SUPPLEMENT => array(
        'type' => 'blob',
        'description' => 'Contens of the supplement.',
      ),
      mediamosa_asset_supplement_db::TAG => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_supplement_db::TAG_LENGTH,
        'description' => 'Tag for fast search and lookup.',
      ),
      mediamosa_asset_supplement_db::OAI_PREFIX => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_supplement_db::OAI_PREFIX_LENGTH,
        'description' => 'OAI Prefix.',
      ),
      mediamosa_asset_supplement_db::OAI_NAMESPACEURI => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_supplement_db::OAI_NAMESPACEURI_LENGTH,
        'description' => 'OAI namespace URI.',
      ),
      mediamosa_asset_supplement_db::OAI_SCHEMALOCATION => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_supplement_db::OAI_SCHEMALOCATION_LENGTH,
        'description' => 'OAI Schema location.',
      ),
      mediamosa_asset_supplement_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when supplement was created.',
      ),
      mediamosa_asset_supplement_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when supplement was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_asset_supplement_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_asset_supplement_db::TAG => array(mediamosa_asset_supplement_db::TAG),
    ),
    'foreign keys' => array(
      mediamosa_asset_supplement_db::ASSET_ID => array(mediamosa_asset_db::TABLE_NAME => mediamosa_asset_db::ID),
    ),
  );

  return $schema;
}
