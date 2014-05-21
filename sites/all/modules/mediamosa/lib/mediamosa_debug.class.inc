<?php
/**
 * @file
 * Debug class.
 */

class mediamosa_debug {
  /**
   * Default debug logger, logs only in debug mode (level 5, normal).
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
   * @param string $type
   *   The category to which this message belongs. Can be any string, but the
   *   general practice is to use the name of the module calling watchdog().
   */
  public static function log($message, array $variables = array(), $type = 'debug', $link = NULL) {
    if (self::is_debug_normal()) {
      mediamosa_watchdog::log($message, $variables, WATCHDOG_DEBUG, $type, $link);
    }
  }

  /**
   * Export the data to the log.
   *
   * @param mixed $data
   *   The data to export to the log.
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
   *   The category to which this message belongs. Can be any string, but the
   *   general practice is to use the name of the module calling watchdog().
   */
  public static function log_export($data, $severity = WATCHDOG_NOTICE, $type = NULL) {
    if (self::is_debug_normal()) {
      if (is_resource($data)) {
        mediamosa_watchdog::log(print_r($data, TRUE), array(), $severity, $type);
      }
      elseif (is_bool($data)) {
        mediamosa_watchdog::log($data ? 'TRUE' : 'FALSE', array(), $severity, $type);
      }
      else {
        mediamosa_watchdog::log(highlight_string(var_export($data, TRUE), TRUE), array(), $severity, $type);
      }
    }
  }

  /**
   * Logs for specific asset.
   *
   * @param string $asset_id
   *   The ID of the asset.
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param string $type
   *   The category to which this message belongs. Can be any string, but the
   *   general practice is to use the name of the module calling watchdog().
   */
  public static function log_asset($asset_id, $message, array $variables = array(), $type = NULL) {
    if (self::is_debug_normal()) {
      mediamosa_watchdog::log_asset($asset_id, $message, $variables, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * Logs for specific mediafile.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile.
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
   *   (optional) The ID of the asset, provide when available.
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   */
  public static function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $type = NULL) {
    if (self::is_debug_normal()) {
      mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * High debug logger, logs only in high debug mode (level 10, high).
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
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   */
  public static function log_high($message, array $variables = array(), $type = NULL) {
    if (self::is_debug_high()) {
      mediamosa_watchdog::log($message, $variables, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * High debug logger, logs for specific asset.
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
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   * @param string $mediafile_id
   *   (optional) The ID of the mediafile.
   */
  public static function log_high_asset($asset_id, $message, array $variables = array(), $type = NULL, $mediafile_id = '') {
    if (self::is_debug_high()) {
      mediamosa_watchdog::log_asset($asset_id, $message, $variables, WATCHDOG_DEBUG, $type, $mediafile_id);
    }
  }

  /**
   * High debug logger, log mediafile ids.
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
   * @param string $type
   *   (optional) The category to which this message belongs. Can be any string,
   *   but the general practice is to use the name of the module calling
   *   watchdog().
   */
  public static function log_high_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = '', $type = NULL) {
    if (self::is_debug_high()) {
      mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * Logs directly to file instead of database.
   *
   * @param string $message
   *   The message to log.
   * @param string $file
   *   The file where the error happened.
   * @param int $line
   *   The line number where the error happened.
   */
  public static function error_log($message, $file = '', $line = '') {

    if (empty($file)) {
      $caller = _drupal_get_last_caller(debug_backtrace());
      $file = $caller['file'];
      $line = $caller['line'];
    }

    error_log(
      format_string(
        'MediaMosa debug: @message in @file on line @line',
        array(
          '@file' => (!empty($file) ? $file : 'unknown'),
          '@line' => (isset($line) ? $line : 'unknown'),
          '@message' => drupal_ucfirst($message),
        )
      )
    );
  }

  /**
   * Returns TRUE / FALSE to indicate debug mode ON / OFF.
   *
   * @return bool
   *   Returns TRUE when debug is on.
   */
  public static function is_debug() {
    return (variable_get('mediamosa_debug_level', 0) > 0);
  }

  /**
   * Returns TRUE / FALSE to indicate normal debug.
   *
   * @return bool
   *   Returns TRUE when normal level debug is on.
   */
  public static function is_debug_normal() {
    return (variable_get('mediamosa_debug_level', 0) >= 5);
  }

  /**
   * Returns TRUE / FALSE to indicate debug mode for logging.
   *
   * @return bool
   *   Returns TRUE when high level debug is on.
   */
  public static function is_debug_high() {
    return (variable_get('mediamosa_debug_level', 0) >= 10);
  }

  /**
   * Log the query string for a db_query result object.
   *
   * @param DatabaseStatementInterface $statement
   *   A database query result resource
   */
  public static function log_query_string(DatabaseStatementInterface $statement) {
    mediamosa_watchdog::log($statement->getQueryString(), array(), WATCHDOG_INFO, 'DB query');
  }
}
