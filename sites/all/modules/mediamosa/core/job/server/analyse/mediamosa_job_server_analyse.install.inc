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
 * Install for mediamosa_job_server_analyse table.
 */
require_once 'mediamosa_job_server_analyse_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_job_server_analyse_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_job_server_analyse_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_job_server_analyse_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_job_server_analyse_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_job_server_analyse_schema() {

  $schema[mediamosa_job_server_analyse_db::TABLE_NAME] = array(
  'description' => 'The job server analyse table.',
    'fields' => array(
      mediamosa_job_server_analyse_db::ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The server job ID.',
      ),
      mediamosa_job_server_analyse_db::ANALYSE_RESULT => array(
        'type' => 'text',
        'default' => NULL,
        'description' => 'The analyse result.',
      ),
      mediamosa_job_server_analyse_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when job analyse was created.',
      ),
      mediamosa_job_server_analyse_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job analyse was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_job_server_analyse_db::ID),
    'foreign keys' => array(
      mediamosa_job_server_analyse_db::ID => array(mediamosa_job_server_db::TABLE_NAME => mediamosa_job_server_db::ID),
    ),
  );

  return $schema;
}
