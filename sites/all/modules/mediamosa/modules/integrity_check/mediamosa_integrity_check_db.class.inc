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

class mediamosa_integrity_check_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_integrity_check'; // 1.x name log_integrity_check.

  // Database fields;
  const ID = 'id';
  const TYPE = 'type';
  const TYPE_ERROR = 'error';
  const TYPE_MISSING_MEDIAFILE = 'MISSING MEDIAFILE';
  const TYPE_MISSING_MEDIARECORD = 'MISSING MEDIARECORD';
  const TYPE_MISSING_STILLFILE = 'MISSING STILLFILE';
  const TYPE_MISSING_STILLRECORD = 'MISSING STILLRECORD';
  const OBJECT_ID = 'object_id';
  const OBJECT_ID_LENGTH = mediamosa_db::HASH_ID_LENGTH;
  const APP_ID = 'app_id';
  const OWNER_ID = 'owner_id';
  const OWNER_ID_LENGTH = 255;
  const CREATED = 'created';
  const CHANGED = 'changed';
  const SIZE = 'size';
  const MTIME = 'mtime';
  const CTIME = 'ctime';
  const DETAILS = 'details';
  const DETAILS_LENGTH = 1000;
}
