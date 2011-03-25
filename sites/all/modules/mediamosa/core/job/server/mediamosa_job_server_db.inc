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
 * DB class jobserver.
 */

class mediamosa_job_server_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job_server'; // 1.x name jobserver_job.

  // Database fields;
  const ID = 'jobserver_job_id';
  const INSTALL_ID = 'install_id'; // 2.0 new
  const INSTALL_ID_LENGTH = 16;
  const JOB_ID = 'job_id';
  const JOB_STATUS = 'status';
  const JOB_STATUS_WAITING = 'WAITING';
  const JOB_STATUS_INPROGRESS = 'INPROGRESS';
  const JOB_STATUS_FINISHED = 'FINISHED';
  const JOB_STATUS_FAILED = 'FAILED';
  const JOB_STATUS_CANCELLED = 'CANCELLED';
  const PROGRESS = 'progress';
  const PROGRESS_LENGTH = 5;
  const JOB_TYPE = 'job_type';
  const JOB_TYPE_TRANSCODE = 'TRANSCODE';
  const JOB_TYPE_STILL = 'STILL';
  const JOB_TYPE_UPLOAD = 'UPLOAD';
  const JOB_TYPE_ANALYSE = 'ANALYSE';
  const JOB_TYPE_DELETE_MEDIAFILE = 'DELETE_MEDIAFILE';
  const PROGRESS_ID = 'progress_id';// @todo: Seems unused.
  const MEDIAFILE_ID_SRC = 'mediafile_src';
  const MEDIAFILE_ID_SRC_LENGTH = 32;
  const MEDIAFILE_DEST = 'mediafile_dest';
  const STARTED = 'started';
  const FINISHED = 'finished';
  const ERROR_DESCRIPTION = 'error_description';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
