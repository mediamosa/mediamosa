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
 * Transcode job table.
 */
class mediamosa_job_transcode_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job_transcode'; // 1.x name transcode_job.

  // Database fields;
  const ID = 'transcode_job_id';
  const JOB_ID = 'job_id';
  const TRANSCODE_PROFILE_ID = 'transcode_profile_id';
  const TOOL = 'tool';
  const TOOL_LENGTH = 10;
  const COMMAND = 'command';
  const FILE_EXTENSION = 'file_extension';
  const FILE_EXTENSION_LENGTH = 5;
  const CREATED = 'created';
  const CHANGED = 'changed';
  const COMPLETED_TRANSCODING_URL = 'completed_transcoding_url';
}
