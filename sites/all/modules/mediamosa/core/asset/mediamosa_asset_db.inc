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
  * The database class of the asset module.
  */

class mediamosa_asset_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset'; // 1.x name asset.

  // Database fields;
  const ID = 'asset_id';
  const PARENT_ID = 'parent_id';
  const APP_ID = 'app_id';
  const PROVIDER_ID = 'provider_id';
  const PROVIDER_ID_LENGTH = 255;
  const OWNER_ID = 'owner_id';
  const OWNER_ID_LENGTH = 255;
  const GROUP_ID = 'group_id';
  const GROUP_ID_LENGTH = 255;
  const VIDEOTIMESTAMP = 'videotimestamp';
  const VIDEOTIMESTAMPMODIFIED = 'videotimestampmodified';
  const PLAY_RESTRICTION_START = 'play_restriction_start';
  const PLAY_RESTRICTION_END = 'play_restriction_end';
  const REFERENCE_ID = 'reference_id';
  const REFERENCE_ID_LENGTH = 255;
  const ISPRIVATE = 'isprivate';
  const ISPRIVATE_TRUE = 'TRUE';
  const ISPRIVATE_FALSE = 'FALSE';
  const MEDIAFILE_DURATION = 'mediafile_duration';
  const MEDIAFILE_DURATION_LENGTH = 11;
  const MEDIAFILE_CONTAINER_TYPE = 'mediafile_container_type';
  const MEDIAFILE_CONTAINER_TYPE_LENGTH = 50;
  const CREATED = 'created';
  const CHANGED = 'changed';
  const IS_UNAPPROPRIATE = 'is_unappropriate';
  const IS_UNAPPROPRIATE_TRUE = 'TRUE';
  const IS_UNAPPROPRIATE_FALSE = 'FALSE';
  const IS_EXTERNAL = 'is_external';
  const IS_EXTERNAL_TRUE = 'TRUE';
  const IS_EXTERNAL_FALSE = 'FALSE';
  const VIEWED = 'viewed';
  const PLAYED = 'played';
  const IS_EMPTY_ASSET = 'is_empty_asset';
  const IS_EMPTY_ASSET_TRUE = 'TRUE';
  const IS_EMPTY_ASSET_FALSE = 'FALSE';
  const IS_PROTECTED = 'is_protected';
  const IS_PROTECTED_TRUE = 'TRUE';
  const IS_PROTECTED_FALSE = 'FALSE';
  const IS_PROTECTED_USER_USERGROUP = 'USER_USERGROUP';
  const IS_PROTECTED_DOMAIN_REALM = 'DOMAIN_REALM';
}
