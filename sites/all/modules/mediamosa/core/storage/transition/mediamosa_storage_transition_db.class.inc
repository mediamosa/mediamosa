<?php
/**
 * @file
 * The storage transition DB class.
 *
 * The storage transition class handles the storages of all mediafiles on local
 * that required to be kept temporarily.
 */

class mediamosa_storage_transition_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_storage_transition';

  // Database fields;
  const MEDIAFILE_ID = 'mediafile_id';
  const MD5 = 'md5';
  const FILESIZE = 'filesize';
  const MD5_LENGTH = 32;
  const CREATED = 'created';
  const ACCESSED = 'accessed';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_storage_transition.install.inc';

    // Get the schema.
    $schema = mediamosa_storage_transition_schema();
    return array_keys($schema[mediamosa_storage_transition_db::TABLE_NAME]['fields']);
  }
}
