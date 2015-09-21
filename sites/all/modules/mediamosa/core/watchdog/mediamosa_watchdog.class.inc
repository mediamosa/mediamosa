<?php
/**
 * @file
 * Log class for MediaMosa.
 */

class mediamosa_watchdog {
  /**
   * Our wrapper around the drupal watchdog.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param int $severity
   *   The severity of the message; one of the following values as defined in
   *   @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   * @param string $link
   *   (optional) A link to associate with the message.
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $type = NULL, $link = NULL) {
    static $logging = FALSE;

    // Prevent loops when we are calling ourselfs because of errors.
    if (!$logging) {
      $logging = TRUE;

      // Use the Drupal watchdog.
      watchdog('MediaMosa' . (empty($type) ? '' : ' - ' . $type), $message, $variables, $severity, $link);
    }

    $logging = FALSE;
  }

  /**
   * Logs an exception.
   *
   * This is a wrapper function for watchdog() which automatically decodes an
   * exception.
   *
   * @param Exception $exception
   *   The exception that is going to be logged.
   * @param string $message
   *   The message to store in the log. If empty, a text that contains all useful
   *   information about the passed-in exception is used.
   * @param array $variables
   *   Array of variables to replace in the message on display. Defaults to the
   *   return value of drupal_decode_exception().
   * @param int $severity
   *   The severity of the message, as per RFC 3164.
   * @param string $type
   *   The category to which this message belongs.
   * @param string $link
   *   A link to associate with the message.
   *
   * @see watchdog_exception()
   */
  public static function log_exception(Exception $exception, $message = NULL, array $variables = array(), $severity = WATCHDOG_ERROR, $type = NULL, $link = NULL) {
    watchdog_exception('MediaMosa' . (empty($type) ? '' : ' - ' . $type), $exception, $message = NULL, $variables, $severity, $link);
  }

  /**
   * Logs for specific asset.
   *
   * @param string $asset_id
   *   The ID of the asset that is associated with the message.
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param int $severity
   *   The severity of the message; one of the following values as defined in
   *   @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   * @param string $mediafile_id
   *   (optional) The ID of the mediafile.
   */
  public static function log_asset($asset_id, $message, array $variables = array(), $severity = WATCHDOG_NOTICE, $type = NULL, $mediafile_id = '') {
    // Unique ID for identifing the watchdog id.
    $link = mediamosa_db::uuid(0);

    // Logging the message.
    self::log($message, $variables, $severity, $type, $link);

    // Get the watchdog ID.
    $wid = mediamosa_db::db_select('watchdog', 'w')
      ->fields('w', array('wid'))
      ->condition('w.link', mediamosa_db::db_like($link), 'LIKE')
      ->execute()
      ->fetchField();

    // Clear the link of watchdog entry.
    mediamosa_db::db_update('watchdog')
      ->fields(array(
        'link' => '',
      ))
      ->condition('wid', $wid)
      ->execute();

    // Insert the assset_id.
    return mediamosa_db::db_insert(mediamosa_watchdog_db::TABLE_NAME)
      ->fields(array(
        mediamosa_watchdog_db::ID => $wid,
        mediamosa_watchdog_db::ASSET_ID => $asset_id,
        mediamosa_watchdog_db::MEDIAFILE_ID => $mediafile_id,
      ))
      ->execute();
  }

  /**
   * Logs for specific mediafile.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile that is associated with the message.
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param string $asset_id
   *   (optional) The ID of the asset. Provide when available.
   * @param int $severity
   *   The severity of the message; one of the following values as defined in
   *   @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   */
  public static function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = '', $severity = WATCHDOG_NOTICE, $type = NULL) {
    // Need to know.
    assert(!empty($mediafile_id));

    // Get the asset id.
    if (empty($asset_id) && !empty($mediafile_id)) {
      $asset_id = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'am')
        ->fields('am', array(mediamosa_asset_mediafile_db::ASSET_ID))
        ->condition('am.' . mediamosa_asset_mediafile_db::ID, $mediafile_id)
        ->execute()
        ->fetchField();
    }

    // Logging the message.
    return self::log_asset($asset_id, $message, $variables, $severity, $type, $mediafile_id);
  }

  /**
   * Export data to log.
   *
   * @param mixed $data
   *   Any data to export to log.
   * @param int $severity
   *   The severity of the message; one of the following values as defined in
   *   @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   */
  public static function log_export($data, $severity = WATCHDOG_NOTICE, $type = NULL) {
    self::log(highlight_string(var_export($data, TRUE), TRUE), array(), $severity, $type);
  }

  /**
   * Log the query string for a db_query result object.
   *
   * @param DatabaseStatementInterface $statement
   *   A database query result resource
   */
  public static function log_query_string(DatabaseStatementInterface $statement) {
    self::log($statement->getQueryString(), array(), WATCHDOG_INFO, 'DB query');
  }
}
