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
 * Used diskspace db.
 */
class mediamosa_statistics_diskspace_used_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_statistics_diskspace_used'; // 1.x name statistics_diskspace_used.

  // Database fields;
  const ID = 'id';
  const APP_ID = 'app_id';
  const TYPE = 'type';
  const TYPE_CONTAINER = 'container';
  const TYPE_GROUP = 'group';
  const TYPE_USER = 'user';
  const TYPE_EUA = 'eua';
  const KEYWORD = 'keyword';
  const KEYWORD_LENGTH = 100;
  const CONTAINER_TYPE = 'container_type';
  const CONTAINER_TYPE_LENGTH = 50;
  const DISKSPACE = 'diskspace';
  const TIMESTAMP = 'timestamp';
}
