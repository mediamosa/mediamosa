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
 * Installer for job table.
 */

require_once 'mediamosa_job.class.inc';
require_once 'mediamosa_job_db.class.inc';

require_once 'still/mediamosa_job_still.install.inc';
require_once 'server/mediamosa_job_server.install.inc';
require_once 'transcode/mediamosa_job_transcode.install.inc';
require_once 'upload/mediamosa_job_upload.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_job_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_job_db::TABLE_NAME);

  // Create Job Still table.
  _mediamosa_job_still_install();

  // Create Job Transcode table.
  _mediamosa_job_transcode_install();

  // Create Job Upload table.
  _mediamosa_job_upload_install();

  // Create Job Server table.
  _mediamosa_job_server_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_job_uninstall() {
  // Drop Job Server table.
  _mediamosa_job_server_uninstall();

  // Drop Job Upload table.
  _mediamosa_job_upload_uninstall();

  // Drop Job Transcode table.
  _mediamosa_job_transcode_uninstall();

  // Drop Job Still table.
  _mediamosa_job_still_uninstall();

  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_job_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_job_schema() {

  $schema[mediamosa_job_db::TABLE_NAME] = array(
  'description' => 'The job table.',
    'fields' => array(
      mediamosa_job_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_job_db::APP_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The application ID.',
      ),
      mediamosa_job_db::ASSET_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => 'The asset ID.',
      ),
      mediamosa_job_db::MEDIAFILE_ID => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => 'The mediafile ID.',
      ),
      mediamosa_job_db::MEDIAFILE_DEST => array(
        'type' => 'varbinary',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'default' => NULL,
        'description' => 'The mediafile destination ID in case of retranscode.',
      ),
      mediamosa_job_db::OWNER_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_job_db::OWNER_ID_LENGTH,
        'default' => NULL,
        'description' => 'The owner ID.',
      ),
      mediamosa_job_db::JOB_STATUS => array(
        'description' => 'The status of the job.',
        'type' => "enum('" . implode("','", array(mediamosa_job_db::JOB_STATUS_WAITING, mediamosa_job_db::JOB_STATUS_INPROGRESS, mediamosa_job_db::JOB_STATUS_FINISHED, mediamosa_job_db::JOB_STATUS_FAILED, mediamosa_job_db::JOB_STATUS_CANCELLED)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_db::JOB_STATUS_WAITING, mediamosa_job_db::JOB_STATUS_INPROGRESS, mediamosa_job_db::JOB_STATUS_FINISHED, mediamosa_job_db::JOB_STATUS_FAILED, mediamosa_job_db::JOB_STATUS_CANCELLED)) . "')",
        'default' => mediamosa_job_db::JOB_STATUS_WAITING,
      ),
      mediamosa_job_db::PROGRESS => array(
        'type' => 'varchar',
        'length' => mediamosa_job_db::PROGRESS_LENGTH,
        'not null' => TRUE,
        'description' => 'The progress of the job from 0.000 to 1.000.',
        'default' => '0.000',
      ),
      mediamosa_job_db::PRIORITY => array(
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
        'description' => 'Priority switch.',
        'default' => 0,
      ),
      mediamosa_job_db::JOB_TYPE => array(
        'description' => 'The type of the job.',
        'type' => "enum('" . implode("','", array(mediamosa_job_db::JOB_TYPE_TRANSCODE, mediamosa_job_db::JOB_TYPE_STILL, mediamosa_job_db::JOB_TYPE_UPLOAD, mediamosa_job_db::JOB_TYPE_ANALYSE, mediamosa_job_db::JOB_TYPE_DELETE_MEDIAFILE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_db::JOB_TYPE_TRANSCODE, mediamosa_job_db::JOB_TYPE_STILL, mediamosa_job_db::JOB_TYPE_UPLOAD, mediamosa_job_db::JOB_TYPE_ANALYSE, mediamosa_job_db::JOB_TYPE_DELETE_MEDIAFILE)) . "')",
        'default' => mediamosa_job_db::JOB_TYPE_UPLOAD,
      ),
      mediamosa_job_db::STARTED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was started.',
      ),
      mediamosa_job_db::FINISHED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was finished.',
      ),
      mediamosa_job_db::CREATE_STILL => array(
        'description' => 'The status of the job.',
        'type' => "enum('" . implode("','", array(mediamosa_job_db::CREATE_STILL_FALSE, mediamosa_job_db::CREATE_STILL_TRUE)) . "')",
        'not null' => TRUE,
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_db::CREATE_STILL_FALSE, mediamosa_job_db::CREATE_STILL_TRUE)) . "')",
        'default' => mediamosa_job_db::CREATE_STILL_FALSE,
      ),
      mediamosa_job_db::HINT => array(
        'description' => 'Hinting.',
        'type' => "enum('" . implode("','", array(mediamosa_job_db::HINT_FALSE, mediamosa_job_db::HINT_TRUE)) . "')",
        'mysql_type' => "enum('" . implode("','", array(mediamosa_job_db::HINT_FALSE, mediamosa_job_db::HINT_TRUE)) . "')",
      ),
      mediamosa_job_db::STILL_PARAMETERS => array(
        'type' => 'text',
        'size' => 'normal',
        'description' => 'Parameters when creating still.',
      ),
      mediamosa_job_db::ERROR_DESCRIPTION => array(
        'type' => 'text',
        'size' => 'normal',
        'description' => 'Possible error description log.',
      ),
      mediamosa_job_db::CREATED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'not null' => TRUE,
        'description' => 'The date and time when job was created.',
      ),
      mediamosa_job_db::CHANGED => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'default' => NULL,
        'description' => 'The date and time when job was changed last.',
      ),
    ),
    'primary key' => array(mediamosa_job_db::ID),
    'indexes' => array(
      'idx_jobtype' => array(mediamosa_job_db::JOB_TYPE),
      'idx_status_assetid_jobtype_jobid' => array(mediamosa_job_db::JOB_STATUS, mediamosa_job_db::ASSET_ID, mediamosa_job_db::JOB_TYPE, mediamosa_job_db::ID),
      'idx_jobid_status' => array(mediamosa_job_db::JOB_STATUS, mediamosa_job_db::ID),
    ),
    'foreign keys' => array(
      mediamosa_job_db::ASSET_ID => array(mediamosa_asset_db::TABLE_NAME => mediamosa_asset_db::ID),
    ),
  );

  return $schema;
}
