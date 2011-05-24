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
  *
  */

class mediamosa_acl_name_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_acl_name'; // 1.x acl_name table.

  // Primary KEY
  const ID = 'acl_name_id';

  // Database fields;
  const APP_ID = 'app_id';
  const ACL_GROUP_ID = 'acl_group_id';
  const ACL_NAME = 'acl_name';
  const ACL_NAME_LENGTH = 255;
  const ACL_PREFIX = 'acl_prefix';
  const ACL_PREFIX_LENGTH = 128;
  const ACL_TYPE = 'acl_type';
  const ACL_TYPE_DOMAIN = 'DOMAIN';
  const ACL_TYPE_REALM = 'REALM';
  const ACL_TYPE_USER = 'USER';
  const ACL_TYPE_USER_GROUP = 'USER_GROUP';
}
