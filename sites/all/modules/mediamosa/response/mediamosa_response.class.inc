<?php
/**
 * @file
 * The XML response class.
 *
 * The mediamosa_response class is used to output the response in XML or Plain
 * format.
 */

class mediamosa_response extends mediamosa {

  // ------------------------------------------------------------------ Consts
  // Was ERRORMESSAGE_ERROR.
  const ERROR = 'error';
  const SUCCESS = 'success';

  // Types.
  const RESPONSE_TYPE_XML = 'xml';
  const RESPONSE_TYPE_ATOM = 'atom';
  const RESPONSE_TYPE_RSS = 'rss';
  const RESPONSE_TYPE_JSON = 'json';
  const RESPONSE_TYPE_PLAIN = 'plain';
  const RESPONSE_TYPE_XML_OEMBED = 'xml_oembed';
  const RESPONSE_TYPE_JSON_OEMBED = 'json_oembed';

  // Result headers.
  const RESPONSE_REQUEST_RESULT = 'request_result';
  const RESPONSE_REQUEST_RESULT_DESCRIPTION = 'request_result_description';
  const RESPONSE_REQUEST_RESULT_ID = 'request_result_id';
  const RESPONSE_REQUEST_URI = 'request_uri';
  const RESPONSE_ITEM_COUNT = 'item_count';
  const RESPONSE_ITEM_COUNT_TOTAL = 'item_count_total';
  const RESPONSE_ITEM_OFFSET = 'item_offset';
  const RESPONSE_VERSION = 'version';

  // Download files.
  const RESPONSE_TYPE_DOWNLOAD = 'download';

  // Media (new 3.1)
  const RESPONSE_TYPE_MEDIA = 'media';

  // No response, HTTP header only.
  const RESPONSE_TYPE_VOID = 'void';

  // ------------------------------------------------------------------ Members
  // The stored response.
  protected $response = array();
  protected $response_success = FALSE;

  // Collect the attributes for the response type.
  protected $response_attributes = array();

  // The rest class object.
  protected $mediamosa_rest = NULL;

  // The request uri.
  protected $request_uri;
  protected $request_method;

  // Start in miliseconds when call was made.
  protected $start_time = 0;

  // Content_type to use for output.
  protected $response_type = mediamosa_response::RESPONSE_TYPE_XML;

  /**
   * The matched REST call object.
   *
   * @var mediamosa_rest_call
   */
  protected $rest_call = NULL;

  // Public counters.
  public $item_count = 0;
  public $item_count_total = 0;
  public $item_offset = 0;

  // --------------------------------------------------------------- Contructor.
  public function __construct() {
    parent::__construct();

    // Save the time we start the rest response
    $this->start_time = microtime(TRUE);

    // Save the request URI.
    $this->request_uri = rawurldecode(mediamosa_unicode::substr(mediamosa::request_uri(), mediamosa_unicode::strlen(base_path())));
    $this->request_uri = $this->request_uri == '' ? '/' : $this->request_uri;

    // Save the method.
    $this->request_method = $_SERVER['REQUEST_METHOD'];

    // All ok, now reset the header.
    $this->reset();
  }

  // --------------------------------------------------------- Public Functions.
  /**
   * Dispatch the REST request execution.
   *
   * @return
   *  Returns optional override on the response type (XML/JSON etc).
   */
  protected function process_call() {
    // Now execute the call.
    return $this->mediamosa_rest->process_call($this->rest_call);
  }

  /**
   * Return the response header array.
   */
  public function get_response_header() {
    return $this->response['header'];
  }

  /**
   * Process possible rest call.
   */
  public function process_rest() {

    // If REST interface is disabled, then don't bother.
    if (!mediamosa::is_app_enabled()) {
      return;
    }

    // Fix hash_salt bug when running simpletest on multiple vhosts with
    // different filesystems for bootstart.inc.
    if (mediamosa::in_simpletest_sandbox()) {
      global $drupal_hash_salt;

      $mediamosa_drupal_hash_salt_copy = mediamosa::variable_get('mediamosa_drupal_hash_salt');
      if (!empty($mediamosa_drupal_hash_salt_copy)) {
        $drupal_hash_salt = $mediamosa_drupal_hash_salt_copy;
      }
    }

    // In case of exceptions, the output will be XML.
    $this->response_type = mediamosa_response::RESPONSE_TYPE_XML;

    try {
      // Set the response handler.
      $previous_handler = set_error_handler(array($this, 'mediamosa_error_handler_response'), E_ALL);

      // Create the REST interface object.
      $this->mediamosa_rest = new mediamosa_rest();

      // Find the rest call.
      $this->rest_call = $this->mediamosa_rest->match_call();

      // No rest call matched? Will only get FALSE when admin switch is on.
      if (!$this->rest_call) {
        // Restore previous handler.
        if (!empty($previous_handler)) {
          set_error_handler($previous_handler, E_ALL);
        }

        return;
      }

      // When admin interface is enabled on the REST interface, then some URLs
      // will conflict with Drupal URLs, specially with URLs used in user
      // module. We allow some of the combinations with /user/ to work under the
      // REST interface.
      if (mediamosa::is_admin_enabled() && !mediamosa::in_simpletest_sandbox()) {
        $request_path = request_path() . '/';

        $urls = user_is_logged_in() ?
          array('^user/[0-9]+/$' => true, '^user/$' => false, '^user/autocomplete/$' => true, '^user/logout/$' => true)
          :
          array('^user/login/$' => true, '^user/password/$' => true, '^user/$' => false);

        foreach ($urls as $regex => $filter) {
          if (preg_match('@' . $regex . '@', $request_path)) {
            if (!$filter) {
              break;
            }

            if (!empty($previous_handler)) {
              set_error_handler($previous_handler, E_ALL);
            }
            return;
          }
        }
      }

      // At this point we take over the output from Drupal and will no longer
      // return to drupal.
      header(mediamosa_settings::X_MEDIAMOSA_VERSION . ': ' . mediamosa_version::get_current_version_str(TRUE));

      // Allow execution to continue even if the request gets canceled.
      // This might happen if the client is waiting for response and cancels.
      // Is enabled by default for POST rest calls.
      if ($this->rest_call[mediamosa_rest_call::NO_USER_ABORT]) {
        @ignore_user_abort(TRUE);
      }

      // Enable sessions.
      if (!drupal_session_started()) {
        drupal_session_start();
      }
      elseif (mediamosa::in_simpletest_sandbox()) {
        session_regenerate_id(FALSE);
      }

      // Some stuff is not setup when we are called inside simpletest and we
      // are called as REST call inside the test. Setup now if we are in
      // simpletest.
      mediamosa::setup_simpletest();

      // Make sure response type is set.
      if (empty($this->rest_call[mediamosa_rest_call::RESPONSE_TYPE])) {
        $this->rest_call[mediamosa_rest_call::RESPONSE_TYPE] = mediamosa_response::RESPONSE_TYPE_XML;
      }

      // Process the call.
      $this->rest_call[mediamosa_rest_call::RESPONSE_TYPE] = $this->process_call($this->rest_call);

      // Check the response parameter.
      $this->response_type = empty($this->rest_call[mediamosa_rest_call::RESPONSE_TYPE]) ? mediamosa_response::RESPONSE_TYPE_XML : $this->rest_call[mediamosa_rest_call::RESPONSE_TYPE];
    }
    catch (mediamosa_exception_redirect_and_exit $e) {
      // Redirect to URI.
      header('Location: ' . $e->get_redirect_uri());

      // We redirect and exit.
      exit();
    }
    catch (OAuthException $e) {
      print($e->getMessage());
      exit();
    }
    catch (mediamosa_exception_error_400 $e) {
      // Bad request.
      header($_SERVER['SERVER_PROTOCOL'] . ' 400 Bad Request');
      exit();
    }
    catch (mediamosa_exception_error_403 $e) {
      // Forbidden
      header($_SERVER['SERVER_PROTOCOL'] . ' 403 Forbidden');
      exit();
    }
    catch (mediamosa_exception_error_404 $e) {
      // Not found
      header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');
      exit();
    }
    catch (mediamosa_exception_error_500 $e) {
      // Internal server error.
      header($_SERVER['SERVER_PROTOCOL'] . ' 500 Internal Server Error');
      exit();
    }
    catch (mediamosa_exception_error_501 $e) {
      // Not implemented
      header($_SERVER['SERVER_PROTOCOL'] . ' 501 Not Implemented');
      exit();
    }
    catch (mediamosa_exception_error $e) {
      // If we are in simpletest, we need to skip logging of mediamosa
      // exceptions, because these might be generated intentially for testing.
      // The test must always check the error response anyway and not rely on
      // logging here.
      if (!mediamosa::in_simpletest_sandbox()) {
        mediamosa_debug::error_log('mediamosa_exception_error caught; ' . $e->getMessage(), $e->getFile(), $e->getLine());
      }

      $e->mediamosa_exception_rest_response($this);
    }
    catch (PDOException $e) {
      mediamosa_debug::error_log('PDOException caught; ' . $e->getMessage() . ', trace; ' . $e->getTraceAsString(), $e->getFile(), $e->getLine());
      try {
        mediamosa_watchdog::log_export(strtr('PDOException caught file @file at line @line; @message, trace; @trace.', array('@file' => $e->getFile(), '@line' => $e->getLine(), '@message' => $e->getMessage(), '@trace' => $e->getTraceAsString())));
      }
      catch (Exception $e) {
        // ignore.
      }

      $this->set_result(self::ERROR, mediamosa_error::HTTP_INTERNAL_SERVER_ERROR, 'PDOException caught; ' . $e->getMessage() . ",\ntrace; " . $e->getTraceAsString() . ",\nQuery string; " . $e->query_string . ",\nArgs; " . print_r($e->args, TRUE));
    }
    catch (Exception $e) {
      mediamosa_debug::error_log('Exception caught; ' . $e->getMessage() . ', trace; ' . $e->getTraceAsString(), $e->getFile(), $e->getLine());
      try {
        mediamosa_watchdog::log_export(strtr('Exception caught file @file at line @line; @message, trace; @trace.', array('@file' => $e->getFile(), '@line' => $e->getLine(), '@message' => $e->getMessage(), '@trace' => $e->getTraceAsString())));
      }
      catch (Exception $e) {
        // ignore.
      }

      $this->set_result(self::ERROR, $e->getCode(), $e->getMessage());
    }

    // Log the rest call.
    $this->log_event_restcall();

    // Stop and output our content.
    // Might run any added exit functions (drupal).
    exit($this->generate_output());
  }

  /**
   * Render a system default template, which is essentially a PHP template.
   *
   * @param $template_file
   *   The filename of the template to render.
   * @param $variables
   *   A keyed array of variables that will appear in the output.
   *
   * @return
   *   The output generated by the template.
   */
  public function render_template($template_file, $variables = array()) {
    // Extract the variables to a local namespace.
    extract($variables, EXTR_SKIP);

    // Start output buffering.
    ob_start();

    // Include the template file.
    include DRUPAL_ROOT . '/' . $template_file;

    // End buffering and return its contents.
    return ob_get_clean();
  }

  /**
   * Render array as XML.
   *
   * @param array $array
   * @param int $tab
   * @param array $options
   */
  public function render_array2xml(array $array, $tab = 0, array $options = array()) {
    $output = '';
    $options += array(
      'formatOutput' => TRUE,
    );

    // Setup tabs.
    $linefeed = $options['formatOutput'] ? "\n" : '';
    $tabs = $options['formatOutput'] ? str_repeat('  ', $tab) : '';

    foreach ($array as $key => $value) {
      if ( ($value === '' && !is_array($value)) || (is_array($value) && empty($value)) ) {
        $output .= $tabs . '<' . $key . '/>' . $linefeed;
        continue;
      }

      $attributes = '';

      // If value is array and contains numeric keys, then we need to repeat the
      // tag with that value. But the value can again be an array itself.
      if (is_array($value)) {
        $first_key = key($value);
        if (is_numeric($first_key)) {
          foreach ($value as $line) {
            $output .= self::render_array2xml(array($key => $line), $tab + 1, $options);
          }

          continue;
        }
        elseif ($first_key[0] == '#') {
          foreach ($value as $attribute => $value2) {
            $attributes = drupal_attributes(@unserialize(mediamosa_unicode::substr($attribute, 1)));
            $output .= $tabs . '<' . $key . $attributes;
            $output .= '>' . $linefeed . self::render_array2xml($value2, $tab + 1, $options) . $tabs;
            $output .= '</' . $key . '>' . $linefeed;
          }
          continue;
        }
      }

      $output .= $tabs . '<' . $key . $attributes;

      if (is_array($value)) {
        $output .= '>' . $linefeed . self::render_array2xml($value, $tab + 1, $options) . $tabs;
      }
      elseif ($value === '') {
        $output .= '/>' . $linefeed;
        continue;
      }
      else {
        $output .= '>' . mediamosa_unicode::xmlentities($value);
      }

      $output .= '</' . $key . '>' . $linefeed;
    }

    return $output;
  }

  /**
   * Generate output.
   */
  protected function generate_output() {
    // Make sure its set.
    $output = '';

    try {
      switch ($this->response_type) {
        case mediamosa_response::RESPONSE_TYPE_XML:
          header('Content-Type: text/xml; charset=utf-8');
          $output = $this->generate_xml();
          break;

        case mediamosa_response::RESPONSE_TYPE_PLAIN:
          header('Content-Type: text/plain; charset=utf-8');
          $output = $this->generate_plain_text();
          break;

        case mediamosa_response::RESPONSE_TYPE_DOWNLOAD:
        case mediamosa_response::RESPONSE_TYPE_MEDIA:
          // Output already handled by REST call.
          $output = '';

          // Any error during REST call we'll handle here.
          if ($this->response['header']['request_result'] == self::ERROR) {
            header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');
            $output = $this->response['header'][self::RESPONSE_REQUEST_RESULT_DESCRIPTION];
          }
          break;

        case mediamosa_response::RESPONSE_TYPE_JSON:
          header('Content-Type: application/json; charset=utf-8');
          $output = $this->generate_json();
          break;

        case mediamosa_response::RESPONSE_TYPE_ATOM:
          header('Content-Type: application/atom+xml; charset=utf-8');
          $output = $this->generate_atom();
          break;

        case mediamosa_response::RESPONSE_TYPE_RSS:
          header('Content-Type: application/rss+xml; charset=utf-8');
          $output = $this->generate_rss();
          break;

        // Oembed.
        case mediamosa_response::RESPONSE_TYPE_JSON_OEMBED:
          header('Content-Type: application/json; charset=utf-8');
          $output = $this->generate_json_oembed();
          break;

        case mediamosa_response::RESPONSE_TYPE_XML_OEMBED:
          header('Content-Type: text/xml; charset=utf-8');
          $output = $this->generate_xml_oembed();
          break;

        case mediamosa_response::RESPONSE_TYPE_VOID:
          $output = '';
          break;

        default:
          // Not implemented
          header($_SERVER['SERVER_PROTOCOL'] . ' 501 Not Implemented');
          exit(strtr("Unknown response type @response_type used!", array('@response_type' => $this->response_type)));
      }
    }
    catch (Exception $e) {
      mediamosa_debug::error_log('Critical exception caught during output; ' . $e->getMessage());
      $this->set_result(self::ERROR, $e->getCode(), $e->getMessage());
    }

    return $output;
  }

  /**
   * Reset the output buffers.
   */
  public function reset() {
    $errors = NULL;

    // We don't reset the errors if there are any.
    if (isset($this->response['errors'])) {
      $errors = $this->response['errors'];
    }

    /**
     * Because ERRORCODE_OKAY does not have a text, we do not retrieve it here.
     * This saves a call to the error table in the database when drupal mode is
     * active.
     */
    $this->response = array(
      'header' => array(
        self::RESPONSE_REQUEST_RESULT => self::SUCCESS,
        self::RESPONSE_REQUEST_RESULT_ID => mediamosa_error::ERRORCODE_OKAY,
        self::RESPONSE_REQUEST_RESULT_DESCRIPTION => '',
        self::RESPONSE_REQUEST_URI => '[' . $this->request_method . '] ' . (mediamosa_unicode::substr($this->request_uri, 0, mediamosa_unicode::strlen('internal')) == 'internal' ? 'Internal MediaMosa call' : $this->request_uri),
        self::RESPONSE_ITEM_COUNT => 0,
        self::RESPONSE_VERSION => mediamosa_version::MEDIAMOSA_VERSION,
      ),
      'items' => array()
    );

    // Reset setting.
    $this->response_success = FALSE;

    if ($errors) {
      $this->response['errors'] = $errors;
    }

    $this->item_count = $this->item_count_total = $this->item_offset = 0;
  }

  /**
   * Add one item to the output.
   *
   * @param array $item
   *   The item with data to add.
   * @param string $id
   *   The ID of the item.
   * @param string $id_name
   *   The name of the ID to use as attribute.
   */
  public function add_item($item, array $attributes = array()) {
    if (empty($attributes)) {
      $this->response['items'][++$this->item_count] = $item;
    }
    else {
      $this->response['items'][++$this->item_count]['#' . serialize($attributes)] = $item;
    }
  }

  /**
   * Add an attribute to the response tag.
   *
   * @param string $name
   *  The name of the attribute.
   * @param string $value
   *  The value.
   */
  public function add_reponse_attribute($name, $value) {
    $this->response_attributes[$name] = $value;
  }

  /**
   * Get the list of response attributes.
   *
   * @return
   *  Array of attributes for <response> tag.
   */
  public function get_reponse_attributes() {
    return $this->response_attributes;
  }

  /**
   * Add an error to the header.
   *
   * @param array|string $mixed_item
   */
  public function add_program_error($mixed_item) {

    if (!isset($this->response['errors'])) {
      $this->response['errors'] = array();
    }

    if (count($this->response['errors']) < 100) {
      $this->response['errors'][count($this->response['errors'])] = $mixed_item;
    }
    else {
      $this->response['errors'][100] = 'More than 100 errors, skipping.';
    }
  }

  /**
   * Set the header result.
   *
   * @param string $request_result
   * @param int $request_result_id
   * @param string $request_result_description
   */
  public function set_result($request_result, $request_result_id, $request_result_description = FALSE) {
    $this->response['header'][self::RESPONSE_REQUEST_RESULT] = $request_result;
    $this->response['header'][self::RESPONSE_REQUEST_RESULT_ID] = $request_result_id;
    $this->response['header'][self::RESPONSE_REQUEST_RESULT_DESCRIPTION] = $request_result_description !== FALSE ? $request_result_description : mediamosa_error::error_code_find_description($request_result_id);
  }

  /**
   * Set when the rest call was successful, but doesn't have any data.
   */
  public function set_result_okay() {
    self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_OKAY);
    $this->response_success = TRUE;
  }

  /**
   * For setting additional custom info in header.
   *
   * @param array|string $mixed_data
   *   The data to set.
   * @param string $name
   *   The parameter name.
   */
  public function set_result_header_extra($mixed_data, $name = 'additional_info') {
    $this->response['header'][$name] = $mixed_data;
  }

  /**
   * Return the time in miliseconds of the duration of the call.
   *
   * @return integer
   */
  public function get_processed_time() {
    return round(microtime(TRUE) - $this->start_time, 4);
  }

  /**
   * Set the URI in the header.
   *
   * @param string $uri
   */
  public function set_matched_uri($uri) {
    $this->response['header']['request_matched_uri'] = (mediamosa_unicode::substr($uri, 0, 1) == '/' ? '' : '/') . $uri;
  }

  /**
   * Set the class in the header.
   *
   * @param string $class_name
   *   The class name to set.
   */
  public function set_class($class_name) {
    $this->response['header']['request_class'] = $class_name;
  }

  /**
   * Set the method used in the header.
   *
   * @param string $method
   */
  public function set_matched_method($method) {
    $this->response['header']['request_matched_method'] = $method;
  }

  /**
   * Log the REST call.
   *
   */
  public function log_event_restcall() {
    if (self::is_log_rest() &&
      (!self::get_log_rest_ip_address() || ip_address() == self::get_log_rest_ip_address()) &&
      (!self::get_log_rest_app_id() || self::get_environment_app_id() == self::get_log_rest_app_id())) {
      mediamosa_statistics::log_event_restcall(
        t('[@method] @request_uri', array(
          '@method' => $this->request_method,
          '@request_uri' => $this->request_uri,
        )),
        $this->get_processed_time(),
        mediamosa_db::query_count(),
        ip_address(),
        time(),
        (self::get_environment_app_id() ? self::get_environment_app_id() : 0)
      );
    }
  }

  // ------------------------------------------------------ Protected functions.
  /**
   * Generate output as plain text.
   *
   * @return string
   */
  protected function generate_plain_text() {
    $output = '';

    for ($x = 1; $x <= $this->item_count; $x++) {
      $output .= $this->response['items'][$x]; // Just concat the data
    }

    $this->log_event_restcall();
    return $output;
  }

  /**
   * Return result as PHP array.
   *
   * @return string
   */
  public function generate_array() {

    $this->response['header']['request_process_time'] = $this->get_processed_time();

    $this->response['header']['item_count_total'] = (!$this->item_count_total ? $this->item_count : $this->item_count_total);
    $this->response['header']['item_offset'] = $this->item_offset;
    $this->response['header']['item_count'] = $this->item_count;

    if (!$this->item_count && $this->response['header']['request_result_id'] == mediamosa_error::ERRORCODE_OKAY && !$this->response_success) {
      self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_EMPTY_RESULT);
    }

    // Sort the header.
    ksort($this->response['header']);

    $this->response = $this->render_array($this->response);

    // Log the rest call.
    $this->log_event_restcall();

    // This return response, which is a array.
    return $this->response;
  }

   /**
   * Render array to the old format.
   */
  public function render_array($array) {
    if (!is_array($array)) {
      return $array;
    }

    $new_array = array();

    foreach ($array as $key => $value) {
      if ($key{0} != '#') {
        $new_array[$key] = $this->render_array($value);
      }
      elseif (is_array($value)) {
        foreach ($value as $key2 => $value2) {
          if (is_array($value2)) {
            $new_array[$key2] = $this->render_array($value2);
          }
          else {
            $new_array[$key2] = $value2;
          }
        }
      }
    }

    return $new_array;
  }

  /**
   * Generate the JSON output.
   *
   * @return string
   */
  public function generate_json() {
    return drupal_json_encode($this->response);
  }

  /**
   * Generate the JSON output for oEmbed.
   *
   * @return string
   */
  public function generate_json_oembed() {
    $response = array();

    foreach ($this->response['items'] as $item) {
      foreach ($item as $key => $value) {
        $response[$key] = $value;
      }
    }

    return drupal_json_encode($response);
  }

  /**
   * Generate the XML output for oEmbed.
   *
   * @return string
   */
  public function generate_xml_oembed() {
    // Create the SimpleXML element.
    $xml_response = new SimpleXMLElement('<?xml version="1.0" encoding="utf-8" ?><oembed></oembed>');

    foreach ($this->response['items'] as $item) {
      foreach ($item as $key => $value) {
        $this->generate_plain_xml_add_child($xml_response, $key, $value);
      }
    }

    $dom_document = new DOMDocument('1.0', 'UTF-8');
    $dom_document->formatOutput = TRUE;

    $dom_document_node = dom_import_simplexml($xml_response);
    $dom_document_node = $dom_document->importNode($dom_document_node, TRUE);
    $dom_document->appendChild($dom_document_node);
    return $dom_document->saveXML();
  }

  /**
   * Generate the Atom output.
   *
   * @return string
   */
  public function generate_atom() {
    global $base_url;

    $this->response['header']['request_process_time'] = $this->get_processed_time();

    $this->response['header']['item_count_total'] = (!$this->item_count_total ? $this->item_count : $this->item_count_total);
    $this->response['header']['item_offset'] = $this->item_offset;
    $this->response['header']['item_count'] = $this->item_count;

    if (!$this->item_count && $this->response['header']['request_result_id'] == mediamosa_error::ERRORCODE_OKAY && !$this->response_success) {
      self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_EMPTY_RESULT);
    }

    // Sort the header.
    ksort($this->response['header']);

    // Log the rest call.
    $this->log_event_restcall();

    // Create the SimpleXML element.
    $xml_response = new SimpleXMLElement('<?xml version="1.0" encoding="utf-8" ?><feed xmlns="http://www.w3.org/2005/Atom"></feed>');

    // Author.
    $xml_element = $xml_response->addChild('author');
    $this->generate_plain_xml_add_child($xml_element, 'name', mediamosa_settings::ATOM_NAME);

    // Generator.
    $xml_element = $xml_response->addChild('generator', mediamosa_settings::ATOM_GENERATOR);
    $xml_element->addAttribute('uri', mediamosa_settings::ATOM_GENERATOR_URI);
    $xml_element->addAttribute('version', mediamosa_settings::ATOM_GENERATOR_VERSION);

    // Icon.
    $xml_element = $xml_response->addChild('icon', $base_url . (drupal_substr($base_url, -1) == '/' ? '' : '/') . drupal_get_path('module', 'mediamosa') . mediamosa_settings::PATH_ICON);

    // Id.
    $xml_element = $xml_response->addChild('id', mediamosa_settings::ATOM_ID);

    // Link.
    $xml_element = $xml_response->addChild('link');
    $xml_element->addAttribute('href', $base_url);

    // Logo.
    $xml_element = $xml_response->addChild('logo', $base_url . (drupal_substr($base_url, -1) == '/' ? '' : '/') . drupal_get_path('module', 'mediamosa') . mediamosa_settings::PATH_LOGO);

    // Subtitle.
    $xml_element = $xml_response->addChild('subtitle', mediamosa_settings::ATOM_SUBTITLE);
    $xml_element->addAttribute('type', 'text');

    // Title.
    $xml_element = $xml_response->addChild('title', mediamosa_settings::ATOM_TITLE);
    $xml_element->addAttribute('type', 'text');

    // Updated.
    $xml_element = $xml_response->addChild('updated', mediamosa_datetime::time_to_rfc_3339());

    //
    // Extension.
    //
    $xml_extension = $xml_response->addChild('extension');

    foreach ($this->response['header'] as $key => $value) {
      $this->generate_plain_xml_add_child($xml_extension, $key, $value);
    }

    if (!empty($this->response['errors'])) {
      $xml_response_errors = $xml_extension->addChild('errors');

      foreach ($this->response['errors'] as $key => $error) {
        $xml_response_errors->addChild('error', htmlspecialchars($error));
      }
    }

    //
    // Entry.
    //
    foreach ($this->response['items'] as $item) {
      $xml_response_item = $xml_response->addChild('entry');

      $mediafile_id = '';
      if (!empty($item['mediafiles']['mediafile'])) {
        $mediafiles = $item['mediafiles']['mediafile'];
        $mediafile = reset($mediafiles);
        $mediafile_id = $mediafile['mediafile_id'];
      }
      elseif (!empty($item['mediafile_id'])) {
        $mediafile_id = $item['mediafile_id'];
      }
      $link = $base_url . '/openapi/mediaItems/@me/@self/' . $item['asset_id'] . '/' . $mediafile_id;

      // Author.
      $xml_element = $xml_response_item->addChild('author');
      $this->generate_plain_xml_add_child($xml_element, 'name', !empty($item['dublin_core']['creator'][0]) ? $item['dublin_core']['creator'][0] : mediamosa_settings::ATOM_NAME);

      // Category.
      $query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'ac');
      $table_alias = $query->join(mediamosa_collection_db::TABLE_NAME, 'c', 'c.coll_id = ac.coll_id');
      $query
        ->condition('ac.' . mediamosa_asset_collection_db::ASSET_ID, $item['asset_id'])
        ->fields('c', array(mediamosa_collection_db::ID, mediamosa_collection_db::TITLE))
        ->range(0, mediamosa_settings::ATOM_ENTRY_CATEGORY_MAX);
      $result = $query->execute();
      foreach ($result as $record) {
        $xml_element = $xml_response_item->addChild('category');
        $this->generate_plain_xml_add_child($xml_element, 'term', $record[mediamosa_collection_db::ID]);
        $this->generate_plain_xml_add_child($xml_element, 'label', $record[mediamosa_collection_db::TITLE]);
      }

      // Content.
      $xml_element = $xml_response_item->addChild('content');
      $xml_element->addAttribute('type', 'xhtml');

      // Description part.
      if (!empty($item['dublin_core']['description'][0])) {
        $xml_element_div = $xml_element->addChild('div', $item['dublin_core']['description'][0]);
        $xml_element_div->addAttribute('xmlns', 'http://www.w3.org/1999/xhtml');
      }

      // Image.
      if (!empty($item['vpx_still_url'])) {
        $xml_element_div = $xml_element->addChild('div');
        $xml_element_div->addAttribute('xmlns', 'http://www.w3.org/1999/xhtml');
        $xml_element_div_img = $xml_element_div->addChild('img');
        $xml_element_div_img->addAttribute('src', $item['vpx_still_url']);
      }
      // "More" link.
      $xml_element_div = $xml_element->addChild('div');
      $xml_element_div->addAttribute('xmlns', 'http://www.w3.org/1999/xhtml');
      $xml_element_div_link = $xml_element_div->addChild('a', 'more');
      $xml_element_div_link->addAttribute('href', $link);
      // MediaItem part.
      if (!empty($item['mediaitem'])) {
        $xml_element_mediaitem = $xml_element->addChild('mediaItem');
        $xml_element_mediaitem->addAttribute('xmlns', 'http://ns.opensocial.org/2008/opensocial');

        foreach ($item['mediaitem'] as $key => $value) {
          //$xml_element_mediaitem_content = $xml_element_mediaitem->addChild($key, $value);
          $this->generate_plain_xml_add_child($xml_element_mediaitem, $key, $value);
        }

        // Unset "mediaitem". We won't use it in the "extension".
        unset($item['mediaitem']);
      }

      // Contributor.
      if (!empty($item['dublin_core']['contributor'][0])) {
        $xml_element = $xml_response_item->addChild('contributor');
        $this->generate_plain_xml_add_child($xml_element, 'name', $item['dublin_core']['contributor'][0]);
      }

      // Id.
      $xml_element = $xml_response_item->addChild('id', mediamosa_settings::ATOM_ENTRY_ID . $item['asset_id']);

      // Link.
      $xml_element = $xml_response_item->addChild('link');
      $xml_element->addAttribute('href', $link);

      // Published.
      if (!empty($item['dublin_core']['date'][0])) {
        $timestamp = mediamosa_datetime::date8601_to_timestamp($item['dublin_core']['date'][0], 'dc_date', mediamosa_sdk::TYPE_DATETIME, 'U');
        $xml_element = $xml_response_item->addChild('published', mediamosa_datetime::time_to_rfc_3339($timestamp));
      }

      // Rights.
      if (!empty($item['dublin_core']['rights'][0])) {
        $xml_element = $xml_response_item->addChild('rights', $item['dublin_core']['rights'][0]);
        $xml_element->addAttribute('type', 'text');
      }

      // Summary.
      if (!empty($item['qualified_dublin_core']['description_abstract'][0])) {
        $xml_element = $xml_response_item->addChild('summary', $item['qualified_dublin_core']['description_abstract'][0]);
        $xml_element->addAttribute('type', 'text');
      }

      // Title.
      if (!empty($item['dublin_core']['title'][0])) {
        $xml_element = $xml_response_item->addChild('title', $item['dublin_core']['title'][0]);
        $xml_element->addAttribute('type', 'text');
      }

      // Updated.
      if (!empty($item['videotimestampmodified'])) {
        $timestamp = mediamosa_datetime::date8601_to_timestamp($item['videotimestampmodified'], 'videotimestampmodified', mediamosa_sdk::TYPE_DATETIME, 'U');
        $xml_element = $xml_response_item->addChild('updated', mediamosa_datetime::time_to_rfc_3339($timestamp));
      }

      // Extension.
      $xml_extension = $xml_response_item->addChild('extension');
      // Process items with a maximum recursion depth (last parameter).
      //
      foreach ($item as $key => $value) {
        $this->generate_plain_xml_add_child($xml_extension, $key, $value);
      }
    }

    $dom_document = new DOMDocument('1.0', 'UTF-8');
    $dom_document->formatOutput = TRUE;

    $dom_document_node = dom_import_simplexml($xml_response);
    $dom_document_node = $dom_document->importNode($dom_document_node, TRUE);
    $dom_document->appendChild($dom_document_node);
    return $dom_document->saveXML();
  }

  /**
   * Generate the RSS output.
   *
   * @return string
   */
  public function generate_rss() {
    global $base_url, $language_content;

    $channel = array(
      'lastBuildDate' => mediamosa_datetime::time_to_rfc_822(),
      'generator' => mediamosa_settings::RSS_GENERATOR,
      'image' => array(
        'url' => $base_url . (drupal_substr($base_url, -1) == '/' ? '' : '/') . drupal_get_path('module', 'mediamosa') . mediamosa_settings::PATH_LOGO,
      ),
    );

    $item_length = variable_get('feed_item_length', 'fulltext');
    $namespaces = array('xmlns:dc' => 'http://purl.org/dc/elements/1.1/');
    $teaser = ($item_length == 'teaser');

    // Items.
    $items = '';
    foreach ($this->response['items'] as $item) {
      $mediafile_id = '';
      if (!empty($item['mediafiles']['mediafile'])) {
        $mediafiles = $item['mediafiles']['mediafile'];
        $mediafile = reset($mediafiles);
        $mediafile_id = $mediafile['mediafile_id'];
      }
      elseif (!empty($item['mediafile_id'])) {
        $mediafile_id = $item['mediafile_id'];
      }
      $link = $base_url . '/openapi/mediaItems/@me/@self/' . $item['asset_id'] . '/' . $mediafile['mediafile_id'];

      // Base data.
      $title = !empty($item['dublin_core']['title'][0]) ? $item['dublin_core']['title'][0] : '';
      $description = !empty($item['dublin_core']['description'][0]) ? $item['dublin_core']['description'][0] : '';
      // Image and "more" link.
      $description = '<p>' . $description . '</p>' . (!empty($item['vpx_still_url']) ? '<p><img src="' . $item['vpx_still_url']. '"></p>' : '') . '<p><a href="' . $link . '" target="_blank">more</a></p>';

      // More data.
      $args = array();
      $args[] = array('key' => 'author', 'value' => !empty($item['dublin_core']['creator'][0]) ? $item['dublin_core']['creator'][0] : '');
      if (!empty($item['dublin_core']['date'][0])) {
        $timestamp = mediamosa_datetime::date8601_to_timestamp($item['dublin_core']['date'][0], 'dc_date', mediamosa_sdk::TYPE_DATETIME, 'U');
        $args[] = array('key' => 'pubDate', 'value' => mediamosa_datetime::time_to_rfc_3339($timestamp));
      }
      $args[] = array('key' => 'guid', 'value' => $item['asset_id'] . ' at ' . $base_url, 'attributes' => array('isPermaLink' => 'false'));
      $args[] = array('key' => 'dc:creator', 'value' => !empty($item['dublin_core']['creator'][0]) ? $item['dublin_core']['creator'][0] : '');

      // Category.
      $query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'ac');
      $table_alias = $query->join(mediamosa_collection_db::TABLE_NAME, 'c', 'c.coll_id = ac.coll_id');
      $query
        ->condition('ac.' . mediamosa_asset_collection_db::ASSET_ID, $item['asset_id'])
        ->fields('c', array(mediamosa_collection_db::ID, mediamosa_collection_db::TITLE))
        ->range(0, mediamosa_settings::ATOM_ENTRY_CATEGORY_MAX);
      $result = $query->execute();
      foreach ($result as $record) {
        if ($record[mediamosa_collection_db::TITLE]) {
          $args[] = array('key' => 'category', 'value' => $record[mediamosa_collection_db::TITLE]);
        }
      }

      // Add the item to the RSS.
      $items .= format_rss_item($title, $link, $description, $args);
    }

    $channel_defaults = array(
      'version'     => '2.0',
      'title'       => t('MediaMosa RSS feed at @place', array('@place' => variable_get('site_name', 'Drupal'),)),
      'link'        => $base_url,
      'description' => variable_get('feed_description', ''),
      'language'    => $language_content->language,
    );
    $channel_extras = array_diff_key($channel, $channel_defaults);
    $channel = array_merge($channel_defaults, $channel);

    $output = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    $output .= "<rss version=\"" . $channel["version"] . "\" xml:base=\"" . $base_url . "\" " . drupal_attributes($namespaces) . ">\n";
    $output .= format_rss_channel($channel['title'], $channel['link'], $channel['description'], $items, $channel['language'], $channel_extras);
    $output .= "</rss>\n";

    drupal_add_http_header('Content-Type', 'application/rss+xml; charset=utf-8');
    return $output;
  }

  /**
   * Generate the XML output.
   *
   * @return string
   *   The generated XML.
   */
  public function generate_xml() {

    // Use dom.
    $dom_document = new DOMDocument('1.0', 'UTF-8');
    $dom_document->formatOutput = TRUE;

    $this->response['header']['item_count_total'] = (!$this->item_count_total ? $this->item_count : $this->item_count_total);
    $this->response['header']['item_offset'] = $this->item_offset;
    $this->response['header']['item_count'] = $this->item_count;

    if (!$this->item_count && $this->response['header']['request_result_id'] == mediamosa_error::ERRORCODE_OKAY && !$this->response_success) {
      self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_EMPTY_RESULT);
    }

    if (mediamosa_debug::is_debug()) {
      $this->response['header']['request_query_count'] = mediamosa_db::query_count();
    }

    // Sort the header.
    ksort($this->response['header']);

    // Response element.
    $dom_element_response = $dom_document->createElement('response');
    $dom_document->appendChild($dom_element_response);

    // Header.
    $dom_element_header = $dom_document->createElement('header');
    $dom_element_response->appendChild($dom_element_header);

    // Items.
    $dom_element_items = $dom_document->createElement('items');
    $dom_element_response->appendChild($dom_element_items);

    if (!empty($this->response['errors'])) {
      $dom_element_errors = $dom_document->createElement('errors');

      foreach ($this->response['errors'] as $key => $error) {
        $dom_element_errors->appendChild(new DOMElement('error', mediamosa_unicode::xmlentities($error)));
      }
    }

    foreach ($this->response['items'] as $item) {
      $this->generate_xml_add_child($dom_document, $dom_element_items, 'item', $item);
    }

    // Add header items at last.
    $this->response['header']['request_process_time'] = $this->get_processed_time();
    foreach ($this->response['header'] as $key => $value) {
      $this->generate_xml_add_child($dom_document, $dom_element_header, $key, $value);
    }

    if (!empty($dom_element_errors)) {
      // Add to document.
      $dom_element_header->appendChild($dom_element_errors);
    }

    return $dom_document->saveXML();
  }

  /**
   * Generate the items array into xml.
   */
  public function generate_xml_items() {

    if (empty($this->response['items'])) {
      return '  <items/>';
    }

    $output = '  <items>' . "\n";
    foreach ($this->response['items'] as $item) {
      $output .= self::render_array2xml(array('item' => $item), 2);
    }
    $output .= '  </items>' . "\n";
    return $output;
  }

  /**
   * Add child item to XML node.
   *
   * @param array $item
   * @param string $key
   * @param array/string $mixed_value
   * @param int $max_depth
   * @return NULL or added item.
   */
  protected function generate_xml_add_child($dom_document, $dom_node, $key, $mixed_value, $max_depth = 99) {
    if (!--$max_depth) {
      return NULL;
    }

    // Check if value is an array.
    if (!is_array($mixed_value)) {
      return $dom_node->appendChild(new DOMElement($key, mediamosa_unicode::xmlentities($mixed_value)));
    }

    // Is an array.
    $first_key = key($mixed_value);
    if (is_numeric($first_key)) {
      foreach ($mixed_value as $mixed_key => $value) {
        $this->generate_xml_add_child($dom_document, $dom_node, $key, $value, $max_depth);
      }

      return NULL;
    }

    if ($first_key[0] == '#') {
      foreach ($mixed_value as $mixed_key => $value) {
        $dom_element = $this->generate_xml_add_child($dom_document, $dom_node, $key, $value, $max_depth);

        if ($mixed_key != '#0') {
          $attributes = @unserialize(mediamosa_unicode::substr($mixed_key, 1));
          if (is_array($attributes)) {
            foreach ($attributes as $attr_name => $attr_value) {
              assert(count($attr_name));
              $dom_element->setAttribute($attr_name, $attr_value);
            }
          }
        }
      }

      return NULL;
    }

    $dom_element_child = $dom_node->appendChild(new DOMElement($key));
    foreach ($mixed_value as $mixed_key => $value) {
      $this->generate_xml_add_child($dom_document, $dom_element_child, $mixed_key, $value, $max_depth);
    }

    return $dom_element_child;
  }

  /**
   * Add child item to XML node.
   *
   * @param array $item
   * @param string $key
   * @param array/string $mixed_value
   * @param int $max_depth
   * @return NULL or added item.
   */
  protected function generate_plain_xml_add_child($item, $key, $mixed_value, $max_depth = 99) {
    if (!--$max_depth) {
      return NULL;
    }

    if (!is_array($mixed_value)) {
      $item_child = $item->addChild($key, mediamosa_unicode::xmlentities($mixed_value));

      if ($key[0] == '#') {
        $attributes = @unserialize(mediamosa_unicode::substr($key, 1));
        if (is_array($attributes)) {
          foreach ($attributes as $attr_name => $attr_value) {
            assert(count($attr_name));
            $item_child->addAttribute($attr_name, $attr_value);
          }
        }
      }

      return $item_child;
    }

    $first_key = key($mixed_value);
    if (is_numeric($first_key)) {
      foreach ($mixed_value as $mixed_key => $value) {
        if (is_array($value)) {
          $item_child = $item->addChild($key);
          foreach ($value as $key_child => $value_child) {
            $this->generate_plain_xml_add_child($item_child, $key_child, $value_child, $max_depth);
          }
        }
        else {
          $item->addChild($key, mediamosa_unicode::xmlentities($value));
        }
      }

      return NULL;
    }

    if ($first_key[0] == '#') {
      foreach ($mixed_value as $mixed_key => $value) {
        $item_child = $this->generate_plain_xml_add_child($item, $key, $value, $max_depth);

        if ($mixed_key != '#0') {
          $attributes = @unserialize(mediamosa_unicode::substr($mixed_key, 1));
          if (is_array($attributes)) {
            foreach ($attributes as $attr_name => $attr_value) {
              assert(count($attr_name));
              $item_child->addAttribute($attr_name, $attr_value);
            }
          }
        }
      }

      return NULL;
    }

    $item_child = $item->addChild($key);
    foreach ($mixed_value as $key_child => $value_child) {
      $this->generate_plain_xml_add_child($item_child, $key_child, $value_child, $max_depth);
    }

    return $item_child;
  }

  /**
   * Replacement for the current error handler.
   *
   * @param string $error_level
   * @param string $message
   * @param string $filename
   * @param int $line
   * @param string $context
   * @return bool
   */
  public function mediamosa_error_handler_response($error_level, $message, $filename, $line, $context) {

    // Possible backtrace caller.
    $caller = NULL;

    if ($error_level) {
      // All these constants are documented at http://php.net/manual/en/errorfunc.constants.php.
      $a_types = array(
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

      $caller = _drupal_get_last_caller(debug_backtrace());

      // We treat recoverable errors as fatal.
      $params = array(
        '@type' => (isset($a_types[$error_level]) ? $a_types[$error_level] : 'Unknown error'),
        '@message' => $message,
        '@function' => $caller['function'],
        '@file' => $caller['file'],
        '@line' => $caller['line']
      );

      // Add to XML output.
      $this->add_program_error(strtr('@type: @message, @function() in file @file on line @line.', $params));
    }

    // Call our parent error handler.
    return parent::mediamosa_error_handler($error_level, $message, $filename, $line, $context, $caller);
  }
}
