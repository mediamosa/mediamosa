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

class mediamosa_acl_app_master_slave_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_acl_app_master_slave';

  // Database fields;
  const ACL_OBJECT_ID = 'acl_object_id';
  const ACL_OBJECT_ID_LENGTH = 32;
  const ACL_OBJECT_TYPE = 'acl_object_type';
  // The object types;
  const ACL_OBJECT_TYPE_MEDIAFILE = 'MEDIAFILE';
  const ACL_OBJECT_TYPE_ASSET = 'ASSET';
  const ACL_OBJECT_TYPE_COLLECTION = 'COLLECTION';

  const APP_ID_SLAVE = 'app_id_slave'; // 1.x app_slave_id
  const APP_ID_MASTER = 'app_id_master'; // 1.x app_maser_id
}
