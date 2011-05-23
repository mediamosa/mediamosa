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
 * Upload job table.
 */
class mediamosa_job_upload_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_job_upload'; // 1.x name upload_job.

  // Database fields;
  const ID = 'upload_job_id';
  const JOB_ID = 'job_id';
  const FILE_SIZE = 'file_size';
  const UPLOADED_FILE_SIZE = 'uploaded_file_size';
  const RETRANSCODE = 'retranscode';
  const RETRANSCODE_TRUE = 'TRUE';
  const RETRANSCODE_FALSE = 'FALSE';
  const CREATE_STILL = 'create_still';
  const CREATE_STILL_TRUE = 'TRUE';
  const CREATE_STILL_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';
}
