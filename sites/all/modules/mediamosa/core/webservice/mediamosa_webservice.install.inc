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
  * Install file for webservice module.
  */

require_once 'mediamosa_webservice.inc';
require_once 'mediamosa_webservice_db.inc';
require_once 'app/mediamosa_webservice_app.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_webservice_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_webservice_db::TABLE_NAME);

  // Create default global enabled settings.
  $a_webservices = array(
    array(
      'handle' => mediamosa_webservice_app::HANDLE_TRANSCODING,
      'description' => 'All transcoding, except transcode jobs are still processed when set to off.',
    ),
    array(
      'handle' => mediamosa_webservice_app::HANDLE_JOBS,
      'description' => 'Enable or disable starting or creating jobs. ',
    ),
    array(
      'handle' => mediamosa_webservice_app::HANDLE_MEDIA_UPLOAD,
      'description' => 'Enable or disable upload mediafiles.',
    ),
    array(
      'handle' => mediamosa_webservice_app::HANDLE_BATCH_UPLOAD,
      'description' => 'Enable or disable batch upload/FTP.',
    ),
    array(
      'handle' => mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT,
      'description' => 'Enable or disable changes to assets, mediafiles and metadata.',
    ),
  );

  // Create default.
  foreach ($a_webservices as $a_webservice) {
    db_insert(mediamosa_webservice_db::TABLE_NAME)
      ->fields(array(
        mediamosa_webservice_db::VERSION => 0,
        mediamosa_webservice_db::HANDLE => $a_webservice['handle'],
        mediamosa_webservice_db::DESCRIPTION => $a_webservice['description'],
        mediamosa_webservice_db::STATUS => mediamosa_webservice_db::STATUS_TRUE,
      ))
      ->execute();
  }

  // Create children.
  _mediamosa_webservice_app_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_webservice_uninstall() {
  // Drop children.
  _mediamosa_webservice_app_uninstall();

  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_webservice_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 * @return array
 */
function mediamosa_webservice_schema() {
  $schema[mediamosa_webservice_db::TABLE_NAME] = array(
    'description' => 'The webservice are modules of mediamosa, which can be enabled/disabled per application.',
    'fields' => array(
      mediamosa_webservice_db::ID => array(
        'description' => 'The webservice ID.',
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_webservice_db::VERSION => array(
        'description' => 'The version of the webservice.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_webservice_db::HANDLE => array(
        'description' => 'The handle of the webservice.',
        'type' => 'varchar',
        'length' => mediamosa_webservice_db::HANDLE_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_webservice_db::DESCRIPTION => array(
        'description' => 'The handle of the webservice.',
        'type' => 'varchar',
        'length' => mediamosa_webservice_db::DESCRIPTION_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_webservice_db::STATUS => array(
        'description' => 'The webservice main switch.',
        'type' => "enum('" . implode("','", array(mediamosa_webservice_db::STATUS_FALSE, mediamosa_webservice_db::STATUS_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_webservice_db::STATUS_FALSE, mediamosa_webservice_db::STATUS_TRUE)) . "')",
        'default' => mediamosa_webservice_db::STATUS_TRUE,
      ),
    ),
    'primary key' => array(mediamosa_webservice_db::ID),
    'unique keys' => array(
      'idx_handle' => array(mediamosa_webservice_db::HANDLE)
    ),
  );

  return $schema;
}
