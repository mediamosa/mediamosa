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
  * The ticket class.
  */

class mediamosa_ticket {

  // ------------------------------------------------------------------ Consts.
  /**
   *  Ticket types
   */
  const TICKET_TYPE_PLAY = 'PLAY';
  const TICKET_TYPE_DOWNLOAD = 'DOWNLOAD';
  const TICKET_TYPE_STILL = 'STILL';
  const TICKET_TYPE_UPLOAD = 'UPLOAD';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Log for ticket.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'ticket');
  }

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Create ticket for upload.
   *
   * @param integer $app_id
   * @param string $mediafile_id
   * @param string $user_id
   * @param string $group_id
   */
  static public function create_for_upload($app_id, $mediafile_id, $user_id, $group_id = '') {
    // Mediafile must exist.
    $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $a_mediafile[mediamosa_asset_mediafile_db::APP_ID], $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID]);

    // Check quota.
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Create upload ticket.
    return mediamosa_ticket::create($app_id, mediamosa_ticket::TICKET_TYPE_UPLOAD, $user_id, $mediafile_id, NULL, $group_id);
  }

  /**
   * Create ticket.
   *
   * @param integer $app_id
   * @param string $ticket_type
   * @param string $owner_id
   * @param string $mediafile_id
   * @param string $ticket_id
   * @param string $group_id
   * @param datetime $issued
   */
  static public function create($app_id, $ticket_type, $owner_id, $mediafile_id, $ticket_id = NULL, $group_id = NULL, $issued = NULL) {
    if (!isset($ticket_id)) {
      $ticket_id = mediamosa_db::uuid($app_id);
    }

    // Fields to insert.
    $fields = array(
      mediamosa_ticket_db::ID => $ticket_id,
      mediamosa_ticket_db::APP_ID => $app_id,
      mediamosa_ticket_db::TICKET_TYPE => $ticket_type,
      mediamosa_ticket_db::OWNER_ID => $owner_id,
      mediamosa_ticket_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_ticket_db::ISSUED => (isset($issued) ? $issued : mediamosa_datetime::utc_current_timestamp_now()),
    );

    if (isset($group_id)) {
      $fields[mediamosa_ticket_db::GROUP_ID] = $group_id;
    }

    // Add changed/created values.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert.
    mediamosa_db::db_insert(mediamosa_ticket_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    mediamosa_statistics::create_streaming_ticket($ticket_id, $ticket_type, $app_id, $mediafile_id);

    // Return the ticket ID.
    return $ticket_id;
  }

  /**
   * Process ticket for play-proxy usage.
   *
   * @param string $ticket_id
   *  A unique ID for the ticket.
   * @param array $mediafile_ext
   *  Is mediafile array with mediafile_metadata array.
   * @param string $response_type
   *  See mediamosa_asset_mediafile_play_proxy::RESPONSE_*
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  Owner of ticket.
   * @param string $still_id
   *  Optional still_id.
   */
  static public function process_play_ticket($ticket_id, $mediafile_ext, $response_type, $app_id, $user_id, $still_id = FALSE) {
    // Not set? Set default.
    if (!isset($response_type)) {
      $response_type = mediamosa_asset_mediafile_play_proxy::RESPONSE_URI;
    }

    if ($response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL) {
      // Find default or at least one still.
      if (!$still_id) {
        // Find default still and make sure its ok.
        $still_id = mediamosa_asset_mediafile_still::find_default($mediafile_ext[mediamosa_asset_mediafile_db::ASSET_ID]);

        if (!$still_id) {
          return;
        }
      }
      else {
        // When we provide the still ID, we need to check it.
        mediamosa_asset_mediafile_still::check($still_id);
      }
    }
    else {
      // Check if the mediafile exists.
      $filename = mediamosa_configuration_storage::mediafile_filename_get($mediafile_ext);
      if (!file_exists($filename)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FILE_NOT_FOUND, array('@filename' => $filename));
      }
    }

    // Based on type, set the parameters.
    switch ($response_type) {
      case mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL:
        // Need extension.
        $file_ext = mediamosa_asset_mediafile::get_file_extension($still_id);

        // Attach ext.
        $ticket_id = $ticket_id . '.' . $file_ext;

        $ticket_type = self::TICKET_TYPE_STILL;
        $ticket_target = mediamosa_configuration_storage::data_still_get_file($still_id);
        $ticket_symlink = mediamosa_configuration_storage::media_get_ticket_file($app_id, $ticket_id);
        break;

      case mediamosa_asset_mediafile_play_proxy::RESPONSE_DOWNLOAD:
        $ticket_type = self::TICKET_TYPE_DOWNLOAD;
        $ticket_target = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_ext[mediamosa_asset_mediafile_db::ID]);

        // Set default filename.
        if (empty($mediafile_ext['filename'])) {
          $mediafile_ext['filename'] = mediamosa_asset_mediafile::UNKNOWN_FILENAME;
        }

        $ticket_symlink = mediamosa_configuration_storage::ticket_download_location_get() . $ticket_id . DIRECTORY_SEPARATOR . $mediafile_ext['filename'];
        break;

      default:
        $ticket_type = self::TICKET_TYPE_PLAY;
        $ticket_target = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_ext[mediamosa_asset_mediafile_db::ID]);

        // Now based on certain extensions, attach the fileext. to the filename.
        if (!empty($mediafile_ext['metadata']['container_type'])) {
          switch ($mediafile_ext['metadata']['container_type']) {
            case mediamosa_asset_mediafile_play_proxy::CONTAINER_TYPE_FLV:
              $ticket_id .= '.flv';
              break;
            case mediamosa_asset_mediafile_play_proxy::CONTAINER_TYPE_MP4:
              $ticket_id .= '.mp4';
              break;
            case mediamosa_asset_mediafile_play_proxy::CONTAINER_TYPE_MP3:
              $ticket_id .= '.mp3';
              break;
            case mediamosa_asset_mediafile_play_proxy::CONTAINER_TYPE_OGG:
              $ticket_id .= '.ogg';
              break;
          }
        }

        $ticket_symlink = mediamosa_configuration_storage::ticket_play_location_get($app_id) . DIRECTORY_SEPARATOR . $ticket_id;
        break;
    }

    // Get the path where the symlink is created.
    $ticket_path = mediamosa_io::dirname($ticket_symlink);

    // Create directory.
    mediamosa_io::mkdir($ticket_path, TRUE, 0775);

    // Make sure the location of the ticket is a directory.
    if (!is_dir($ticket_path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_FOUND, array('@location' => $ticket_path));
    }

    // Must be able to write ticket symlink there.
    if (!mediamosa_io::is_writable($ticket_path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_WRITABLE, array('@location' => $ticket_path));
    }

    // Get the mediafile for ticket.
    $mediafile_id = ($response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL && $still_id ? $still_id : (isset($mediafile_ext['mediafile_id']) ? $mediafile_ext['mediafile_id'] : ''));

    // Insert Ticket.
    self::create($app_id, $ticket_type, $user_id, $mediafile_id, $ticket_id);

    // Create the symlink ticket.
    if (file_exists($ticket_symlink)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK, array('@location' => $ticket_target . ' (link already exists)'));
    }

    if (!mediamosa_io::symlink($ticket_target, $ticket_symlink)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK, array('@location' => $ticket_target));
    }

    // Return the ticket, we might have changed it.
    return $ticket_id;
  }

  /**
   * Run at fixed intervals.
   */
  static public function ticket_cleanup() {

    if (!mediamosa_configuration_storage::is_mount_setup()) {
      self::log('Can not run clean up expired tickets; mount point not setup, need to specify the linux or windows mount point in mediamosa configuration.');
      return;
    }

    // Get the current time.
    $db_time_now = mediamosa_datetime::utc_current_timestamp_now();

    // Retrieve location of the play tickets.
    $ticket_play_location = mediamosa_configuration_storage::ticket_play_location_get();

    // Clean up the play tickets.
    mediamosa_io::exec(
      strtr(
        'find @ticket_play_location/. -maxdepth 2 -mindepth 1 -name "@wildmatch" -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_play_location' => $ticket_play_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_settings::TICKET_EXPIRATION_PLAY / 60),
        )
      )
    );

    // Clean up the tickets with extensions
    mediamosa_io::exec(
      strtr(
        'find @ticket_play_location/. -maxdepth 2 -mindepth 1 -name "@wildmatch.*" -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_play_location' => $ticket_play_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_settings::TICKET_EXPIRATION_PLAY / 60),
        )
      )
    );

    // Now remove play tickets from the database.
    $result = mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_ticket} WHERE #ticket_type = :ticket_type AND #issued <= DATE_SUB(:now, INTERVAL #expire_play SECOND)',
      array(
        '#mediamosa_ticket' => mediamosa_ticket_db::TABLE_NAME,
        '#ticket_type' => mediamosa_ticket_db::TICKET_TYPE,
        ':ticket_type' => mediamosa_ticket_db::TICKET_TYPE_PLAY,
        '#issued' => mediamosa_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_play' => mediamosa_settings::TICKET_EXPIRATION_PLAY,
      )
    );

    // Set the download location.
    $ticket_download_location = mediamosa_configuration_storage::ticket_download_location_get();

    // Clean up the download directories.
    mediamosa_io::exec(
      strtr(
        'find @ticket_download_location/. -maxdepth 2 -mindepth 2 -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_download_location' => $ticket_download_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_settings::TICKET_EXPIRATION_DOWNLOAD / 60),
        )
      )
    );

    // Clean up the download directories.
    mediamosa_io::exec(
      strtr(
        'find @ticket_download_location/. -maxdepth 1 -mindepth 1 -name "@wildmatch" -type d -mmin +@ticket_expiration -delete',
        array(
          '@ticket_download_location' => $ticket_download_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_settings::TICKET_EXPIRATION_DOWNLOAD / 60),
        )
      )
    );

    // Clean up symlinks ticket.
    mediamosa_io::exec(
      strtr(
        'find @ticket_media_location -maxdepth 3 -mindepth 3 -regex "^.*/[0-9]+/[a-zA-Z0-9]/[a-zA-Z0-9]+\(,[0-9]+\)?\(\.[a-zA-Z0-9]+\)?$" -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_media_location' => mediamosa_configuration_storage::media_get_ticket_path(),
          '@ticket_expiration' => (int) (mediamosa_settings::TICKET_EXPIRATION_STILL / 60),
        )
      )
    );

    // Now remove still from the database.
    $result = mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_ticket} WHERE #ticket_type = :ticket_type AND #issued <= DATE_SUB(:now, INTERVAL #expire_play SECOND)',
      array(
        '#mediamosa_ticket' => mediamosa_ticket_db::TABLE_NAME,
        '#ticket_type' => mediamosa_ticket_db::TICKET_TYPE,
        ':ticket_type' => mediamosa_ticket_db::TICKET_TYPE_STILL,
        '#issued' => mediamosa_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_play' => mediamosa_settings::TICKET_EXPIRATION_STILL,
      )
    );

    // Now remove expired upload tickets from the database.
    $result = mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_ticket} WHERE #ticket_type = :ticket_type AND #issued <= DATE_SUB(:now, INTERVAL #expire_play SECOND)',
      array(
        '#mediamosa_ticket' => mediamosa_ticket_db::TABLE_NAME,
        '#ticket_type' => mediamosa_ticket_db::TICKET_TYPE,
        ':ticket_type' => mediamosa_ticket_db::TICKET_TYPE_UPLOAD,
        '#issued' => mediamosa_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_play' => mediamosa_settings::TICKET_EXPIRATION_UPLOAD,
      )
    );
  }

  /**
   * Get the mediafile_id using the ticket_id.
   *
   * @param $ticket_id
   *   The ticket ID to use.
   *
   * @return
   *   The mediafile ID or FALSE.
   */
  static public function get_mediafile_id($ticket_id) {
      // Convert $ticket_id to $mediafile_id.
      return mediamosa_db::db_select(mediamosa_ticket_db::TABLE_NAME, 't')
        ->fields('t', array(mediamosa_ticket_db::MEDIAFILE_ID))
        ->condition(mediamosa_ticket_db::ID, $ticket_id)
        ->condition(mediamosa_ticket_db::TICKET_TYPE, mediamosa_ticket_db::TICKET_TYPE_STILL)
        ->execute()
        ->fetchField();
  }

  /**
   * Get the uploadticket, but make sure its not timed-out.
   *
   * @param $ticket_id
   *  The ticket ID to get.
   * @param string $ticket_type
   *  Matching ticket type.
   */
  static public function get_valid($ticket_id, $ticket_type) {

    // Get the current time.
    $db_time_now = mediamosa_datetime::utc_current_timestamp_now();

    // Get the correct expire date.
    $expire_seconds = mediamosa_settings::TICKET_EXPIRATION_UPLOAD;
    switch ($ticket_type) {
      case mediamosa_ticket_db::TICKET_TYPE_DOWNLOAD:
        $expire_seconds = mediamosa_settings::TICKET_EXPIRATION_DOWNLOAD;
        break;

      case mediamosa_ticket_db::TICKET_TYPE_PLAY:
        $expire_seconds = mediamosa_settings::TICKET_EXPIRATION_PLAY;
        break;

      case mediamosa_ticket_db::TICKET_TYPE_STILL:
        $expire_seconds = mediamosa_settings::TICKET_EXPIRATION_UPLOAD;
        break;

      case mediamosa_ticket_db::TICKET_TYPE_UPLOAD:
      default:
        break;
    }

    // Do the query.
    return mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_ticket} WHERE #ticket_id = :ticket_id AND #ticket_type = :ticket_type AND #issued > DATE_SUB(:now, INTERVAL #expire_seconds SECOND)',
      array(
        '#mediamosa_ticket' => mediamosa_ticket_db::TABLE_NAME,
        '#ticket_id' => mediamosa_ticket_db::ID,
        ':ticket_id' => $ticket_id,
        '#ticket_type' => mediamosa_ticket_db::TICKET_TYPE,
        ':ticket_type' => $ticket_type,
        '#issued' => mediamosa_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_seconds' => $expire_seconds,
      )
    )->fetchAssoc();
  }

  /**
   * Wrapper for upload type.
   *
   * @param string $ticket_id
   */
  static public function get_valid_upload($ticket_id) {
    return self::get_valid($ticket_id, mediamosa_ticket_db::TICKET_TYPE_UPLOAD);
  }

  /**
   * Remove the ticket.
   * No check is made on ownership or app_id.
   *
   * @param $ticket_id
   */
  static public function delete($ticket_id) {

    // Delete it.
    mediamosa_db::db_delete(mediamosa_ticket_db::TABLE_NAME)
      ->condition(mediamosa_ticket_db::ID, $ticket_id)
      ->execute();
  }
}
