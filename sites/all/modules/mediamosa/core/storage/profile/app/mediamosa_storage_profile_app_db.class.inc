<?php
/**
 * @file
 * MediaMosa storage profile application.
 *
 * The storage profile application is the link between the client application
 * and the storage profile. It also stores specific data for this client
 * application.
 */

class mediamosa_storage_profile_app_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_storage_profile_app';

  // Database fields;
  const ID = 'app_storage_id';
  const APP_ID = 'app_id';
  const PROFILE_ID = 'storage_profile_id';
  const PROFILE_ID_STILLS = 'storage_stills_profile_id';
  const PATH = 'path';
  const PATH_LENGTH = 100;
  const PATH_STILLS = 'path_stills';
  const PATH_STILLS_LENGTH = 100;
  const CREATED = 'created';
  const CHANGED = 'changed';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_storage_profile_app.install.inc';

    // Get the schema.
    $schema = mediamosa_storage_profile_app_schema();
    return array_keys($schema[mediamosa_storage_profile_app_db::TABLE_NAME]['fields']);
  }
}
