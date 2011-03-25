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
 * Job statistics db.
 */
class mediamosa_statistics_job_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_statistics_job';

  // Database fields;
  const ID = 'id';
  const APP_ID = 'app_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const JOB_TYPE = 'job_type';
  const JOB_TYPE_TRANSCODE = 'TRANSCODE';
  const JOB_TYPE_RETRANSCODE = 'RETRANSCODE';
  const JOB_TYPE_STILL = 'STILL';
  const JOB_TYPE_UPLOAD = 'UPLOAD';
  const JOB_TYPE_ANALYSE = 'ANALYSE';
  const JOB_TYPE_DELETE_MEDIAFILE = 'DELETE_MEDIAFILE';
  const STARTED = 'started';
  const FINISHED = 'finished';
  const TRANSCODE_PROFILE_ID = 'transcode_profile_id';
  const NID = 'nid';
  const VID = 'vid';
  const FILESIZE = 'filesize';
}
