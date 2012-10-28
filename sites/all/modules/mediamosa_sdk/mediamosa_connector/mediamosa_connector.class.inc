<?php
/**
 * @file
 * The mediamosa_connector class.
 *
 * This module contains the login procedure, cookie handling and basic GET
 * and POST functions to communicate with MediaMosa.
 */

// Prevent notice/warnings on code checkers.
if (!function_exists('drupal_generate_test_ua')) {
  function drupal_generate_test_ua() {}
}

/**
 * mediamosa_connector is a class which contains all functions for communication
 * with MediaMosa backend REST interface.
 */
class mediamosa_connector {

  // ------------------------------------------------------------------ Consts.
  // Maximum of items in result.
  const MAX_RESULT_SET = 200;

  // Maximum of entries in our log, lets not overflow memory.
  // Will log the first # of requests.
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

    // Started when?
    $start = microtime(TRUE);

    // Do the request.
    $mediamosa_connector_response = self::http_request($url, $options);

    // Ended when?
    $end = microtime(TRUE);

    // Only add to log if we allow it for memory.
    if (count($this->log) < self::MAX_LOG_ENTRIES) {
      // Log the request.
      $this->log[] = array(
        'method' => $options['method'],
        'url' => $url,
        'response_time' => round($end - $start, 3),
        'result_id' => (!empty($mediamosa_connector_response->xml) ? (int) $mediamosa_connector_response->xml->header->request_result_id : 0),
        'result_description' => (isset($mediamosa_connector_response->xml) ? (string) $mediamosa_connector_response->xml->header->request_result_description : ''),
        'errors' => (isset($mediamosa_connector_response->xml) && isset($mediamosa_connector_response->xml->header->errors) ? (is_array($mediamosa_connector_response->xml->header->errors) ? implode('<br />', $mediamosa_connector_response->xml->header->errors) : (string) $mediamosa_connector_response->xml->header->errors) : NULL),
        'data' => $mediamosa_connector_response->data,
      );
    }

    if (variable_get('mediamosa_connector_debug', FALSE)) {
      $_SESSION['mediamosa_connector_debug_info'] = $this->log;
    }

    // Must be HTTP 200 response.
    if ((int) $mediamosa_connector_response->code != 200) {
      return FALSE;
    }

    // Basic version.
    $version = '1.0.0';
    if (!empty($mediamosa_connector_response->xml->header->version)) {
      $version = $mediamosa_connector_response->xml->header->version; // Position is the same on all MediaMosa versions.
    }

    // Store an version array in the response object.
    $mediamosa_connector_response->mediamosa_version = mediamosa_sdk::parse_version($version);

    // Return result.
    return $mediamosa_connector_response;
  }

  /**
   * Perform an HTTP request.
   *
   * Original code from Drupal 7.2. See orignal copyright http://drupal.org.
   *
   * Changes to code to make it compatible with Drupal 6 and 7 simpletest. Also
   * will work outside Drupal.
   *
   * This is a flexible and powerful HTTP client implementation. Correctly
   * handles GET, POST, PUT or any other HTTP requests. Handles redirects.
   *
   * @param $url
   *   A string containing a fully qualified URI.
   * @param array $options
   *   (optional) An array that can have one or more of the following elements:
   *   - headers: An array containing request headers to send as name/value pairs.
   *   - method: A string containing the request method. Defaults to 'GET'.
   *   - data: A string containing the request body, formatted as
   *     'param=value&param=value&...'. Defaults to NULL.
   *   - max_redirects: An integer representing how many times a redirect
   *     may be followed. Defaults to 3.
   *   - timeout: A float representing the maximum number of seconds the function
   *     call may take. The default is 30 seconds. If a timeout occurs, the error
   *     code is set to the HTTP_REQUEST_TIMEOUT constant.
   *   - context: A context resource created with stream_context_create().
   *
   * @return object
   *   An object that can have one or more of the following components:
   *   - request: A string containing the request body that was sent.
   *   - code: An integer containing the response status code, or the error code
   *     if an error occurred.
   *   - protocol: The response protocol (e.g. HTTP/1.1 or HTTP/1.0).
   *   - status_message: The status message from the response, if a response was
   *     received.
   *   - redirect_code: If redirected, an integer containing the initial response
   *     status code.
   *   - redirect_url: If redirected, a string containing the redirection location.
   *   - error: If an error occurred, the error message. Otherwise not set.
   *   - headers: An array containing the response headers as name/value pairs.
   *     HTTP header names are case-insensitive (RFC 2616, section 4.2), so for
   *     easy access the array keys are returned in lower case.
   *   - data: A string containing the response body that was received.
   */
  private function http_request($url, array $options = array()) {
    // Create our response object.
    $mediamosa_connector_response = new mediamosa_connector_response();

    // Parse the URL and make sure we can handle the schema.
    $uri = @parse_url($url);

    if ($uri == FALSE) {
      $mediamosa_connector_response->error = 'unable to parse URL';
      $mediamosa_connector_response->code = -1001;
      return $mediamosa_connector_response;
    }

    if (!isset($uri['scheme'])) {
      $mediamosa_connector_response->error = 'missing schema';
      $mediamosa_connector_response->code = -1002;
      return $mediamosa_connector_response;
    }

    timer_start(__FUNCTION__);

    // Merge the default options.
    $options += array(
      'headers' => array(),
      'method' => 'GET',
      'data' => NULL,
      'max_redirects' => 3,
      'timeout' => 30.0,
      'context' => NULL,
    );

    // stream_socket_client() requires timeout to be a float.
    $options['timeout'] = (float) $options['timeout'];

    switch ($uri['scheme']) {
      case 'http':
      case 'feed':
        $port = isset($uri['port']) ? $uri['port'] : 80;
        $socket = 'tcp://' . $uri['host'] . ':' . $port;
        // RFC 2616: "non-standard ports MUST, default ports MAY be included".
        // We don't add the standard port to prevent from breaking rewrite rules
        // checking the host that do not take into account the port number.
        $options['headers']['Host'] = $uri['host'] . ($port != 80 ? ':' . $port : '');
        break;

      case 'https':
        // Note: Only works when PHP is compiled with OpenSSL support.
        $port = isset($uri['port']) ? $uri['port'] : 443;
        $socket = 'ssl://' . $uri['host'] . ':' . $port;
        $options['headers']['Host'] = $uri['host'] . ($port != 443 ? ':' . $port : '');
        break;

      default:
        $mediamosa_connector_response->error = 'invalid schema ' . $uri['scheme'];
        $mediamosa_connector_response->code = -1003;
        return $mediamosa_connector_response;
    }

    if (empty($options['context'])) {
      $fp = @stream_socket_client($socket, $errno, $errstr, $options['timeout']);
    }
    else {
      // Create a stream with context. Allows verification of a SSL certificate.
      $fp = @stream_socket_client($socket, $errno, $errstr, $options['timeout'], STREAM_CLIENT_CONNECT, $options['context']);
    }

    // Make sure the socket opened properly.
    if (!$fp) {
      // When a network error occurs, we use a negative number so it does not
      // clash with the HTTP status codes.
      $mediamosa_connector_response->code = -$errno;
      $mediamosa_connector_response->error = trim($errstr) ? trim($errstr) : t('Error opening socket @socket', array('@socket' => $socket));

      if (function_exists('variable_set')) {
        // Mark that this request failed. This will trigger a check of the web
        // server's ability to make outgoing HTTP requests the next time that
        // requirements checking is performed.
        // See system_requirements()
        variable_set('drupal_http_request_fails', TRUE);
      }

      return $mediamosa_connector_response;
    }

    // Construct the path to act on.
    $path = isset($uri['path']) ? $uri['path'] : '/';
    if (isset($uri['query'])) {
      $path .= '?' . $uri['query'];
    }

    // Merge the default headers.
    $options['headers'] += array(
      'User-Agent' => 'Drupal (+http://drupal.org/)',
    );

    // Only add Content-Length if we actually have any content or if it is a POST
    // or PUT request. Some non-standard servers get confused by Content-Length in
    // at least HEAD/GET requests, and Squid always requires Content-Length in
    // POST/PUT requests.
    $content_length = strlen($options['data']);
    if ($content_length > 0 || $options['method'] == 'POST' || $options['method'] == 'PUT') {
      $options['headers']['Content-Length'] = $content_length;
    }

    // If the server URL has a user then attempt to use basic authentication.
    if (isset($uri['user'])) {
      $options['headers']['Authorization'] = 'Basic ' . base64_encode($uri['user'] . (!empty($uri['pass']) ? ":" . $uri['pass'] : ''));
    }

    // If the database prefix is being used by SimpleTest to run the tests in a
    // copied database then set the user-agent header to the database prefix so
    // that any calls to other Drupal pages will run the SimpleTest prefixed
    // database. The user-agent is used to ensure that multiple testing sessions
    // running at the same time won't interfere with each other as they would if
    // the database prefix were stored statically in a file or database
    // variable.
    if (defined(DRUPAL_CORE_COMPATIBILITY)) {
      switch (DRUPAL_CORE_COMPATIBILITY) {
        case '6.x':
          global $db_prefix;

          if (isset($db_prefix) && is_string($db_prefix) && preg_match('/^simpletest\d+$/', $db_prefix, $matches)) {
            $options['headers']['User-Agent'] = 'User-Agent: ' . $matches[0];
          }
          break;

        case '7.x':
          $test_info = &$GLOBALS['drupal_test_info'];
          if (!empty($test_info['test_run_id'])) {
            $options['headers']['User-Agent'] = drupal_generate_test_ua($test_info['test_run_id']);
          }
          break;

        default:
          assert(0); // Unsupported Drupal version.
          break;
      }
    }

    $mediamosa_connector_response->request = $options['method'] . ' ' . $path . " HTTP/1.0\r\n";
    foreach ($options['headers'] as $name => $value) {
      $mediamosa_connector_response->request .= $name . ': ' . trim($value) . "\r\n";
    }
    $mediamosa_connector_response->request .= "\r\n" . $options['data'];

    // Calculate how much time is left of the original timeout value.
    $timeout = $options['timeout'] - timer_read(__FUNCTION__) / 1000;
    if ($timeout > 0) {
      stream_set_timeout($fp, floor($timeout), floor(1000000 * fmod($timeout, 1)));
      fwrite($fp, $mediamosa_connector_response->request);
    }

    // Fetch response. Due to PHP bugs like http://bugs.php.net/bug.php?id=43782
    // and http://bugs.php.net/bug.php?id=46049 we can't rely on feof(), but
    // instead must invoke stream_get_meta_data() each iteration.
    $info = stream_get_meta_data($fp);
    $alive = !$info['eof'] && !$info['timed_out'];
    $response = '';

    while ($alive) {
      // Calculate how much time is left of the original timeout value.
      $timeout = $options['timeout'] - timer_read(__FUNCTION__) / 1000;
      if ($timeout <= 0) {
        $info['timed_out'] = TRUE;
        break;
      }
      stream_set_timeout($fp, floor($timeout), floor(1000000 * fmod($timeout, 1)));
      $chunk = fread($fp, 1024);
      $response .= $chunk;
      $info = stream_get_meta_data($fp);
      $alive = !$info['eof'] && !$info['timed_out'] && $chunk;
    }
    fclose($fp);

    if ($info['timed_out']) {
      $mediamosa_connector_response->code = HTTP_REQUEST_TIMEOUT;
      $mediamosa_connector_response->error = 'request timed out';
      return $mediamosa_connector_response;
    }
    // Parse response headers from the response body.
    // Be tolerant of malformed HTTP responses that separate header and body with
    // \n\n or \r\r instead of \r\n\r\n.
    list($response, $mediamosa_connector_response->data) = preg_split("/\r\n\r\n|\n\n|\r\r/", $response, 2);
    $response = preg_split("/\r\n|\n|\r/", $response);

    // Parse the response status line.
    list($protocol, $code, $status_message) = explode(' ', trim(array_shift($response)), 3);
    $mediamosa_connector_response->protocol = $protocol;
    $mediamosa_connector_response->status_message = $status_message;

    $mediamosa_connector_response->headers = array();

    // Parse the response headers.
    $line = trim(array_shift($response));
    while ($line) {
      list($name, $value) = explode(':', $line, 2);
      $name = strtolower($name);
      if (isset($mediamosa_connector_response->headers[$name]) && $name == 'set-cookie') {
        // RFC 2109: the Set-Cookie response header comprises the token Set-
        // Cookie:, followed by a comma-separated list of one or more cookies.
        $mediamosa_connector_response->headers[$name] .= ',' . trim($value);
      }
      else {
        $mediamosa_connector_response->headers[$name] = trim($value);
      }

      // Get next line.
      $line = trim(array_shift($response));
    }

    $responses = array(
      100 => 'Continue',
      101 => 'Switching Protocols',
      200 => 'OK',
      201 => 'Created',
      202 => 'Accepted',
      203 => 'Non-Authoritative Information',
      204 => 'No Content',
      205 => 'Reset Content',
      206 => 'Partial Content',
      300 => 'Multiple Choices',
      301 => 'Moved Permanently',
      302 => 'Found',
      303 => 'See Other',
      304 => 'Not Modified',
      305 => 'Use Proxy',
      307 => 'Temporary Redirect',
      400 => 'Bad Request',
      401 => 'Unauthorized',
      402 => 'Payment Required',
      403 => 'Forbidden',
      404 => 'Not Found',
      405 => 'Method Not Allowed',
      406 => 'Not Acceptable',
      407 => 'Proxy Authentication Required',
      408 => 'Request Time-out',
      409 => 'Conflict',
      410 => 'Gone',
      411 => 'Length Required',
      412 => 'Precondition Failed',
      413 => 'Request Entity Too Large',
      414 => 'Request-URI Too Large',
      415 => 'Unsupported Media Type',
      416 => 'Requested range not satisfiable',
      417 => 'Expectation Failed',
      500 => 'Internal Server Error',
      501 => 'Not Implemented',
      502 => 'Bad Gateway',
      503 => 'Service Unavailable',
      504 => 'Gateway Time-out',
      505 => 'HTTP Version not supported',
    );
    // RFC 2616 states that all unknown HTTP codes must be treated the same as the
    // base code in their class.
    if (!isset($responses[$code])) {
      $code = floor($code / 100) * 100;
    }
    $mediamosa_connector_response->code = $code;

    switch ($code) {
      case 200: // OK
        $mediamosa_connector_response->set_response($mediamosa_connector_response->data, $mediamosa_connector_response->headers['content-type']);
      case 304: // Not modified
        break;

      case 301: // Moved permanently
      case 302: // Moved temporarily
      case 307: // Moved temporarily
        $location = $mediamosa_connector_response->headers['location'];
        $options['timeout'] -= timer_read(__FUNCTION__) / 1000;
        if ($options['timeout'] <= 0) {
          $mediamosa_connector_response->code = HTTP_REQUEST_TIMEOUT;
          $mediamosa_connector_response->error = 'request timed out';
        }
        elseif ($options['max_redirects']) {
          // Redirect to the new location.
          $options['max_redirects']--;
          $mediamosa_connector_response = self::http_request($location, $options);
          $mediamosa_connector_response->redirect_code = $code;
        }

        $mediamosa_connector_response->redirect_url = $location;
        break;

      default:
        $mediamosa_connector_response->error = $status_message;
    }

    return $mediamosa_connector_response;
  }

  /**
   * Check the result.
   *
   * @param xml_object $mediamosa_connector_response
   * @param string $ok
   * @param string $error
   */
  public static function check_result($mediamosa_connector_response, $ok = NULL, $error = NULL) {
    $result_id = $mediamosa_connector_response ? (int)$mediamosa_connector_response->xml->header->request_result_id : 0;

    if ($result_id != mediamosa_sdk::ERRORCODE_OKAY && $result_id != mediamosa_sdk::ERRORCODE_EMPTY_RESULT) {
      if (isset($error)) {
        $error = t($error, array('%code' => $result_id, '%message' => $mediamosa_connector_response->xml->header->request_result_description));
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
   * @deprecated: Stop using this function, as it will be removed in the future.
   *              Instead use the SimpleXML object directly.
   *
   * @param object $xml
   */
  private static function _simplexml2array($xml) {
    if (is_object($xml) && get_class($xml) == 'mediamosa_connector_response') {
      return self::_simplexml2array($xml->xml);
    }

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
   * @deprecated: Stop using this function, as it will be removed in the future.
   *              Instead use the SimpleXML object directly.
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
   * @deprecated: Stop using this function, as it will be removed in the future.
   *              Instead use the SimpleXML object directly.
   *
   * @param object $mediamosa_connector_response
   */
  public static function response2array($mediamosa_connector_response) {
    return $mediamosa_connector_response ? self::responsexml2array($mediamosa_connector_response->xml) : FALSE;
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
  public static function variable_get($name, $default = NULL) {
    // TODO
    return variable_get($name, $default);
  }

  /**
   * Variable_set wrapper for connection dependent variables.
   */
  public static function variable_set($name, $value) {
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
