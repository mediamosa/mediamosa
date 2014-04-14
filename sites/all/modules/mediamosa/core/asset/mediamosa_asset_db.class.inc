<?php
/**
 * @file
 * The database class of the asset module.
 */

class mediamosa_asset_db {
  // ------------------------------------------------------------------- Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset';

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
  const ASSET_TYPE = 'asset_type';
  const ASSET_TYPE_LENGTH = 50;
  const MEDIAFILE_DURATION = 'mediafile_duration';
  const MEDIAFILE_DURATION_LENGTH = 11;
  const MEDIAFILE_CONTAINER_TYPE = 'mediafile_container_type';
  const MEDIAFILE_CONTAINER_TYPE_LENGTH = 50;
  const MEDIAFILE_MIME_TYPE = 'mediafile_mime_type';
  const MEDIAFILE_MIME_TYPE_LENGTH = 80;
  const MEDIAFILE_FILENAME = 'mediafile_filename';
  const MEDIAFILE_FILENAME_LENGTH = 255;
  const CREATED = 'created';
  const CHANGED = 'changed';
  const IS_INAPPROPRIATE = 'is_unappropriate';
  const IS_INAPPROPRIATE_TRUE = 'TRUE';
  const IS_INAPPROPRIATE_FALSE = 'FALSE';
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
