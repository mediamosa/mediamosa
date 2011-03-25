<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
 * Install for statistics used diskspace.
 */

require_once 'mediamosa_statistics_rest_log_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_rest_log_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_statistics_rest_log_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_rest_log_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_statistics_rest_log_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_statistics_rest_log_schema() {

  $schema[mediamosa_statistics_rest_log_db::TABLE_NAME] = array(
  'description' => 'The statistics rest log table.',
    'fields' => array(
      mediamosa_statistics_rest_log_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_statistics_rest_log_db::REQUEST => array(
        'type' => 'text',
        'size' => 'normal',
        'not null' => TRUE,
        'description' => 'The request URI.',
      ),
      mediamosa_statistics_rest_log_db::PROCESS_TIME => array(
        'type' => 'float',
        'size' => 'normal',
        'not null' => TRUE,
        'description' => 'The process time.',
      ),
      mediamosa_statistics_rest_log_db::QUERY_COUNT => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => 'The query count.',
      ),
    ),
    'primary key' => array(mediamosa_statistics_rest_log_db::ID),
  );

  return $schema;
}
