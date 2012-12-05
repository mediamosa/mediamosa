<?php
// $Id$

/**
 * @file
 * The storage profile DB class.
 */
class mediamosa_storage_profile_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_storage_profile';

  // Database fields;
  const NID = 'nid';
  const VID = 'vid';
  const ID = 'storage_profile_id';
  const APP_ID = 'app_id';
  const VERSION = 'version';
  const PROFILE = 'profile';
  const PROFILE_LENGTH = 100;
  const TYPE = 'storage_profile_type';
  const TYPE_LENGTH = 100;
  const IS_DEFAULT_PROFILE = 'is_default_profile';
  const IS_DEFAULT_PROFILE_TRUE = 'TRUE';
  const IS_DEFAULT_PROFILE_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_storage_profile.install.inc';
    
    // Get the schema.
    $a_schema = mediamosa_storage_profile_schema();
    return array_keys($a_schema[mediamosa_storage_profile_db::TABLE_NAME]['fields']);
  }
}
