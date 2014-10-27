<?php
/**
 * @file
 * MediaMosa Exception classes.
 */

/**
 * The basic mediamosa_exception class
 */
class mediamosa_exception extends Exception {

  /**
   * Our exception severity levels.
   *
   * Level is used how critical to log the message (if logging is enabled).
   */
  const MEDIAMOSA_EXCEPTION_SEVERITY_HIGH = 80;
  const MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL = 50;
  const MEDIAMOSA_EXCEPTION_SEVERITY_LOW = 20;

  /**
   * Constructor.
   *
   * @param int $errorcode
   *   The error code.
   * @param string $message
   *   (optional) The error message.
   * @param int $severity
   *   The severity of the message; The higher the serverity, the critical the
   *   type in log.
   * @param bool $log_error
   *   Master switch if exception should be logged. TRUE is log exception.
   */
  public function __construct($errorcode, $message = '', $severity = self::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    $message = empty($message) ? sprintf('MediaMosa exception was thrown: %d', $errorcode) : $message;

    if ($log_error && ($severity >= self::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL || mediamosa_debug::is_debug())) {
      $log_serverity = $severity >= self::MEDIAMOSA_EXCEPTION_SEVERITY_HIGH ? WATCHDOG_CRITICAL : WATCHDOG_NOTICE;

      $levels = watchdog_severity_levels();

      // Get backtrace when you can.
      $backtrace = $this->getTraceAsString();

      mediamosa_watchdog::log(
        '@message (Exception thrown (severity: @severity) on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
        array(
          '@severity' => $levels[$log_serverity],
          '@message' => $message,
          '@line' => $this->getLine(),
          '@file' => $this->getFile(),
          '@backtrace' => empty($backtrace) ? 'not available' : $backtrace,
        ),
        $log_serverity
      );
    }

    parent::__construct($message, (int) $errorcode, null);
  }

  public function mediamosa_exception_rest_response() {
    mediamosa::get()->set_result(mediamosa_response::ERROR, $this->getCode(), $this->getMessage());
  }

  /**
   * Test the value $test and fail when not true.
   *
   * @param mixed $test
   *   Any value that needs to be tested.
   * @param string $message
   *   (optional) The message to show.
   *
   * @throws RtmException
   *   When test fails.
   */
  public static function assert($test, $message = '') {

    if (!$test) {
      $caller = _drupal_get_last_caller(debug_backtrace());

      throw new mediamosa_exception_program_error(
        '{message}Assertion failed in {function} {file} ({line}).',
        array(
          '{function}' => $caller['function'],
          '{line}' => $caller['line'],
          '{file}' => $caller['file'],
          '{message}' => empty($message) ? '' : ($message . ', '),
        )
      );
    }
  }
}

/**
 * Throw for override of output. Will log error and redirect to URI/URL and
 * will exit() at end. Will not generate output. The redirect uri will be
 * extended with the GET values 'status_code' and 'status_message', both filled
 * from the exception code and message.
 */
class mediamosa_exception_redirect_and_exit extends mediamosa_exception {
  /**
   * The redirect URL that is used when exception is catched.
   *
   * @var string
   */
  protected $redirectUri;

  /**
   * Constructor.
   *
   * @param string $redirect_uri
   *   Redirect URI used when exception is caught.
   * @param int $errorcode
   *   The exception error code, should match the mediamosa_error code list.
   * @param string $message
   *   The exception message.
   * @param int $severity
   *   The severity of the message; The higher the serverity, the critical the
   *   type in log.
   */
  public function __construct($redirect_uri, $errorcode, $message = '', $severity = self::MEDIAMOSA_EXCEPTION_SEVERITY_LOW) {
    $this->redirectUri = $redirect_uri;

    parent::__construct($errorcode, $message, $severity);
  }

  /**
   * Return redirect url.
   *
   * Will add the status_code and status_message.
   */
  public function get_redirect_uri() {
    $params = array(
      'status_code' => $this->getCode(),
      'status_message' => $this->getCode() == mediamosa_sdk::ERRORCODE_OKAY ? '' : $this->getMessage(),
    );

    return mediamosa_http::glue_params($this->redirectUri, $params);
  }
}

/**
 * Base class exception for errors.
 *
 * The base class must be used in case the exception is mediamosa error with
 * original error code and message.
 */
class mediamosa_exception_error extends mediamosa_exception {
  /**
   * An associative array, set during constructor.
   * - 'code'
   *   The mediamosa error code.
   * - 'message'
   *   The processed message.
   *
   * @var array
   */
  protected $error;

  /**
   * Constructor.
   *
   * @param int $mediamosa_errorcode
   *   The mediamosa error code.
   * @param $args
   *   An associative array of replacements to make. Occurrences in $string of
   *   any key in $args are replaced with the corresponding value
   * @param int $severity
   *   The severity of the message; The higher the serverity, the critical the
   *   type in log.
   * @param bool $log_error
   *   Master switch if exception should be logged. TRUE is log exception.
   */
  public function __construct($mediamosa_errorcode, $args = array(), $severity = self::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    $this->error = mediamosa_error::error_code_find($mediamosa_errorcode, $args);
    parent::__construct($this->error['code'], $this->error['message'], $severity, $log_error);
  }

  /**
   * Return the constructed error array.
   *
   * @return array
   *   An associative array;
   *   - 'message'
   *     The error code message.
   *   - 'code'
   *     The mediamosa error code.
   */
  public function mediamosa_exception_error_array_get() {
    return $this->error;
  }
}

/**
 * Wrapper class.
 */
class mediamosa_exception_error_debug extends mediamosa_exception_error {
  public function __construct($mediamosa_errorcode, $args = array(), $severity = self::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL) {
    parent::__construct($mediamosa_errorcode, $args, $severity, mediamosa_debug::is_debug());
  }
}

/**
 * Exceptions extended on mediamosa_exception.
 */
class mediamosa_exception_error_unexpected_error extends mediamosa_exception_program_error {
  public function __construct() {
    parent::__construct('Unexpected error!');
  }
}

class mediamosa_exception_program_error extends mediamosa_exception {
  public function __construct($message, $args = array()) {
    $message = strtr($message, $args);
    parent::__construct(mediamosa_error::HTTP_INTERNAL_SERVER_ERROR, $message, self::MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
  }
}

/**
 * Thrown when order by parameter is not allowed
 */
class mediamosa_exception_error_sort_field_error extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@field"]));
    parent::__construct(mediamosa_error::ERRORCODE_SORT_FIELD_ERROR, $args);
  }
}

class mediamosa_exception_error_invalid_timestamp extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@name"]));
    parent::__construct(mediamosa_error::ERRORCODE_INVALID_TIMESTAMP, $args);
  }
}

class mediamosa_exception_error_missing_timestamp extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@name"]));
    parent::__construct(mediamosa_error::ERRORCODE_MISSING_TIMESTAMP, $args);
  }
}

// When access has been denied
class mediamosa_exception_error_access_denied extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_NOT_AUTHORIZED, $args);
  }
}

// When asset was unappropriate.
class mediamosa_exception_error_is_inappropriate extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_IS_UNAPPROPRIATE, $args);
  }
}

// When ega authentaction failed
class mediamosa_exception_error_authentication_failure extends mediamosa_exception_error {
  public function __construct($args = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_ACCESS_DENIED, $args);
  }
}

class mediamosa_exception_error_empty_result extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_EMPTY_RESULT, $args);
  }
}

class mediamosa_exception_error_acl_group_already_exists extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@group"]));
    parent::__construct(mediamosa_error::ERRORCODE_ACL_GROUP_ALREADY_EXISTS, $args);
  }
}

class mediamosa_exception_error_acl_group_not_found extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@group"]));
    parent::__construct(mediamosa_error::ERRORCODE_ACL_GROUP_NOT_FOUND, $args);
  }
}

class mediamosa_exception_error_acl_name_not_found extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@hostname"]));
    parent::__construct(mediamosa_error::ERRORCODE_ACL_NAME_NOT_FOUND, $args);
  }
}

class mediamosa_exception_error_group_not_empty extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@group"]));
    parent::__construct(mediamosa_error::ERRORCODE_GROUP_NOT_EMPTY, $args);
  }
}

class mediamosa_exception_error_invalid_data_parameter extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args["@parameter"]));
    assert(isset($args["@data"]));
    parent::__construct(mediamosa_error::ERRORCODE_INVALID_DATA_PARAMETER, $args);
  }
}

class mediamosa_exception_error_job_could_not_be_removed extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args['@job_id']));
    parent::__construct(mediamosa_error::ERRORCODE_JOB_COULD_NOT_BE_REMOVED, $args, self::MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
  }
}

class mediamosa_exception_error_job_not_found extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args['@job_id']));
    parent::__construct(mediamosa_error::ERRORCODE_JOB_NOT_FOUND, $args);
  }
}

class mediamosa_exception_error_asset_not_found extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    assert(isset($args['@asset_id']));
    parent::__construct(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, $args);
  }
}

class mediamosa_exception_error_mediafile_not_found extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    assert(isset($args['@mediafile_id']));
    parent::__construct(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, $args);
  }
}

class mediamosa_exception_error_invalid_ticket extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    assert(isset($args['@ticket_id']));
    parent::__construct(mediamosa_error::ERRORCODE_INVALID_TICKET, $args);
  }
}

class mediamosa_exception_error_userman_invalid_user extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    assert(isset($args['@user_id']));
    parent::__construct(mediamosa_error::ERRORCODE_USERMAN_INVALID_USER, $args);
  }
}

class mediamosa_exception_error_cant_retype_group_must_be_empty extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args['@group']));
    parent::__construct(mediamosa_error::ERRORCODE_CANT_RETYPE_GROUP_MUST_BE_EMPTY, $args);
  }
}

class mediamosa_exception_error_query_error extends mediamosa_exception_error {
  public function __construct($args = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_QUERY_ERROR, $args, self::MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
  }
}

class mediamosa_exception_error_job_mediafile_not_found extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args['@mediafile_id']));
    parent::__construct(mediamosa_error::ERRORCODE_JOB_MEDIAFILE_NOT_FOUND, $args);
  }
}

class mediamosa_exception_error_still_is_not_creatable extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args['@mediafile_id']));
    parent::__construct(mediamosa_error::ERRORCODE_STILL_IS_NOT_CREATABLE, $args);
  }
}

// In case stream wrapper can not be located.
class mediamosa_exception_error_streamwrapper_not_found extends mediamosa_exception_error {
  public function __construct($args = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_STORAGE_STREAMWRAPPER_NOT_FOUND, $args, self::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, FALSE);
  }
}

class mediamosa_exception_error_file_not_found extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args['@filename']));
    parent::__construct(mediamosa_error::ERRORCODE_FILE_NOT_FOUND, $args);
  }
}

class mediamosa_exception_error_webservice_disabled extends mediamosa_exception_error {
  public function __construct($args = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_WEBSERVICE_DISABLED, $args);
  }
}

class mediamosa_exception_error_rest_call_is_disabled extends mediamosa_exception_error {
  public function __construct($args = array()) {
    assert(isset($args['@uri']));
    parent::__construct(mediamosa_error::ERRORCODE_REST_CALL_IS_DISABLED, $args);
  }
}
// Throw when CQL is combined with normal search/order by parameters
class mediamosa_exception_error_cql_exclusive extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::ERRORCODE_CQL_EXCLUSIVE);
  }
}

// Throw when CQL is combined with normal search/order by parameters
class mediamosa_exception_error_cql_error extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    assert(isset($args['@error']));
    parent::__construct(mediamosa_error::ERRORCODE_CQL_ERROR, $args);
  }
}

// Throw when master/slave access is disabled for this app.
class mediamosa_exception_error_masterslave_disallowed extends mediamosa_exception_error_debug {
  public function __construct($args = array()) {
    assert(isset($args['@acl_id']));
    parent::__construct(mediamosa_error::ERRORCODE_MASTERSLAVE_DISALLOWED, $args);
  }
}

// Bad request.
class mediamosa_exception_error_400 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_BAD_REQUEST);
  }
}

// Forbidden
class mediamosa_exception_error_403 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_FORBIDDEN);
  }
}

// Not found.
class mediamosa_exception_error_404 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_NOT_FOUND);
  }
}

/**
 * Thrown when error is fatal or can not be handled.
 */
class mediamosa_exception_error_500 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_INTERNAL_SERVER_ERROR);
  }
}

/**
 * THrown when trying to access operation of function that is not implemented.
 */
class mediamosa_exception_error_501 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_NOT_IMPLEMENTED);
  }
}
