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
  * Debug class.
  */
class mediamosa_debug {

  /**
   * Default debug logger, logs only in debug mode (level 5, normal).
   *
   * @param string $message
   * @param array $variables
   * @param string $type
   */
  static public function log($message, array $variables = array(), $type = NULL, $link = NULL) {
    if (self::is_debug()) {
      mediamosa_watchdog::log($message, $variables, WATCHDOG_DEBUG, $type, $link);
    }
  }

  /**
   * Log all types.
   *
   * @param miced $data
   * @param string $severity
   * @param string $type
   */
  static public function log_export($data, $severity = WATCHDOG_NOTICE, $type = NULL) {
    if (self::is_debug()) {
      mediamosa_watchdog::log(highlight_string(var_export($data, TRUE), TRUE), array(), $severity, $type);
    }
  }

  /**
   * Log asset ids.
   */
  static public function log_asset($asset_id, $message, array $variables = array(), $type = NULL) {
    if (self::is_debug()) {
      mediamosa_watchdog::log_asset($asset_id, $message, $variables, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * Log mediafile ids.
   */
  static public function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $type = NULL) {
    if (self::is_debug()) {
      mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * High debug logger, logs only in high debug mode (level 10, high).
   *
   * @param string $message
   * @param array $variables
   * @param string $type
   */
  static public function log_high($message, array $variables = array(), $type = NULL) {
    if (self::is_debug_high()) {
      mediamosa_watchdog::log($message, $variables, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * High debug logger, log asset ids.
   */
  static public function log_high_asset($asset_id, $message, array $variables = array(), $type = NULL) {
    if (self::is_debug_high()) {
      mediamosa_watchdog::log_asset($asset_id, $message, $variables, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * High debug logger, log mediafile ids.
   */
  static public function log_high_mediafile($mediafile_id, $message, array $variables = array(), $asset_id, $type = NULL) {
    if (self::is_debug_high()) {
      mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, WATCHDOG_DEBUG, $type);
    }
  }

  /**
   * Log without logging to the database, go to php.log instead.
   */
  static public function error_log($message, $filename = NULL, $line = NULL) {
    error_log(strtr('In @filename at line @line; @message.', array('@filename' => ($filename ? $filename : 'unknown'), '@line' => (isset($line) ? $line : 'unknown'), '@message' => $message)));
  }

  /**
   * Returns TRUE / FALSE to indicate debug mode ON / OFF.
   */
  static public function is_debug() {
    return (variable_get('mediamosa_debug_level', 0) > 0);
  }

  /**
   * Returns TRUE / FALSE to indicate debug mode for logging.
   */
  static public function is_debug_high() {
    return (variable_get('mediamosa_debug_level', 0) >= 10);
  }

  /**
   * Log the query string for a db_query result object..
   */
  static public function log_query_string($query_object) {
    mediamosa_watchdog::log($query_object->queryString, array(), WATCHDOG_INFO, 'DB query');
  }
}