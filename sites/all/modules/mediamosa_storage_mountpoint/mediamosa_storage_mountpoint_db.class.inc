<?php
/**
 * @file
 * The mountpoint storage profile DB class.
 */

class mediamosa_storage_mountpoint_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_storage_profile_mountpoint';

  // Additional database fields besides the default onces;
  const NID = 'nid';
  const VID = 'vid';
  const ID = 'storage_profile_id';
  const MOUNTPOINT = 'mountpoint';
  const MOUNTPOINT_LENGTH = 100;
  const MOUNTPOINT_WINDOWS = 'mountpoint_windows';
  const MOUNTPOINT_WINDOWS_LENGTH = 100;

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_storage_mountpoint.install';

    // Get the schema.
    $schema = mediamosa_storage_mountpoint_schema();
    return array_keys($schema[mediamosa_storage_mountpoint_db::TABLE_NAME]['fields']);
  }
}
