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
  * Webservice app installer.
  */

require_once 'mediamosa_webservice_app.class.inc';
require_once 'mediamosa_webservice_app_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_webservice_app_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_webservice_app_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_webservice_app_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_webservice_app_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 * @return array
 */
function mediamosa_webservice_app_schema() {
  $schema[mediamosa_webservice_app_db::TABLE_NAME] = array(
    'description' => 'The webservice are modules of mediamosa, which can be enabled/disabled per application.',
    'fields' => array(
      mediamosa_webservice_app_db::ID => array(
        'description' => 'The webservice app ID.',
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_webservice_app_db::WEBSERVICE_HANDLE => array(
        'description' => 'The handle of the webservice.',
        'type' => 'varchar',
        'length' => mediamosa_webservice_app_db::WEBSERVICE_HANDLE_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_webservice_app_db::APP_ID => array(
        'description' => 'The APP ID.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_webservice_app_db::STATUS => array(
        'description' => 'The webservice application switch.',
        'type' => "enum('" . implode("','", array(mediamosa_webservice_app_db::STATUS_FALSE, mediamosa_webservice_app_db::STATUS_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_webservice_app_db::STATUS_FALSE, mediamosa_webservice_app_db::STATUS_TRUE)) . "')",
        'default' => mediamosa_webservice_app_db::STATUS_TRUE,
      ),
    ),
    'primary key' => array(mediamosa_webservice_app_db::ID, mediamosa_webservice_app_db::WEBSERVICE_HANDLE),
  );

  return $schema;
}

