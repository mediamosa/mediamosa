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
 * The install file for asset media
 */

require_once 'mediamosa_asset_mediafile_metadata.class.inc';
require_once 'mediamosa_asset_mediafile_metadata_db.class.inc';
require_once 'property/mediamosa_asset_mediafile_metadata_property_db.class.inc';
require_once 'property/mediamosa_asset_mediafile_metadata_property.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_mediafile_metadata_install() {
  // Create the asset mediafile metadata property table first.
  _mediamosa_asset_mediafile_metadata_property_install();

  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_mediafile_metadata_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);

  // Drop the asset mediafile metadata property table.
  _mediamosa_asset_mediafile_metadata_property_uninstall();
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_mediafile_metadata_schema() {

  $schema[mediamosa_asset_mediafile_metadata_db::TABLE_NAME] = array(
  'description' => 'The asset media metadata table.',
    'fields' => array(
      mediamosa_asset_mediafile_metadata_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The asset mediafile ID.',
      ),
      mediamosa_asset_mediafile_metadata_db::PROP_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => 'The property ID of the metadata.',
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_db::VAL_CHAR => array(
        'type' => 'text',
        'size' => 'normal',
        'description' => 'The value of the property stored as a string.',
      ),
      mediamosa_asset_mediafile_metadata_db::VAL_DATETIME => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'description' => 'The date and time of the property value.',
      ),
      mediamosa_asset_mediafile_metadata_db::VAL_INT => array(
        'type' => 'int',
        'size' => 'big',
        //'not null' => TRUE,
        'description' => 'The numeric value of the property.',
        //'default' => 0,
      ),
    ),
    'primary key' => array(mediamosa_asset_mediafile_metadata_db::ID),
    'unique keys' => array(
      'idx_mediafileid_propid' => array(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, mediamosa_asset_mediafile_metadata_db::PROP_ID),
    ),
    'foreign keys' => array(
      mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID => array(mediamosa_asset_mediafile_db::TABLE_NAME => mediamosa_asset_mediafile_db::ID),
      mediamosa_asset_mediafile_metadata_db::PROP_ID => array(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME => mediamosa_asset_mediafile_metadata_property_db::ID),
    ),
  );

  return $schema;
}
