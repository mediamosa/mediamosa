<?php
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
  const MACHINE_NAME = 'machine_name';
  const MACHINE_NAME_LENGTH = 255;
  const ID = 'transcode_profile_id';
  const APP_ID = 'app_id';
  const VERSION = 'version';
  const PROFILE = 'profile';
  const PROFILE_LENGTH = 100;
  const IS_DEFAULT_PROFILE = 'is_default_profile';
  const IS_DEFAULT_PROFILE_TRUE = 'TRUE';
  const IS_DEFAULT_PROFILE_FALSE = 'FALSE';
  const TOOL = 'tool';
  const TOOL_LENGTH = 50;
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
