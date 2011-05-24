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
 * Install for mediamosa_server_job table.
 */
require_once 'mediamosa_server_job_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_job_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_server_job_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_job_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_server_job_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_job_schema() {

  $schema[mediamosa_server_job_db::TABLE_NAME] = array(
  'description' => 'The transcode job table.',
    'fields' => array(
      mediamosa_server_job_db::SERVER_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'The server ID points to mediamosa_server nid field.',
      ),
      mediamosa_server_job_db::JOB_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'The job ID.',
      ),
      mediamosa_server_job_db::PROGRESS => array(
        'type' => 'int',
        'size' => 'tiny',
        'unsigned' => TRUE,
        'description' => 'The progress.',
        'default' => 0,
      ),
      mediamosa_server_job_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when server job was created.',
      ),
      mediamosa_server_job_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when server job was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_server_job_db::SERVER_ID, mediamosa_server_job_db::JOB_ID),
    'foreign keys' => array(
      mediamosa_server_job_db::JOB_ID => array(mediamosa_job_db::TABLE_NAME => mediamosa_job_db::ID),
    ),
  );

  return $schema;
}
