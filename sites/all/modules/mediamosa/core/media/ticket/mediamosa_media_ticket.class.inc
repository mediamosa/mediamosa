<?php
/**
 * @file
 * The ticket class.
 */

class mediamosa_media_ticket {

  // ------------------------------------------------------------------- Consts.
  /**
   *  Ticket types
   */
  const TICKET_TYPE_PLAY = 'PLAY';
  const TICKET_TYPE_DOWNLOAD = 'DOWNLOAD';
  const TICKET_TYPE_STILL = 'STILL';
  const TICKET_TYPE_UPLOAD = 'UPLOAD';

  // ---------------------------------------------------------------- Functions.
  /**
   * Log for ticket.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'ticket');
  }

  /**
   * Create ticket for upload.
   *
   * @param integer $app_id
   * @param string $mediafile_id
   * @param string $user_id
   * @param string $group_id
   */
  public static function create_for_upload($app_id, $mediafile_id, $user_id, $group_id = '') {
    // Mediafile must exist.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $mediafile[mediamosa_asset_mediafile_db::APP_ID], $mediafile[mediamosa_asset_mediafile_db::OWNER_ID]);

    // Check quota.
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Create upload ticket.
    return mediamosa_media_ticket::create($app_id, mediamosa_media_ticket::TICKET_TYPE_UPLOAD, $user_id, $mediafile_id, NULL, $group_id);
  }

  /**
   * Create ticket.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $ticket_type
   *   The ticket type. See mediamosa_media_ticket_db::TICKET_TYPE_*
   * @param string $owner_id
   *   The owner of the ticket.
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $ticket_id
   *   (optional) The ticket ID.
   * @param string $group_id
   *   (optional) The group ID.
   */
  public static function create($app_id, $ticket_type, $owner_id, $mediafile_id, $ticket_id = NULL, $group_id = NULL) {
    // Create ticket ID when not provided.
    if (empty($ticket_id)) {
      $ticket_id = mediamosa_db::uuid($app_id);
    }

    // Fields to insert.
    $fields = array(
      mediamosa_media_ticket_db::ID => $ticket_id,
      mediamosa_media_ticket_db::APP_ID => $app_id,
      mediamosa_media_ticket_db::TICKET_TYPE => $ticket_type,
      mediamosa_media_ticket_db::OWNER_ID => $owner_id,
      mediamosa_media_ticket_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_media_ticket_db::ISSUED => mediamosa_datetime::utc_current_timestamp_now(),
    );

    if (isset($group_id)) {
      $fields[mediamosa_media_ticket_db::GROUP_ID] = $group_id;
    }

    // Add changed/created values.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert.
    mediamosa_db::db_insert(mediamosa_media_ticket_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    mediamosa_statistics::create_streaming_ticket($ticket_id, $ticket_type, $app_id, $mediafile_id);

    // Return the ticket ID.
    return $ticket_id;
  }

  /**
   * Run at fixed intervals.
   */
  public static function ticket_cleanup() {
    if (!mediamosa_storage::is_storage_setup()) {
      self::log('Can not run clean up expired tickets; storage file system not setup, need to specify the linux or windows storage file system in mediamosa configuration.');
      return;
    }

    // Get the current time.
    $db_time_now = mediamosa_datetime::utc_current_timestamp_now();

    // Retrieve location of the play tickets.
    $ticket_play_location = mediamosa_storage::get_realpath_media_ticket();

    // Clean up the play tickets.
    mediamosa_io::exec(
      strtr(
        'find @ticket_play_location. -maxdepth 2 -mindepth 1 -name "@wildmatch" -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_play_location' => $ticket_play_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_storage::TICKET_EXPIRATION_PLAY / 60),
        )
      )
    );

    // Clean up the tickets with extensions
    mediamosa_io::exec(
      strtr(
        'find @ticket_play_location. -maxdepth 2 -mindepth 1 -name "@wildmatch.*" -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_play_location' => $ticket_play_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_storage::TICKET_EXPIRATION_PLAY / 60),
        )
      )
    );

    // Now remove play tickets from the database.
    $result = mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_media_ticket} WHERE #ticket_type = :ticket_type AND #issued <= DATE_SUB(:now, INTERVAL #expire_play SECOND)',
      array(
        '#mediamosa_media_ticket' => mediamosa_media_ticket_db::TABLE_NAME,
        '#ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE,
        ':ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE_PLAY,
        '#issued' => mediamosa_media_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_play' => mediamosa_storage::TICKET_EXPIRATION_PLAY,
      )
    );

    // Set the download location.
    $ticket_download_location = mediamosa_storage::get_realpath_media_download();

    // Clean up the download symlinks.
    mediamosa_io::exec(
      strtr(
        'find @ticket_download_location. -maxdepth 2 -mindepth 2 -name "@wildmatch" -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_download_location' => $ticket_download_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_storage::TICKET_EXPIRATION_DOWNLOAD / 60),
        )
      )
    );

    // Clean up the download directories.
    mediamosa_io::exec(
      strtr(
        'find @ticket_download_location. -maxdepth 1 -mindepth 1 -name "@wildmatch" -type d -mmin +@ticket_expiration -delete',
        array(
          '@ticket_download_location' => $ticket_download_location,
          '@wildmatch' => str_repeat('?', mediamosa_settings::UUID_LENGTH),
          '@ticket_expiration' => (int) (mediamosa_storage::TICKET_EXPIRATION_DOWNLOAD / 60),
        )
      )
    );

    // Clean up symlinks ticket.
    mediamosa_io::exec(
      strtr(
        'find @ticket_media_location -maxdepth 3 -mindepth 3 -regex "^.*/[0-9]+/[a-zA-Z0-9]/[a-zA-Z0-9]+\(,[0-9]+\)?\(\.[a-zA-Z0-9]+\)?$" -type l -mmin +@ticket_expiration -delete',
        array(
          '@ticket_media_location' => mediamosa_storage::get_realpath_media_ticket(),
          '@ticket_expiration' => (int) (mediamosa_storage::TICKET_EXPIRATION_STILL / 60),
        )
      )
    );

    // Now remove still from the database.
    $result = mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_media_ticket} WHERE #ticket_type = :ticket_type AND #issued <= DATE_SUB(:now, INTERVAL #expire_play SECOND)',
      array(
        '#mediamosa_media_ticket' => mediamosa_media_ticket_db::TABLE_NAME,
        '#ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE,
        ':ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE_STILL,
        '#issued' => mediamosa_media_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_play' => mediamosa_storage::TICKET_EXPIRATION_STILL,
      )
    );

    // Now remove expired upload tickets from the database.
    $result = mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_media_ticket} WHERE #ticket_type = :ticket_type AND #issued <= DATE_SUB(:now, INTERVAL #expire_play SECOND)',
      array(
        '#mediamosa_media_ticket' => mediamosa_media_ticket_db::TABLE_NAME,
        '#ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE,
        ':ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE_UPLOAD,
        '#issued' => mediamosa_media_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_play' => mediamosa_storage::TICKET_EXPIRATION_UPLOAD,
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
  public static function get_mediafile_id($ticket_id) {
      // Convert $ticket_id to $mediafile_id.
      return mediamosa_db::db_select(mediamosa_media_ticket_db::TABLE_NAME, 't')
        ->fields('t', array(mediamosa_media_ticket_db::MEDIAFILE_ID))
        ->condition(mediamosa_media_ticket_db::ID, $ticket_id)
        ->condition(mediamosa_media_ticket_db::TICKET_TYPE, mediamosa_media_ticket_db::TICKET_TYPE_STILL)
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
  public static function get_valid($ticket_id, $ticket_type) {

    // Get the current time.
    $db_time_now = mediamosa_datetime::utc_current_timestamp_now();

    // Get the correct expire date.
    $expire_seconds = mediamosa_storage::TICKET_EXPIRATION_UPLOAD;
    switch ($ticket_type) {
      case mediamosa_media_ticket_db::TICKET_TYPE_DOWNLOAD:
        $expire_seconds = mediamosa_storage::TICKET_EXPIRATION_DOWNLOAD;
        break;

      case mediamosa_media_ticket_db::TICKET_TYPE_PLAY:
        $expire_seconds = mediamosa_storage::TICKET_EXPIRATION_PLAY;
        break;

      case mediamosa_media_ticket_db::TICKET_TYPE_STILL:
        $expire_seconds = mediamosa_storage::TICKET_EXPIRATION_UPLOAD;
        break;

      case mediamosa_media_ticket_db::TICKET_TYPE_UPLOAD:
      default:
        break;
    }

    // Do the query.
    return mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_media_ticket} WHERE #ticket_id = :ticket_id AND #ticket_type = :ticket_type AND #issued > DATE_SUB(:now, INTERVAL #expire_seconds SECOND)',
      array(
        '#mediamosa_media_ticket' => mediamosa_media_ticket_db::TABLE_NAME,
        '#ticket_id' => mediamosa_media_ticket_db::ID,
        ':ticket_id' => $ticket_id,
        '#ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE,
        ':ticket_type' => $ticket_type,
        '#issued' => mediamosa_media_ticket_db::ISSUED,
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
  public static function get_valid_upload($ticket_id) {
    return self::get_valid($ticket_id, mediamosa_media_ticket_db::TICKET_TYPE_UPLOAD);
  }

  /**
   * Remove the ticket.
   *
   * No check is made on ownership or app_id.
   *
   * @param string $ticket_id
   *   The ticket ID.
   */
  public static function delete($ticket_id) {

    // Delete it.
    mediamosa_db::db_delete(mediamosa_media_ticket_db::TABLE_NAME)
      ->condition(mediamosa_media_ticket_db::ID, $ticket_id)
      ->execute();
  }
}
