<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * DB Class for mediamosa_server_upload module.
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
  const DESCRIPTION = 'description';
  const DESCRIPTION_LENGTH = 255;
  const SERVER_URI = 'server_uri'; // was in 2.1.5 called 'uri'.
  const SERVER_URI_LENGTH = 255;
  const SERVER_PATH = 'server_path';
  const SERVER_PATH_LENGTH = 255;
  const SERVER_PATH_BASE_URI = '{base_uri}';
  const CONTAINERS = 'containers'; // Now TEXT type.
  const CONTAINERS_LENGTH = 1024;
  const MIME_TYPE = 'mime_type';
  const MIME_TYPE_LENGTH = 1024;
  const OBJECT_CODE = 'object_code';
  const SERVER_NAME = 'server_name';
  const SERVER_NAME_LENGTH = 255;
  const SLOTS = 'slots';
  const TOOLS = 'tools'; // Now TEXT type.
  const TOOLS_LENGTH = 1024;
  const URI_UPLOAD_PROGRESS = 'uri_upload_progress'; // client
  const URI_UPLOAD_PROGRESS_LENGTH = 255;
  const URI_UPLOAD_PROGRESS_SERVER_TYPE = 'uri_upload_progress_server_type';
  const URI_UPLOAD_PROGRESS_SERVER = 'uri_upload_progress_server';
  const URI_UPLOAD_PROGRESS_SERVER_LENGTH = 255;
  const VERSION = 'version';

  // Server types
  const SERVER_TYPE_DOWNLOAD = 'download';
  const SERVER_TYPE_STILL = 'still';
  const SERVER_TYPE_STREAMING = 'streaming';
  const SERVER_TYPE_JOB_PROCESSOR = 'job_processor';
  const SERVER_TYPE_UPLOAD = 'upload';

  const UPLOAD_PROGRESS_SERVER_TYPE_APC = 'APC';
  const UPLOAD_PROGRESS_SERVER_TYPE_PROXY = 'PROXY';

  /**
   * Returns all fields.
   */
  static public function get_fields() {
    require_once 'mediamosa_server.install.inc';

    // Get the schema.
    $a_schema = mediamosa_server_schema();
    return array_keys($a_schema[mediamosa_server_db::TABLE_NAME]['fields']);
  }
}
