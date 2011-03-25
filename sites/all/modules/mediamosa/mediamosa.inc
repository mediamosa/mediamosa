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
 * Main class MediaMosa.
 *
 *
 * Notes;
 * Drupal functions are mapped to this class as statics. This is done to prevent to much
 * core dependency on drupal and change behavor (if required). Exceptions are the db_*
 * (database) functions which are statics in the mediamosa_db class.
 *
 */

class mediamosa {

  // ------------------------------------------------------------------ Parameters.
  static private $is_db_logging = FALSE;

  static private $num_of_queries = array();
  static private $num_of_slow_queries = array();
  static private $total_query_time = array();

  // Stores last error that went through error_handler.
  static private $last_php_error = NULL;

  // ------------------------------------------------------------------ Constructor.
  function __construct() {
    // Checks before we do anything.
    $this->prechecks();
  }

  // ------------------------------------------------------------------ Public Functions.
  /**
   * Do basic checks on settings
   * will throw mediamosa_exception when checks are not met.
   */
  public function prechecks() {
    global $multibyte;
    if ($multibyte != UNICODE_MULTIBYTE) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_PRECHECK_FAILED, array('@message' => 'unicode multibyte is not enabled'));
    }
  }

  // Check if we will log rest calls to log().
  public function is_log_rest() {
    return variable_get('mediamosa_watchdog_rest', FALSE);
  }

  // Drupal usage for getting the current URI.
  public function get_request_uri() {
    return trim(implode('/', arg()), '/');
  }

  // Return the request_uri var, which can be /rest/?foo=foo
  static public function request_uri() {
    return request_uri(); // To Drupal mapped.
  }

  // ------------------------------------------------------------------ Protected Functions.

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Retrieve the main response object.
   * Must overridden in any child class.
   *
   * @return object
   *  The core object.
   */
  static public function get($new_mediamosa_response = NULL) {
    static $mediamosa_response = NULL;

    if (isset($new_mediamosa_response)) {
      // Save current, we need to return it.
      $current_mediamosa_response = $mediamosa_response;

      // Change to this.
      $mediamosa_response = $new_mediamosa_response;

      // Return current to restore later.
      return $current_mediamosa_response;
    }

    if (!isset($mediamosa_response)) {
      $mediamosa_response = new mediamosa_response();
    }

    return $mediamosa_response;
  }

  /**
   * Returns the timezone of the current session. Is by default Europe/Amsterdam.
   * Is set when REST call is run and the application timezone is used.
   */
  static public function get_restcall_timezone($timezone = NULL) {
    static $timezone_static = NULL;

    if (isset($timezone)) {
      $timezone_static = $timezone;
    }

    if (empty($timezone_static)) {
      return mediamosa_settings::MEDIAMOSA_DEFAULT_TIMEZONE; // 'Europe/Amsterdam'
    }

    return $timezone_static;
  }

  /**
   * Set the current timezone for this session.
   *
   * @param unknown_type $timezone
   */
  static public function set_restcall_timezone($timezone) {
    mediamosa::get_restcall_timezone($timezone);
  }

  /**
   * Convert incoming UTC date to the application timezone.
   *
   * @param string $utc_date
   */
  static public function utcdate2appdate($utc_date) {
    return mediamosa_datetime::utc_date2app_date($utc_date, self::get_restcall_timezone());
  }

  /**
   * Return the current defined installation id.
   */
  static public function get_server_id() {
    return mediamosa::variable_get('mediamosa_installation_id', 'default');
  }

  /**
   * MediaMosa error handler.
   *
   * @param string $error_level
   * @param string $message
   * @param string $filename
   * @param integer $line
   * @param string $context
   * @param array $caller
   * @return boolean
   */
  static public function mediamosa_error_handler($error_level, $message, $filename, $line, $context, $caller = NULL) {
    assert(isset($context));
    assert(isset($line));
    assert(isset($filename));

    if ($error_level) {

      // These constants are documented at http://php.net/manual/en/errorfunc.constants.php.
      $types = array(
        E_ERROR => 'Error',
        E_WARNING => 'Warning',
        E_PARSE => 'Parse error',
        E_NOTICE => 'Notice',
        E_CORE_ERROR => 'Core error',
        E_CORE_WARNING => 'Core warning',
        E_COMPILE_ERROR => 'Compile error',
        E_COMPILE_WARNING => 'Compile warning',
        E_USER_ERROR => 'User error',
        E_USER_WARNING => 'User warning',
        E_USER_NOTICE => 'User notice',
        E_STRICT => 'Strict warning',
        E_RECOVERABLE_ERROR => 'Recoverable fatal error'
      );

      // If the backtrace was provided, then use it.
      $caller = isset($caller) ? $caller : _drupal_get_last_caller(debug_backtrace());

      // Always set the last error.
      self::$last_php_error = array(
        'type' => isset($types[$error_level]) ? $types[$error_level] : 'Unknown error',
        'message' => $message,
        'function' => $caller['function'],
        'file' => $caller['file'],
        'line' => $caller['line']
      );

      // Log to file, just to be sure if watchdog should fail.
      mediamosa_debug::error_log($message . ' (function: )' . $caller['function'], $caller['file'], $caller['line']);

      // Only log if error_reporting returns TRUE.
      if (error_reporting()) {
        // Unlike drupal, we don't treat recoverable errors as fatal, preventing Drupal to display error page.
        _drupal_log_error(array(
          '%type' => isset($types[$error_level]) ? $types[$error_level] : 'Unknown error',
          '!message' => check_plain($message),
          '%function' => $caller['function'],
          '%file' => $caller['file'],
          '%line' => $caller['line'],
        ), FALSE);
      }
    }

    // Lets return TRUE as error handled.
    return TRUE;
  }

  /**
   * Returns the last php error that was handled by our error handler.
   */
  static public function last_php_error() {

    // If no last error, then return FALSE.
    return !isset(self::$last_php_error) ? FALSE : self::$last_php_error;
  }

  /**
   * Returns message from the last_php_error.
   * Currently just returns 'message' value, but might return more in future.
   * Intended for usage inside exception messages, like 'reason' for failure.
   */
  static public function last_php_error_message() {

    // If no last error, then return 'unknown' text.
    return isset(self::$last_php_error) ? self::$last_php_error['message'] : 'unknown';
  }

  /**
   * Is the Drupal Interface enabled?
   * @return bool
   */
  static public function is_admin_enabled() {
    return mediamosa::variable_get('mediamosa_admin', FALSE);
  }

  /**
   * Is the APP REST Interface enabled?
   *
   * @return bool
   */
  static public function is_app_enabled() {
    // If we are running an unit test, then we are enabled unless disable_app is specified.
    if (mediamosa::in_simpletest_sandbox()) {
      // The name of the http var.
      $disable_app = 'HTTP_' . mediamosa_unicode::strtoupper(str_replace('-', '_', mediamosa_settings::X_MEDIAMOSA_DISABLE_APP));

      // Based if this is present, we always are APP on simpletest mode.
      return isset($_SERVER[$disable_app]) ? FALSE : TRUE;
    }

    return (mediamosa::variable_get('mediamosa_app', FALSE) || self::is_app_upload_enabled() || self::is_app_download_enabled());
  }

  /**
   * Is the APP upload REST Interface enabled?
   *
   * @return bool
   */
  static public function is_app_download_enabled() {
    return mediamosa::variable_get('mediamosa_app_download', FALSE);
  }

  /**
   * Is the APP download REST Interface enabled?
   *
   * @return bool
   */
  static public function is_app_upload_enabled() {
    return mediamosa::variable_get('mediamosa_app_upload', FALSE);
  }

  /**
   * Bypass Drupal 'user/' path collisions.
   *
   * @return boolean
   */
  static public function show_drupal_login() {

    // Check if interface is enabled, if so, check if we are logged in.
    // If not logged in, allow user/login and user/password url.
    if (mediamosa::is_admin_enabled()) {
      if (user_is_logged_in()) {
        // Logged in as Drupal user, allow.
        return TRUE;
      }

      // Not logged in as Drupal user.
      // We allow login and password recovery.
      if (mediamosa_unicode::substr(mediamosa::request_uri(), -10) == 'user/login' || mediamosa_unicode::substr(mediamosa::request_uri(), -13) == 'user/password') {
        return TRUE;
      }
    }

    return FALSE;
  }

  /**
   * Exit the page with an 503 error.
   *
   * @param string $message
   *  Message for the user.
   */
  static public function exit_503($message) {
    drupal_maintenance_theme();
    drupal_add_http_header('Status', '503 Service unavailable');
    drupal_set_title(t('Site offline'));
    print theme('maintenance_page', array('content' => filter_xss_admin($message)));
    drupal_page_footer();
    exit;
  }

  /**
   * Exit the page with an 404 error.
   *
   * @param string $message
   *  Message for the user.
   */
  static public function exit_404($message) {
    drupal_maintenance_theme();
    drupal_add_http_header('Status', '404 Page not found');
    drupal_set_title(t('Page not found'));
    print theme('maintenance_page', array('content' => filter_xss_admin($message)));
    drupal_page_footer();
    exit;
  }

  /**
   * Duplicated t() drupal function mapped here as self::t()
   * For now we don't translate the core text, to save CPU time.
   * Dont really see the point of translating text that is only
   * visible to programmers anyway.
   * Removed theme function here too.
   *
   * @param string $string
   *  Message.
   * @param array $args
   *  Optional arguments.
   * @param string $langcode
   *  Language (ignored).
   * @return string.
   *  Formatted string.
   */
  static public function t($string, $args = array(), $langcode = FALSE) {
    assert($langcode || $langcode === FALSE);
    assert(is_array($args));

    if (empty($args)) {
      return $string;
    }

    // Transform arguments before inserting them.
    foreach ($args as $key => $value) {
      switch ($key[0]) {
        case '@':
        default:
          $args[$key] = check_plain($value);
          break;

        case '%':
        default:
          $args[$key] = '<em>' . check_plain($value) . '</em>';
          break;

        case '!':
          // Pass-through.
          break;
      }
    }

    return strtr($string, $args);
  }

  /**
   * Returns TRUE or FALSE if we are inside a sandbox.
   */
  static public function in_simpletest_sandbox() {
    $test_info = &$GLOBALS['drupal_test_info'];
    return (!empty($test_info['test_run_id']) ? TRUE : FALSE);
  }

  /**
   * Wrapper around the drupal_http_request function.
   * See drupal functions for details.
   */
  static public function drupal_http_request($url, array $a_options = array()) {
    return drupal_http_request($url, $a_options);
  }

  /**
   * Wrapper around variable_get().
   */
  static public function variable_get($name, $default = NULL) {
    return variable_get($name, $default);
  }

  /**
   * Wrapper around variable_set().
   */
  static public function variable_set($name, $value) {
    variable_set($name, $value);
  }

  /**
   * Wrapper around the drupal_goto function.
   * See drupal functions for details.
   */
  static public function drupal_goto($path = '', $query = NULL, $fragment = NULL, $http_response_code = 302) {
    return drupal_goto($path, $query, $fragment, $http_response_code);
  }

  /**
   * Wrapper around drupal_install_schema().
   *
   * @param string $module
   *   Module name.
   * @param string $table_prefix
   *   Prefix of the tables. Default empty string.
   * @param boolean $make_foreignkey
   *   Make foreign keys (TRUE), or not (FALSE). Default TRUE.
   */
  static public function drupal_install_schema($module, $table_prefix = '', $make_foreignkey = TRUE) {
    try {
      $schema = drupal_get_schema_unprocessed($module);
      _drupal_schema_initialize($schema, $module, FALSE);

      // Changes we might want in MediaMosa.
      $schema = mediamosa_db::db_install_schema_preprocess($module, $schema);

      foreach ($schema as $name => $table) {
        mediamosa_db::db_create_table($table_prefix . $name, $table);
      }
    }
    catch (Exception $e) {
      // ignore.
    }
    // Mark it installed, so its not install again.
    $versions = drupal_get_schema_versions($module);
    $version = $versions ? max($versions) : SCHEMA_INSTALLED;
    drupal_set_installed_schema_version($module, $version);

    // Drupal 7 does not create foreign keys yet, so we will.
    if ($make_foreignkey) {
      mediamosa_db::db_install_schema_foreign_keys($module);
    }
  }

  /**
   * Wrapper around drupal_uninstall_schema()
   */
  static public function drupal_uninstall_schema($module) {
    return drupal_uninstall_schema($module);
  }

  /**
   * Enable db logging for timed queries.
   */
  static public function enable_db_logging($name) {
    $previous_db_logging = self::$is_db_logging;
    self::log('DB logging starting.', array(), WATCHDOG_DEBUG, $name);
    self::$is_db_logging = $name;
    self::$num_of_slow_queries[$name] = self::$num_of_queries[$name] = 0;
    self::$total_query_time[$name] = microtime(TRUE);

    return $previous_db_logging;
  }

  /**
   * Enable db logging for timed queries.
   */
  static public function disable_db_logging($name) {
    if (self::$is_db_logging !== FALSE) {
      self::log('DB logging ending, time; #time, total queries; #num_of_queries, slow queries; #num_of_slow_queries.', array('#time' => round(microtime(TRUE) - self::$total_query_time[self::$is_db_logging], 4), '#num_of_queries' => self::$num_of_queries[self::$is_db_logging], '#num_of_slow_queries' => self::$num_of_slow_queries[self::$is_db_logging]), WATCHDOG_DEBUG, self::$is_db_logging);

      unset(self::$num_of_queries[self::$is_db_logging]);
      unset(self::$num_of_slow_queries[self::$is_db_logging]);
      unset(self::$total_query_time[self::$is_db_logging]);
    }

    self::$is_db_logging = $name;
  }

  /**
   * Enable db logging for timed queries.
   */
  static public function is_db_logging() {
    return self::$is_db_logging;
  }

  /**
   * Internals are considered safe when they match our password
   * stored in a variable. Although its not impossible to intercept
   * the password from a internal REST call, we still consider this
   * good protection.
   */
  static public function get_internal_password() {
    $passwrd = self::variable_get('mediamosa_internal_password', NULL);

    if (isset($passwrd)) {
      return $passwrd;
    }

    // Create new one.
    $passwrd = md5(drupal_random_bytes(64));

    // Store it.
    variable_set('mediamosa_internal_password', $passwrd);

    // Log it, this should only happen when have a clean install.
    mediamosa_watchdog::log('Generated new internal password.');

    // Return the new passwrd.
    return $passwrd;
  }

  /**
   * Test if we are running on windows.
   */
  static public function is_windows_platform() {
    return (mediamosa_unicode::substr(PHP_OS, 0, 3) == 'WIN');
  }
}
