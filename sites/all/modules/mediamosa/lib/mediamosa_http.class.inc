<?php
/**
 * @file
 * HTTP related functions.
 */

/**
 * Represents the HTTP helper class.
 */
class mediamosa_http {

  // ------------------------------------------------------------------- Consts.
  const METHOD_GET = 'GET';
  const METHOD_POST = 'POST';

  // ---------------------------------------------------------------- Functions.
  /**
   * Parse a URL and return its components.
   *
   * Version differs from parse_url(), as it will make sure all keys are set.
   *
   * @param string $url
   *   The url to parse.
   *
   * @return array
   *   An associative array is returned with keys;
   *   - scheme - e.g. http
   *   - host
   *   - port
   *   - user
   *   - pass
   *   - path
   *   - query - after the question mark ?
   *   - fragment - after the hashmark #
   */
  public static function parse_url($url) {
    $urls = parse_url($url);

    if (!$urls) {
      return FALSE;
    }

    $urls += array(
      'scheme' => 'http',
      'host' => 'localhost',
      'port' => 0, // 0 should default to 80
      'user' => '',
      'pass' => '',
      'path' => '',
      'query' => '', // after the question mark ?
      'fragment' => '', // after the hashmark #
    );

    // Strip /.
    $urls['host'] = trim($urls['host'], '/');
    $urls['path'] = trim($urls['path'], '/');

    return $urls;
  }

  /**
   * Opposite of parse_url.
   *
   * @param array $urls
   *   The array returned by parse_url()
   * @param string $return_parts
   *  Return the array with host and uri.
   */
  public static function build_url(array $urls, $return_parts = FALSE) {
    // We expect these.
    assert(in_array($urls['scheme'], array('ftp', 'http', 'https')));

    $result = array(
      'host' => array($urls['scheme'] . '://'),
      'uri' => array(),
    );

    // Add user : pass
    if ($urls['user'] != '' || $urls['pass'] != '') {
      $result['host'][] = $urls['user'];
      $result['host'][] = ':';
      $result['host'][] = $urls['pass'];
      $result['host'][] = '@';
    }

    // Host.
    $result['host'][] = $urls['host'];

    // Optional port
    if ($urls['port']) {
      $result['host'][] = ':' . $urls['port'];
    }

    // Path.
    if ($urls['path'] != '') {
      $result['uri'][] = '/' . $urls['path'];
    }

    // Query.
    if ($urls['query'] != '') {
      $result['uri'][] = '?' . $urls['query'];
    }

    // Fragment.
    if ($urls['fragment'] != '') {
      $result['uri'][] = $urls['fragment'];
    }

    // Now build url.
    return $return_parts ? array('host' => implode('', $result['host']), 'uri' => implode('', $result['uri'])) : (implode('', $result['host']) . implode('', $result['uri']));
  }

  /**
   * Wrapper around http_build_query().
   *
   * @param array $query
   *   The query to rebuild.
   *
   * @return string
   *   The URL encoded string.
   */
  public static function http_build_query(array $query) {
    return strtr(http_build_query($query, '', '&'), array('%5B' => '[', '%5D' => ']', '%5b' => '[', '%5d' => ']'));
  }

  /**
   * Splits a URL-encoded query string into an array.
   *
   * Fixed version of drupal_get_query_array().
   *
   * @param $query
   *   The query string to split.
   *
   * @return
   *   An array of URL decoded couples $param_name => $value.
   */
  public static function drupal_get_query_array($query) {
    $result = array();
    if (!empty($query)) {
      foreach (explode('&', $query) as $param) {
        $param = explode('=', $param);
        $result[rawurldecode($param[0])] = isset($param[1]) ? rawurldecode($param[1]) : '';
      }
    }
    return $result;
  }

  /**
   * Splitup the query string.
   *
   * Opposite of http_build_query().
   *
   * @param string $query_str
   *   The query string to split.
   *
   * @param array
   *   The result array.
   */
  public static function parse_query($query_str) {
    // Split the url.
    $query = array();

    // Split values.
    foreach (explode('&', $query_str) as $valuepair) {
      if (strpos($valuepair, '=')) {
        list($name, $value) = explode('=', $valuepair, 2);
        $query[urldecode($name)][] = urldecode($value);
      }
      else {
        $query[urldecode($valuepair)][] = '';
      }
    }

    // Now make single item queries with 0 keys no array.
    foreach ($query as $name => $value) {
      if (count($value) == 1 && key($value) == 0) {
        $query[$name] = reset($value);
      }
    }

    return $query;
  }

  /**
   * Glue part of (parsed) url.
   * @see parse_url()
   *
   * @param array $parsed
   *   An associative array like parse_url() returns.
   *
   * @return string $uri
   */
  public static function glue_url(array $parsed) {
    $uri = '';

    $uri .= isset($parsed['scheme']) ? $parsed['scheme'] . ':' . ((drupal_strtolower($parsed['scheme']) == 'mailto') ? '' : '//') : '';
    $uri .= isset($parsed['user']) ? $parsed['user'] . ($parsed['pass'] ?  ':' . $parsed['pass'] : '') . '@' : '';
    $uri .= isset($parsed['host']) ? $parsed['host'] : '';
    $uri .= isset($parsed['port']) ? ':' . $parsed['port'] : '';
    $uri .= $uri ? '/' : '';
    // Cut the first '/' character.
    $uri .= isset($parsed['path']) ? (drupal_substr($parsed['path'], 0, 1) == '/' ? drupal_substr($parsed['path'], 1) : $parsed['path']) : '';
    $uri .= isset($parsed['query']) ? '?' . $parsed['query'] : '';
    $uri .= isset($parsed['fragment']) ? '#' . $parsed['fragment'] : '';

    return $uri;
  }

  /**
   * Fix our URL to start with 'http://' when missing http schema.
   *
   * @param string $url
   *   The url to check for http / https.
   *
   * @return string
   *   URL including http schema.
   */
  public static function expectHTTP($url) {
    return ( (drupal_substr($url, 0, 7) != 'http://' && drupal_substr($url, 0, 8) != 'https://') ? 'http://' . $url : $url);
  }

  /**
   * Convert uri to url.
   *
   * Simplefix for invalid chars.
   */
  public static function uri2url($uri) {
    $uri = trim($uri, "/ \n\r\t\0");
    return self::expectHTTP($uri);
  }

  /**
   * Do an internal REST call.
   *
   * @param string $host
   *   The host url.
   * @param string $rest_uri
   *   The URI of the REST call. Can only be internal REST allowed calls.
   * @param string $data
   *   A string containing the request body. Defaults to NULL.
   * @param string $method
   *   The HTTP method.
   * @param array $options
   *   (optional) An array which can have one or more of following keys:
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 3.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 30 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   *
   * @return object
   *   An object which can have one or more of the following parameters:
   *   - request
   *       A string containing the request body that was sent.
   *   - code
   *       An integer containing the response status code, or the error code if
   *       an error occurred.
   *   - protocol
   *       The response protocol (e.g. HTTP/1.1 or HTTP/1.0).
   *   - status_message
   *       The status message from the response, if a response was received.
   *   - redirect_code
   *       If redirected, an integer containing the initial response status code.
   *   - redirect_url
   *       If redirected, a string containing the redirection location.
   *   - error
   *       If an error occurred, the error message. Otherwise not set.
   *   - headers
   *       An array containing the response headers as name/value pairs.
   *   - data
   *       A string containing the response body that was received.
   */
  public static function do_internal_call($host, $rest_uri, $data = NULL, $method = self::METHOD_GET, array $options = array()) {
    // Make sure it starts with 'http://'.
    $host = self::expectHTTP(mediamosa_unicode::strtolower($host));

    $url = strtr(
      '@host/internal/@rest_uri',
      array(
        '@host' => trim($host, '/'),
        '@rest_uri' => $rest_uri,
      )
    );

    // Setup defaults.
    $options += array(
      'method' => $method,
    );

    if (isset($data)) {
      $options['data'] = $data;
    }

    // Include the internal password so we gain access.
    $options['headers'][mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS] = mediamosa::get_internal_password();

    // Do the request.
    return self::drupal_http_request($url, $options);
  }

  /**
   * Do call to URL using CURL lib.
   *
   * @param string $url
   *   The URL to call.
   * @param array $options
   *   Additional options array;
   *     - header
   *       name => value pair for http header.
   *     - use_curlproxy
   *       set to TRUE to enable call through curlproxy (if specified).
   *     - do_internal
   *       Add the internal pass to the header. Remember to add /internal
   *       yourself to the url.
   *     - do_head
   *       Set to TRUE to do HEAD instead of GET.
   */
  public static function do_call($url, array $options = array()) {
    // Set defaults.
    $options += array(
      'use_curlproxy' => FALSE,
      'headers' => array(),
      'do_internal' => FALSE,
      'do_head' => FALSE,
    );

    $curlproxy_http = NULL;
    if ($options['use_curlproxy']) {
      // Get the proxy url.
      $curlproxy_http = mediamosa::variable_get('mediamosa_curlproxy_http');
    }

    // Include the internal password so we gain access.
    if ($options['do_internal']) {
      $options['headers'][mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS] = mediamosa::get_internal_password();
    }

    // If the database prefix is being used by SimpleTest to run the tests in a copied
    // database then set the user-agent header to the database prefix so that any
    // calls to other Drupal pages will run the SimpleTest prefixed database. The
    // user-agent is used to ensure that multiple testing sessions running at the
    // same time won't interfere with each other as they would if the database
    // prefix were stored statically in a file or database variable.
    $test_info = &$GLOBALS['drupal_test_info'];
    if (!empty($test_info['test_run_id'])) {
      $options['headers']['User-Agent'] = drupal_generate_test_ua($test_info['test_run_id']);
    }

    // Init curl.
    $curl_handle = curl_init($url);

    // Set HTTPS to accept any certificate.
    if (mediamosa_unicode::substr(mediamosa_unicode::strtolower($url), 0, mediamosa_unicode::strlen('https://')) == 'https://') {
      curl_setopt($curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
    }

    $curl_options = array();
    foreach ($options['headers'] as $name => $value) {
      $curl_options[CURLOPT_HTTPHEADER][] = $name . ': ' . $value;
    }

    // FIXME: need to find out other way to trigger URL.
    if ($options['do_head']) {
      curl_setopt($curl_handle, CURLOPT_TIMEOUT, 1);
      curl_setopt($curl_handle, CURLOPT_CONNECTTIMEOUT, 1);

      // Do a HEAD call here.
      //curl_setopt($curl_handle, CURLOPT_NOBODY, TRUE);
    }

    if (!empty($curl_options)) {
      curl_setopt_array($curl_handle, $curl_options);
    }

    curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

    // Set to kill notices about missing user agent.
    //curl_setopt($curl_handle, CURLOPT_USERAGENT, 'MediaMosa2');

    // Check for possible proxy setting, if set, use it.
    if ($curlproxy_http) {
      curl_setopt($curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
      curl_setopt($curl_handle, CURLOPT_PROXY, $curlproxy_http);
    }

    $result = curl_exec($curl_handle);
    curl_close($curl_handle);

    // Clear cache files. Call might have removed files.
    mediamosa_io::clearstatcache();

    return $result;
  }

  /**
   * Do a head call.
   *
   * Triggers the URL without getting or waiting for the output.
   *
   * @param string $url
   *   The URL to call.
   * @param array $options
   *   Additional options array;
   *     - header
   *       name => value pair for http header.
   *     - use_curlproxy
   *       set to TRUE to enable call through curlproxy (if specified).
   *     - do_internal
   *       Add the internal pass to the header. Remember to add /internal
   *       yourself to the url.
   *
   * @see do_call()
   */
  public static function do_head_call($url, array $options = array()) {
    $options['do_head'] = TRUE;
    mediamosa_debug::log('Doing HEAD call to @url', array('@url' => $url));
    $data = self::do_call($url, $options);
    mediamosa_debug::log('Done HEAD call to @url', array('@url' => $url));
    return $data;
  }

  /**
   * Wrapper for internal HEAD calls.
   *
   * @param $url
   *   The URL to call, prefix url with /internal.
   * @param $options
   *   Additional options array;
   *     - header
   *       name => value pair for http header.
   *     - use_curlproxy
   *       set to TRUE to enable call through curlproxy (if specified).
   *
   * @see do_head_call()
   * @see do_call()
   */
  public static function do_head_internal_call($url, array $options = array()) {
    $options['do_internal'] = TRUE;
    return self::do_head_call($url, $options);
  }

  /**
   * Trigger uri with the hostname and settings taken from
   * mediamosa_cron_url_app_*.
   *
   * @param string $uri
   *  If you need to change the uri to trigger.
   * @param bool $do_head
   *  Do HEAD call (trigger the uri, do wait for result).
   * @param array $options
   *  HTTP options.
   */
  public static function do_internal_call_cron($uri, $do_head = FALSE, array $options = array()) {
    // Just trigger, it has his own protection on overrun.
    $url = variable_get('mediamosa_cron_url_app', 'http://localhost');
    $use_localhost = variable_get('mediamosa_cron_url_app_local', FALSE);

    $options += array(
      'method' => 'GET',
    );

    $urls = mediamosa_http::parse_url($url);
    if ($use_localhost) {
      $options['headers']['Host'] = $urls['host'];
      $urls['host'] = 'localhost';
    }

    $urls['path'] = ($urls['path'] != '' ? $urls['path'] . '/' : '') . trim($uri, '/');
    $url = mediamosa_http::build_url($urls);

    if ($do_head) {
      // Not expecting data array.
      assert(!isset($options['data']));

      mediamosa_http::do_head_internal_call($url, $options);
      $result = $url;

      // Log URL.
      mediamosa_debug::log('Triggering internal CRON call to @url.', array('@url' => $url));
    }
    else {
      $options['headers'][mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS] = mediamosa::get_internal_password();
      if (isset($options['data']) && is_array($options['data'])) {
        $options['data'] = http_build_query($options['data'], '', '&');
      }

      if ($options['method'] == 'POST') {
        $options['headers']['Content-Type'] = 'application/x-www-form-urlencoded';
      }

      $result = mediamosa_http::drupal_http_request($url, $options);

      // Log URL.
      mediamosa_debug::log('Internal CRON call to @url.', array('@url' => $url));
    }

    // Return the triggered url.
    return $result;
  }


  /**
   * Perform an HTTP request.
   *
   * Copy of the Drupal HTTP Request.
   * It overwrites our 'Host:' entry in our headers definition, old versions
   * worked fine. We need 'Host:' todo localhost calls with Host name
   * specified in header. We can not always call directly on hostname because
   * of firewall, so we call localhost with hostname in header instead.
   *
   *
   *
   * This is a flexible and powerful HTTP client implementation. Correctly
   * handles GET, POST, PUT or any other HTTP requests. Handles redirects.
   *
   * @param $url
   *   A string containing a fully qualified URI.
   * @param $options
   *   (optional) An array which can have one or more of following keys:
   *   - headers
   *       An array containing request headers to send as name/value pairs.
   *   - method
   *       A string containing the request method. Defaults to 'GET'.
   *   - data
   *       A string containing the request body. Defaults to NULL.
   *   - max_redirects
   *       An integer representing how many times a redirect may be followed.
   *       Defaults to 3.
   *   - timeout
   *       A float representing the maximum number of seconds the function call
   *       may take. The default is 30 seconds. If a timeout occurs, the error
   *       code is set to the HTTP_REQUEST_TIMEOUT constant.
   *
   * @return
   *   An object which can have one or more of the following parameters:
   *   - request
   *       A string containing the request body that was sent.
   *   - code
   *       An integer containing the response status code, or the error code if
   *       an error occurred.
   *   - protocol
   *       The response protocol (e.g. HTTP/1.1 or HTTP/1.0).
   *   - status_message
   *       The status message from the response, if a response was received.
   *   - redirect_code
   *       If redirected, an integer containing the initial response status code.
   *   - redirect_url
   *       If redirected, a string containing the redirection location.
   *   - error
   *       If an error occurred, the error message. Otherwise not set.
   *   - headers
   *       An array containing the response headers as name/value pairs.
   *   - data
   *       A string containing the response body that was received.
   */
  public static function drupal_http_request($url, array $options = array()) {
    $result = new stdClass();

    // Parse the URL and make sure we can handle the schema.
    $uri = @parse_url($url);

    if ($uri == FALSE) {
      $result->error = 'unable to parse URL';
      $result->code = -1001;
      return $result;
    }

    if (!isset($uri['scheme'])) {
      $result->error = 'missing schema';
      $result->code = -1002;
      return $result;
    }

    timer_start(__FUNCTION__);

    // Merge the default options.
    $options += array(
      'headers' => array(),
      'method' => 'GET',
      'data' => NULL,
      'max_redirects' => 3,
      'timeout' => 30,
    );

    // MediaMosa specific;
    // Data in option array should not be set with GET.
    assert($options['method'] != 'GET' || ($options['method'] == 'GET' && (!isset($options['data']) || !mediamosa_unicode::strlen($options['data']) ) ));
    switch ($uri['scheme']) {
      case 'http':
        $port = isset($uri['port']) ? $uri['port'] : 80;
        $host = $uri['host'] . ($port != 80 ? ':' . $port : '');
        $fp = @fsockopen($uri['host'], $port, $errno, $errstr, $options['timeout']);
        break;
      case 'https':
        // Note: Only works when PHP is compiled with OpenSSL support.
        $port = isset($uri['port']) ? $uri['port'] : 443;
        $host = $uri['host'] . ($port != 443 ? ':' . $port : '');
        $fp = @fsockopen('ssl://' . $uri['host'], $port, $errno, $errstr, $options['timeout']);
        break;
      default:
        $result->error = 'invalid schema ' . $uri['scheme'];
        $result->code = -1003;
        return $result;
    }

    // Make sure the socket opened properly.
    if (!$fp) {
      // When a network error occurs, we use a negative number so it does not
      // clash with the HTTP status codes.
      $result->code = -$errno;
      $result->error = trim($errstr);

      // Mark that this request failed. This will trigger a check of the web
      // server's ability to make outgoing HTTP requests the next time that
      // requirements checking is performed.
      variable_set('drupal_http_request_fails', TRUE);

      return $result;
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

    // RFC 2616: "non-standard ports MUST, default ports MAY be included".
    // We don't add the standard port to prevent from breaking rewrite rules
    // checking the host that do not take into account the port number.
    if (empty($options['headers']['Host'])) { // HOTFIX MediaMosa.
      $options['headers']['Host'] = $host;
    }
    // Only add Content-Length if we actually have any content or if it is a POST
    // or PUT request. Some non-standard servers get confused by Content-Length in
    // at least HEAD/GET requests, and Squid always requires Content-Length in
    // POST/PUT requests.
    $content_length = mediamosa_unicode::strlen($options['data']);
    if ($content_length > 0 || $options['method'] == 'POST' || $options['method'] == 'PUT') {
      $options['headers']['Content-Length'] = $content_length;
    }

    // If the server URL has a user then attempt to use basic authentication.
    if (isset($uri['user'])) {
      $options['headers']['Authorization'] = 'Basic ' . base64_encode($uri['user'] . (!empty($uri['pass']) ? ":" . $uri['pass'] : ''));
    }

    // If the database prefix is being used by SimpleTest to run the tests in a copied
    // database then set the user-agent header to the database prefix so that any
    // calls to other Drupal pages will run the SimpleTest prefixed database. The
    // user-agent is used to ensure that multiple testing sessions running at the
    // same time won't interfere with each other as they would if the database
    // prefix were stored statically in a file or database variable.
    $test_info = &$GLOBALS['drupal_test_info'];
    if (!empty($test_info['test_run_id'])) {
      $options['headers']['User-Agent'] = drupal_generate_test_ua($test_info['test_run_id']);
    }

    $request = $options['method'] . ' ' . $path . " HTTP/1.0\r\n";
    foreach ($options['headers'] as $name => $value) {
      $request .= $name . ': ' . trim($value) . "\r\n";
    }
    $request .= "\r\n" . $options['data'];
    $result->request = $request;
    fwrite($fp, $request);

    // Fetch response.
    $response = '';
    while (!feof($fp)) {
      // Calculate how much time is left of the original timeout value.
      $timeout = $options['timeout'] - timer_read(__FUNCTION__) / 1000;
      if ($timeout <= 0) {
        $result->code = HTTP_REQUEST_TIMEOUT;
        $result->error = 'request timed out';
        return $result;
      }
      stream_set_timeout($fp, floor($timeout), floor(1000000 * fmod($timeout, 1)));
      $response .= fread($fp, 1024);
    }
    fclose($fp);

    // Clear cache files. Call might have removed files.
    mediamosa_io::clearstatcache();

    // Parse response headers from the response body.
    list($response, $result->data) = explode("\r\n\r\n", $response, 2);
    $response = preg_split("/\r\n|\n|\r/", $response);

    // Parse the response status line.
    list($protocol, $code, $status_message) = explode(' ', trim(array_shift($response)), 3);
    $result->protocol = $protocol;
    $result->status_message = $status_message;

    $result->headers = array();

    // Parse the response headers.
    while (TRUE) {
      $line = trim(array_shift($response));

      if (!$line) {
        break;
      }

      list($name, $value) = explode(':', $line, 2);
      $name = mediamosa_unicode::strtolower($name);
      if (isset($result->headers[$name]) && $name == 'set-cookie') {
        // RFC 2109: the Set-Cookie response header comprises the token Set-
        // Cookie:, followed by a comma-separated list of one or more cookies.
        $result->headers[$name] .= ',' . trim($value);
      }
      else {
        $result->headers[$name] = trim($value);
      }
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
    $result->code = $code;

    switch ($code) {
      case 200: // OK
      case 304: // Not modified
        break;
      case 301: // Moved permanently
      case 302: // Moved temporarily
      case 307: // Moved temporarily
        $location = $result->headers['location'];
        $options['timeout'] -= timer_read(__FUNCTION__) / 1000;
        if ($options['timeout'] <= 0) {
          $result->code = HTTP_REQUEST_TIMEOUT;
          $result->error = 'request timed out';
        }
        elseif ($options['max_redirects']) {
          // Redirect to the new location.
          $options['max_redirects']--;
          $result = self::drupal_http_request($location, $options);
          $result->redirect_code = $code;
        }
        $result->redirect_url = $location;
        break;
      default:
        $result->error = $status_message;
    }

    return $result;
  }

  /**
   * Glue additional params to the url.
   *
   * @param string $url
   *   The complete url, may include query_string.
   * @param array $params
   *   The array containing the GET params. Name => value pairs.
   *
   * @return string
   *   The result url.
   */
  public static function glue_params($url, array $params) {
    $query_data = array();

    // Lets see if there is a http query.
    $pos = mediamosa_unicode::strpos($url, '?');
    if ($pos !== FALSE) {
      if (mediamosa_unicode::strlen($url) > $pos + 1) {
        parse_str(mediamosa_unicode::substr($url, $pos + 1), $query_data);
      }

      // Remove the query string.
      $url = mediamosa_unicode::substr($url, 0, $pos);
    }

    $query_data = array_merge($query_data, $params);

    // Build the query again.
    $url .= '?' . http_build_query($query_data, '', '&');
    return $url;
  }
}
