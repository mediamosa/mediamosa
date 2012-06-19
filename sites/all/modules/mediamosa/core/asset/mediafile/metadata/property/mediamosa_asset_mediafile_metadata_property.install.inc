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

require_once 'mediamosa_asset_mediafile_metadata_property.class.inc';
require_once 'mediamosa_asset_mediafile_metadata_property_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_mediafile_metadata_property_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_mediafile_metadata_property_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_mediafile_metadata_property_schema() {

  $schema[mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME] = array(
  'description' => 'The asset mediafile metadata property table.',
    'fields' => array(
      mediamosa_asset_mediafile_metadata_property_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
        'unsigned' => TRUE,
      ),
      mediamosa_asset_mediafile_metadata_property_db::NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_asset_mediafile_metadata_property_db::NAME_LENGTH,
        'not null' => TRUE,
        'description' => 'The name of the property.',
      ),
      mediamosa_asset_mediafile_metadata_property_db::TYPE => array(
        'description' => 'The type of the property.',
        'type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR)) . "')",
        'default' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      mediamosa_asset_mediafile_metadata_property_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when asset mediafile metadata property was created.',
      ),
      mediamosa_asset_mediafile_metadata_property_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when asset mediafile metadata property was changed.',
      ),
    ),
    'primary key' => array(mediamosa_asset_mediafile_metadata_property_db::ID),
    'unique keys' => array(
      'uni_name_type' => array(mediamosa_asset_mediafile_metadata_property_db::NAME, mediamosa_asset_mediafile_metadata_property_db::TYPE),
    ),
  );

  return $schema;
}
