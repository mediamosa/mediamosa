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
 * DB class for statistics rest log.
 */
class mediamosa_statistics_rest_log_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_statistics_rest_log'; // 1.x name statistics_rest_log.

  // Database fields;
  const ID = 'id';
  const REQUEST = 'request'; // 1x: varchar, 2.x text.
  const PROCESS_TIME = 'process_time';
  const QUERY_COUNT = 'query_count';
  const IP_ADDRESS = 'ip_address';
  const IP_ADDRESS_LENGTH = 15;
  const DATE = 'date';
  const APP_ID = 'app_id';
  const APP_ID_DEFAULT = 0;
}
