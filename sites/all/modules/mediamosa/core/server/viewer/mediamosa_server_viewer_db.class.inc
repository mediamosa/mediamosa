<?php
/**
 * @file
 * DB Class for mediamosa_server_viewer module.
 */

class mediamosa_server_viewer_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server_viewer';

  // Database fields;
  const NID_VIEWER = 'nid_viewer';
  const NID_SERVER = 'nid_server';
  const WEIGHT = 'weight';
  const STATUS = 'status';

  const STATUS_ENABLED = 'ENABLED';
  const STATUS_DISABLED = 'DISABLED';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_server_viewer.install.inc';

    // Get the schema.
    $schema = mediamosa_server_schema();
    return array_keys($schema[self::TABLE_NAME]['fields']);
  }
}
