<?php
/**
 * @file
 * The REST class object contains the REST layer;
 * translates uri into rest call functions and handles all out going data
 * and errors.
 */

class mediamosa_rest {
  // ------------------------------------------------------------------ Consts

  // ------------------------------------------------------------------ Constructor
  function __construct() {
  }

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Return all available versions used in REST calls.
   *
   * @return array
   *  All version numbers
   */
  public static function get_all_versions($latest = '') {

    // Find all REST calls of enabled modules.
    $rest_calls = module_invoke_all('mediamosa_register_rest_call');

    // The REST calls versions.
    $versions = array();

    // Collect all versions in the restcalls.
    foreach ($rest_calls as $rest_call) {
      foreach ($rest_call as $method => $rest_call_info) {
        $versions[$rest_call_info[mediamosa_rest_call::VERSION]] = t('version @version', array('@version' => $rest_call_info[mediamosa_rest_call::VERSION]));
      }
    }

    // Must be first.
    $versions[mediamosa_version::LATEST] = empty($latest) ? t('Latest') : $latest;

    // Sort on version.
    krsort($versions);

    return $versions;
  }

  /**
   * Match the current URL to REST call, and return the REST call object.
   *
   * @param string $uri
   *  The request URL.
   * @param string $method
   *  The method.
   */
  public static function get_uri_definition($request_uri, $request_method = FALSE) {

    if (!$request_method) {
      $request_method = $_SERVER['REQUEST_METHOD'];

      // If its a HEAD call, switch method to GET.
      if ($request_method == mediamosa_rest_call::METHOD_HEAD) {
        $request_method = mediamosa_rest_call::METHOD_GET;
      }
    }

    // Find all REST calls of enabled modules.
    $rest_calls = self::get_rest_calls();
    if (empty($rest_calls[$request_method])) {
      return FALSE;
    }

    // Sort the rest calls for the requested method.
    krsort($rest_calls[$request_method]);

    // Remove any double //, so explode will work fine.
    while (strpos($request_uri, '//') !== FALSE) {
      $request_uri = str_replace('//', '/', $request_uri);
    }

    // Split the current request URI.
    $request_uris = explode('/', $request_uri);

    // Make sure empty strings in require uris are removed.
    foreach ($request_uris as $key => $part) {
      if (trim($part) == '') {
        unset($request_uris[$key]);
      }
    }

    // We need the original.
    $request_uri_cpy = implode('/', $request_uris);

    // Check if URL internal / external.
    if ($request_uris[0] == 'internal' || $request_uris[0] == 'external') {
      unset($request_uris[0]);
      $request_uri = implode('/', $request_uris);
      $request_uris = explode('/', $request_uri);
    }

    // Reset our matching call.
    $rest_call_return = FALSE;

    // Now find a match.
    foreach ($rest_calls[$request_method] as $uri => $rest_call) {
      $uri_parts_array = ($uri != '' ? explode('/', $uri) : array());

      // Number of parts must match.
      if (count($request_uris) != count($uri_parts_array)) {
        continue;
      }

      // Both empty? Then we match '/'.
      if (!count($request_uris) && !count($uri_parts_array)) {
        $rest_call[mediamosa_rest_call::URI] = $uri;
        $rest_call_return = $rest_call;
        break;
      }

      // Build the REGEX so we can see if it matches.
      $regex_array = array();
      foreach ($uri_parts_array as $pos => $uri_part) {
        if (mediamosa_unicode::substr($uri_part, 0, 2) == '#$') {
          $regex_array[] = "[0-9]*(:[a-zA-Z]+)?";
          $rest_call[mediamosa_rest_call::URI_PARAMS][mediamosa_unicode::substr($uri_part, 2)] = $request_uris[$pos];
        }
        elseif (mediamosa_unicode::substr($uri_part, 0, 1) == '$') {
          $regex_array[] = "[^\\/]*"; // MediaMosa is completly lose, any char EXCEPT /.

          // Replace + for space, seems arg() does urldecode the %## values into
          // chars, but not the + into spaces.
          $rest_call[mediamosa_rest_call::URI_PARAMS][mediamosa_unicode::substr($uri_part, 1)] = str_replace('+', ' ', ($request_uris[$pos]));
        }
        else {
          $regex_array[] = $uri_part;
        }
      }

      // Does it match with our REGEX?
      $regex_string = '@^' . implode('/', $regex_array) . '$@';
      if (preg_match($regex_string, $request_uri)) {
        // Check the match_uri() static function.
        if (method_exists($rest_call[mediamosa_rest_call::CLASS_NAME], mediamosa_rest_call::MATCH_URI_FUNCTION)) {
          if (!call_user_func(array($rest_call[mediamosa_rest_call::CLASS_NAME], mediamosa_rest_call::MATCH_URI_FUNCTION), $request_uri)) {
            continue;
          }
        }

        // Check the number of wildchar parts.
        // Less wildchar means better match.
        $uri_parts = explode('/', $uri);
        $wildchar = 0;
        foreach ($uri_parts as $uri_part) {
          if (drupal_substr($uri_part, 0, 1) == '$') {
            $wildchar++;
          }
        }
        if (!$rest_call_return) {
          $rest_call[mediamosa_rest_call::URI] = $uri;
          $rest_call_return = $rest_call;
        }
      }
    }

    if (!$rest_call_return) {
      // Not found.
      return FALSE;
    }

    mediamosa_debug::log("Matched URI '@uri' to REST call '@rest_uri' [@method]", array('@uri' => $request_uri_cpy, '@rest_uri' => $rest_call_return[mediamosa_rest_call::URI], '@method' => $request_method));

    // Store the original.
    $rest_call_return[mediamosa_rest_call::URI_REQUEST] = $request_uri_cpy;

    // Return found REST call and set all its defaults.
    return self::set_default_rest_call($rest_call_return);
  }

  /**
   * Setup default and test parameters in REST array.
   *
   * @param array $rest_call
   */
  public static function set_default_rest_call(array $rest_call) {
    // Make sure all are valid values.
    assert(in_array($rest_call[mediamosa_rest_call::METHOD], array(mediamosa_rest_call::METHOD_GET, mediamosa_rest_call::METHOD_DELETE, mediamosa_rest_call::METHOD_POST, mediamosa_rest_call::METHOD_PUT, mediamosa_rest_call::METHOD_OPTIONS)));
    assert(in_array($rest_call[mediamosa_rest_call::STATUS], array(mediamosa_rest_call::STATUS_ACTIVE, mediamosa_rest_call::STATUS_DEPRECATED, mediamosa_rest_call::STATUS_DISABLED)));
    assert(isset($rest_call[mediamosa_rest_call::CLASS_NAME]));
    assert(isset($rest_call[mediamosa_rest_call::MODULE_NAME]));
    assert(isset($rest_call[mediamosa_rest_call::VERSION]));

    // Default values.
    $rest_call += array(
      mediamosa_rest_call::METHOD => mediamosa_rest_call::METHOD_GET,
      mediamosa_rest_call::STATUS => mediamosa_rest_call::STATUS_ACTIVE,
      mediamosa_rest_call::RESPONSE_TYPE => mediamosa_response::RESPONSE_TYPE_XML,
      mediamosa_rest_call::BEHAVE_AS_EXTERNAL => FALSE,
      mediamosa_rest_call::ACCESS => mediamosa_rest_call::ACCESS_DEFAULT,
      mediamosa_rest_call::NO_USER_ABORT => in_array($rest_call[mediamosa_rest_call::METHOD], array(mediamosa_rest_call::METHOD_POST, mediamosa_rest_call::METHOD_DELETE)) || $_SERVER['REQUEST_METHOD'] == mediamosa_rest_call::METHOD_HEAD,
      mediamosa_rest_call::FOR_INTERFACE => array(mediamosa_rest_call::FOR_INTERFACE_APP),
    );

    return $rest_call;
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Get a list of REST calls registered with our drupal hook.
   *
   * @param int $limit
   *  Maximum of items in result.
   * @param int $offset
   *  Offset in items list.
   * @param array $options
   *  - key ('method')
   *    The name of the first array key to use.
   *  - key2 ('uri')
   *    The name of the second array key to use (keep empty to skip).
   *  - get_class_name (FALSE)
   *    Get the class filename stored in the registry for REST class.
   *  - restrict_interface (TRUE)
   *    Test if the interface matches the interface of the REST call. E.g. test
   *    if an upload REST call is called from the upload interface, else skip.
   */
  public static function get_rest_calls($limit = 0, $offset = 0, array $options = array()) {

    $options += array(
      'key' => mediamosa_rest_call::METHOD,
      'key2' => mediamosa_rest_call::URI,
      'get_class_name' => FALSE,
      'restrict_interface' => TRUE,
    );

    // Find all REST calls of enabled modules.
    $rest_calls = module_invoke_all('mediamosa_register_rest_call');
    $rest_calls_doc = module_invoke_all('mediamosa_register_rest_call_doc');
    $rest_calls = array_merge_recursive($rest_calls, $rest_calls_doc);

    // Sort on URI.
    ksort($rest_calls);

    // Start with an empty result.
    $results = array();

    if (!$limit) {
      $limit = -1; // get all.
    }

    if ($options['restrict_interface']) {
      // Restriction for just a few specific calls.
      $current_interface = array();
      if (mediamosa::is_app_app_enabled()) {
        $current_interface[] = mediamosa_rest_call::FOR_INTERFACE_APP;
      }
      if (mediamosa::is_app_upload_enabled()) {
        $current_interface[] = mediamosa_rest_call::FOR_INTERFACE_UPLOAD;
      }
      if (mediamosa::is_app_download_enabled()) {
        $current_interface[] = mediamosa_rest_call::FOR_INTERFACE_DOWNLOAD;
      }
      if (mediamosa::is_app_openapi_enabled()) {
        $current_interface[] = mediamosa_rest_call::FOR_INTERFACE_OPENAPI;
      }
    }

    $pos = 1;
    foreach ($rest_calls as $uri => $rest_call_2) {

      foreach ($rest_call_2 as $method => $rest_call) {
        if ($offset && $offset--) {
          $pos++;
          continue;
        }

        if ($options['restrict_interface']) {
          if (empty($rest_call[mediamosa_rest_call::FOR_INTERFACE])) {
            $rest_call[mediamosa_rest_call::FOR_INTERFACE] = array(
              mediamosa_rest_call::FOR_INTERFACE_APP,
              mediamosa_rest_call::FOR_INTERFACE_UPLOAD,
              mediamosa_rest_call::FOR_INTERFACE_DOWNLOAD,
            );
          }
          $intersec = array_intersect($current_interface, $rest_call[mediamosa_rest_call::FOR_INTERFACE]);
          if (empty($intersec)) {
            $pos++;
            continue;
          }
        }

        // Set method.
        $rest_call[mediamosa_rest_call::METHOD] = $method;

        // Setup default.
        $rest_call = self::set_default_rest_call($rest_call);

        $rest_call[mediamosa_rest_call::URI] = $uri;
        $rest_call[mediamosa_rest_call::ID] = $pos++;

        // Sort on Property.
        ksort($rest_call);

        if ($options['get_class_name']) {
          $rest_call[mediamosa_rest_call::CLASS_FILE_NAME] = mediamosa_db::db_query("SELECT filename FROM {registry} WHERE name = :name AND type = 'class'", array(':name' => $rest_call[mediamosa_rest_call::CLASS_NAME]))->fetchField();
        }

        // Copy into result.
        if (!empty($options['key2'])) {
          $results[$rest_call[$options['key']]][$rest_call[$options['key2']]] = $rest_call;
        }
        else {
          assert(!isset($results[$rest_call[$options['key']]]));
          $results[$rest_call[$options['key']]] = $rest_call;
        }

        // no more?
        if (!$limit) {
          break;
        }

        if ($limit) {
          $limit--;
        }
      }

      if (!$limit) {
        break;
      }
    }

    return $results;
  }

  /**
   * Match the incoming URL with a REST call and return match REST object.
   */
  final public function match_call($uri = NULL, $method = NULL) {
    // Get the mediamosa object.
    $mediamosa = mediamosa::get();

    if (!isset($uri)) {
      $uri = $mediamosa->get_request_uri();
    }

    // Translate the URI to possible REST call.
    $rest_call = $this->get_uri_definition($uri, $method);

    // If FALSE, then the URI didn't match with a REST call.
    if (!$rest_call) {
      if (mediamosa::is_admin_enabled() && mediamosa::in_simpletest_sandbox_enable_app()) {
        return FALSE;
      }

      // Check the allowed paths.
      foreach (module_implements('mediamosa_pass_call_drupal') as $module) {
        $allowed_path = module_invoke($module, 'mediamosa_pass_call_drupal');
        if ($allowed_path) {
          return FALSE;
        }
      }

      // Throw so mediamosa handles the output.
      throw new mediamosa_exception_error(mediamosa_error::HTTP_NOT_FOUND, array(
        '@uri' => print_r($uri, TRUE),
        '@method' => ($method ? print_r($method, TRUE) : '-'),
        '@params' => '-',
      ));
    }

    // Add the matched uri to the response.
    $mediamosa->set_matched_uri($rest_call[mediamosa_rest_call::URI]);

    // Add the method uri to the response.
    $mediamosa->set_matched_method($rest_call[mediamosa_rest_call::METHOD]);

    // Add the found class to the response.
    $mediamosa->set_class($rest_call[mediamosa_rest_call::CLASS_NAME]);

    // Return the processed REST call.
    return $rest_call;
  }

  /**
   * Dispatch the REST request execution.
   *
   * @param array $rest_call
   *   Array containing the data needed to run the REST call.
   */
  final public function process_call(array $rest_call) {
    // Get the mediamosa object.
    $mediamosa = mediamosa::get();

    if (!class_exists($rest_call[mediamosa_rest_call::CLASS_NAME], TRUE)) {
      mediamosa_watchdog::log_export($rest_call);
      mediamosa_watchdog::log_export(arg());
      throw new mediamosa_exception_program_error(strtr('Class #class was not found!', array('#class' => $rest_call[mediamosa_rest_call::CLASS_NAME])));
    }

    // Create class from it.
    $the_rest_call = new $rest_call[mediamosa_rest_call::CLASS_NAME]($rest_call);

    // Check if override for class selection is here.
    if (method_exists($the_rest_call, 'get_object_rest_call')) {
      $the_rest_call = $the_rest_call->get_object_rest_call($rest_call);
    }

    // Process the REST call.
    $response_type = $the_rest_call->process_call();

    // If offset was specified, then include it in the header.
    if ($the_rest_call->isset_given_param(mediamosa_rest_call::OFFSET)) {
      $mediamosa->item_offset = $the_rest_call->get_param_value(mediamosa_rest_call::OFFSET);
    }

    return $response_type;
  }

  /**
   * Returns the var_setup of the REST call.
   *
   * @param array $rest_call
   *  The found REST call with get_uri_definition().
   */
  public static function get_var_setup(array $rest_call) {
    if (!class_exists($rest_call[mediamosa_rest_call::CLASS_NAME], TRUE)) {
      throw new mediamosa_exception_program_error(strtr('Class #class was not found!', array('#class' => $rest_call[mediamosa_rest_call::CLASS_NAME])));
    }

    // Create class from it.
    $the_rest_call = new $rest_call[mediamosa_rest_call::CLASS_NAME]($rest_call);

    // Check if override for class selection is here.
    if (method_exists($the_rest_call, 'get_object_rest_call')) {
      $the_rest_call = $the_rest_call->get_object_rest_call($rest_call);
    }

    // Turn off so we dont fail.
    $the_rest_call->set_check_for_unspecified(FALSE);

    // Set as uri values, so we dont fail checks.
    if (isset($rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES])) {
      foreach ($rest_call[mediamosa_rest_call::DEFAULT_PARAMS_VALUES] as $param => $value) {
        $the_rest_call->{mediamosa_rest_call::URI_PARAMS}[$param] = $value;
      }
    }

    // Return the var setup.
    return $the_rest_call->get_var_setup();
  }
}
