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
  * The REST call object provides the REST call function data and excecution.
  */
abstract class mediamosa_rest_call {
  // ------------------------------------------------------------------ Members.
  // Original REST call arguments.
  public $a_rest_args = array();

  // Check if there are unspecified arguments and throw exception when we have unspecified arguments.
  protected $check_for_unspecified = FALSE; // Keep on FALSE so get_var_setup() will not fail if it calls process_vars()

  // default parameter description.
  protected $mediamosa_parameter_docs = array(
    'asset_id' => 'Unique asset id',
    'coll_id' => 'Unique collection id',
    'user_id' => 'User id',
    'group_id' => 'Group id',
    'isprivate' => '',
  );

  // Setup default app.
  private $calling_app = array(mediamosa_app_db::APP_ID => 0, mediamosa_app_db::TIMEZONE => mediamosa_settings::MEDIAMOSA_DEFAULT_TIMEZONE_INTERNAL);

  // ------------------------------------------------------------------ Constants.
  const ID = 'uri_id';
  const URI = 'uri';
  const URI_PARAMS = 'uri_params';
  const URI_REQUEST = 'uri_request'; // Original URI.

  const RESPONSE_TYPE = 'response_type';
  const RESPONSE_TYPE_XML = 'text/xml';
  const RESPONSE_TYPE_PLAIN = 'text/plain';

  // Download files.
  const RESPONSE_TYPE_DOWNLOAD = 'application/force-download';

  // Download stills.
  const RESPONSE_TYPE_STILL = 'image/jpeg';

  //const RESPONSE_TYPE_MEDIAMOSA = 'application/mediamosa'; // future.

  const METHOD = 'method';
  const METHOD_GET = 'GET';
  const METHOD_POST = 'POST';
  const METHOD_PUT = 'PUT';
  const METHOD_DELETE = 'DELETE';
  // Do not use for defining the method of your REST call, use GET. Incoming
  // HEAD requests are translated to GET and will match any specified GET rest
  // call.
  const METHOD_HEAD = 'HEAD';

  const CLASS_NAME = 'class_name';
  const CLASS_FILE_NAME = 'class_file_name';

  const MODULE_NAME = 'module_name';

  const STATUS = 'status';
  const STATUS_ACTIVE = 'active';
  const STATUS_DEPRECATED = 'deprecated';
  const STATUS_DISABLED = 'disabled';

  const VERSION = 'version';
  const VERSION_MAX_LEN = 12;

  // Set these so we have default values when we need them for calling get_var_setup();
  const DEFAULT_PARAMS_VALUES = 'default_params_values';

  // Access to the REST call.
  const ACCESS = 'access';      // Value OR'd;
  const ACCESS_INTERNAL = 1;    // Access to REST call when internally accessed.
  const ACCESS_INTERNAL_ONLY = 1;    // Access to REST call internally only..
  const ACCESS_EXTERNAL = 2;    // Access to REST call when externally accessed.
  const ACCESS_EXTERNAL_ONLY = 2;    // Access to REST call externally only.
  const ACCESS_AUTHENTICATED = 4; // Access to REST call when authenticated (true came back in the auth hook).
  const ACCESS_AUTHENTICATED_ONLY = 4; // Access to REST call when authenticated only.
  const ACCESS_DEFAULT = 5;     // Access default allowes internals and authenticated access.
                                // External access needs to be specified.
  const ACCESS_FOR_EXTERNAL = 7; // Default for external calls (normal/internal/external).

  const TITLE = 'title';             // Title of the REST call (In English, required).
  const DESCRIPTION = 'description'; // Description of the REST call (In English, required).
  const EXAMPLE_REQUEST = 'example_request';   // Example request url.
  const EXAMPLE_RESPONSE = 'example_response'; // Example response output.
  const RESPONSE_FIELDS = 'response_fields';   // Response fields in output.

  const BEHAVE_AS_EXTERNAL = 'behave_as_external'; // Set to true if you want this rest call to behave like external without /external.

  // Specify to TRUE will make sure the REST can not be canceled by user and
  // will still continue to execute even when connection to client has been
  // lost. This will always be true for POST rest calls.
  const NO_USER_ABORT = 'no_user_abort';

  /**
   * 'alias_for' contains the URI of the REST call that also runs on this rest URI.
   * Any settings in this array is instead used of the original.
   */
  const ALIAS_FOR = 'alias_for';

  // Default var names / values.
  const APP_ID = 'app_id';
  const LIMIT = 'limit';
  const OFFSET = 'offset';
  const ORDER_BY = 'order_by';
  const ORDER_DIRECTION = 'order_direction';
  const IS_APP_ADMIN = 'is_app_admin';

  // VAR array names.
  const VARS = 'vars';
  const VARS_DESCRIPTION = 'description';

  const VAR_TYPE = 'type';
  const VAR_DEFAULT_VALUE = 'default_value';
  const VAR_RANGE_START = 'range_start';
  const VAR_RANGE_END = 'range_end';
  const VAR_IS_ARRAY = 'is_array';         // default no.
  const VAR_IS_REQUIRED = 'is_required';   // default no.
  const VAR_TRIM_VALUE = 'trim_value';     // default no.
  const VAR_DESCRIPTION = 'description';   // Description of the var (In English).
  const VAR_ALLOWED_VALUES = 'a_allowed_values'; // Optional array for checking against allowed values.
  const VAR_IS_READ_ONLY = 'read_only'; // default no. Some vars may not be changed.
  const VAR_IS_INTERNAL_ONLY = 'internal_only'; // If true, then this variable is only accepted by internals.
  const VAR_OVERRIDE_VALUE = 'override_value'; // Set this to override any value given from URI/GET/POST.

  const VAR_IS_HIDDEN = 'hidden'; // Hide var for outside world.
  const VAR_IS_HIDDEN_YES = 'yes';
  const VAR_IS_HIDDEN_NO = 'no';

  // Values VAR_IS_REQUIRED
  const VAR_IS_REQUIRED_YES = 'YES';
  const VAR_IS_REQUIRED_NO = 'NO';

  // Values VAR_IS_ARRAY
  const VAR_IS_ARRAY_YES = 'YES';        // Make it an array, if not.
  const VAR_IS_ARRAY_NO = 'NO';          // May not be array, if not, take 1st element.

  // Values TRIM_VALUE
  const VAR_TRIM_VALUE_YES = 'YES';
  const VAR_TRIM_VALUE_NO = 'NO';

  // Values VAR_IS_READ_ONLY
  const VAR_IS_READ_ONLY_YES = 'YES';
  const VAR_IS_READ_ONLY_NO = 'NO';

  // Values VAR_IS_INTERNAL_ONLY
  const VAR_IS_INTERNAL_ONLY_YES = 'YES';
  const VAR_IS_INTERNAL_ONLY_NO = 'NO';

  // Param fields
  const PARAM_VALUE = 'value';
  const PARAM_VALUE_ORG = 'value_org';
  const PARAM_TYPE = 'type';
  const PARAM_ISSET_GIVEN = 'isgiven'; // Value was set by GET/POST/URI
  const PARAM_ISSET_DEFAULT = 'isset_default'; // Value isset, but was set with default value.
  const PARAM_VALIDATED = 'validated';
  const PARAM_RANGE_START = 'range_start';
  const PARAM_RANGE_END = 'range_end';
  const PARAM_IS_READ_ONLY = 'read_only';
  const PARAM_IS_REQUIRED = 'is_required';
  const PARAM_ALLOWED_VALUES = 'allowed_values';

  /**
   * Main constructor.
   *
   * @param rest_call $o_rest_call
   */
  function __construct($rest_call) {
    // Copy us.
    $this->{self::URI} = $rest_call[self::URI];
    $this->{self::URI_REQUEST} = explode('/', (isset($rest_call[self::URI_REQUEST]) ? $rest_call[self::URI_REQUEST] : $rest_call[self::URI]));
    $this->{self::RESPONSE_TYPE} = $rest_call[self::RESPONSE_TYPE];
    $this->{self::METHOD} = $rest_call[self::METHOD];
    $this->{self::CLASS_NAME} = $rest_call[self::CLASS_NAME];
    $this->{self::STATUS} = $rest_call[self::STATUS];
    $this->{self::VERSION} = $rest_call[self::VERSION];
    $this->{self::BEHAVE_AS_EXTERNAL} = $rest_call[self::BEHAVE_AS_EXTERNAL];

    $this->{self::TITLE} = isset($rest_call[self::TITLE]) ? $rest_call[self::TITLE] : '';
    $this->{self::DESCRIPTION} = isset($rest_call[self::DESCRIPTION]) ? $rest_call[self::DESCRIPTION] : '';
    $this->{self::EXAMPLE_REQUEST} = isset($rest_call[self::EXAMPLE_REQUEST]) ? $rest_call[self::EXAMPLE_REQUEST] : '';
    $this->{self::EXAMPLE_RESPONSE} = isset($rest_call[self::EXAMPLE_RESPONSE]) ? $rest_call[self::EXAMPLE_RESPONSE] : '';
    $this->{self::RESPONSE_FIELDS} = isset($rest_call[self::RESPONSE_FIELDS]) ? $rest_call[self::RESPONSE_FIELDS] : '';

    // Access.
    $this->{self::ACCESS} = $rest_call[self::ACCESS];

    // Other.
    $this->{self::URI_PARAMS} = isset($rest_call[self::URI_PARAMS]) ? $rest_call[self::URI_PARAMS] : array();
    $this->{self::DEFAULT_PARAMS_VALUES} = isset($rest_call[self::DEFAULT_PARAMS_VALUES]) ? $rest_call[self::DEFAULT_PARAMS_VALUES] : array();
  }

  // ----------------------------------------------------- Functions (abstract).

  // Get the rest input info.
  abstract function get_var_setup();
  // The execution of the call.
  abstract function do_call();

  // ------------------------------------------------------- Functions (public).
  /**
   * Dispatch the rest call processing.
   */
  public function process_call() {
    // Make sure its not disabled.
    if ($this->{self::STATUS} == self::STATUS_DISABLED) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_CALL_IS_DISABLED, array('@uri' => $this->{self::URI}));
    }

    // Check if we are internal and allowed.
    if ($this->is_internal()) {
      if (!($this->{self::ACCESS} & self::ACCESS_INTERNAL)) {
        throw new mediamosa_exception_error_authentication_failure();
      }

      // The name of the http var.
      $internal_pass = 'HTTP_' . mediamosa_unicode::strtoupper(str_replace('-', '_', mediamosa_settings::X_MEDIAMOSA_INTERNAL_PASS));
      // Match password in HTTP header, must match.
      if (!isset($_SERVER[$internal_pass]) || $_SERVER[$internal_pass] != mediamosa::get_internal_password()) {
        mediamosa_watchdog::log('Unmatching internal password (@pass) for REST URL: @url', array('@url' => $this->{self::URI}, '@pass' => $_SERVER[$internal_pass]), WATCHDOG_WARNING);
        throw new mediamosa_exception_error_authentication_failure();
      }
    }
    // Check if we are external and allowed.
    if ($this->is_external() && !($this->{self::ACCESS} & self::ACCESS_EXTERNAL)) {
      throw new mediamosa_exception_error_authentication_failure();
    }

    // Do login / authorization, unless the call is an external.
    $this->do_app_authorized();

    // If we are internal and no app_id is set, we have access to all.
    // Just to be sure the first app_id is 0, so single app_id code will return empty.
    if ($this->is_internal() && !isset($_GET['app_id'])) {
      $_GET['app_id'] = array(0) + array(mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID), mediamosa_app_db::APP_ID));
    }

    // Get the var setup.
    $var_setup = $this->get_var_setup();

    // Assert when failed.
    assert($var_setup);

    // @todo: make this an setting per app or else we are not 1.x compatible. Default should be FALSE.
    $this->check_for_unspecified = FALSE;// turned off for now

    // Check rest args.
    $var_setup = $this->process_rest_args($var_setup);
    assert($var_setup);

    // Validate reg args.
    $this->validate_rest_args($var_setup);

    // Setup the calling application.
    $this->setup_calling_app($var_setup);

    // Set the rest call timezone.
    mediamosa::set_restcall_timezone($this->calling_app[mediamosa_app_db::TIMEZONE]);

    // Convert input datetime to UTC based on the app timezone.
    $this->appdate2utc_rest_args($var_setup);

    // Call the function.
    $this->do_call();
  }

  /**
   * Do authorize app.
   */
  private function do_app_authorized() {
    // Run the app_authorized hook. One TRUE is authorized.
    $a_results = module_invoke_all('mediamosa_app_authorized');
    $authorized = FALSE;
    foreach ($a_results as $result) {
      if ($result === TRUE) {
        $authorized = TRUE;
        break;
      }
    }

    // If we are internal or external, we dont care.
    if ($this->is_internal() || $this->is_external()) {
      return;
    }

    if (!$authorized) {
      throw new mediamosa_exception_error_authentication_failure();
    }
  }

  /**
   * Get the default vars we always expect in every rest call
   */
  protected function get_var_setup_default(array $var_setup, $app_id_required = TRUE) {
    $var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::APP_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
      self::VAR_IS_REQUIRED => ($app_id_required ? ($this->is_internal() ? self::VAR_IS_REQUIRED_NO : self::VAR_IS_REQUIRED_YES) : self::VAR_IS_REQUIRED_NO),
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DEFAULT_VALUE => 0,
      self::VAR_DESCRIPTION => mediamosa::t('The application ID.'),
    );

    // @todo: remove when all calls are scanned, see next function.
    $var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::IS_APP_ADMIN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t('NOT USED.'),
      self::VAR_DEFAULT_VALUE => 'false',
    );

    // Add default parameter descriptions
    foreach ($var_setup[mediamosa_rest_call::VARS] as $key => $value) {
      if (!isset($value[self::VAR_DESCRIPTION])) {
        if (isset($this->mediamosa_parameter_docs[$key])) {
          $var_setup[mediamosa_rest_call::VARS][$key][self::VAR_DESCRIPTION] = $this->mediamosa_parameter_docs[$key];
        }
      }
    }

    return $var_setup;
  }

  /**
   * Get the default vars we always expect in every rest call
   * plus is_app_admin
   */
  protected function get_var_setup_app_admin(array $var_setup, $app_id_required = TRUE) {
    $var_setup = $this->get_var_setup_default($var_setup, $app_id_required);

    // add is_app_admin.
    $var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::IS_APP_ADMIN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t('A boolean to indicate the call is done by the APP admin. It overrides user autorisation.'),
      self::VAR_DEFAULT_VALUE => 'false',
    );

    return $var_setup;
  }

  /**
   * Enrich the VAR setup with the default LIMIT and OFFSET
   *
   * @param array $var_setup
   *   Provide current array to merge.
   */
  protected function get_var_setup_range(array $var_setup = array(), $limit_max = mediamosa_settings::LIMIT_MAX, $limit_default = mediamosa_settings::LIMIT_DEFAULT) {

    $var_setup[self::VARS][self::LIMIT] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_LIMIT,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => $limit_default,
      self::VAR_IS_READ_ONLY => self::VAR_IS_READ_ONLY_YES, // Once set, dont change.
      self::VAR_DESCRIPTION => mediamosa::t('Limit of the items of the result set.'),
      self::VAR_RANGE_START => 0,
      self::VAR_RANGE_END => $limit_max,
    );

    $var_setup[self::VARS][self::OFFSET] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 0,
      self::VAR_IS_READ_ONLY => self::VAR_IS_READ_ONLY_YES, // Once set, dont change.
      self::VAR_DESCRIPTION => mediamosa::t('The starting items position in the result set.'),
    );

    return $var_setup;
  }

  /**
   * Gives true back if call is internal call.
   */
  protected function is_internal() {
    return count($this->{self::URI_REQUEST}) && $this->{self::URI_REQUEST}[0] == 'internal';
  }

  /**
   * Gives true back if call is internal call.
   */
  protected function is_external() {
    return ((count($this->{self::URI_REQUEST}) && $this->{self::URI_REQUEST}[0] == 'external') || $this->{self::BEHAVE_AS_EXTERNAL});
  }

  /**
   * Return the active version to use for APP.
   *
   * Returns NULL for latest version or array.
   *
   * @see mediamosa_version()
   *
   * @param integer $app_id
   */
  protected function app_active_version($app_id) {
    $app = mediamosa_app::get_by_appid($app_id);

    // Is at latest version?
    if ($app[mediamosa_app_db::ACTIVE_VERSION] == mediamosa_version::LATEST || empty($app[mediamosa_app_db::ACTIVE_VERSION])) {
      return FALSE;
    }

    // Return in array.
    return mediamosa_version::get_version($app[mediamosa_app_db::ACTIVE_VERSION]);
  }

  /**
   * Enrich the VAR setup with the default ORDER BY, ORDER_DIRECTION
   *
   * @param array $var_setup
   *   Provide current array to merge.
   * @param string $default_order_by
   *  Default field to order on.
   * @param array $a_allowed_values_order_by
   *  array containing allowed values.
   * @param string $default_order_direction
   *  ASC (default) / DESC direction.
   * @return array
   *  Returns the altered $var_setup.
   */
  protected function get_var_setup_order_by(array $var_setup = array(), $default_order_by = '', array $allowed_values_order_by = array(), $default_order_direction = mediamosa_type::ORDER_DIRECTION_ASC) {

    // Either must be set.
    assert(in_array($default_order_direction, array(mediamosa_type::ORDER_DIRECTION_ASC, mediamosa_type::ORDER_DIRECTION_DESC)));

    $var_setup[self::VARS][self::ORDER_BY] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM_UNDERSCORE,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_DEFAULT_VALUE => $default_order_by,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t('On what column the result set should be ordered.'),
    );

    // Add allowed values for order by.
    if (count($allowed_values_order_by)) {
      $var_setup[self::VARS][self::ORDER_BY][self::VAR_ALLOWED_VALUES] = $allowed_values_order_by;
    }

    $var_setup[self::VARS][self::ORDER_DIRECTION] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ORDER_DIRECTION,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_DEFAULT_VALUE => $default_order_direction,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t('The direction of the result set.'),
    );

    return $var_setup;
  }

  /**
   * Turn on or off the check for unspecified.
   * @param $bool
   */
  public function set_check_for_unspecified($bool) {
    $this->check_for_unspecified = $bool;
  }

  /**
   * Retrieve the value of a supplied parameter.
   *
   * @param string $param
   * @param bool $translate_bool
   *  Translate the bool to string variant.
   * @param array $a_translate_bool
   *  Array containing mapping to TRUE / FALSE string values.
   *  default: array(FALSE => 'FALSE', TRUE => 'TRUE')
   * @return mixed
   */
  public function get_param_value($param, $translate_bool = FALSE, array $a_translate_bool = array(FALSE => 'FALSE', TRUE => 'TRUE')) {
    if (!isset($this->a_rest_args[$param])) {
      throw new mediamosa_exception_program_error('Calling get_param with non-existing param (@param), should not happen; define it in allowed vars for your rest call class "@restcall"', array('@param' => $param, '@restcall' => $this->{self::CLASS_NAME}));
    }

    if (!$this->a_rest_args[$param][self::PARAM_VALIDATED]) {
      // Save the original value.
      $this->a_rest_args[$param][self::PARAM_VALUE_ORG] = (empty($this->a_rest_args[$param][self::PARAM_VALUE]) ? NULL : $this->a_rest_args[$param][self::PARAM_VALUE]);

      $a_param = $this->a_rest_args[$param];
      $this->a_rest_args[$param][self::PARAM_VALUE] = mediamosa_type::check($param, $a_param[self::PARAM_TYPE], $a_param[self::PARAM_VALUE], $a_param[self::PARAM_IS_REQUIRED] == self::VAR_IS_REQUIRED_YES, isset($a_param[self::PARAM_RANGE_START]) ? $a_param[self::PARAM_RANGE_START] : NULL, isset($a_param[self::PARAM_RANGE_END]) ? $a_param[self::PARAM_RANGE_END] : NULL, isset($a_param[self::PARAM_ALLOWED_VALUES]) && count($a_param[self::PARAM_ALLOWED_VALUES]) ? $a_param[self::PARAM_ALLOWED_VALUES] : NULL);
      $this->a_rest_args[$param][self::PARAM_VALIDATED] = TRUE;
    }

    return ($translate_bool && is_bool($this->a_rest_args[$param][self::PARAM_VALUE])) ? $a_translate_bool[$this->a_rest_args[$param][self::PARAM_VALUE]] : $this->a_rest_args[$param][self::PARAM_VALUE];
  }

  /**
   * Retrieve the original value of a supplied parameter.
   *
   * @param string $param
   *
   * @return mixed
   */
  public function get_param_value_org($param) {
    if (!isset($this->a_rest_args[$param])) {
      throw new mediamosa_exception_program_error('Calling get_param with non-existing param (@param), should not happen; define it in allowed vars for your rest call class "@restcall"', array('@param' => $param, '@restcall' => $this->{self::CLASS_NAME}));
    }

    return $this->a_rest_args[$param][self::PARAM_VALUE_ORG];
  }

  /**
   * Set the value of the supplied parameter.
   * Use only internally, expected to be correct value, not value checked.
   *
   * @param string $param
   * @param mixed $value
   */
  public function set_param_value($param, $value) {
    if (isset($this->a_rest_args[$param]) && $this->a_rest_args[$param][self::PARAM_IS_READ_ONLY] == self::VAR_IS_READ_ONLY_YES) {
      throw new mediamosa_exception_program_error('Do not set param @param, its read only.', array('@param' => $param));
    }

    // Set value.
    $this->a_rest_args[$param][self::PARAM_VALUE] = $value;

    // Expected to ok, we cant validate anyway.
    $this->a_rest_args[$param][self::PARAM_VALIDATED] = TRUE;

    // Ok not set by default or URI/POST/GET.
    $this->a_rest_args[$param][self::PARAM_ISSET_DEFAULT] = FALSE;
    $this->a_rest_args[$param][self::PARAM_ISSET_GIVEN] = FALSE;
  }

  /**
   * Unset param from var listing.
   *
   * @param string $param
   */
  public function unset_param($param) {
    $this->set_param_value($param, NULL);
  }

  /**
   * Will return TRUE if the param is set (by any means set)
   *
   * @param string $param
   * @return boolean
   */
  public function isset_param($param) {
    return isset($this->a_rest_args[$param]) && isset($this->a_rest_args[$param][self::PARAM_VALUE]);
  }

  /**
   * Will return TRUE if the param was set by POST/GET/URI.
   * If the param is set any other way (like default value or set_param), it
   * will return FALSE.
   *
   * @param string $param
   * @return boolean
   */
  public function isset_given_param($param) {
    return isset($this->a_rest_args[$param]) && $this->a_rest_args[$param][self::PARAM_ISSET_GIVEN];
  }

  /**
   * Will return TRUE if the param was set by default and not by input.
   *
   * @param string $param
   * @return boolean
   */
  public function isset_default_param($param) {
    return isset($this->a_rest_args[$param]) && $this->a_rest_args[$param][self::PARAM_ISSET_DEFAULT];
  }

  /**
   * Will return TRUE if the param is empty.
   *
   * @param string $param
   * @return boolean
   */
  public function empty_param($param) {
    if (!self::isset_param($param)) {
      return TRUE; // Not set, is empty.
    }

    if (is_string($this->a_rest_args[$param][self::PARAM_VALUE])) {
      return trim($this->a_rest_args[$param][self::PARAM_VALUE]) == '';
    }

    if (is_int($this->a_rest_args[$param][self::PARAM_VALUE])) {
      return $this->a_rest_args[$param][self::PARAM_VALUE] == 0;
    }

    // Default empty behavour.
    return empty($this->a_rest_args[$param][self::PARAM_VALUE]);
  }

  /**
   * Returns the app_ids param value (always array).
   */
  public function get_param_value_app() {
    return $this->get_param_value(self::APP_ID);
  }

  /**
   * Returns the offset param value.
   */
  public function get_param_value_offset() {
    return $this->get_param_value(self::OFFSET);
  }

  /**
   * Returns the offset param value.
   */
  public function get_param_value_limit() {
    return $this->get_param_value(self::LIMIT);
  }

  /**
   * Returns the order_by param value.
   */
  public function get_param_value_order_by() {
    return $this->get_param_value(self::ORDER_BY);
  }

  /**
   * Returns the order_direction param value.
   */
  public function get_param_value_order_direction() {
    return $this->get_param_value(self::ORDER_DIRECTION);
  }

  /**
   * Return from $_GET / $_POST the value, if found.
   * Use this function if you need to go before the REST call process_rest_args.
   *
   * @param string $param
   */
  static public function get_param_value_global($param) {
    // Is mediamosa_params is set, we take priority on those.
    if (isset($GLOBALS['mediamosa_params'])) {
      return isset($GLOBALS['mediamosa_params'][$param]) ? $GLOBALS['mediamosa_params'][$param] : NULL;
    }

    return isset($_GET[$param]) ? $_GET[$param] : (isset($_POST[$param]) ? $_POST[$param] : NULL);
  }

  /**
   * Return the method used on this REST call.
   * @NOTE: This means you can not hack the METHOD here, it depends on
   *        the assumsion the HTTP_METHOD does match the method specified
   *        in the REST description array.
   */
  public function get_method() {
    return $this->{self::METHOD};
  }

  /**
   * Returns TRUE / FALSE if method used is POST.
   */
  public function is_method_post() {
    return $this->get_method() == self::METHOD_POST;
  }

  /**
   * Returns TRUE / FALSE if method used is GET.
   */
  public function is_method_get() {
    return $this->get_method() == self::METHOD_GET;
  }

  /**
   * Returns TRUE / FALSE if method used is DELETE.
   */
  public function is_method_delete() {
    return $this->get_method() == self::METHOD_DELETE;
  }

  /**
   * Returns TRUE / FALSE if method used is PUT.
   */
  public function is_method_put() {
    return $this->get_method() == self::METHOD_PUT;
  }

  /**
   * Returns the timezone of the REST call.
   */
  public function get_timezone() {
    return $this->calling_app[mediamosa_app_db::TIMEZONE];
  }
  // ---------------------------------------------------- Functions (protected).
  /**
   * Process the supplied parameters.
   *
   * @param array $var_setup
   */
  protected function process_rest_args(array $var_setup) {

    // Clear it.
    $this->a_rest_args = array();
    foreach ($this->{self::URI_PARAMS} as $key_uri => $value_uri) {
      if (!isset($var_setup[self::VARS][$key_uri]) && $this->check_for_unspecified) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_uri));
      }

      $this->a_rest_args[$key_uri][self::PARAM_VALUE] = $value_uri;
    }

    /**
     * If $GLOBALS['mediamosa_params'] is available, use it instead of _GET and
     * _POST. This is required if we run the REST call in a sandbox (not
     * simpletest) inside a page.
     */
    if (isset($GLOBALS['mediamosa_params'])) {
      // Fill the $a_rest_args array with rest input.
      foreach ($GLOBALS['mediamosa_params'] as $key_params => $value_params) {
        if ($key_params == 'q') {
          continue; // Ignore the q variable.
        }
        // Set by default off.

        if (!isset($var_setup[self::VARS][$key_params]) && $this->check_for_unspecified) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_params));
        }

        if (isset($this->a_rest_args[$key_params])) {
          if ($this->a_rest_args[$key_params][self::PARAM_VALUE] != $value_params) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_params, '@value1' => $this->a_rest_args[$key_params][self::PARAM_VALUE], '@value2' => print_r($value_params, TRUE)));
          }

          continue;
        }

        $this->a_rest_args[$key_params][self::PARAM_VALUE] = $value_params;
      }
    }
    else {
      // Fill the $a_rest_args array with rest input.
      foreach ($_GET as $key_get => $value_get) {
        if ($key_get == 'q') {
          continue; // Ignore the q variable.
        }
        // Set by default off.

        if (!isset($var_setup[self::VARS][$key_get]) && $this->check_for_unspecified) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_get));
        }

        if (isset($this->a_rest_args[$key_get])) {
          if ($this->a_rest_args[$key_get][self::PARAM_VALUE] != $value_get) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_get, '@value1' => $this->a_rest_args[$key_get][self::PARAM_VALUE], '@value2' => print_r($value_get, TRUE)));
          }

          continue;
        }

        $this->a_rest_args[$key_get][self::PARAM_VALUE] = $value_get;
      }

      // Now do the same to _POST.
      foreach ($_POST as $key_post => $value_post) {
        if (!isset($var_setup[self::VARS][$key_post]) && $this->check_for_unspecified) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_post));
        }

        if (isset($this->a_rest_args[$key_post])) {
          if ($this->a_rest_args[$key_post][self::PARAM_VALUE] != $value_post) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_post, '@value1' => $this->a_rest_args[$key_post][self::PARAM_VALUE], '@value2' => print_r($value_post, TRUE)));
          }

          continue;
        }

        $this->a_rest_args[$key_post][self::PARAM_VALUE] = $value_post;
      }
    }

    // All args are now known. Now walk through the setup and set the defaults.
    foreach ($var_setup[self::VARS] as $param => &$a_var) {

      // Set by default off.
      $this->a_rest_args[$param][self::PARAM_ISSET_DEFAULT] = FALSE; // Set by default
      $this->a_rest_args[$param][self::PARAM_ISSET_GIVEN] = FALSE;

      // Make sure its set, even when empty.
      if (!isset($this->a_rest_args[$param][self::PARAM_VALUE])) {
        $this->a_rest_args[$param][self::PARAM_VALUE] = NULL; // Default NULL.

        // Defaults.
        if (isset($a_var[self::VAR_DEFAULT_VALUE])) {
          $this->a_rest_args[$param][self::PARAM_VALUE] = $a_var[self::VAR_DEFAULT_VALUE];
          $this->a_rest_args[$param][self::PARAM_ISSET_DEFAULT] = TRUE; // Set by default.
        }
      }
      else {
        $this->a_rest_args[$param][self::PARAM_ISSET_GIVEN] = TRUE; // Was found in GET/POST/URI.
      }

      // Set not validated so we can validate when we need it.
      $this->a_rest_args[$param][self::PARAM_VALIDATED] = FALSE;

      // If variable is internal value only, then we set the value to default or NULL. Put it on READ ONLY.
      // If REST has tried set then we throw exception.
      if (isset($a_var[self::VAR_IS_INTERNAL_ONLY]) && $a_var[self::VAR_IS_INTERNAL_ONLY] == self::VAR_IS_INTERNAL_ONLY_YES && !$this->is_internal()) {
        if ($this->isset_given_param($param) && !self::is_internal()) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INTERNAL_ONLY, array('@param' => $param, '@type' => $a_var[self::PARAM_TYPE]));
        }

        $this->a_rest_args[$param][self::PARAM_VALUE] = isset($a_var[self::VAR_DEFAULT_VALUE]) ? $a_var[self::VAR_DEFAULT_VALUE] : NULL;
      }

      // array check.
      if (isset($this->a_rest_args[$param][self::PARAM_VALUE])) {
        if (!isset($a_var[self::VAR_IS_ARRAY]) || $a_var[self::VAR_IS_ARRAY] == self::VAR_IS_ARRAY_NO) {
          if (is_array($this->a_rest_args[$param][self::PARAM_VALUE])) {
            $this->a_rest_args[$param][self::PARAM_VALUE] = reset($this->a_rest_args[$param][self::PARAM_VALUE]);
          }
        }
        elseif (!is_array($this->a_rest_args[$param][self::PARAM_VALUE])) {
          $this->a_rest_args[$param][self::PARAM_VALUE] = $this->a_rest_args[$param][self::PARAM_VALUE] == '' ? array() : array($this->a_rest_args[$param][self::PARAM_VALUE]);
        }
      }

      // Check if we need to trim it.
      if (isset($a_var[self::VAR_TRIM_VALUE]) && $a_var[self::VAR_TRIM_VALUE] == self::VAR_TRIM_VALUE_YES) {
        $this->a_rest_args[$param][self::PARAM_VALUE] = trim($this->a_rest_args[$param][self::PARAM_VALUE]);
      }

      // Set default value for is_required.
      if (!isset($a_var[self::VAR_IS_REQUIRED])) {
        $a_var[self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;
      }

      // Set read only to no, if not set.
      if (!isset($a_var[self::VAR_IS_READ_ONLY])) {
        $a_var[self::VAR_IS_READ_ONLY] = self::VAR_IS_READ_ONLY_NO;
      }

      // If override is set, override the GET/POST/URI input.
      if (isset($a_var[self::VAR_OVERRIDE_VALUE])) {
        $this->a_rest_args[$param][self::PARAM_VALUE] = $a_var[self::VAR_OVERRIDE_VALUE];
        $this->a_rest_args[$param][self::PARAM_ISSET_GIVEN] = TRUE; // lets simulate input from GET/POST/URI.
      }

      // Copy Read only status.
      $this->a_rest_args[$param][self::PARAM_IS_READ_ONLY] = $a_var[self::VAR_IS_READ_ONLY];

      // Is required.
      $this->a_rest_args[$param][self::PARAM_IS_REQUIRED] = $a_var[self::VAR_IS_REQUIRED];

      // if param is specified with range and the param is provided during rest
      // call or had defaulft value, then check range.
      $this->a_rest_args[$param][self::PARAM_RANGE_START] = isset($a_var[self::VAR_RANGE_START]) && $this->isset_param($param) ? $a_var[self::VAR_RANGE_START] : NULL;
      $this->a_rest_args[$param][self::PARAM_RANGE_END] = isset($a_var[self::VAR_RANGE_END]) && $this->isset_param($param) ? $a_var[self::VAR_RANGE_END] : NULL;

      // Type.
      $this->a_rest_args[$param][self::PARAM_TYPE] = $a_var[self::VAR_TYPE];

      // Allowed values;
      $this->a_rest_args[$param][self::PARAM_ALLOWED_VALUES] = isset($a_var[self::VAR_ALLOWED_VALUES]) ? $a_var[self::VAR_ALLOWED_VALUES] : array();
    }

    return $var_setup;
  }

  /**
   * Validate the supplied parameters and type validation.
   *
   * @param array $var_setup
   */
  protected function validate_rest_args(array $var_setup) {
    foreach ($this->a_rest_args as $name => $param) {
      if (empty($param[self::PARAM_TYPE])) {
        unset($this->a_rest_args[$name]); // set but not in var_set_up, unset.
        continue;
      }

      // Save the original value.
      $this->a_rest_args[$name][self::PARAM_VALUE_ORG] = (empty($this->a_rest_args[$name][self::PARAM_VALUE]) ? NULL : $this->a_rest_args[$name][self::PARAM_VALUE]);

      // If allowed values is set and default value is set and the given value
      // is not allowed and the value is empty, then value will get the default
      // value.
      if (!empty($this->a_rest_args[$name][self::PARAM_ALLOWED_VALUES]) &&
          empty($this->a_rest_args[$name][self::PARAM_VALUE]) &&
          isset($var_setup[self::VARS][$name][self::VAR_DEFAULT_VALUE]) &&
          !in_array($this->a_rest_args[$name][self::PARAM_VALUE], $this->a_rest_args[$name][self::PARAM_ALLOWED_VALUES])
        ) {

        // No value was given, but there is a default, then set the default.
        $this->a_rest_args[$name][self::PARAM_VALUE] = $var_setup[self::VARS][$name][self::VAR_DEFAULT_VALUE];

        // The default value must be allowed.
        assert(in_array($this->a_rest_args[$name][self::PARAM_VALUE], $this->a_rest_args[$name][self::PARAM_ALLOWED_VALUES]));
      }

      // check() will throw when requirements are not met.
      $this->a_rest_args[$name][self::PARAM_VALUE] = mediamosa_type::check($name, $param[self::PARAM_TYPE], $this->a_rest_args[$name][self::PARAM_VALUE], $this->a_rest_args[$name][self::PARAM_IS_REQUIRED] == self::VAR_IS_REQUIRED_YES, $this->a_rest_args[$name][self::PARAM_RANGE_START], $this->a_rest_args[$name][self::PARAM_RANGE_END], !empty($this->a_rest_args[$name][self::PARAM_ALLOWED_VALUES]) ? $this->a_rest_args[$name][self::PARAM_ALLOWED_VALUES] : NULL);
      $this->a_rest_args[$name][self::PARAM_VALIDATED] = TRUE;
    }
  }

  /**
   * Setup the application that is calling this REST call. This can be NULL
   * because an REST call can be called without application (like externals).
   *
   * In case of no application we revert to Anonymous application (application
   * that has no app_id and default settings.
   *
   * @param array $var_setup
   */
  protected function setup_calling_app(array $var_setup) {
    $app_ids = $this->isset_param(self::APP_ID) ? $this->get_param_value_app() : 0;
    $app_id = is_array($app_ids) ? reset($app_ids) : $app_ids;

    // No app means internal APP, defaults to UTC.
    $this->calling_app = array(mediamosa_app_db::APP_ID => 0, mediamosa_app_db::TIMEZONE => mediamosa_settings::MEDIAMOSA_DEFAULT_TIMEZONE_INTERNAL);

    // Get the app.
    if (!empty($app_id)) {
      $app = mediamosa_app::get_by_appid($app_id);
      if (empty($app)) {
        throw new mediamosa_exception(mediamosa_error::ERRORCODE_INVALID_APP_ID, array('@app_id' => $app_id));
      }

      $this->calling_app = array(mediamosa_app_db::APP_ID => $app[mediamosa_app_db::APP_ID], mediamosa_app_db::TIMEZONE => $app[mediamosa_app_db::TIMEZONE]);
    }
  }

  /**
   * Convert incoming dates to UTC.
   *
   * @param array $var_setup
   */
  protected function appdate2utc_rest_args(array $var_setup) {

    // Now loop through input and convert to UTC. Specify TYPE_DATETIME_UTC when you need to skip conversion.
    foreach ($this->a_rest_args as $name => $param) {
      if ($param[self::PARAM_TYPE] == mediamosa_sdk::TYPE_DATETIME && $this->isset_param($name) && !empty($this->a_rest_args[$name][self::PARAM_VALUE]) && $this->a_rest_args[$name][self::PARAM_VALUE] != '0000-00-00 00:00:00') {
        $this->a_rest_args[$name][self::PARAM_VALUE] = mediamosa_datetime::app_date2utc_date($param[self::PARAM_VALUE], $this->calling_app[mediamosa_app_db::TIMEZONE]);
      }
    }
  }
}
