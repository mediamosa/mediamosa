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

require_once 'mediamosa_watchdog.class.inc';
require_once 'mediamosa_watchdog_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_watchdog_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_watchdog_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_watchdog_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_watchdog_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 * @return array
 */
function mediamosa_watchdog_schema() {

  $schema[mediamosa_watchdog_db::TABLE_NAME] = array(
  'description' => 'The extender for the watchdog table.',
    'fields' => array(
      mediamosa_watchdog_db::ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_watchdog_db::ASSET_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The asset ID.',
      ),
      mediamosa_watchdog_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The asset mediafile ID.',
      ),
    ),
    'primary key' => array(mediamosa_watchdog_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_watchdog_db::ASSET_ID => array(mediamosa_watchdog_db::ASSET_ID),
      'idx_' . mediamosa_watchdog_db::MEDIAFILE_ID => array(mediamosa_watchdog_db::MEDIAFILE_ID),
    ),
    // Foreign key not possible because watchdog entries will be removed by
    // drupal, so we need to accept that we'll have rows here without watchdog.
  );

  return $schema;
}
