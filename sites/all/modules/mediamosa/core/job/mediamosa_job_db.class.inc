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
 * DB class job.
 */

class mediamosa_job_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job'; // 1.x name job.

  // Database fields;
  const ID = 'job_id';
  const APP_ID = 'app_id';
  const ASSET_ID = 'asset_id';
  const ASSET_ID_LENGTH = mediamosa_db::HASH_ID_LENGTH;
  const MEDIAFILE_ID = 'mediafile_id';
  const MEDIAFILE_ID_LENGTH = mediamosa_db::HASH_ID_LENGTH;
  const MEDIAFILE_DEST = 'mediafile_dest';
  const MEDIAFILE_DEST_LENGTH = mediamosa_db::HASH_ID_LENGTH;
  const OWNER_ID = 'owner';
  const OWNER_ID_LENGTH = 255;
  const JOB_STATUS = 'status';
  const JOB_STATUS_WAITING = 'WAITING';
  const JOB_STATUS_INPROGRESS = 'INPROGRESS';
  const JOB_STATUS_FINISHED = 'FINISHED';
  const JOB_STATUS_FAILED = 'FAILED';
  const JOB_STATUS_CANCELLED = 'CANCELLED';
  const JOB_STATUS_CANCELLING = 'CANCELING';// Future exp; THIS IS NOT YET ADDED TO THE ENUM IN DB 'job' table.
  const PROGRESS = 'progress';
  const PROGRESS_LENGTH = 5;
  const PRIORITY = 'priority';
  const JOB_TYPE = 'job_type';
  const JOB_TYPE_TRANSCODE = 'TRANSCODE';
  const JOB_TYPE_RETRANSCODE = 'RETRANSCODE';
  const JOB_TYPE_STILL = 'STILL';
  const JOB_TYPE_UPLOAD = 'UPLOAD';
  const JOB_TYPE_ANALYSE = 'ANALYSE';
  const JOB_TYPE_DELETE_MEDIAFILE = 'DELETE_MEDIAFILE';
  const STARTED = 'started';
  const FINISHED = 'finished';
  const ERROR_DESCRIPTION = 'error_description';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const CREATE_STILL = 'create_still';
  const CREATE_STILL_TRUE = 'TRUE';
  const CREATE_STILL_FALSE = 'FALSE';
  const STILL_PARAMETERS = 'still_parameters';
  const HINT = 'hint';
  const HINT_TRUE = 'TRUE';
  const HINT_FALSE = 'FALSE';
  // TODO: Remove on the next release.
  const NO_HINT = 'no_hint';
  const NO_HINT_TRUE = 'TRUE';
  const NO_HINT_FALSE = 'FALSE';
}
