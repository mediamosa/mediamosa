<?php
// $Id$

/**
 * @file
 * The storage mapping DB class.
 */
class mediamosa_storage_mapping_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_storage_mapping';

  // Database fields;
  const ID = 'app_storage_id';
  const APP_ID = 'app_id';
  const PROFILE_ID = 'storage_profile_id';
  const PATH = 'path';
  const PATH_LENGTH = 100;
  const IS_DEFAULT_MAPPING = 'is_default_mapping';
  const IS_DEFAULT_MAPPING_TRUE = 'TRUE';
  const IS_DEFAULT_MAPPING_FALSE = 'FALSE';
  const CREATED = 'created';
  const CHANGED = 'changed';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_storage_mapping.install.inc';
    
    // Get the schema.
    $a_schema = mediamosa_storage_mapping_schema();
    return array_keys($a_schema[mediamosa_storage_mapping_db::TABLE_NAME]['fields']);
  }
}
