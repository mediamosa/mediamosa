<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
 * This module contains the login procedure, cookie handling and basic GET
 * and POST functions to communicate with MediaMosa.
 */

/**
 * mediamosa_connector is a class which conains all functions for communication
 * with MediaMosa backend REST interface.
 */
class mediamosa_connector {

  // ------------------------------------------------------------------ Consts.
  // Maximum of items in result.
  const MAX_RESULT_SET = 200;

  // Maximum of entries in our log, lets not overflow memory.
  // Log will store the last # of requests and will drop the first.
  const MAX_LOG_ENTRIES = 50;

  // Connector error codes.
  const ERROR_NONE = 0;
  const ERROR_CONNECTOR_NOT_SETUP = 1;
  const ERROR_UNABLE_TO_LOGIN = 2;
  const ERROR_RESPONSE_RETURNED_ERROR = 3;
  const ERROR_UNKNOWN = 4;

  // ------------------------------------------------------------------ Members.
  // The URL to connect to.
  private $url;

  // The cookie for the MediaMosa session.
  private $cookie;

  // The user to use.
  private $user_id;

  // The group, optional.
  private $group_id;
  private $is_app_admin;
  private $log = array();

  // Stored username, password.
  private $username;
  private $password;

  // Contains possible last error code.
  private $last_error = self::ERROR_NONE;
  private $last_error_text = '';

  // -------------------------------------------------------------- Constructor.
  public function __construct($username = NULL, $password = NULL, $url = NULL) {
    $this->url = isset($url) ? $url : variable_get('mediamosa_connector_url', NULL);

    // User / password.
    $this->username = isset($username) ? $username : variable_get('mediamosa_connector_username', NULL);
    $this->password = isset($password) ? $password : variable_get('mediamosa_connector_password', NULL);

    // Get cookie.
    $cookies = variable_get('mediamosa_connector_cookies', array());
    $this->cookie = !empty($cookies[$this->url][$this->username][$this->password]) ? $cookies[$this->url][$this->username][$this->password] : NULL;
  }

  // --------------------------------------------------------- Public Functions.
  /**
   * Return the last error.
   */
  public function get_last_error() {
    return $this->last_error;
  }

  /**
   * Return the last error as text.
   *
   * Will return FALSE when no error was found.
   */
  public function get_last_error_text() {

    if (!empty($this->last_error_text)) {
      return $this->last_error_text;
    }

    return FALSE;
  }

  /**
   * Return an ID we can use to identify this connection.
   */
  public function get_connection_id() {
    return empty($this->username) || empty($this->url) ? FALSE : sha1($this->username . '@' . $this->url);
  }

  /**
   * Reset the session, so login will be retried.
   */
  public function reset_session($username = NULL, $password = NULL, $url = NULL) {
    $url = isset($url) ? $url : $this->url;

    // User / password.
    $username = isset($username) ? $username : $this->username;
    $password = isset($password) ? $password : $this->password;

    $this->cookie = NULL;
    $cookies = variable_get('mediamosa_connector_cookies', array());
    if (isset($cookies[$this->url][$this->username][$this->password])) {
      unset($cookies[$this->url][$this->username][$this->password]);
      variable_set('mediamosa_connector_cookies', $cookies);
    }

    // Reset last error.
    $this->last_error = self::ERROR_NONE;
    $this->last_error_text = '';
  }

  /**
   * @param string $uri
   *  The REST uri.
   * @param array $options
   *   (optional) An array which can have one or more of following keys:
   *   - user_id (mediamosa)
   *       The user_id of the Mediamosa call. Default is empty.
   *   - group_id (mediamosa)
   *       The group_id of the Mediamosa call. Default is empty.
   *   - fatal
   *       Boolean value, TRUE makes the call throw an exception if the call
   *       failed. That means, failed to call, failed to access the rest etc.
   *       Empty results are not fatal. It will either return a response or
   *       throw exception. Default is FALSE (call will returns response or
   *       FALSE).
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - data
   *       A string containing the request body.
   *       Or
   *       A array as name => value.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 1.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 60 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   *
   */
  public function request($uri, array $options = array()) {

    // No response yet.
    $response = FALSE;

    // Setup options.
    $options += array(
      'fatal' => FALSE, // Default is not to react to problems with exceptions.
      'user_id' => '',
      'group_id' => '',
      'mediamosa_version' => '', // Latest.
    );

    // Use user_id?
    if (!empty($options['user_id'])) {
      assert(empty($options['data']['user_id']));
      $options['data']['user_id'] = $options['user_id'];
    }

      // Use group_id?
    if (!empty($options['group_id'])) {
      assert(empty($options['data']['group_id']));
      $options['data']['group_id'] = $options['group_id'];
    }

    // Use mediamosa_version?
    if (!empty($options['mediamosa_version'])) {
      assert(empty($options['data']['mediamosa_version']));
      $options['data']['mediamosa_version'] = $options['mediamosa_version'];
    }

    // Copy settings.
    $fatal = $options['fatal'];
    unset($options['fatal'], $options['user_id'], $options['group_id']);

    // Initialize as unknown failure.
    $this->last_error = self::ERROR_UNKNOWN;
    $this->last_error_text = t('Unable to execute Mediamosa REST call @uri', array('@uri' => $uri));

    try {
      if (empty($this->url)) {
        throw new Exception(t('Unable to do request, connector not setup.'), self::ERROR_CONNECTOR_NOT_SETUP);
      }

      // If cookie not set, then login first.
      if (!isset($this->cookie) && !$this->do_login()) {
        throw new Exception(t('Unable to do request, failed to login.'), self::ERROR_UNABLE_TO_LOGIN);
      }

      // Try the request.
      $response = $this->do_request($uri, $options);

      // If response failed, then try to login.
      if (!empty($response) && !empty($response->xml) && in_array((int) $response->xml->header->request_result_id, array(mediamosa_sdk::ERRORCODE_ACCESS_DENIED, mediamosa_sdk::ERRORCODE_NOT_AUTHORIZED))) {

        // Not authorized? Try to login.
        if (self::do_login()) {
          $response = $this->do_request($uri, $options);
        }
      }

      // Check the final response.
      if (!empty($response) && !empty($response->xml) && !empty($response->xml->header)) {
        $result_id = (int) $response->xml->header->request_result_id;
        $result_description = (string) $response->xml->header->request_result_description;

        if ($fatal && !in_array($result_id, array(mediamosa_sdk::ERRORCODE_OKAY, mediamosa_sdk::ERRORCODE_EMPTY_RESULT))) {

          if (empty($result_description)) {
            $this->last_error_text = t('Unable to execute Mediamosa REST call @uri (@code)', array('@uri' => $uri, '@code' => $result_id));
          }
          else {
            $this->last_error_text = t('@description (@code)', array('@description' => $result_description, '@code' => $result_id));
          }

          throw new Exception($this->last_error_text, self::ERROR_RESPONSE_RETURNED_ERROR);
        }

        return $response;
      }
    }
    catch (Exception $e) {
      // Catch and log.
      self::watchdog('mediamosa connector', $e->getMessage());

      $this->last_error = $e->getCode();
      $this->last_error_text = $e->getMessage();
    }

    // When fatal, throw exception.
    if ($fatal) {
      throw new Exception($this->last_error_text, $this->last_error);
    }

    return FALSE;
  }

  /**
   * Same as request() accept force to GET method.
   *
   * @param string $uri
   * @param array $options
   */
  public function request_get($uri, array $options = array()) {
    $options['method'] = 'GET';
    return $this->request($uri, $options);
  }

  /**
   * Same as request() accept force to GET method.
   *
   * @param string $uri
   * @param array $options
   */
  public function request_post($uri, array $options = array()) {
    $options['method'] = 'POST';
    return $this->request($uri, $options);
  }

  // -------------------------------------------------------- Private Functions.
  private function do_login() {
    if (empty($this->url)) {
      self::watchdog('mediamosa connector', 'Unable to do request, connector not setup.');
      return FALSE;
    }

    // Remove cookie, so we are sure we get a new session.
    $this->reset_session();

    // Step 1: request the challenge
    $data = array('dbus' => 'AUTH DBUS_COOKIE_SHA1 ' . $this->username);
    $response = $this->do_request('login', array('method' => 'POST', 'data' => $data));

    if (!$response || !$response->xml) {
      return FALSE;
    }

    if ($response->xml->header->request_result_id == mediamosa_sdk::HTTP_NOT_FOUND) {
      drupal_set_message(t('Enable the Application Authentication module, unable to login.'), 'error');
      return FALSE;
    }

    // Check if we got DATA response.
    if (drupal_substr($response->xml->items->item->dbus, 0, 5) != 'DATA ') {
      return FALSE;
    }

    // Parse the response.
    // FIXME: We only store the 1st cookie here(!).
    list($this->cookie) = explode(';', $response->headers['set-cookie']);

    // Store the cookie for later usage.
    $cookies = variable_get('mediamosa_connector_cookies', array());
    $cookies[$this->url][$this->username][$this->password] = $this->cookie;
    variable_set('mediamosa_connector_cookies', $cookies);

    // Set DBUS data.
    $dbus_data = explode(' ', $response->xml->items->item->dbus);

    // Step 2: Do challenge.
    $challenge = $dbus_data[3];
    $random = drupal_substr(md5(microtime(TRUE)), 0, 10);
    $challenge_response = sha1(sprintf('%s:%s:%s', $challenge, $random, $this->password));
    $data = array('dbus' => sprintf('DATA %s %s', $random, $challenge_response));
    $response = $this->do_request('login', array('method' => 'POST', 'data' => $data));
    if (!$response || !$response->xml) {
      return FALSE;
    }

    // Lets check if its ok.
    return drupal_substr($response->xml->items->item->dbus, 0, 2) == 'OK';
  }

  /**
   * @param string $uri
   *  The REST uri.
   * @param array $options
   *   (optional) An array which can have one or more of following keys:
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - data
   *       A string containing the request body.
   *       Or
   *       A array as name => value.
   *       Note: The request body is only used on POST and PUT calls.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 1.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 60 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   */
  private function do_request($uri, array $options = array()) {
    // Merge the default options.
    $options += array(
      'headers' => array(),
      'method' => 'GET',
      'data' => NULL,
      'max_redirects' => 1, // REST call should not redirect more than 1.
      'timeout' => 60,
    );

    // Set cookie.
    if (isset($this->cookie)) {
      $options['headers']['Cookie'] = $this->cookie;
    }

    if (isset($options['data'])) {
      if (is_array($options['data'])) {
        $options['data'] = http_build_query($options['data'], '', '&');
      }
    }

    $url = $this->url;
    $url .= (drupal_substr($url, -1, 1) == '/' ? '' : '/'); // Make sure it ends with '/'.

    switch ($options['method']) {
      case 'GET':
        // Method GET does not allow data body, put the urlencode after the uri.
        $query_string_pos = strpos($uri, '?');
        $query_string = array();
        if ($query_string_pos !== FALSE) {
          $query_string[] = drupal_substr($uri, $query_string_pos + 1);

          // Strip it.
          $uri = drupal_substr($uri, 0, $query_string_pos);
        }

        // If data is set, then add it to the query string.
        if (isset($options['data']) && $options['data'] != '') {
          $query_string[] = $options['data'];
        }

        // Its in the URL, so skip it.
        unset($options['data']);

        // Now add to uri (if any).
        if (count($query_string)) {
          $uri .= '?' . implode('&', $query_string);
        }
        break;

      case 'POST':
        // POST needs data as www-form-urlencoded.
        $options['headers']['Content-Type'] = 'application/x-www-form-urlencoded';
        break;
    }

    // Add clean uri.
    $url .= $uri;
    $start = microtime(TRUE);

    // The call to 7.x is different.
    if (DRUPAL_CORE_COMPATIBILITY == '7.x') {
      $response = drupal_http_request($url, $options);
    }
    else {
      $response = drupal_http_request($url, $options['headers'], $options['method'], $options['data']);

      // Make the header items names lowercase like in D7.
      foreach ($response->headers as $name => $value) {
        unset($response->headers[$name]);
        $response->headers[drupal_strtolower($name)] = $value;
      }
    }

    $end = microtime(TRUE);

    // If the log size overflows our buffer, lose the first.
    if (count($this->log) > self::MAX_LOG_ENTRIES) {
      array_shift($this->log);
    }

    // Set to empty.
    $response->xml = NULL;
    // If we detect xml output, fill the xml member.
    if ((int) $response->code == 200 && (strpos($response->headers['content-type'], 'text/xml') !== FALSE || drupal_substr($response->data, 0, drupal_strlen('<?xml')) == '<?xml')) {
      try {
        $response->xml = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        // Dont care.
        $response->xml = NULL;
      }
    }

    // Log the request.
    $this->log[] = array(
      'method' => $options['method'],
      'url' => $url,
      'response_time' => round($end - $start, 3),
      'result_id' => (isset($response->xml) ? (int)$response->xml->header->request_result_id : 0),
      'result_description' => (isset($response->xml) ? (string) $response->xml->header->request_result_description : ''),
      'errors' => (isset($response->xml) && isset($response->xml->header->errors) ? (is_array($response->xml->header->errors) ? implode('<br />', $response->xml->header->errors) : (string) $response->xml->header->errors) : NULL),
      'data' => $response->data,
    );

    if (variable_get('mediamosa_connector_debug', FALSE)) {
      $_SESSION['mediamosa_connector_debug_info'] = $this->log;
    }

    // Must be HTTP 200 response.
    if ((int) $response->code != 200) {
      return FALSE;
    }

    // Basic version.
    $version = '1.0.0';
    if (!empty($response->xml->header->version)) {
      $version = $response->xml->header->version; // Position is the same on all MediaMosa versions.
    }

    // Store an version array in the response object.
    $response->mediamosa_version = mediamosa_sdk::parse_version($version);

    // Return result.
    return $response;
  }

  /**
   * Check the result.
   *
   * @param xml_object $response
   * @param string $ok
   * @param string $error
   */
  static public function check_result($response, $ok = NULL, $error = NULL) {
    $result_id = $response ? (int)$response->xml->header->request_result_id : 0;

    if ($result_id != mediamosa_sdk::ERRORCODE_OKAY && $result_id != mediamosa_sdk::ERRORCODE_EMPTY_RESULT) {
      if (isset($error)) {
        $error = t($error, array('%code' => $result_id, '%message' => $response->xml->header->request_result_description));
        drupal_set_message($error, 'error');
      }

      return FALSE;
    }

    if (isset($ok)) {
      drupal_set_message(t($ok));
    }

    return TRUE;
  }

  /**
   * Simple function to correctly convert simplexml to array.
   *
   * @param object $xml
   */
  private static function _simplexml2array($xml) {
    $xml_copy = $xml;
    if (is_object($xml) && get_class($xml) == 'SimpleXMLElement') {
      foreach ($xml->attributes() as $key => $value) {
        if ($value) {
          $attributes[$key] = (string)$value;
        }
      }

      $xml = get_object_vars($xml);
    }

    if (is_array($xml)) {
      if (empty($xml)) {
        return (string)$xml_copy;
      }

      $result = array();
      foreach ($xml as $key => $value) {
        $result[$key] = self::_simplexml2array($value);
      }

      return $result;
    }

    return (string)$xml;
  }

  /**
   * Basicly the same as simplexml2array, with the exception that it will fix
   * the problem with single and multi items where a single item will end up
   * in a non-indexed array and multi items will.
   *
   * @param object $xml
   */
  public static function responsexml2array($xml) {
    if (!$xml) {
      return $xml;
    }

    // Convert to nice array.
    $xml = self::_simplexml2array($xml);

    if (!isset($xml['items']['item'])) {
      return $xml;
    }

    // Fix the problem with 1 or 2+ items in array.
    if (!isset($xml['items']['item'][0])) {
      $tmp = array(0 => $xml['items']['item']);
      unset($xml['items']);
      $xml['items']['item'] = $tmp;
    }

    // Fix problem with 1 or 2+ mediafiles in array.
    foreach ($xml['items']['item'] as &$item) {
      if (!empty($item['mediafiles']) && !isset($item['mediafiles']['mediafile'][0])) {
        $tmp = array(0 => $item['mediafiles']['mediafile']);
        unset($item['mediafiles']['mediafile']);
        $item['mediafiles']['mediafile'] = $tmp;
      }
    }

    return $xml;
  }

  /**
   * Correctly convert to array and return the items object from the mediamosa
   * response as array. Will return FALSE when supplied with FALSE.
   *
   * @param object $response
   */
  public static function response2array($response) {
    return $response ? self::responsexml2array($response->xml) : FALSE;
  }

  /**
   * Wrapper around watchdog, minor diff. with drupal 6 version.
   */
  public function watchdog($type, $message, $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL) {
    if (DRUPAL_CORE_COMPATIBILITY == '6.x') {
      $type = substr($type, 0, 16);// Type is max len 16 chars.
    }

    watchdog($type, $message, $variables, $severity, $link);
  }

  /**
   * Variable_get wrapper for connection dependent variables.
   */
  public function variable_get($name, $default = NULL) {
    // TODO
    return variable_get($name, $default);
  }

  /**
   * Variable_set wrapper for connection dependent variables.
   */
  public function variable_set($name, $value) {
    // TODO
    variable_set($name, $value);
  }

  /**
   * Variable_del wrapper for connection dependent variables.
   */
  public function variable_del($name) {
    // TODO
    variable_del($name);
  }
}
