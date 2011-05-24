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
 * Install for mediamosa_job_upload table.
 */
require_once 'mediamosa_job_upload_db.class.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_job_upload_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_job_upload_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_job_upload_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_job_upload_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_job_upload_schema() {

  $schema[mediamosa_job_upload_db::TABLE_NAME] = array(
  'description' => 'The transcode job table.',
    'fields' => array(
      mediamosa_job_upload_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
      'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_job_upload_db::JOB_ID => array(
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'The job ID.',
      ),
      mediamosa_job_upload_db::FILE_SIZE => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'The file size.',
        'default' => 0,
      ),
      mediamosa_job_upload_db::UPLOADED_FILE_SIZE => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'The uploaded file size.',
        'default' => 0,
      ),
      mediamosa_job_upload_db::RETRANSCODE => array(
        'description' => 'Enable to prevent dark empty stills.',
        'type' => "enum('" . implode("','", array(mediamosa_job_upload_db::RETRANSCODE_FALSE, mediamosa_job_upload_db::RETRANSCODE_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_upload_db::RETRANSCODE_FALSE, mediamosa_job_upload_db::RETRANSCODE_TRUE)) . "')",
        'default' => mediamosa_job_upload_db::RETRANSCODE_FALSE,
      ),
      mediamosa_job_upload_db::CREATE_STILL => array(
        'description' => 'Enable to prevent dark empty stills.',
        'type' => "enum('" . implode("','", array(mediamosa_job_upload_db::CREATE_STILL_FALSE, mediamosa_job_upload_db::CREATE_STILL_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_upload_db::CREATE_STILL_FALSE, mediamosa_job_upload_db::CREATE_STILL_TRUE)) . "')",
        'default' => mediamosa_job_upload_db::CREATE_STILL_FALSE,
      ),
      mediamosa_job_upload_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when upload job was created.',
      ),
      mediamosa_job_upload_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when upload job was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_job_upload_db::ID),
    'foreign keys' => array(
      mediamosa_job_upload_db::JOB_ID => array(mediamosa_job_db::TABLE_NAME => mediamosa_job_db::ID),
    ),
  );

  return $schema;
}
