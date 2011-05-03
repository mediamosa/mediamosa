<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Upload server functions.
 */

class mediamosa_server {
  // ------------------------------------------------------------------ Consts.
  /**
   * Mime type separator.
   */
  const MIME_TYPE_SEPARATOR = '|';

  /**
   * Log for asset_mediafile_uplaod.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'server');
  }

  /**
   * Trigger process servers.
   */
  static public function trigger($call = '/internal/cron/jobserver') {

    // Get all enabled transcode servers (ON / CLOSE).
    $servers = self::get_enabled_job_processor();

    $a_triggered = array();
    foreach ($servers as $server) {

      // Get URL.
      $url = mediamosa_http::uri2url($server[mediamosa_server_db::SERVER_URI]) . $call;

      // Prevent multiple triggers if url is on more than one server.
      if (in_array($url, $a_triggered)) {
        continue;
      }

      // Mark triggered.
      $a_triggered[] = $url;

      // Log it.
      mediamosa_debug::log('Triggering job server @ @uri', array('@uri' => $url));

      // Dont trigger in sandbox.
      if (mediamosa::in_simpletest_sandbox()) {
        // So we wait till finished.
        MediaMosaTestCase::staticInternalRestCallGet($url);
      }
      else {
        // Trigger.
        mediamosa_http::do_head_internal_call($url);
      }
    }
  }

  /**
   * Get random upload server.
   */
  static public function get_random_upload() {
    return mediamosa_db::db_query_range(
      'SELECT * FROM {#mediamosa_server} WHERE #server_type = :server_type AND #status = :status_on ORDER BY RAND()',
      0,
      1,
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#status' => mediamosa_server_db::SERVER_STATUS,
        ':status_on' => mediamosa_server_db::SERVER_STATUS_ON,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_UPLOAD,
      )
    )->fetchAssoc();
  }

  /**
   * Get random upload server.
   */
  static public function get_upload_by_nid($nid) {
    return mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_server} WHERE #nid = :nid AND #status = :status_on',
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#status' => mediamosa_server_db::SERVER_STATUS,
        ':status_on' => mediamosa_server_db::SERVER_STATUS_ON,
        '#nid' => mediamosa_server_db::NID,
        ':nid' => $nid,
      )
    )->fetchAssoc();
  }

  /**
   * Returns status information from all enabled servers.
   *
   * @param array $server_types (optional)
   *  The server types to get status from.
   */
  static public function get_status_enabled(array $server_types = array()) {

    // Get the enabled servers.
    $servers = mediamosa_server::get_enabled($server_types);

    $result = array();
    foreach ($servers as $server) {

      $uri = mediamosa_http::expect_url($server[mediamosa_server_db::SERVER_URI]);

      $response = mediamosa_http::do_internal_call($uri, 'server/status?include_status=TRUE');
      if ($response->code != '200') {
        self::log('Call to server/status did not return a 200 result (header: @header)', array('@header' => var_export($response, TRUE)));
      }

      if (!empty($response->data)) {
        $xml = new mediamosa_connector_response($response->data);

        $result[] = array(
          'enabled_server' => $server,
          'status' => (isset($xml->items->item) ? mediamosa_lib::simplexml2array($xml->items->item) : array()),
        );
      }
    }

    return $result;
  }

  /**
   * Returns status information from all enabled job processor servers.
   */
  static public function get_status_enabled_job_processor() {
    return self::get_status_enabled(array(mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR));
  }

  /**
   * Trigger cleanup of the temp. stuff on the server generated by the simpletests.
   */
  static public function cleanup_simpletest_enabled_servers($server_types = array(mediamosa_server_db::SERVER_TYPE_DOWNLOAD, mediamosa_server_db::SERVER_TYPE_STILL, mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR, mediamosa_server_db::SERVER_TYPE_UPLOAD)) {
    $servers = mediamosa_server::get_enabled($server_types);
    foreach ($servers as $server) {
      $uri = mediamosa_http::expect_url($server[mediamosa_server_db::SERVER_URI]);

      // FIXME: need fix for uri.
      mediamosa_http::do_head_internal_call($uri . 'internal/simpletest/cleanup');
    }

    // Also trigger on the main Internal REST Call URL.
    mediamosa_http::do_internal_call_cron('internal/simpletest/cleanup', TRUE);
  }

  /**
   * Get all enabled servers (either ON or CLOSE) based on given type.
   *
   * @param array $server_types (optional)
   *  The types to retrieve.
   */
  static public function get_enabled(array $server_types = array()) {
    $query = mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms')
      ->fields('ms');

    // Any specific types?
    if (!empty($server_types)) {
      $query->condition(mediamosa_server_db::SERVER_TYPE, $server_types, 'IN');
    }

    return $query
      ->condition(mediamosa_server_db::SERVER_STATUS, mediamosa_server_db::SERVER_STATUS_OFF, '!=')
      ->orderBy(mediamosa_server_db::SERVER_TYPE)
      ->execute();
  }

  /**
   * Get all enabled processing servers (either ON or CLOSE).
   */
  static public function get_enabled_job_processor($server_types = array()) {
    return self::get_enabled(array(mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR));
  }

  /**
   * Get all enabled download servers.
   */
  static public function get_enabled_download() {
    return self::get_enabled(array(mediamosa_server_db::SERVER_TYPE_DOWNLOAD));
  }

  /**
   * Get all enabled still servers.
   */
  static public function get_enabled_still() {
    return self::get_enabled(array(mediamosa_server_db::SERVER_TYPE_STILL));
  }

  /**
   * Return streamable servers.
   *
   * @param $container_type
   */
  static public function is_streamable($container_type) {
    static $streamable_container_types = array();

    if (!isset($streamable_container_types[$container_type])) {
      $is_streamable = mediamosa_db::db_query(
        "SELECT COUNT(ms.#nid) AS total FROM {#mediamosa_server} AS ms
         JOIN {#mediamosa_streaming_server_container} AS mssc ON ms.#nid = mssc.#nid
         WHERE mssc.#container = :container AND ms.#server_type = :server_type AND ms.#server_status = :server_status_on",
        array(
          '#nid' => mediamosa_server_db::NID,
          '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
          '#mediamosa_streaming_server_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
          '#container' => mediamosa_server_streaming_container_db::CONTAINER,
          ':container' => $container_type,
          '#server_type' => mediamosa_server_db::SERVER_TYPE,
          ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
          '#server_status' => mediamosa_server_db::SERVER_STATUS,
          ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
        )
      )->fetchField();

      $streamable_container_types[$container_type] = $is_streamable ? TRUE : FALSE;
    }

    return ($streamable_container_types[$container_type]);
  }

  /**
   * Find the streaming server based on container and codec.
   *
   * @param string $container_type
   *  Container type like mp4;mpg etc.
   * @param string $video_codec
   *  Codec.
   * @param string $mime_type
   *  Mime type.
   */
  static public function get_streaming($container_type, $video_codec, $mime_type = NULL) {
    static $servers = array();
    static $servers_mime = array();

    // Container + codec.

    if (isset($servers[$container_type . ':' . $video_codec]) && $servers[$container_type . ':' . $video_codec]) {
      return $servers[$container_type . ':' . $video_codec];
    }

    // Choose a random available streaming server.
    // try to match container:codec first.
    $servers[$container_type . ':' . $video_codec] = mediamosa_db::db_query(
      "SELECT mss.* FROM {#mediamosa_server} AS mss
      JOIN {#mediamosa_server_streaming_container} AS mssc ON mssc.#nid = mss.#nid
      WHERE mssc.#container = :container AND mss.#server_type = :server_type AND mss.#server_status = :server_status_on ORDER BY rand() LIMIT 1",
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#nid' => mediamosa_server_db::NID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $container_type . ':' . $video_codec,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
        '#server_status' => mediamosa_server_db::SERVER_STATUS,
        ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
      )
    )->fetchAssoc();

    if ($servers[$container_type . ':' . $video_codec] && $servers[$container_type . ':' . $video_codec]) {
      return $servers[$container_type . ':' . $video_codec];
    }

    // Container.

    if (isset($servers[$container_type]) && $servers[$container_type]) {
      return $servers[$container_type];
    }

    $servers[$container_type] = mediamosa_db::db_query(
      "SELECT mss.* FROM {#mediamosa_server} AS mss
      JOIN {#mediamosa_server_streaming_container} AS mssc ON mssc.#nid = mss.#nid
      WHERE mssc.#container = :container AND mss.#server_type = :server_type AND mss.#server_status = :server_status_on ORDER BY rand() LIMIT 1",
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
        '#nid' => mediamosa_server_db::NID,
        '#container' => mediamosa_server_streaming_container_db::CONTAINER,
        ':container' => $container_type,
        '#server_status' => mediamosa_server_db::SERVER_STATUS,
        ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
      )
    )->fetchAssoc();

    if (isset($servers[$container_type]) && $servers[$container_type]) {
      return $servers[$container_type];
    }

    // Mime type.

    if (!$mime_type) {
      return FALSE;
    }

    $mime_type_main = explode('/', $mime_type);
    $mime_type_main = $mime_type_main[0] . '/*';

    if (isset($servers_mime[$mime_type]) && $servers_mime[$mime_type]) {
      return $servers_mime[$mime_type];
    }

    if (isset($servers_mime[$mime_type_main]) && $servers_mime[$mime_type_main]) {
      return $servers_mime[$mime_type_main];
    }

    $query = mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms');
    $query->fields('ms', array());
    $result = $query->execute();
    foreach ($result as $record) {
      if ($record[mediamosa_server_db::MIME_TYPE]) {
        if (strpos($record[mediamosa_server_db::MIME_TYPE], self::MIME_TYPE_SEPARATOR) === FALSE) {
          $servers_mime[$record[mediamosa_server_db::MIME_TYPE]] = $record;
        }
        else {
          $servers_mime_types = explode(self::MIME_TYPE_SEPARATOR, $record[mediamosa_server_db::MIME_TYPE]);
          foreach ($servers_mime_types as $servers_mime_type) {
            $servers_mime[$servers_mime_type] = $record;
          }
        }
      }
    }

    if (isset($servers_mime[$mime_type]) && $servers_mime[$mime_type]) {
      return $servers_mime[$mime_type];
    }

    if (isset($servers_mime[$mime_type_main]) && $servers_mime[$mime_type_main]) {
      return $servers_mime[$mime_type_main];
    }

    return FALSE;
  }

  /**
   * Get the info of streaming server based on container and codec.
   *
   * @param string $container_type
   *  Container type like mp4;mpg etc.
   * @param string $video_codec
   *  Codec.
   * @param string $uri
   *  URI.
   */
  static public function get_streaming_info($container_type, $video_codec, $uri) {
    static $servers = NULL;

    // Is there loaded data in the $servers variable?
    if (!isset($servers)) {
      // Choose a random available streaming server.
      $servers = mediamosa_db::db_query(
        "SELECT mssc.#container FROM {#mediamosa_server} AS mss
        JOIN {#mediamosa_server_streaming_container} AS mssc ON mssc.#nid = mss.#nid
        WHERE mss.#server_type = :server_type AND mss.#server_status = :server_status_on",
        array(
          '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
          '#mediamosa_server_streaming_container' => mediamosa_server_streaming_container_db::TABLE_NAME,
          '#nid' => mediamosa_server_db::NID,
          '#container' => mediamosa_server_streaming_container_db::CONTAINER,
          '#server_type' => mediamosa_server_db::SERVER_TYPE,
          ':server_type' => mediamosa_server_db::SERVER_TYPE_STREAMING,
          '#server_status' => mediamosa_server_db::SERVER_STATUS,
          ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
        )
      )->fetchAllAssoc(mediamosa_server_streaming_container_db::CONTAINER, PDO::FETCH_ASSOC);
    }

    // Object.
    if (isset($servers[$container_type . ':' . $video_codec]) || isset($servers[$container_type])) {
      $response_object_available = TRUE;
    }
    else {
      $response_object_available = FALSE;
    }

    // Plain.
    if (isset($uri) && trim($uri) != '') {
      $response_plain_available = TRUE;
    }
    else {
      $response_plain_available = $response_object_available;
    }

    // Metafile.
    if ($container_type == mediamosa_asset_mediafile_play_proxy::CONTAINER_TYPE_MP4 || $container_type == mediamosa_asset_mediafile_play_proxy::CONTAINER_TYPE_MP3 || $container_type == mediamosa_asset_mediafile_play_proxy::CONTAINER_TYPE_WMV) {
      $container_type_ok = TRUE;
    }
    else {
      $container_type_ok = FALSE;
    }
    $response_metafile_available = ($response_plain_available && $container_type_ok);

    return array(
      'response_object_available' => $response_object_available,
      'response_plain_available' => $response_plain_available,
      'response_metafile_available' => $response_metafile_available,
    );
  }

  /**
   * Find the object code based on container and video_codec (optional).
   *
   * @param string $container_type
   *  Container type like mp4;mpg etc.
   * @param string $video_codec
   *  Codec.
   * @param string $mime_type
   *  Mime type.
   */
  static public function get_objectcode_streaming($container_type, $video_codec, $mime_type = NULL) {

    $server = self::get_streaming($container_type, $video_codec, $mime_type);

    if ($server) {
      return $server[mediamosa_server_db::OBJECT_CODE];
    }

    return FALSE;
  }

  /**
   * Find the uri based on container and video_codec (optional).
   *
   * @param string $container_type
   *  Container type like mp4;mpg etc.
   * @param string $video_codec
   *  Codec.
   * @param string $mime_type
   *  Mime type.
   */
  static public function get_uri_streaming($container_type, $video_codec, $mime_type = NULL) {

    $server = self::get_streaming($container_type, $video_codec, $mime_type);

    if ($server) {
      return self::get_server_url($server);
    }

    return FALSE;
  }

  /**
   * Get the default still server.
   */
  static public function get_still() {
    static $server = NULL;

    if (isset($server)) {
      return $server;
    }

    $server = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_server} WHERE #server_type = :server_type AND #server_status = :server_status_on LIMIT 1',
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_STILL,
        '#server_status' => mediamosa_server_db::SERVER_STATUS,
        ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
      )
    )->fetchAssoc();

    return $server;
  }

  /**
   * Get the URI of the default still server.
   */
  static public function get_uri_still() {
    $server = self::get_still();

    return self::get_server_url($server);
  }

  /**
   * Basic search, for now return all.
   */
  static public function search() {
    return mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms')
      ->fields('ms')
      ->execute();
  }

  /**
   * Basic search on containers, for now return all.
   */
  static public function search_containers() {
    return mediamosa_db::db_select(mediamosa_server_streaming_container_db::TABLE_NAME, 'mssc')
      ->fields('mssc')
      ->execute();
  }

  /**
   * Basic search on tools, for now return all.
   */
  static public function search_tools() {
    return mediamosa_db::db_select(mediamosa_server_tool_db::TABLE_NAME, 'mst')
      ->fields('mst')
      ->execute();
  }

  /**
   * Gives back the old server URL.
   *
   * @param array $server
   *   Mediamosa_server table array.
   */
  static public function get_server_url(array $server) {
    return str_replace(mediamosa_server_db::SERVER_PATH_BASE_URI, $server[mediamosa_server_db::SERVER_URI], $server[mediamosa_server_db::SERVER_PATH]);
  }
}
