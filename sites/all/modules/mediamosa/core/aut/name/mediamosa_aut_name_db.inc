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
  *
  */

class mediamosa_aut_name_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_aut_name'; // 1.x aut_name table.

  // Primary KEY
  const ID = 'aut_name_id';

  // Database fields;
  const APP_ID = 'app_id';
  const AUT_GROUP_ID = 'aut_group_id';
  const AUT_NAME = 'aut_name';
  const AUT_NAME_LENGTH = 255;
  const AUT_PREFIX = 'aut_prefix';
  const AUT_PREFIX_LENGTH = 128;
  const AUT_TYPE = 'aut_type';
  const AUT_TYPE_DOMAIN = 'DOMAIN';
  const AUT_TYPE_REALM = 'REALM';
  const AUT_TYPE_USER = 'USER';
  const AUT_TYPE_USER_GROUP = 'USER_GROUP';
}
