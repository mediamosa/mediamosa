<?php
/**
 * @file
 * DB Class for mediamosa_viewer_app module.
 */

class mediamosa_viewer_app_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_viewer_app';

  // Database fields;
  const APP_ID = 'app_id';
  const NID_SERVER = 'nid_server';
  const NID_VIEWER = 'nid_viewer';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_viewer.install.inc';

    // Get the schema.
    $schema = mediamosa_viewer_schema();
    return array_keys($schema[mediamosa_viewer_app_db::TABLE_NAME]['fields']);
  }
}
