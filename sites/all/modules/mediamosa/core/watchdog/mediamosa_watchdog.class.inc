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
 * Log class for MediaMosa.
 */

class mediamosa_watchdog {
  /**
   * Our wrapper around the drupal watchdog.
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $type = NULL, $link = NULL) {
    static $logging = FALSE;

    // Prevent loops when we are calling ourselfs because of errors.
    if (!$logging) {
      $logging = TRUE;

      // We'll use the drupal watchdog.
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
   * @param integer $severity
   *   The severity of the message, as per RFC 3164.
   * @param string $type
   *   The category to which this message belongs.
   * @param string $link
   *   A link to associate with the message.
   *
   * @see watchdog_exception()
   */
  public static function log_exception(Exception $exception, $message = NULL, array $variables = array(), $severity = WATCHDOG_ERROR, $type = NULL, $link = NULL) {
    try {
      watchdog_exception('MediaMosa' . (empty($type) ? '' : ' - ' . $type), $exception, $message = NULL, $variables, $severity, $link);
    }
    catch (Exception $e) {
    }
  }

  /**
   * Log asset ids.
   */
  public static function log_asset($asset_id, $message, array $variables = array(), $severity = WATCHDOG_NOTICE, $type = NULL, $mediafile_id = '') {
    // Unique id for identifing the watchdog id.
    $link = mediamosa_db::uuid(0);

    // Logging the message.
    self::log($message, $variables, $severity, $type, $link);

    // Get the watchdog id.
    $query = mediamosa_db::db_select('watchdog', 'w')
      ->fields('w', array('wid'))
      ->condition('w.link', mediamosa_db::db_like($link), 'LIKE');
    $wid = $query->execute()->fetchField();

    // Cleaned the link of watchdog.
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
   * Log mediafile ids.
   */
  public static function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $severity = WATCHDOG_NOTICE, $type = NULL) {
    // Need to know.
    assert(!empty($mediafile_id));

    // Fill the asset id.
    if (!$asset_id && $mediafile_id) {
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'am')
        ->fields('am', array(mediamosa_asset_mediafile_db::ASSET_ID))
        ->condition('am.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);

      $asset_id = $query->execute()->fetchField();
    }

    // Logging the message and insert the asset_id.
    return self::log_asset($asset_id, $message, $variables, $severity, $type, $mediafile_id);
  }

  /**
   * Gives pretty log entries.
   *
   * @param any $data
   * @param string $severity
   * @param string $type
   */
  public static function log_export($data, $severity = WATCHDOG_NOTICE, $type = NULL) {
    self::log(highlight_string(var_export($data, TRUE), TRUE), array(), $severity, $type);
  }

  /**
   * Log an DB Query timing.
   *
   * @param time $f_time
   *  The total time the query took in 0.000 format.
   * @param string $query
   *  Your message, Best to keep as the query.
   */
  public static function log_db($f_time, $query) {
    $name = self::is_db_logging();
    if (!$name) {
      return;
    }
    // Only log stuff that takes longer than 1 second.
    if ($f_time > 1) {
      self::$num_of_slow_queries[$name]++;
      self::log_debug('Query timing, #times, "#query".', array('#time' => $f_time, '#query' => $query), $name);
    }

    self::$num_of_queries[$name]++;
  }

  /**
   * Log the query string for a db_query result object..
   */
  public static function log_query_string($query_object) {
    self::log($query_object->queryString, array(), WATCHDOG_INFO, 'DB query');
  }
}
