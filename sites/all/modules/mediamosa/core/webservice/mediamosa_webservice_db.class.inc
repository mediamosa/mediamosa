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
  * The DB class of webservice module.
  * previously known as webservice_management in 1.x
  */
class mediamosa_webservice_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_webservice';

  // Database fields;
  const ID = 'webservice_id'; // 1.x webservice_management_id
  const VERSION = 'version';
  const HANDLE = 'handle';
  const HANDLE_LENGTH = 50;
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 255;
  const STATUS = 'status';
  const STATUS_TRUE = 'TRUE';
  const STATUS_FALSE = 'FALSE';
}
