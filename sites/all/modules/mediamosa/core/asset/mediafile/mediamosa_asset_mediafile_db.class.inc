<?php
/**
 * @file
 * The database class of the asset_mediafile module.
 */

class mediamosa_asset_mediafile_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_asset_mediafile';

  // Database fields;
  const ID = 'mediafile_id';
  const ASSET_ID = 'asset_id';
  const MEDIAFILE_ID_SOURCE = 'mediafile_id_source';
  const TAG = 'tag';
  const TAG_LENGTH = 32;
  const APP_ID = 'app_id';
  const OWNER_ID = 'owner_id';
  const OWNER_ID_LENGTH = 255;
  const GROUP_ID = 'group_id';
  const GROUP_ID_LENGTH = 255;
  const IS_ORIGINAL_FILE = 'is_original_file';
  const IS_ORIGINAL_FILE_TRUE = 'TRUE';
  const IS_ORIGINAL_FILE_FALSE = 'FALSE';
  const IS_DOWNLOADABLE = 'is_downloadable';
  const IS_DOWNLOADABLE_TRUE = 'TRUE';
  const IS_DOWNLOADABLE_FALSE = 'FALSE';
  const IS_STREAMABLE = 'is_streamable';
  const IS_STREAMABLE_TRUE = 'TRUE';
  const IS_STREAMABLE_FALSE = 'FALSE';
  const IS_STILL = 'is_still';
  const IS_STILL_TRUE = 'TRUE';
  const IS_STILL_FALSE = 'FALSE';
  const FILENAME = 'filename';
  const FILENAME_LENGTH = 255;
  const URI = 'uri';
  const URI_LENGTH = 255;
  const SANNAS_MOUNT_POINT = 'sannas_mount_point';
  const TRANSCODE_PROFILE_ID = 'transcode_profile_id';
  const TOOL = 'tool';
  const TOOL_LENGTH = 10;
  const COMMAND = 'command';
  const FILE_EXTENSION = 'file_extension';
  const FILE_EXTENSION_LENGTH = 5;
  const IS_PROTECTED = 'is_protected';
  const IS_PROTECTED_FALSE = 'FALSE';
  const IS_PROTECTED_USER_USERGROUP = 'USER_USERGROUP';
  const IS_PROTECTED_DOMAIN_REALM = 'DOMAIN_REALM';
  const IS_PROTECTED_TRUE = 'TRUE';
  const CREATED = 'created';
  const CHANGED = 'changed';
  const TRANSCODE_INHERITS_ACL = 'transcode_inherits_acl';
  const TRANSCODE_INHERITS_ACL_TRUE = 'TRUE';
  const TRANSCODE_INHERITS_ACL_FALSE = 'FALSE';
}
