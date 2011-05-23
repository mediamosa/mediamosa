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
 * App database class object.
 */
class mediamosa_app_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_app';

  // Database fields;
  const NID = 'nid';  // 1.x - caid
  const VID = 'vid';
  const APP_ID = 'app_id'; // 1.x id (this is the APP_ID)
  const APP_NAME = 'app_name';
  const APP_NAME_LENGTH = 30;
  const TRANSCODING_POSITION_FROM = 'transcoding_position_from';
  const TRANSCODING_POSITION_TO = 'transcoding_position_to';
  const QUOTA = 'quota';
  const QUOTA_USED = 'quota_used';
  const SHARED_KEY = 'shared_key';
  const SHARED_KEY_LENGTH = 100;
  const ACTIVE = 'active';
  const ACTIVE_TRUE = 'TRUE';
  const ACTIVE_FALSE = 'FALSE';
  const ALWAYS_HINT_MP4 = 'always_hint_mp4';
  const ALWAYS_HINT_MP4_TRUE = 'TRUE';
  const ALWAYS_HINT_MP4_FALSE = 'FALSE';
  const ALWAYS_INSERT_MD = 'always_insert_md';
  const ALWAYS_INSERT_MD_TRUE = 'TRUE';
  const ALWAYS_INSERT_MD_FALSE = 'FALSE';
  const IS_METADATA_PUBLIC = 'is_metadata_public';
  const IS_METADATA_PUBLIC_TRUE = 'TRUE';
  const IS_METADATA_PUBLIC_FALSE = 'FALSE';
  const SHOW_EXTERNAL_ASSETS = 'show_external_assets';
  const SHOW_EXTERNAL_ASSETS_TRUE = 'TRUE';
  const SHOW_EXTERNAL_ASSETS_FALSE = 'FALSE';
  const OAUTH_CONSUMER_KEY = 'oauth_consumer_key';
  const OAUTH_CONSUMER_KEY_LENGTH = 255;
  const OAUTH_CONSUMER_SECRET = 'oauth_consumer_secret';
  const OAUTH_CONSUMER_SECRET_LENGTH = 255;
  const ALLOW_MASTERSLAVE_APPS = 'allow_masterslave_apps';
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 255;
  const OWNER = 'owner';
  const OWNER_LENGTH = 50;
  const TECHNICAL_NAME = 'technical_name';
  const TECHNICAL_NAME_LENGTH = 100;
  const TECHNICAL_PHONE = 'technical_phone';
  const TECHNICAL_PHONE_LENGTH = 40;
  const TECHNICAL_EMAIL = 'technical_email';
  const TECHNICAL_EMAIL_LENGTH = 50;
  const ADMINISTRATIVE_NAME = 'administrative_name';
  const ADMINISTRATIVE_NAME_LENGTH = 100;
  const ADMINISTRATIVE_PHONE = 'administrative_phone';
  const ADMINISTRATIVE_PHONE_LENGTH = 40;
  const ADMINISTRATIVE_EMAIL = 'administrative_email';
  const ADMINISTRATIVE_EMAIL_LENGTH = 50;
  const PLAY_PROXY_URL = 'play_proxy_url';
  const PLAY_PROXY_URL_LENGTH = 255;
  const VIEW_ASSET_URL = 'view_asset_url';
  const VIEW_ASSET_URL_LENGTH = 255;
  const PREVIEW_PROFILE_ID = 'preview_profile_id';
  const DOWNLOAD_URL = 'download_url';
  const DOWNLOAD_URL_LENGTH = 255;
  const STREAM_URL = 'stream_url';
  const STREAM_URL_LENGTH = 255;
  const STILL_URL = 'still_url';
  const STILL_URL_LENGTH = 255;
  const ACTIVE_VERSION = 'active_version';
  const ACTIVE_VERSION_LENGTH = 25;
  const TRANSCODING_MAX_SLOTS = 'transcoding_max_slots';
  const TIMEZONE = 'timezone';
  const TIMEZONE_LENGTH = 32;
  const STILL_DEFAULT_SIZE = 'still_default_size';
  const STILL_DEFAULT_SIZE_LENGTH = 25;
  const STILL_PADDING = 'still_padding';
  const STILL_PADDING_YES = 'YES';
  const STILL_PADDING_NO = 'NO';
  const STILL_MAINTAIN_ASPECT_RATIO = 'still_maintain_aspect_ratio';
  const STILL_MAINTAIN_ASPECT_RATIO_YES = 'YES';
  const STILL_MAINTAIN_ASPECT_RATIO_NO = 'NO';
  const TOOL_ANALYSE = 'tool_analyse';

  // ------------------------------------------------------------------ Functions.
  /**
   * Returns all fields.
   */
  static public function get_fields() {
    require_once 'mediamosa_app.install.inc';

    // Get the schema.
    $a_schema = mediamosa_app_schema();
    return array_keys($a_schema[mediamosa_app_db::TABLE_NAME]['fields']);
  }
}
