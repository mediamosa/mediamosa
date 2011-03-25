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
 * Contains time and date related functions.
 */


/**
 * Static class for date/time functions.
 */
class mediamosa_datetime {

  /**
   * Returns a valid timestamp or throws error when timestamp was invalid or
   * unable to convert. Uses iso8601 datetime strings as input.
   *
   * @param string $mixed_param_value
   *  iso8601 datetime.
   * @param string $param
   *  Name of the param.
   * @param string $param_type
   *  See mediamosa_sdk::TYPE_*.
   *
   * @see mediamosa_type.inc
   */
  static public function date8601_to_timestamp($param_value, $param, $param_type = mediamosa_sdk::TYPE_DATETIME, $format = 'Y-m-d H:i:s') {
    // Lets be sure its a string.
    assert(is_string($param_value));

    $iso8601 = '#^(\d{4}((-)?(0[1-9]|1[0-2])((-)?(0[1-9]|[1-2][0-9]|3[0-1])((T| )(24:00(:00(\.[0]+)?)?|(([0-1][0-9]|2[0-3])(:)[0-5][0-9])((:)[0-5][0-9](\.[\d]+)?)?)((\+|-)(14:00|(0[0-9]|1[0-3])(:)[0-5][0-9])|(|Z)))?)?)?)$#';
    if (!preg_match($iso8601, $param_value)) {

      // Let's try the old style: dd/mm/yyyy
      if (!preg_match('/^[0-9]{2}\/[0-9]{2}\/[0-9]{4}$/', $param_value)) {

        // This is not ISO 8601 and not the old style...
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
      }
      else {
        // Old style to ISO 8601 converter.
        $a_old_date = explode('/', $param_value);
        $param_value = $a_old_date[2] . '-' . $a_old_date[1] . '-' . $a_old_date[0];
      }
    }

    $value = preg_split("/[-:T\\s]/", $param_value);

    if (!count($value) || count($value) > 6) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
    }

    $value[0] = (int)$value[0];// YYYY.
    $value[1] = isset($value[1]) && $value[1] > 0 ? (int) $value[1] : 1;// MM.
    $value[2] = isset($value[2]) && $value[2] > 0 ? (int) $value[2] : 1;// DD.
    $value[3] = isset($value[3]) ? (int) $value[3] : 0;// HH.
    $value[4] = isset($value[4]) ? (int) $value[4] : 0;// MM.
    $value[5] = isset($value[5]) ? (int) $value[5] : 0;// SS.

    if (!checkdate($value[1], $value[2], $value[0])) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
    }

    // Get current timezone
    $tz = date_default_timezone_get();

    try {
      // Set to GMT/UTC so we can convert without timezone problems.
      date_default_timezone_set('UTC');

      // Convert to integer
      $time = gmmktime($value[3], $value[4], $value[5], $value[1], $value[2], $value[0]);

      // Back to GMT date again, make it valid.
      $result = gmdate($format, $time);
    }
    catch (Exception $e) {
      // Make sure we return to old time zone.
      date_default_timezone_set($tz);
      throw $e;
    }

    // Back to original timezone, so we dont screw with other time functions.
    date_default_timezone_set($tz);

    if ($result === FALSE) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => $param, '@type' => $param_type));
    }

    return $result;
  }

  /**
   * Return the current time UTC in SQL datetime format
   * (YYYY-MM-DD HH:MM:SS)
   *
   * @param $no_reset
   *   Boolean to control whether the timestamp is re-created or the static is
   *   used (if set).
   *
   * @return timestamp
   */
  static public function utc_current_timestamp_now($reset = FALSE) {
    static $utc_timestamp = NULL;

    if (isset($utc_timestamp) && !$reset) {
      return $utc_timestamp;
    }

    $utc_timestamp = (string)mediamosa_db::db_query('SELECT UTC_TIMESTAMP')->fetchField();
    return $utc_timestamp;
  }

  /**
   * Return the current time UTC with additional hours in SQL datetime format
   * (YYYY-MM-DD HH:MM:SS)
   *
   * @param $no_reset
   *   Boolean to control whether the timestamp is re-created or the static is
   *   used (if set).
   * @param $hours
   *   Hours to add.
   *
   * @return timestamp
   */
  static public function utc_current_timestamp_add_hours($reset = FALSE, $hours) {
    static $utc_timestamp_add_hours = array();

    if (isset($utc_timestamp_add_hours[$hours]) && !$reset) {
      return $utc_timestamp_add_hours[$hours];
    }

    $utc_timestamp_add_hours[$hours] = (string)mediamosa_db::db_query('SELECT DATE_ADD(UTC_TIMESTAMP, INTERVAL :hours HOUR)', array(':hours' => $hours))->fetchField();
    return $utc_timestamp_add_hours[$hours];
  }

  /**
   * Return the current time and in SQL datetime format (YYYY-MM-DD HH:MM:SS)
   *
   * Use this function for storing new time into database.
   *
   * This function is deprecated, use utc_current_timestamp_now() instead.
   *
   * @param $no_reset Boolean to control wheter the timestamp is re-created or the static is used (if set).
   * @return timestamp
   */
  static public function db_current_timestamp_now($reset = FALSE) {
    static $now_timestamp = NULL;

    if (isset($now_timestamp) && !$reset) {
      return $now_timestamp;
    }

    // Lets not break installer, dont use mediamosa_db here. String cast here,
    // d7 seems to have a problem with float values coming from this string?
    $now_timestamp = (string)db_query(strtr('SELECT #CURRENT_TIMESTAMP', array('#CURRENT_TIMESTAMP' => mediamosa_db::CURRENT_TIMESTAMP)))->fetchField();

    return $now_timestamp;
  }

  /**
   * Convert incoming application datetime to UTC datetime.
   *
   * @param string $app_date
   * @param string $app_timezone
   */
  static public function app_date2utc_date($app_date, $app_timezone) {

    // Walk through array, when array.
    if (is_array($app_date)) {
      foreach ($app_date as $key => $app_date_2) {
        $app_date[$key] = self::app_date2utc_date($app_date_2, $app_timezone);
      }

      return $app_date;
    }

    // Parse the date, date_parse is new in 5.2.
    $date_parsed = date_parse($app_date);

    // Get current timezone.
    $tz = date_default_timezone_get();

    try {
      // Set to timezone of app.
      assert(date_default_timezone_set($app_timezone));

      // Convert to integer
      $time = mktime($date_parsed['hour'], $date_parsed['minute'], $date_parsed['second'], $date_parsed['month'], $date_parsed['day'], $date_parsed['year']);

      // Back to GMT date again, make it valid.
      $result = gmdate('Y-m-d H:i:s', $time);
    }
    catch (Exception $e) {
      // Make sure we return to old time zone.
      date_default_timezone_set($tz);
      throw $e;
    }

    // Back to original timezone, so we dont screw with other time functions.
    date_default_timezone_set($tz);

    return $result;
  }

  /**
   * Convert time string to UTC datetime.
   *
   * @param string $app_date
   * @param string $app_timezone
   */
  static public function time_string2utc_date($app_date, $app_timezone) {

    // Walk through array, when array.
    if (is_array($app_date)) {
      foreach ($app_date as $key => $app_date_2) {
        $app_date[$key] = self::time_string2utc_date($app_date_2, $app_timezone);
      }

      return $app_date;
    }

    // Get current timezone.
    $tz = date_default_timezone_get();

    try {
      // Set to timezone of app.
      assert(date_default_timezone_set($app_timezone));

      $timestamp = strtotime($app_date);
      $result = gmdate('Y-m-d H:i:s', $timestamp);
    }
    catch (Exception $e) {
      // Make sure we return to old time zone.
      date_default_timezone_set($tz);
      throw $e;
    }

    // Back to original timezone, so we dont screw with other time functions.
    date_default_timezone_set($tz);

    return $result;
  }

  /**
   * Convert date from UTC to timezone of APP.
   *
   * @param string $utc_date
   * @param integer $app_id
   */
  static public function utc_date2app_date($utc_date, $app_timezone) {

    // Walk through array, when array.
    if (is_array($utc_date)) {
      foreach ($utc_date as $key => $utc_date_2) {
        $utc_date[$key] = self::utc_date2app_date($utc_date_2, $app_timezone);
      }

      return $utc_date;
    }

    // Parse the date, date_parse is new in 5.2.
    $date_parsed = date_parse($utc_date);

    // Get current timezone.
    $timezone_old = date_default_timezone_get();

    try {
      // Set to timezone of app.
      assert(date_default_timezone_set($app_timezone));

      // Convert to integer
      $time = gmmktime($date_parsed['hour'], $date_parsed['minute'], $date_parsed['second'], $date_parsed['month'], $date_parsed['day'], $date_parsed['year']);

      // Back to app date again, make it valid.
      $result = date('Y-m-d H:i:s', $time);
    }
    catch (Exception $e) {
      // Make sure we return to old time zone.
      date_default_timezone_set($timezone_old);
      throw $e;
    }

    // Back to original timezone, so we dont screw with other time functions.
    date_default_timezone_set($timezone_old);

    return $result;
  }

  // Convert any date into unix timestamp.
  static public function date2unix($any_date) {
    // Parse the date, date_parse is new in 5.2.
    $date_parsed = date_parse($any_date);

    // Convert to integer
    return gmmktime($date_parsed['hour'], $date_parsed['minute'], $date_parsed['second'], $date_parsed['month'], $date_parsed['day'], $date_parsed['year']);
  }
}
