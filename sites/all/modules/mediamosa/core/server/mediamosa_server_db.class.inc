<?php
/**
 * @file
 * DB Class for mediamosa_server module.
 */

class mediamosa_server_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server';

  // Database fields;
  const NID = 'nid';
  const VID = 'vid';
  const SERVER_STATUS = 'server_status';
  const SERVER_STATUS_ON = 'ON';
  const SERVER_STATUS_OFF = 'OFF';
  const SERVER_STATUS_CLOSE = 'CLOSE';
  const SERVER_TYPE = 'server_type';
  const MACHINE_NAME = 'machine_name';
  const MACHINE_NAME_LENGTH = 255;
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 255;
  const SERVER_URI = 'server_uri';
  const SERVER_URI_LENGTH = 255;
  const SERVER_PATH = 'server_path';
  const SERVER_PATH_LENGTH = 255;
  const SERVER_PATH_BASE_URI = '{base_uri}';
  const CONTAINERS = 'containers';
  const CONTAINERS_LENGTH = 1024;
  const MIME_TYPE = 'mime_type';
  const MIME_TYPE_LENGTH = 1024;
  const OBJECT_CODE = 'object_code';
  const SERVER_NAME = 'server_name';
  const SERVER_NAME_LENGTH = 255;
  const SLOTS = 'slots';
  const TOOLS = 'tools';
  const URI_UPLOAD_PROGRESS = 'uri_upload_progress';
  const URI_UPLOAD_PROGRESS_LENGTH = 255;
  const URI_UPLOAD_PROGRESS_SERVER_TYPE = 'uri_upload_progress_server_type';
  const URI_UPLOAD_PROGRESS_SERVER = 'uri_upload_progress_server';
  const URI_UPLOAD_PROGRESS_SERVER_LENGTH = 255;
  const VERSION = 'version';

  // Server types
  const SERVER_TYPE_DOWNLOAD = 'DOWNLOAD';
  const SERVER_TYPE_STILL = 'STILL';
  const SERVER_TYPE_STREAMING = 'STREAMING';
  const SERVER_TYPE_JOB_PROCESSOR = 'JOB_PROCESSOR';
  const SERVER_TYPE_UPLOAD = 'UPLOAD';

  const UPLOAD_PROGRESS_SERVER_TYPE_APC = 'APC';
  const UPLOAD_PROGRESS_SERVER_TYPE_PROXY = 'PROXY';

  /**
   * Returns all fields.
   */
  public static function get_fields() {
    require_once 'mediamosa_server.install.inc';

    // Get the schema.
    $schema = mediamosa_server_schema();
    return array_keys($schema[mediamosa_server_db::TABLE_NAME]['fields']);
  }
}
