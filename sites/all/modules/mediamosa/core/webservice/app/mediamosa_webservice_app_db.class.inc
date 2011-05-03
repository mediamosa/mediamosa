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
  * Webservice APP db class.
  */

/**
  * @file
  * The DB class of webservice module.
  * previously known as webservice_management in 1.x
  *
  * FIXME: tables are linked with each other using varchar(50) (handle)
  *        This should be the unique ID of webservice. Handle should be
  *        used as ID for webservice and not stored in the _app table.
  *
  */
class mediamosa_webservice_app_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_webservice_app';

  // Database fields;
  const ID = 'webservice_app_id';  // 1.x - wmcaid
  const WEBSERVICE_HANDLE = 'webservice_handle'; // 1.x - webservice_management_handle, links with ID of webservice. (see FIXME)
  const WEBSERVICE_HANDLE_LENGTH = 50;
  const APP_ID = 'app_id'; // 1.x - caid.
  const STATUS = 'status';
  const STATUS_TRUE = 'TRUE';
  const STATUS_FALSE = 'FALSE';
}
