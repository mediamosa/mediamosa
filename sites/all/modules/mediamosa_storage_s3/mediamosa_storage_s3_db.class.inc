<?php
// $Id$

/**
 * @file
 * The S3 storage profile DB class.
 */
class mediamosa_storage_s3_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_storage_profile_s3';

  // Additional database fields besides the default onces;
  const KEY = 'aws_key';
  const KEY_LENGTH = 20;
  const SECRET = 'aws_secret';
  const SECRET_LENGTH = 40;
  //const CLOUDFRONT_KEYPAIR = 'aws_cloudfront_keypair';
  //const CLOUDFRONT_KEYPAIR_LENGTH = 40;
  
  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_storage_s3.install';
    
    // Get the schema.
    $a_schema = mediamosa_storage_s3_schema();
    return array_keys($a_schema[mediamosa_storage_s3_db::TABLE_NAME]['fields']);
  }
}
