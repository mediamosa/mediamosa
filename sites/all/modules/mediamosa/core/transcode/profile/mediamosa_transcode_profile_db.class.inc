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
 * The transcode profile DB class.
 */
class mediamosa_transcode_profile_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_transcode_profile';

  // Database fields;
  const NID = 'nid';
  const VID = 'vid';
  const ID = 'transcode_profile_id';
  const APP_ID = 'app_id';
  const VERSION = 'version';
  const PROFILE = 'profile';
  const PROFILE_LENGTH = 100;
  const IS_DEFAULT_PROFILE = 'is_default_profile';
  const IS_DEFAULT_PROFILE_TRUE = 'TRUE';
  const IS_DEFAULT_PROFILE_FALSE = 'FALSE';
  const TOOL = 'tool';
  const TOOL_LENGTH = 10;
  const FILE_EXTENSION = 'file_extension';
  const FILE_EXTENSION_LENGTH = 4;
  const COMMAND = 'command';
  const COMMAND_LENGTH = 1000; // used by REST call.
  const CREATED = 'created';
  const CHANGED = 'changed';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_transcode_profile.install.inc';

    // Get the schema.
    $a_schema = mediamosa_transcode_profile_schema();
    return array_keys($a_schema[mediamosa_transcode_profile_db::TABLE_NAME]['fields']);
  }
}
