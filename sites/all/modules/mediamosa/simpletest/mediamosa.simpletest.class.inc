<?php
/**
 * @file
 * The default simpletest class.
 */

/**
 * Defines for the groups.
 * MEDIAMOSA_TEST_GROUP_
 */
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE', 'MediaMosa');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_APP', 'MediaMosa Client Applications');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_ASSET', 'MediaMosa Assets');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_ASSET_CQL', 'MediaMosa Assets Search / CQL');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_ASSET_MEDIAFILE', 'MediaMosa Asset Mediafiles');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_ASSET_SUPPLEMENT', 'MediaMosa Asset Supplements');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_ACL', 'MediaMosa ACL');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_COLLECTION', 'MediaMosa Collections');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_FTP_USER', 'MediaMosa FTP User');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_JOB', 'MediaMosa Jobs');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_LIBRARY', 'MediaMosa Library');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_MEDIA', 'MediaMosa Media');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_OAUTH', 'MediaMosa oAuth');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_OEMBED', 'MediaMosa oEmbed');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_OPENAPI', 'MediaMosa OpenAPI');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_SIMPLETEST', 'MediaMosa Simpletest Parents');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_STATISTICS', 'MediaMosa Statistics');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_STILL', 'MediaMosa Stills');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_STORAGE', 'MediaMosa Storage');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_TOOLS', 'MediaMosa Tools');
define('MEDIAMOSA_TEST_GROUP_MEDIAMOSA_CORE_USER', 'MediaMosa User / Groups');

/**
 * The basic lowest simpletest class.
 */
class MediaMosaTestCase extends DrupalWebTestCase {

  /**
   * Files to delete with teardown.
   *
   * @var array
   */
  protected $unlinks = array();

  /**
   * Change if your test needs different timezone.
   *
   * @var string
   */
  protected $default_timezone = mediamosa_settings::MEDIAMOSA_DEFAULT_TIMEZONE;

  // ------------------------------------------------------------------ Consts.
  // Default old user/group.
  const SIMPLETEST_USER_ID = 'Simpletest user id';
  const SIMPLETEST_GROUP_ID = 'Simpletest test group';

  // -------------------------------------------------------------- Constructor.
  function __construct($test_id = NULL) {
    parent::__construct($test_id);
    $this->skipClasses[__CLASS__] = TRUE;
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Clean up tables, copy of simpletest_clean_environment without the
   * drupal messages.
   *
   * Only run when you are not running simpletests(!). Any running tests will
   * fail.
   *
   * @see simpletest_clean_environment()
   */
  public static function clean() {
    simpletest_clean_database();
    simpletest_clean_temporary_directories();
    simpletest_clean_results_table();

    // Detect test classes that have been added, renamed or deleted.
    registry_rebuild();
    cache_clear_all('simpletest', 'cache');
  }


  /**
   * Because run() can create unused tables.
   */
  public function runSafe() {
    try {
      session_regenerate_id(FALSE);

      $this->run();
    }
    catch (Exception $e) {
      // Do tearDown first, so we are in the right db again.
      $this->tearDown();

      mediamosa_watchdog::log(
        'MediaMosaTestCase::runSafe() caught exception! @message (Exception thrown on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
        array(
          '@message' => $e->getMessage(),
          '@line' => $e->getLine(),
          '@file' => $e->getFile(),
          '@backtrace' => $e->getTraceAsString(),
        )
      );

      throw $e;
    }
  }

  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Make this global so we can output debug to the test.
    $GLOBALS['_mediamosa_simpletest'] = $this;

    $vars_to_copy = array('mediamosa_current_mount_point');
    $values = array();

    // Store certain vars.
    foreach ($vars_to_copy as $var_to_copy) {
      $values[$var_to_copy] = mediamosa::variable_get($var_to_copy);
    }

    // Set debug on high.
    $values['mediamosa_debug_level'] = 10;

    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array('mediamosa_sdk', 'simpletest', 'mediamosa', 'mediamosa_app'), $args));
    if (drupal_substr(phpversion(), 0, 3) < '5.3') {
      call_user_func_array(array($this, 'parent::setUp'), $args);
    }
    else {
      call_user_func_array('parent::setUp', $args);
    }

    // Now set it, so its in the sandbox.
    foreach ($values as $name => $value) {
      // Skip NULL.
      if (!is_null($value)) {
        mediamosa::variable_set($name, $value);
      }
    }

    // Lets put debug on high.
    mediamosa::variable_set('mediamosa_debug_level', 10);

    // Must be TRUE;
    assert(mediamosa::in_simpletest_sandbox());
  }

  /**
   * In Drupal Web Test class, the error log is only visible when verbose is
   * on. Lets dump it anyway.
   */
  protected function mediamosa_dump_error_log() {
    // FIXME: public:// does not seem to work here.
    $log = 'sites/default/files/simpletest/' . substr($this->databasePrefix, 10) . '/error.log';
    $found = FALSE;
    if (mediamosa_io::file_exists($log)) {
      $found = 0;
      foreach (file($log) as $line) {
        if (preg_match('/\[.*?\] (.*?): (.*?) in (.*) on line (\d+)/', $line, $match)) {
          // Parse PHP fatal errors for example: PHP Fatal error: Call to
          // undefined function break_me() in /path/to/file.php on line 17
          $caller = array(
            'line' => $match[4],
            'file' => $match[3],
            'function' => 'Dumplog',
          );
          $this->assert(FALSE, $line, $match[1], $caller);
        }
        else {
          $this->assert(FALSE, $line, 'Error log dump');
        }
        $found++;
      }
    }

    return $found;
  }

  /**
   * Ending the test.
   */
  protected function tearDown() {
    // Dump our PHP error log 1st (if any).
    $this->mediamosa_dump_error_log();

    // Unlink files.
    foreach ($this->unlinks as $filename) {
      mediamosa_io::unlink($filename);

      // Show deletion.
      $this->pass(t('File @filename deleted.', array('@filename' => $filename)));
    }

    // Clean up...
    $this->unlinks = array();

    // Our tables are not dropped because simpletest uses a test profile in
    // which our tables are not present. So we need to do it ourselfs.
    // We'll only drop tables from this simpletest.
    $prefix = Database::getConnection()->prefixTables('{mediamosa_}') . '%';

    // Get tables that match our pattern.
    $tables = db_find_tables($prefix);

    $matches = $ret = array();
    foreach ($tables as $table) {
      // Make sure we drop correct; tables MUST start with simpletest.
      if (preg_match('/simpletest\d+mediamosa.*/', $table, $matches)) {
        list(, $name) = explode('mediamosa_', $table, 2);

        // If the drop fails then we consider that its already dropped.
        try {
          db_drop_table('mediamosa_' . $name);
        }
        catch (PDOException $e) {
          // ignore.
        }
      }
    }

    // Dump the log.
    $this->dump_watchdog();

    // Call parent.
    parent::tearDown();
  }

  /**
   * Return my assertions.
   */
  public function get_assertions() {
    return $this->assertions;
  }

  /**
   * Returns if we are in verbose mode.
   *
   * @return boolean
   *   Returns TRUE when verbose is on.
   */
  public function isVerbose() {
    return variable_get('simpletest_verbose', TRUE);
  }

  /**
   * Dump the log so we know what happened.
   */
  protected function dump_watchdog() {
    if (self::isVerbose()) {
      // Dump the log.
      $result = db_query('SELECT * FROM {watchdog}');

      $texts = array('Dumping contents of watchdog');

      foreach ($result as $log_item) {
        $items = empty($log_item->variables) ? array() : unserialize($log_item->variables);
        $texts[] = strtr($log_item->message, is_array($items) ? $items : array());
      }

      // In verbose mode only.
      $this->verbose(implode('<br />', $texts), 'Watchdog dump');
    }
  }

  /**
   * Logs verbose message in a text file.
   *
   * The a link to the vebose message will be placed in the test results via
   * as a passing assertion with the text '[verbose message]'.
   *
   * @param $message
   *   The verbose message to be stored.
   *
   * @see simpletest_verbose()
   */
  protected function verbose($message, $type = 'Verbose message') {
    if ($id = simpletest_verbose($message)) {
      $url = file_create_url($this->originalFileDirectory . '/simpletest/verbose/' . get_class($this) . '-' . $id . '.html');
      $this->error(l(t($type), $url, array('attributes' => array('target' => '_blank'))), 'User notice');
    }
  }

  /**
   * Fire an assertion that is always positive.
   *
   * @param $message
   *   The message to display along with the assertion.
   * @param $group
   *   The type of assertion - examples are "Browser", "PHP".
   * @param $caller
   *   By default, the assert comes from a function whose name starts with
   *   'test'. Instead, you can specify where this assert originates from
   *   by passing in an associative array as $caller. Key 'file' is
   *   the name of the source file, 'line' is the line number and 'function'
   *   is the caller function itself.
   *
   * @return
   *   TRUE.
   */
  protected function pass($message = NULL, $group = 'Other', array $caller = NULL) {
    return $this->assert(TRUE, $message, $group, $caller);
  }

  /**
   * Helper function to figure caller locations in files.
   *
   * @param $caller
   *   How much back into trace history.
   *
   * @return
   *   Array with file and line values.
   */
  protected function backtrace($caller = 2) {
    $backtrace = debug_backtrace();

    // No backtrace.
    if (empty($backtrace[$caller])) {
      return array('file' => 'unknown', 'line' => 0);
    }

    if ($caller > 0) {
      $this->pass(t('Function @parent_function (line @parent_line) is called from file @file on line @line.', array('@parent_function' => $backtrace[$caller - 1]['function'], '@parent_line' => $backtrace[$caller - 1]['line'], '@file' => $backtrace[$caller]['file'], '@line' => $backtrace[$caller]['line'])));
    }

    return array('file' => (empty($backtrace[$caller]['file']) ? 'unknown' : $backtrace[$caller]['file']), 'line' => empty($backtrace[$caller]['line']) ? 0 : empty($backtrace[$caller]['line']));
  }

  /**
   * Pretty export of var_export.
   *
   * @param mixed $data
   */
  public function var_export($data, $group = 'Other', array $caller = NULL) {
    $this->pass(highlight_string('<?php ' . var_export($data, TRUE), TRUE), $group, $caller);
  }

  /**
   * Pretty export of var_export.
   *
   * @param mixed $data
   */
  public function var_export_verbose($data) {
    $this->verbose(highlight_string('<?php ' . var_export($data, TRUE), TRUE));
  }

  /**
   * Pretty export of var_export.
   *
   * @param mixed $data
   */
  public function var_export_fail($data, $group = 'Other', array $caller = NULL) {
    $this->fail(highlight_string('<?php ' . var_export($data, TRUE), TRUE), $group, $caller);
  }

  /**
   * Call if your outside the unittest and you want to log.
   *
   * @param $data
   */
  public static function staticMessage($data) {
    // Remember that this does not work for REST calls called inside the test.
    // Then the simpletest was never created; REST calls are HTTP requests
    // outside the scope of this test.
    if (isset($GLOBALS['_mediamosa_simpletest'])) {
      $GLOBALS['_mediamosa_simpletest']->var_export_fail($data, 'static-message');
    }
  }

  /**
   * Static call to restCall
   */
  public static function staticRestCall($uri, $request_method = 'POST', array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    assert(isset($GLOBALS['_mediamosa_simpletest']));
    $GLOBALS['_mediamosa_simpletest']->restCall($uri, $request_method, $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Static call to restCall POST
   */
  public static function staticRestCallPost($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    assert(isset($GLOBALS['_mediamosa_simpletest']));
    $GLOBALS['_mediamosa_simpletest']->restCall($uri, 'POST', $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Static call to restCall GET
   */
  public static function staticRestCallGet($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    assert(isset($GLOBALS['_mediamosa_simpletest']));
    $GLOBALS['_mediamosa_simpletest']->restCall($uri, 'GET', $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Static call to restCall POST
   */
  public static function staticInternalRestCallPost($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    assert(isset($GLOBALS['_mediamosa_simpletest']));
    $GLOBALS['_mediamosa_simpletest']->restCall($uri, 'POST', $parameters, $headers, $expected_result_ids, TRUE);
  }

  /**
   * Static call to restCall GET
   */
  public static function staticInternalRestCallGet($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    assert(isset($GLOBALS['_mediamosa_simpletest']));
    $GLOBALS['_mediamosa_simpletest']->restCall($uri, 'GET', $parameters, $headers, $expected_result_ids, TRUE);
  }

  /**
   * Override drupalGet so we can override the request on the drupal interface.
   *
   * @param $path
   * @param $options
   * @param $headers
   */
  protected function drupalGet($path, array $options = array(), array $headers = array()) {

    // Just let them know we need the Drupal part.
    $headers[] = mediamosa_settings::X_MEDIAMOSA_DISABLE_APP . ': True';

    $this->pass('Passing through drupalGet()');
    return parent::drupalGet($path, $options, $headers);
  }


  /**
   * Implements drupalPost().
   */
  protected function drupalPost($path, $edit, $submit, array $options = array(), array $headers = array(), $form_html_id = NULL, $extra_post = NULL) {
    // Just let them know we need the Drupal part.
    $headers[] = mediamosa_settings::X_MEDIAMOSA_DISABLE_APP . ': True';

    return parent::drupalPost($path, $edit, $submit, $options, $headers, $form_html_id, $extra_post);
  }

  /**
   * Perform an HTTP request.
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
   * @param boolean $is_response_xml
   *   If TRUE, then the data will be parsed as XML.
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
   *   - xml: XML parsed data, if $is_response_xml == TRUE.
   */
  protected function drupal_http_request($url, array $options = array(), $is_response_xml = FALSE) {
    $this->var_export($url);

    $response = drupal_http_request($url, $options);

    if ($is_response_xml) {
      try {
        $response->xml = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        $this->assert(FALSE, 'simple XML creation failed! ' . $e->getMessage());
        return;
      }
    }

    $this->var_export($response);

    return $response;
  }

  /**
   * Do a REST call inside the sandbox.
   *
   * @param string $uri
   *  The uri of the REST call.
   * @param string $request_method
   *  POST / GET / PUT / DELETE.
   * @param array $parameters
   *  name => value
   * @param array $headers
   *  Name: value
   * @param array $expected_result_ids
   *  Contains the mediamosa_error::ERRORCODE_* that we expect. For example, you
   *  might expect an error instead of OKAY; specify the error code here.
   */
  protected function restCall($uri, $request_method = 'POST', array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {

    if ($do_internal) {
      // Set internal password.
      $headers[] = mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS . ': ' . mediamosa::get_internal_password();

      // Add internal to uri.
      if (strpos($uri, 'internal/') === FALSE) {
        $uri = 'internal/' . $uri;
      }
    }

    // Prepare the CURL options.
    $curl_opts = array(
      CURLOPT_NOBODY => FALSE,
      CURLOPT_HTTPHEADER => $headers,
      CURLOPT_HEADER => TRUE,
    );

    if (strcasecmp($request_method, 'GET') == 0) {
      $curl_opts += array(
        CURLOPT_HTTPGET => TRUE,
        CURLOPT_URL => url($uri, array('absolute' => TRUE, 'query' => $parameters)),
      );
    }
    else {
      $curl_opts += array(
        CURLOPT_POST => TRUE,
        CURLOPT_URL => url($uri, array('absolute' => TRUE)),
        CURLOPT_POSTFIELDS => http_build_query($parameters),
      );
    }

    // Perform the CURL request.
    $result = $this->curlExec($curl_opts);

    // Parse the headers and get the body.
    $lines = explode("\n", $result);
    $line_count = count($lines);

    $http_result = explode(' ', $lines[0]);

    // Unset the HTTP header line.
    unset($lines[0]);

    // Iterate until an empty row is found.
    $out = array();
    for ($i = 1; $i < $line_count; $i++) {
      if (trim($lines[$i]) != '') {
        list($key, $value) = explode(':', $lines[$i], 2);
        $out['headers'][trim($key)] = trim($value);
        unset($lines[$i]);
      }
      else {
        $out['body'] = trim(implode("\n", $lines));
        break;
      }
    }

    // Create a SimpleXMLElement from the restcall's output.
    try {
      // We expect char '<' on REST calls, if not we need to output it.
      if ($out['body'][0] != '<') {
        $this->var_export($out['body']);
      }
      else {
        $this->var_export_verbose(mediamosa_unicode::substr($out['body'], 0, 10000) . (mediamosa_unicode::strlen($out['body']) > 10000 ? '...<snap>...' : ''));
      }
    }
    catch (Exception $e) {
      $this->assert(FALSE, 'var_export of REST response body failed! ' . $e->getMessage());
      return;
    }

    try {
      $out['xml'] = new mediamosa_connector_response($out['body']);
    }
    catch (Exception $e) {
      $this->assert(FALSE, 'simple XML creation failed! ' . $e->getMessage());
      return;
    }

    // Get the result ID.
    if (!empty($out['xml']->header)) {
      $result_id = empty($out['xml']->header->request_result_id) ? 0 : (int) $out['xml']->header->request_result_id;
      $result_description = empty($out['xml']->header->request_result_description) ? 'no description' : (string) $out['xml']->header->request_result_description;
    }
    elseif (!empty($out['xml']->extension)) {
      $result_id = empty($out['xml']->extension->request_result_id) ? 0 : (int) $out['xml']->extension->request_result_id;
      $result_description = empty($out['xml']->extension->request_result_description) ? 'no description' : (string) $out['xml']->extension->request_result_description;
    }

    // Look for a request_result_id.
    if (isset($result_id) && isset($result_description)) {
      $this->assertTrue(in_array($result_id, $expected_result_ids), t('@method request to: "@uri", got result ID @result_id (@description) was expecting one of; {@expected}.', array('@method' => $request_method, '@uri' => $uri, '@result_id' => $result_id, '@description' => $result_description, '@expected' => implode(', ', $expected_result_ids))));
    }
    else {
      $this->assertTrue(TRUE, t('@method request to: "@uri".', array('@method' => $request_method, '@uri' => $uri)));
    }

    $this->assertFalse($http_result[1] == 404, 'REST call was found');

    // Return the request data.
    return $out;
  }

  /**
   * Wrapper for POST restCall
   */
  protected function restCallPost($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    return $this->restCall($uri, 'POST', $parameters, $headers, $expected_result_ids, $do_internal);
  }

  /**
   * Wrapper for GET restCall
   */
  protected function restCallGet($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    return $this->restCall($uri, 'GET', $parameters, $headers, $expected_result_ids);
  }

  /**
   * Wrapper for PUT restCall
   */
  protected function restCallPut($uri, array $parameters = array(), array $headers = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY)) {
    return $this->restCall($uri, 'PUT', $parameters, $headers, $expected_result_ids);
  }

  /**
   * Create symlink.
   *
   * @param string $source
   * @param string $dest
   */
  protected function symlink($source, $dest) {
    // Copy the test video
    $this->assertTrue(
      mediamosa_io::symlink($source, $dest) != FALSE,
      t("Symlink: Created symlink from '@source' to target '@target'", array('@source' => $source, '@target' => $dest))
    );

    $this->assertTrue(
      mediamosa_io::file_exists($dest),
      t("Symlink: New file '@target' exists.", array('@target' => $dest))
    );

    // Register to be removed.
    $this->unlinks[] = $dest;
  }

  /**
   * Remove file.
   *
   * @param string $dest
   */
  protected function unlink($dest) {
    $this->assertTrue(
      mediamosa_io::unlink($dest) !== FALSE,
      t("Unlink: Removing file '@target'", array('@target' => $dest))
    );

    $this->assertTrue(
      mediamosa_io::file_exists($dest) == FALSE,
      t("Unlink: File '@target' is removed.", array('@target' => $dest))
    );
  }

  /**
   * Create folder.
   *
   * @param string $uri
   *   The uri of the directory to create.
   * @param boolean $recursive
   *   Enable recursive to create the sub maps required to create directory.
   * @param integer $mode
   *   Default mode value for created directory, defaults to mode 0775.   *
   * @see mediamosa_io::mkdir()
   */
  protected function mkdir($uri, $recursive = TRUE, $mode = 0775) {
    // Create the directory first.
    mediamosa_io::mkdir($uri, $recursive, $mode);
    $this->pass(t('Created directory @uri (@realpath)', array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri))));
  }

  /**
   * Create empty file.
   *
   * @param string $uri
   *   The file uri.
   * @param string $text
   *   The text for assert.
   *
   * @return boolean
   *   TRUE when success, FALSE otherwise.
   */
  protected function touch($uri, $text = 'Create empty file @uri (@realpath).') {
    mediamosa_io::touch($uri);
    $this->unlinks[] = mediamosa_io::realpath($uri);
    return $this->file_exists($uri, $text);
  }

  /**
   * Copy file.
   *
   * @param string $source_uri
   *   Path to the source file.
   * @param string $destination_uri
   *   The destination path. If dest is a URL, the copy operation may fail if
   *   the wrapper does not support overwriting of existing files.
   */
  protected function copy($source_uri, $destination_uri) {
    $this->file_exists($source_uri, t("Copy: Source file '@source' exists.", array('@source' => $source_uri)));

    // Copy the file.
    $this->assertTrue(
      mediamosa_io::copy($source_uri, $destination_uri) != FALSE,
      t("Copy: Copied from '@source' to target '@target'", array('@source' => $source_uri, '@target' => $destination_uri))
    );

    $this->file_exists($destination_uri, t("Copy: New file '@target' exists.", array('@target' => $destination_uri)));
  }

  /**
   * Check if file exists. Must be file type.
   *
   * @param string $uri
   *   The file to check.
   *
   * @param boolean
   *   Returns TRUE when file exists.
   */
  protected function file_exists($uri, $text = 'Check if file @uri (@realpath) exists.') {
    // Check exists.
    $result = mediamosa_io::file_exists($uri);

    // Must be file.
    $this->is_file($uri);

    $this->assertTrue(
      $result,
      strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri)))
    );

    return $result;
  }

  /**
   * Check if dir exists. Must be dir/path type.
   *
   * @param string $uri
   *   The dir exists.
   *
   * @return boolean
   *   Returns TRUE when dir exists, FALSE otherwise.
   */
  protected function dir_exists($uri, $text = 'Check if dir @uri (@realpath) exists.') {
    // Check exists.
    $result = mediamosa_io::file_exists($uri);

    // Must be dir.
    $this->is_dir($uri);

    $this->assertTrue(
      $result,
      strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri)))
    );

    return $result;
  }

  /**
   * Check if file does not exists.
   *
   * @param string $uri
   *   The file or path to check.
   *
   * @return boolean
   *   Returns TRUE when file does not exists, FALSE otherwise.
   */
  protected function file_not_exists($uri, $text = 'Check if file @uri (@realpath) does not exists.') {
    // Check exists.
    $result = mediamosa_io::file_exists($uri);

    $this->assertFalse(
      $result,
      strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri)))
    );

    // Return reverse.
    return !$result;
  }

  /**
   * Check if dir/path does not exists.
   *
   * Expects the uri to be a dir/path.
   *
   * @param string $uri
   *   The file or path to check.
   *
   * @return boolean
   *   Returns TRUE when dir does not exists, FALSE otherwise.
   */
  protected function dir_not_exists($uri, $text = 'Check if dir @uri (@realpath) does not exists.') {
    // Check exists.
    $result = mediamosa_io::file_exists($uri);

    $this->assertFalse(
      $result,
      strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri)))
    );

    // Return reverse.
    return !$result;
  }

  /**
   * Expect $uri to be file.
   *
   * @param string $uri
   *   The uri to check.
   *
   * @return boolean
   *   Returns TRUE when file, FALSE otherwise.
   */
  protected function is_file($uri, $text = 'Is file type @uri (@realpath)') {
    $result = mediamosa_io::is_file($uri);
    $this->assertTrue($result, strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri))));
    return $result;
  }

  /**
   * Expect $uri not to be file.
   *
   * @param string $uri
   *   The uri to check.
   *
   * @return boolean
   *   Returns TRUE when not file, FALSE otherwise.
   */
  protected function is_not_file($uri, $text = 'Is not file type @uri (@realpath)') {
    $result = mediamosa_io::is_file($uri);
    $this->assertFalse($result, strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri))));
    return $result;
  }

  /**
   * Expect $uri to be dir.
   *
   * @param string $uri
   *   The uri to check.
   *
   * @return boolean
   *   Returns TRUE when dir/path, FALSE otherwise.
   */
  protected function is_dir($uri, $text = 'Is dir type @uri (@realpath)') {
    $result = mediamosa_io::is_dir($uri);
    $this->assertTrue($result, strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri))));
    return $result;
  }
  /**
   * Expect $uri not to be dir.
   *
   * @param string $uri
   *   The uri to check.
   *
   * @return boolean
   *   Returns TRUE when not dir/path, FALSE otherwise.
   */
  protected function is_not_dir($uri, $text = 'Is not dir type @uri (@realpath)') {
    $result = mediamosa_io::is_dir($uri);
    $this->assertFalse($result, strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri))));
    return $result;
  }

  /**
   * Check if the file has the expected md5.
   *
   * @param string $uri
   *   The file to check.
   * @param string $md5
   *   The md5 string.
   */
  protected function file_is_same($uri, $md5, $text = 'Check if file @uri (@realpath) matches the expected md5.') {
    $md5_file = mediamosa_io::md5_file($uri);
    $this->assert($md5 == $md5_file, strtr($text, array('@uri' => $uri, '@realpath' => mediamosa_io::realpath_safe($uri))));
  }

  /**
   * Return the path+filename to the test video.
   *
   * Changing this file will have an effect on some tests, as some expect this
   * file.
   */
  protected function getTestVideo() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/test_video.wmv';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test video.
   */
  protected function getTestVideo_ogg() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/bunny.ogg';
    $this->pass(t('Got test OGG video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test video.
   */
  protected function getTestVideo_BlackStill() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/black_test.wmv';
    $this->pass(t('Got test video for black still @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test image (jpeg).
   */
  protected function getTestImage_NotBlackJpg() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/notblack-001.jpeg';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test image (jpg).
   */
  protected function getTestImage_Jpg() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/cat.jpg';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test image (png).
   */
  protected function getTestImage_Png() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/cat.png';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Return the path+filename to the test image (gif).
   */
  protected function getTestImage_BlobGif() {
    $filename = drupal_get_path('module', 'mediamosa') . '/lib/testfiles/test_blob.gif';
    $this->pass(t('Got test video @video.', array('@video' => $filename)));
    return $filename;
  }

  /**
   * Create a random app or specified one.
   *
   * @param array $a_attributes
   *  Basic app parameters like the DB version (mediamosa_app_db).
   */
  protected function createApp($fields = array()) {

    // Generate the name.
    $name = $this->randomName();

    // Apps are now always > APP_ID_MAX so we are outside the scope of existing
    // apps. This will enable testing for Solr extension (or other code that is
    // ouside the sandbox.
    $app_id_max = max(mediamosa_settings::APP_ID_MIN, mediamosa_settings::APP_ID_MAX) + 1;

    // Add defaults.
    $fields += array(
      mediamosa_app_db::APP_ID => rand($app_id_max, $app_id_max + 1000),
      mediamosa_app_db::TIMEZONE => $this->default_timezone,
    );

    // Make sure its not in normal app id range.
    assert($fields[mediamosa_app_db::APP_ID] >= $app_id_max);

    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_APP, $name);

    $node->{mediamosa_app_db::APP_ID} = $fields[mediamosa_app_db::APP_ID];
    $node->{mediamosa_app_db::SHARED_KEY} = user_password();
    $node->{mediamosa_app_db::APP_NAME} = $name;
    $node->{mediamosa_app_db::ACTIVE} = 'TRUE';
    $node->{mediamosa_app_db::TRANSCODING_POSITION_TO} = 0;
    $node->{mediamosa_app_db::QUOTA} = 0;
    $node->{mediamosa_app_db::QUOTA_USED} = 0;
    $node->{mediamosa_app_db::ACTIVE_VERSION} = '';
    $node->{mediamosa_app_db::TIMEZONE} = $fields[mediamosa_app_db::TIMEZONE];
    $node->{mediamosa_app_db::OAUTH_CONSUMER_KEY} = $this->randomName();
    $node->{mediamosa_app_db::OAUTH_CONSUMER_SECRET} = user_password();

    node_save($node);

    // Log the creation in the resutls.
    $this->assert(TRUE, t("App created with node ID; '%id',  app_id; '%app_id', name; '%name' and shared key; '%shared_key'.", array('%id' => $node->nid, '%app_id' => $node->{mediamosa_app_db::APP_ID}, '%name' => $node->title, '%shared_key' => $node->{mediamosa_app_db::SHARED_KEY})));

    return (array) $node;
  }

  /**
   * Create a download server.
   */
  protected function createDownloadServer() {
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER, 'http://localhost/download/{TICKET}');

    $node->{mediamosa_server_db::VERSION} = 0;
    $node->{mediamosa_server_db::SERVER_STATUS} = mediamosa_server_db::SERVER_STATUS_ON;
    $node->{mediamosa_server_db::SERVER_TYPE} = mediamosa_server_db::SERVER_TYPE_DOWNLOAD;
    $node->{mediamosa_server_db::SERVER_URI} = 'http://localhost/';
    $node->{mediamosa_server_db::SERVER_PATH} = mediamosa_server_db::SERVER_PATH_BASE_URI . 'download/{TICKET}';
    $node->{mediamosa_server_db::CONTAINERS} = NULL;
    $node->{mediamosa_server_db::OBJECT_CODE} = NULL;
    $node->{mediamosa_server_db::SERVER_NAME} = NULL;
    $node->{mediamosa_server_db::SLOTS} = NULL;
    $node->{mediamosa_server_db::TOOLS} = NULL;
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS} = NULL;

    node_save($node);
    $this->pass(t('Created node Downloadserver with node ID @nid', array('@nid' => $node->nid)));
  }

  /**
   * Create a job.
   */
  protected function createJob($fields = array()) {
    $fields += array(
      mediamosa_job_db::APP_ID => rand(1, 1000),
      mediamosa_job_db::ASSET_ID => mediamosa_db::uuid(rand(1, 1000)),
      mediamosa_job_db::MEDIAFILE_ID => mediamosa_db::uuid(rand(1, 1000)),
      mediamosa_job_db::OWNER_ID => mediamosa_db::uuid(rand(1, 1000)),
      mediamosa_job_db::JOB_STATUS => mediamosa_job_db::JOB_STATUS_INPROGRESS,
      mediamosa_job_db::PROGRESS => '0.000',
      mediamosa_job_db::PRIORITY => 0,
      mediamosa_job_db::JOB_TYPE => mediamosa_job_db::JOB_TYPE_UPLOAD,
      mediamosa_job_db::STARTED => mediamosa_datetime::utc_current_timestamp_now(),
      mediamosa_job_db::FINISHED => mediamosa_datetime::utc_current_timestamp_now(),
      mediamosa_job_db::ERROR_DESCRIPTION => '',
      mediamosa_job_db::CREATE_STILL => mediamosa_job_db::CREATE_STILL_FALSE,
    );
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert the job into the database.
    $fields[mediamosa_job_db::ID] = mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)->fields($fields)->execute();
    return $fields[mediamosa_job_db::ID];
  }

  /**
   * Create an upload job.
   */
  protected function createUploadJob($job_id, $fields = array()) {
    $fields[mediamosa_job_upload_db::JOB_ID] = $job_id;

    $fields += array(
      mediamosa_job_upload_db::FILE_SIZE => 0,
      mediamosa_job_upload_db::UPLOADED_FILE_SIZE => 0,
      mediamosa_job_upload_db::RETRANSCODE => mediamosa_job_upload_db::RETRANSCODE_FALSE,
      mediamosa_job_upload_db::CREATE_STILL => mediamosa_job_upload_db::CREATE_STILL_FALSE,
    );

    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert the upload job into the database.
    $fields[mediamosa_job_upload_db::ID] = mediamosa_db::db_insert(mediamosa_job_upload_db::TABLE_NAME)->fields($fields)->execute();
    return $fields[mediamosa_job_upload_db::ID];
  }

  /**
   * Return the default Profile ID.
   *
   * @param $app_id
   *   The Application ID.
   */
  protected function getDefaultProfileId($app_id) {
    // Get the profile.
    $profile = mediamosa_transcode_profile::get_default($app_id);

    $this->assertTrue($profile, 'Found default transcode profile');
    $this->var_export_verbose($profile);

    return $profile[mediamosa_transcode_profile_db::ID];
  }

  /**
   * Create a mediafile metadata property.
   *
   * @param prop_name
   *  Property name
   * @param type
   *  Type of the property
   * @return
   *  Property id
   */
  protected function createMediafileMetadataProperty($prop_name, $prop_type) {
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id($prop_name, $prop_type);

    // Check the prop_id.
    $this->assertTrue(
      $prop_id > 0,
      t("Prop_id (@prop_id) has valid ID (> 0)", array('@prop_id' => $prop_id))
    );

    // Check database.
    $this->checkMediafileMetadataPropertyDb($prop_id, $prop_name, $prop_type);

    return $prop_id;
  }

  /**
   * Check mediafile metadata property in DB.
   *
   * @param allowed_properties
   *  An associative array (property name => property type).
   * @return
   *  TRUE if the check was successful.
   */
  /**
   * Check if the creation of mediafile metadata property was correct.
   *
   * @param integer $prop_id
   *  Property ID to check.
   * @param string $name
   *  Name of the property.
   * @param string $type
   *  Type of the property.
   */
  protected function checkMediafileMetadataPropertyDb($prop_id, $prop_name, $prop_type) {
    $property = mediamosa_asset_mediafile_metadata_property::get_property($prop_id, TRUE);

    $this->assertTrue($property, strtr('Got property for @prop_id', array('@prop_id' => $prop_id)));

    $this->assertTrue($property[mediamosa_asset_mediafile_metadata_property_db::NAME] == $prop_name && $property[mediamosa_asset_mediafile_metadata_property_db::TYPE] == $prop_type, strtr('Property contents for @prop_id success.', array('@prop_id' => $prop_id)));

    return $property;
  }

  /**
   * Create / update a mediafile metadata value for a mediafile.
   *
   * @param mediafile_id
   *  Unique id for the mediafile.
   * @param prop_name
   *  Property name.
   * @param type
   *  Type of the property.
   * @param value
   *  The value of the given value.
   */
  protected function createMediafileMetadata($mediafile_id, $value, $prop_name, $type) {
    mediamosa_asset_mediafile_metadata::create_mediafile_metadata($mediafile_id, $value, $prop_name, $type);
  }

  /**
   * Create multiple metadata for a mediafile.
   * @param mediafile_id
   * @param metadatas An array (array(value => value, prop_name => prop_name, type => type))
   */
  protected function createMediafileMultipleMetadata($mediafile_id, $metadatas) {
    mediamosa_asset_mediafile_metadata::create_mediafile_multiple_metadata($mediafile_id, $metadatas);
  }

  /**
   * Check mediafile metadata.
   * @param allowed_values
   *  An associative array (property id => array(value => value, name => name, type => type))
   * @return
   *  TRUE if the check was successful
   */
  protected function checkMediafileMetadata($mediafile_id, $allowed_values) {
    $is_valid = TRUE;
    $values = array();

    $result = mediamosa_db::db_query('SELECT #prop_id, #val_char, #val_datetime, #val_int FROM {#mediamosa_asset_mediafile_metadata} WHERE #mediafile_id = :mediafile_id', array(
      '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
      '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
      '#val_char' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
      '#val_datetime' => mediamosa_asset_mediafile_metadata_db::VAL_DATETIME,
      '#val_int' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
      '#mediafile_id' => mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
      ':mediafile_id' => $mediafile_id,
    ));

    foreach ($result as $value) {
      $prop = mediamosa_asset_mediafile_metadata_property::get_property($value[mediamosa_asset_mediafile_metadata_db::PROP_ID], TRUE);

      if ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR) {
        $val = $value[mediamosa_asset_mediafile_metadata_db::VAL_CHAR];
      }
      elseif ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_INT) {
        $val = $value[mediamosa_asset_mediafile_metadata_db::VAL_INT];
      }
      elseif ($prop['type'] == mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME) {
        $val = $value[mediamosa_asset_mediafile_metadata_db::VAL_DATETIME];
      }
      else {
        // Shouldn't be here.
        assert(0);
        //$this->assertTrue(FALSE, t("Prop: @prop, value: @value,", array('@prop' => print_r($prop, TRUE), '@value' => print_r($value, TRUE))));
      }

      $info = array(
        'value' => $val,
        'name' => $prop['prop_name'],
        'type' => $prop['type'],
      );

      if (empty($allowed_values[$value[mediamosa_asset_mediafile_metadata_db::PROP_ID]]) || $allowed_values[$value[mediamosa_asset_mediafile_metadata_db::PROP_ID]] != $info) {
        $is_valid = FALSE;
        break;
      }
      $values[$value[mediamosa_asset_mediafile_metadata_db::PROP_ID]] = $info;
    }

    if ($is_valid) {
      foreach ($allowed_values as $prop_id => $value) {
        if (empty($values[$prop_id]) || $values[$prop_id] != $value) {
          $is_valid = FALSE;
          break;
        }
      }
    }

    return $is_valid;
  }

  /**
   * Get a metadata value of a mediafile.
   * @param mediafile_id Unique id for the mediafile
   * @param prop_name Property name
   * @param type Type of the property
   * @return Property value
   */
  protected function getMediafileMetadata($mediafile_id, $prop_name, $type) {
    return mediamosa_asset_mediafile_metadata::get_mediafile_metadata($mediafile_id, $prop_name, $type);
  }

  /**
   * Get all the metadata values of a mediafile.
   * @param mediafile_id Unique id for the mediafile
   * @return Value of the properties in an associative array
   */
  protected function getAllMediafileMetadata($mediafile_id) {
    return mediamosa_asset_mediafile_metadata::get_all_mediafile_metadata($mediafile_id);
  }

  /**
   * Delete a mediafile metadata value.
   * @param mediafile_id Unique id for the mediafile
   * @param prop_name Property name
   * @param type Type of the property
   */
  protected function deleteMediafileMetadata($mediafile_id, $prop_name, $type) {
    return mediamosa_asset_mediafile_metadata::delete_mediafile_metadata($mediafile_id, $prop_name, $type);
  }

  /**
   * Clean a mediafile from all of the metadata values.
   * @param mediafile_id Unique id for the mediafile
   */
  protected function deleteAllMediafileMetadata($mediafile_id) {
    return mediamosa_asset_mediafile_metadata::delete_all_mediafile_metadata($mediafile_id);
  }

  /**
   * Returns the absolute local filesystem path of a stream URI.
   *
   * @param string $uri
   *   A stream wrapper URI or a filesystem path, possibly including one or more
   *   symbolic links.
   *
   * @return string
   *   The absolute local filesystem path, or FALSE on failure.
   */
  protected function realpath($uri, $expected_realpath = '') {
    $realpath = mediamosa_io::realpath($uri);

    if (empty($expected_realpath)) {
      $this->pass(t("Converted uri '@uri' to '@realpath'", array('@uri' => $uri, '@realpath' => $realpath)));
    }
    else {
      $this->assert($realpath == $expected_realpath, t("Converted uri '@uri' to '@realpath'", array('@uri' => $uri, '@realpath' => $realpath)));
    }
    return $realpath;
  }
}
