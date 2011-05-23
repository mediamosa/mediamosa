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

class mediamosa_acl_group_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_acl_group'; // 1.x acl_group table.

  // Primary KEY
  const ID = 'acl_group_id';

  // Database fields;
  const APP_ID = 'app_id';
  const ACL_GROUP_NAME = 'acl_group_name';
  const ACL_GROUP_NAME_LENGTH = 128;
  const ACL_GROUP_TYPE = 'acl_group_type';
  const ACL_GROUP_TYPE_REALM = 'REALM';
  const ACL_GROUP_TYPE_DOMAIN = 'DOMAIN';
}
