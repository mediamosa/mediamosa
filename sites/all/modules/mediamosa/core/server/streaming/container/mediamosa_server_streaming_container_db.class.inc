<?php
/**
 * @file
 * DB Class for mediamosa_server_streaming_container module.
 */

class mediamosa_server_streaming_container_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_server_streaming_container'; // 1.x name streaming_server_container.

  // Database fields;
  const ID = 'streaming_server_container_id';
  const NID = 'nid';
  const CONTAINER = 'container';
  const CONTAINER_LENGTH = 255;
}
