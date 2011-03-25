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
  * The installation file of the asset metadata group module.
  */

require_once 'mediamosa_asset_metadata_property_group.inc';
require_once 'mediamosa_asset_metadata_property_group_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_asset_metadata_property_group_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_asset_metadata_property_group_db::TABLE_NAME);

  // Create the groups.
  $a_property_groups = array(
    'dublin_core',
    'qualified_dublin_core',
    'czp',
  );

  foreach ($a_property_groups as $property_group) {
    $a_fields = array(
      mediamosa_asset_metadata_property_group_db::NAME => $property_group,
    );

    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    db_insert(mediamosa_asset_metadata_property_group_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_asset_metadata_property_group_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_asset_metadata_property_group_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_asset_metadata_property_group_schema() {

  $schema[mediamosa_asset_metadata_property_group_db::TABLE_NAME] = array(
  'description' => 'The asset metadata property group table.',
    'fields' => array(
      mediamosa_asset_metadata_property_group_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_asset_metadata_property_group_db::NAME => array(
        'type' => 'varchar',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
        'length' => mediamosa_asset_metadata_property_group_db::NAME_LENGTH,
      ),
      mediamosa_asset_metadata_property_group_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when asset metadata property was created.',
      ),
      mediamosa_asset_metadata_property_group_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when asset metadata property was changed.',
      ),
    ),
    'primary key' => array(mediamosa_asset_metadata_property_group_db::ID),
    'unique keys' => array(
      'uni_propgroupname' => array(mediamosa_asset_metadata_property_group_db::NAME),
    ),
  );

  return $schema;
}
