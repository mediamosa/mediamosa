<?php
/**
 * @file
 * DB Class for mediamosa_viewer module.
 */

class mediamosa_viewer_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_viewer';

  // Database fields;
  const NID = 'nid';
  const VID = 'vid';
  const NAME = 'name';
  const NAME_LENGTH = 80;
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 255;
  const OBJECT_CODE = 'object_code';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_viewer.install.inc';

    // Get the schema.
    $schema = mediamosa_viewer_schema();
    return array_keys($schema[mediamosa_viewer_db::TABLE_NAME]['fields']);
  }
}
