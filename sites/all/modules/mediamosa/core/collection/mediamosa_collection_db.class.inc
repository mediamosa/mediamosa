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
  * The database class of the asset module.
  */

class mediamosa_collection_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_collection'; // 1.x name collection.

  // Database fields;
  const ID = 'coll_id';
  const APP_ID = 'app_id';
  const TITLE = 'title';
  const TITLE_LENGTH = 255;
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 1000;
  const OWNER_ID = 'owner_id';
  const OWNER_ID_LENGTH = 255;
  const GROUP_ID = 'group_id';
  const GROUP_ID_LENGTH = 255;
  const ISPRIVATE = 'isprivate';
  const ISPRIVATE_TRUE = 'TRUE';
  const ISPRIVATE_FALSE = 'FALSE';
  const _PRIVATE = 'private';
  const PRIVATE_ = 'private';
  const PRIVATE_TRUE = 'TRUE';
  const PRIVATE_FALSE = 'FALSE';
  const _PUBLIC = 'public';
  const PUBLIC_ = 'public';
  const PUBLIC_TRUE = 'TRUE';
  const PUBLIC_FALSE = 'FALSE';
  const CATEGORY = 'category';
  const CATEGORY_TRUE = 'TRUE';
  const CATEGORY_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const PUBLIC_ASSIGN = 'public_assign';
  const PUBLIC_ASSIGN_TRUE = 'TRUE';
  const PUBLIC_ASSIGN_FALSE = 'FALSE';
  const IS_UNAPPROPRIATE = 'is_unappropriate';
  const IS_UNAPPROPRIATE_TRUE = 'TRUE';
  const IS_UNAPPROPRIATE_FALSE = 'FALSE';
}
