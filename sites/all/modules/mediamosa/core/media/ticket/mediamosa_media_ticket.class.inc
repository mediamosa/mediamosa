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
  const TICKET_TYPE_VIEW = 'PLAY';
  const TICKET_TYPE_DOWNLOAD = 'DOWNLOAD';
  const TICKET_TYPE_STILL = 'STILL';
  const TICKET_TYPE_UPLOAD = 'UPLOAD';

  /**
   * Default expiration settings, in seconds.
   */
  const TICKET_EXPIRATION_VIEW = 10;
  const TICKET_EXPIRATION_DOWNLOAD = 10;
  const TICKET_EXPIRATION_STILL = 10;

  // Not symbolic link based.
  // 24 hours.
  const TICKET_EXPIRATION_UPLOAD = 1440;

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
   * Get the expire time for view tickets.
   *
   * @return integer
   *   The number of minutes until ticket is expired.
   */
  public static function get_ticket_expiration_view() {
    return variable_get('mediamosa_ticket_expiration_view', self::TICKET_EXPIRATION_VIEW);
  }

  /**
   * Get the expire time for download tickets.
   *
   * @return integer
   *   The number of minutes until ticket is expired.
   */
  public static function get_ticket_expiration_download() {
    return variable_get('mediamosa_ticket_expiration_download', self::TICKET_EXPIRATION_DOWNLOAD);
  }

  /**
   * Get the expire time for still tickets.
   *
   * @return integer
   *   The number of minutes until ticket is expired.
   */
  public static function get_ticket_expiration_still() {
    return variable_get('mediamosa_ticket_expiration_still', self::TICKET_EXPIRATION_STILL);
  }

  /**
   * Get the expire time for upload tickets.
   *
   * @return integer
   *   The number of minutes until ticket is expired.
   */
  public static function get_ticket_expiration_upload() {
    return variable_get('mediamosa_ticket_expiration_upload', self::TICKET_EXPIRATION_UPLOAD);
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
   * Cleanup expired tickets by type.
   *
   * Cleans up expired tickets symlinks and removes expired tickets from
   * database.
   *
   * @param string $type
   *   Ticket type, see mediamosa_media_ticket::TICKET_TYPE_*.
   * @param integer $mindepth
   *   Minimum depth.
   * @param integer $maxdepth
   *   Maximum depth.
   * @param array $args
   *   Possible extra arguments.
   *
   * @throws mediamosa_exception_program_error
   */
  public static function ticket_cleanup_by_type($type, $mindepth = 4, $maxdepth = 4, array $args = array()) {
    // Get media path.
    $path = mediamosa_storage::get_realpath_media_ticket();

    switch ($type) {
      case self::TICKET_TYPE_VIEW:
        $expire = self::get_ticket_expiration_view();
        break;

      case self::TICKET_TYPE_DOWNLOAD:
        $expire = self::get_ticket_expiration_download();
        break;

      case self::TICKET_TYPE_STILL:
        $expire = self::get_ticket_expiration_still();
        break;

      default:
        throw new mediamosa_exception_program_error('Unknown ticket type:' . $type);
    }

    // Delete tickets.
    mediamosa_db::db_delete(mediamosa_media_ticket_db::TABLE_NAME)
      ->condition(mediamosa_media_ticket_db::TICKET_TYPE, $type)
      ->where(mediamosa_media_ticket_db::ISSUED . ' <= DATE_SUB(:now, INTERVAL ' . $expire . ' MINUTE)', array(':now' => mediamosa_datetime::utc_current_timestamp_now(TRUE)))
      ->execute();

    $args += array(
      '-maxdepth' => $mindepth,
      '-mindepth' => $maxdepth,
      '-mmin' => '+' . (int) $expire,
    );

    $params = array();
    foreach ($args as $cmd => $arg) {
      $params[] = $cmd . ' ' . escapeshellarg($arg);
    }

    // Remove the symlinks.
    if (mediamosa_io::file_exists($path . '/' . mediamosa_unicode::strtolower($type))) {
      mediamosa_io::exec(strtr('find @path @params -type l -delete', array('@path' => escapeshellarg($path . '/' . mediamosa_unicode::strtolower($type) . '/'), '@params' => implode(' ', $params))));
    }

    // Now remove the ticket dir.
    $args['-maxdepth']--;
    $args['-mindepth']--;

    // Add one hour.
    $args['-mmin'] = '+' . ($expire + 60);

    // Add reg ex.
    $args['-regex'] = "^.*/[0-9]+/[a-zA-Z0-9]/[a-zA-Z0-9]{" . mediamosa_sdk::UUID_LENGTH . "}+?$";

    $params = array();
    foreach ($args as $cmd => $arg) {
      $params[] = $cmd . ' ' . escapeshellarg($arg);
    }

    if (mediamosa_io::file_exists($path . '/' . mediamosa_unicode::strtolower($type))) {
      mediamosa_io::exec(strtr('find @path @params -type d -delete', array('@path' => escapeshellarg($path . '/' . mediamosa_unicode::strtolower($type) . '/'), '@params' => implode(' ', $params))));
    }
  }

  /**
   * Clean up expired tickets.
   */
  public static function ticket_cleanup() {
    if (!mediamosa_storage::is_storage_setup()) {
      self::log('Can not run clean up expired tickets; storage file system not setup, need to specify the linux or windows storage file system in mediamosa configuration.');
      return;
    }

    // Expire on type.
    foreach (array(
      self::TICKET_TYPE_DOWNLOAD,
      self::TICKET_TYPE_STILL,
      self::TICKET_TYPE_VIEW,
    ) as $type) {
      self::ticket_cleanup_by_type($type);
    }

    // Clear our stat cache, 'find' exec does not clear our cache.
    mediamosa_io::clearstatcache();
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
   *   The ticket ID to get.
   * @param string $ticket_type
   *   Matching ticket type.
   *
   * @return array|FALSE
   *   Either array with ticket info or FALSE when ticket is no longer valid.
   */
  public static function get_valid($ticket_id, $ticket_type) {

    // Get the current time.
    $db_time_now = mediamosa_datetime::utc_current_timestamp_now();

    // Get the correct expire date.
    switch ($ticket_type) {
      case mediamosa_media_ticket_db::TICKET_TYPE_DOWNLOAD:
        $expire_minutes = mediamosa_media_ticket::get_ticket_expiration_download();
        break;

      case mediamosa_media_ticket_db::TICKET_TYPE_VIEW:
        $expire_minutes = mediamosa_media_ticket::get_ticket_expiration_view();
        break;

      case mediamosa_media_ticket_db::TICKET_TYPE_STILL:
        $expire_minutes = mediamosa_media_ticket::get_ticket_expiration_still();
        break;

      case mediamosa_media_ticket_db::TICKET_TYPE_UPLOAD:
      default:
        $expire_minutes = mediamosa_media_ticket::get_ticket_expiration_upload();
        break;
    }

    // Do the query.
    return mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_media_ticket} WHERE #ticket_id = :ticket_id AND #ticket_type = :ticket_type AND #issued > DATE_SUB(:now, INTERVAL #expire_seconds MINUTE)',
      array(
        '#mediamosa_media_ticket' => mediamosa_media_ticket_db::TABLE_NAME,
        '#ticket_id' => mediamosa_media_ticket_db::ID,
        ':ticket_id' => $ticket_id,
        '#ticket_type' => mediamosa_media_ticket_db::TICKET_TYPE,
        ':ticket_type' => $ticket_type,
        '#issued' => mediamosa_media_ticket_db::ISSUED,
        ':now' => $db_time_now,
        '#expire_seconds' => $expire_minutes,
      )
    )->fetchAssoc();
  }

  /**
   * Wrapper for upload type.
   *
   * @param string $ticket_id
   *   The ticket ID.
   *
   * @return array|FALSE
   *   Either array with ticket info or FALSE when ticket is no longer valid.
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
