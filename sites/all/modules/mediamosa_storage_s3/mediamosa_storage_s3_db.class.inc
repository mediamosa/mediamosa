<?php
/**
 * @file
 * The S3 storage profile DB class.
 */

class mediamosa_storage_s3_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_storage_profile_s3';

  // Additional database fields besides the default onces;
  const NID = 'nid';
  const VID = 'vid';
  const PROFILE_ID = 'storage_profile_id';
  const KEY = 'aws_key';
  const KEY_LENGTH = 20;
  const SECRET = 'aws_secret';
  const SECRET_LENGTH = 40;

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_storage_s3.install';

    // Get the schema.
    $schema = mediamosa_storage_s3_schema();
    return array_keys($schema[mediamosa_storage_s3_db::TABLE_NAME]['fields']);
  }
}
