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

require_once 'mediamosa_statistics_job_db.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_job_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_statistics_job_db::TABLE_NAME);
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_job_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_statistics_job_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_statistics_job_schema() {

  $schema[mediamosa_statistics_job_db::TABLE_NAME] = array(
  'description' => 'The statistics_job table.',
    'fields' => array(
      mediamosa_statistics_job_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_statistics_job_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_statistics_job_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => 'The mediafile ID.',
      ),
      mediamosa_statistics_job_db::JOB_TYPE => array(
        'description' => 'The type of the job.',
        'type' => "enum('" . implode("','", array(mediamosa_statistics_job_db::JOB_TYPE_TRANSCODE, mediamosa_statistics_job_db::JOB_TYPE_STILL, mediamosa_statistics_job_db::JOB_TYPE_UPLOAD, mediamosa_statistics_job_db::JOB_TYPE_ANALYSE, mediamosa_statistics_job_db::JOB_TYPE_DELETE_MEDIAFILE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_statistics_job_db::JOB_TYPE_TRANSCODE, mediamosa_statistics_job_db::JOB_TYPE_STILL, mediamosa_statistics_job_db::JOB_TYPE_UPLOAD, mediamosa_statistics_job_db::JOB_TYPE_ANALYSE, mediamosa_statistics_job_db::JOB_TYPE_DELETE_MEDIAFILE)) . "')",
        'default' => mediamosa_statistics_job_db::JOB_TYPE_TRANSCODE,
      ),
      mediamosa_statistics_job_db::STARTED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was started.',
      ),
      mediamosa_statistics_job_db::FINISHED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was finished.',
      ),
      mediamosa_statistics_job_db::TRANSCODE_PROFILE_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'description' => 'The transcode profile ID.',
      ),
      mediamosa_statistics_job_db::NID => array(
        'description' => 'The Drupal node id for transcode profile.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_statistics_job_db::VID => array(
        'description' => 'The Drupal node revision id for transcode profile.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_statistics_job_db::FILESIZE => array(
        'type' => 'int',
        'size' => 'big',
        'description' => 'The filesize.',
      ),
    ),
    'primary key' => array(mediamosa_statistics_job_db::ID),
    'indexes' => array(
      'idx_app_id' => array(mediamosa_statistics_job_db::APP_ID),
      'idx_mediafile_id' => array(mediamosa_statistics_job_db::MEDIAFILE_ID),
      'idx_started' => array(mediamosa_statistics_job_db::STARTED),
      'idx_finished' => array(mediamosa_statistics_job_db::FINISHED),
    ),
  );

  return $schema;
}
