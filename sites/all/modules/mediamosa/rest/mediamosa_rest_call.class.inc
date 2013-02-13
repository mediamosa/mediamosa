<?php
/**
 * @file
 * The REST call object provides the REST call function data and excecution.
 */

abstract class mediamosa_rest_call {
  // ------------------------------------------------------------------ Members.
  // Original REST call arguments.
  protected $rest_args = array();

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

  // ---------------------------------------------------------------- Constants.
  const ID = 'uri_id';
  const URI = 'uri';
  const URI_PARAMS = 'uri_params';
  const URI_REQUEST = 'uri_request'; // Original URI.
  const MATCH_URI_FUNCTION = 'match_uri';

  // Response type, see mediamosa_response for list.
  const RESPONSE_TYPE = 'content_type';

  const METHOD = 'method';
  const METHOD_GET = 'GET';
  const METHOD_POST = 'POST';
  const METHOD_PUT = 'PUT';
  const METHOD_DELETE = 'DELETE';
  // Do not use for defining the method of your REST call, use GET. Incoming
  // HEAD requests are translated to GET and will match any specified GET rest
  // call.
  const METHOD_HEAD = 'HEAD';

  // Type OPTIONS is used rarely. For now we use it for verifing cross domain
  // uploads calls from ajax front-ends.
  const METHOD_OPTIONS = 'OPTIONS';

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
  //
  // The access value is an OR'd value from different access types.
  const ACCESS = 'access';

  // Allow internal access (need internal authentication).
  const ACCESS_INTERNAL = 1;
  // Internal only switch;
  const ACCESS_INTERNAL_ONLY = 1;

  // Allow external access (no authentication required).
  const ACCESS_EXTERNAL = 2;
  // External access only (no other authentication required).
  const ACCESS_EXTERNAL_ONLY = 2;

  // Access allowed by authentication hook (DBUS etc).
  const ACCESS_AUTHENTICATED = 4;
  // Access only allowed by authentication hook (DBUS etc).
  const ACCESS_AUTHENTICATED_ONLY = 4;

  // Default allows Internal and authentication access.
  const ACCESS_DEFAULT = 5;

  // Allow access by all access rules.
  const ACCESS_FOR_EXTERNAL = 7;

  const TITLE = 'title';             // Title of the REST call (In English, required).
  const DESCRIPTION = 'description'; // Description of the REST call (In English, required).
  const EXAMPLE_REQUEST = 'example_request';   // Example request url.
  const EXAMPLE_RESPONSE = 'example_response'; // Example response output.
  const RESPONSE_FIELDS = 'response_fields';   // Response fields in output.

  const BEHAVE_AS_EXTERNAL = 'behave_as_external'; // Set to true if you want this rest call to behave like external without /external.

  const FOR_INTERFACE = 'for_interface';
  const FOR_INTERFACE_APP = 'for_interface_app';
  const FOR_INTERFACE_UPLOAD = 'for_interface_upload';
  const FOR_INTERFACE_DOWNLOAD = 'for_interface_download';
  const FOR_INTERFACE_OPENAPI = 'for_interface_openapi';

  // Specify to TRUE will make sure the REST can not be canceled by user and
  // will still continue to execute even when connection to client has been
  // lost. This will always be true for POST rest calls.
  const NO_USER_ABORT = 'no_user_abort';

  // Default var names / values.
  const APP_ID = 'app_id';
  const LIMIT = 'limit';
  const OFFSET = 'offset';
  const ORDER_BY = 'order_by';
  const ORDER_DIRECTION = 'order_direction';
  const IS_APP_ADMIN = 'is_app_admin';
  const MEDIAMOSA_VERSION = 'mediamosa_version';

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
  const VAR_ALIAS_FOR = 'alias_for'; // Contains array of alt. variable names that contain the same value.

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

  // Filter options.
  const FILTEROP_CONTAINS = 'contains';
  const FILTEROP_EQUALS = 'equals';
  const FILTEROP_STARTSWITH = 'startsWith';
  const FILTEROP_PRESENT = 'present';

  // Sort order.
  const SORTORDER_ASCENDING = 'ascending';
  const SORTORDER_DESCENDING = 'descending';

  // MediaItem.
  const USER_OWNER = '@owner';
  const USER_VIEWER = '@viewer';
  const USER_ME = '@me';
  const GROUP_SELF = '@self';
  const MEDIAITEM_TYPE = 'video';

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
      if (empty($_SERVER[$internal_pass]) || $_SERVER[$internal_pass] != mediamosa::get_internal_password()) {
        mediamosa_watchdog::log('Unmatching internal password (@pass) for REST URL: @url', array('@url' => $this->{self::URI}, '@pass' => (empty($_SERVER[$internal_pass]) ? 'No pass found!' : $_SERVER[$internal_pass])), WATCHDOG_WARNING);
        throw new mediamosa_exception_error_authentication_failure();
      }
    }

    // Check if we are external and allowed.
    if ($this->is_external() && !($this->{self::ACCESS} & self::ACCESS_EXTERNAL)) {
      throw new mediamosa_exception_error_authentication_failure();
    }

    // Do login / authorization, unless the call is an internal/external.
    $this->do_app_authorized();

    // If we are internal and no app_id is set, we have access to all.
    // Just to be sure the first app_id is 0, so single app_id code will return
    // empty.
    if ($this->is_internal() && !isset($_GET['app_id'])) {
      $_GET['app_id'] = array(0) + array(mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID), mediamosa_app_db::APP_ID));
      mediamosa::set_environment_app_ids($_GET['app_id']);
    }

    // Get the var setup.
    $var_setup = $this->get_var_setup();

    // Assert when failed.
    assert($var_setup);

    // Was a good idea at the time, lets turn it off now.
    $this->check_for_unspecified = FALSE;

    // Check rest args.
    $var_setup = $this->process_rest_args($var_setup);
    assert($var_setup);

    // Validate reg args.
    $this->validate_rest_args($var_setup);

    // Convert input datetime to UTC based on the app timezone.
    $this->appdate2utc_rest_args();

    // Call the function.
    $this->do_call();

    return $this->{self::RESPONSE_TYPE};
  }

  /**
   * Do authorize app.
   */
  private function do_app_authorized() {
    // If we are internal or external, return. Remember that we completly ignore
    // the session application ID as internal or external may not use it.
    if ($this->is_internal() || $this->is_external()) {
      if ($this->is_external()) {
        // Make sure app_id can not be used with externals.
        unset($_REQUEST['app_id'], $_POST['app_id'], $_GET['app_id']);
      }
      return;
    }

    // App already logged in?
    $app_id = mediamosa_app::get_session_app_id();
    if ((int) $app_id > 0) {
      $_GET['app_id'] = (int) $app_id;
      return;
    }

    // Now walk through the app authorized hooks until one returns TRUE.
    foreach (module_implements('mediamosa_app_authorized') as $module) {
      $function = $module . '_' . 'mediamosa_app_authorized';
      if (function_exists($function) && call_user_func_array($function, array()) === TRUE) {
        return;
      }
    }

    // Not authorized.
    throw new mediamosa_exception_error_authentication_failure();
  }

  /**
   * Get the default vars we always expect in every rest call
   */
  protected function get_var_setup_default(array $var_setup, $app_id_required = TRUE) {
    $var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::APP_ID] = array(
      self::VAR_IS_READ_ONLY => self::VAR_IS_READ_ONLY_YES, // Once set, dont change.
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

    $var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::MEDIAMOSA_VERSION] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAMOSA_VERSION,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DESCRIPTION => mediamosa::t("Provide the version to get the REST call returned in the format of the specified MediaMosa version. Keep empty or skip to retrieve in current (latest) version. Provide '2' to retrieve the REST call in 2.x format. Or be more specific by providing up to 3 digits; '2.3.0'. This setting will override any setting used by client application or the default setting in MediaMosa."),
      self::VAR_DEFAULT_VALUE => '',
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
       // Once set, dont change.
      self::VAR_IS_READ_ONLY => self::VAR_IS_READ_ONLY_YES,
      self::VAR_DESCRIPTION => mediamosa::t('Limit of the items of the result set.'),
      self::VAR_RANGE_START => 0,
      self::VAR_RANGE_END => $limit_max,
    );

    $var_setup[self::VARS][self::OFFSET] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_NO,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 0,
      // Once set, dont change.
      self::VAR_IS_READ_ONLY => self::VAR_IS_READ_ONLY_YES,
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
   * Add access control allow origin header.
   *
   * Only call this function when the current call is allowed for cross domain.
   * Used for upload calls and OPTIONS calls.
   */
  protected function add_header_access_control_allow_origin() {
    header('Access-Control-Allow-Origin: *');
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
    $app_versions = &drupal_static(__FUNCTION__, array());

    if (!isset($app_versions[$app_id])) {
      $app = mediamosa_app::get_by_appid($app_id);

      // Is at latest version?
      $app_versions[$app_id] = ($app[mediamosa_app_db::ACTIVE_VERSION] == mediamosa_version::LATEST || empty($app[mediamosa_app_db::ACTIVE_VERSION])) ? FALSE : mediamosa_version::get_version($app[mediamosa_app_db::ACTIVE_VERSION]);
    }

    // Return in array.
    return $app_versions[$app_id];
  }

  /**
   * Enrich the VAR setup with the default ORDER BY, ORDER_DIRECTION
   *
   * @param array $var_setup
   *   Provide current array to merge.
   * @param string $default_order_by
   *  Default field to order on.
   * @param array $allowed_values_order_by
   *  array containing allowed values.
   * @param string $default_order_direction
   *  ASC (default) / DESC direction.
   *
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
   *
   * This flag will enable to throw exception when any _GET or _POST var that is
   * not specified in the var setup of the REST call object.
   *
   * @param boolean $check_for_unspecified
   *   Supply TRUE when check for unspecified is unabled.
   */
  public function set_check_for_unspecified($check_for_unspecified) {
    $this->check_for_unspecified = $check_for_unspecified;
  }

  /**
   * Retrieve the value of a supplied parameter.
   *
   * @param string $param
   *   Name of the parameter.
   * @param bool $translate_bool
   *  Translate the bool to string variant.
   * @param array $translate_bools
   *  Array containing mapping to TRUE / FALSE string values.
   *  default: array(FALSE => 'FALSE', TRUE => 'TRUE')
   * @return mixed
   */
  public function get_param_value($param, $translate_bool = FALSE, array $translate_bools = array(FALSE => 'FALSE', TRUE => 'TRUE')) {
    if (!isset($this->rest_args[$param])) {
      throw new mediamosa_exception_program_error('Calling get_param with non-existing param (@param), should not happen; define it in allowed vars for your rest call class "@restcall"', array('@param' => $param, '@restcall' => $this->{self::CLASS_NAME}));
    }

    if (!$this->rest_args[$param][self::PARAM_VALIDATED]) {
      // Save the original value.
      $this->rest_args[$param][self::PARAM_VALUE_ORG] = (empty($this->rest_args[$param][self::PARAM_VALUE]) ? NULL : $this->rest_args[$param][self::PARAM_VALUE]);

      $a_param = $this->rest_args[$param];
      $this->rest_args[$param][self::PARAM_VALUE] = mediamosa_type::check($param, $a_param[self::PARAM_TYPE], $a_param[self::PARAM_VALUE], $a_param[self::PARAM_IS_REQUIRED] == self::VAR_IS_REQUIRED_YES, isset($a_param[self::PARAM_RANGE_START]) ? $a_param[self::PARAM_RANGE_START] : NULL, isset($a_param[self::PARAM_RANGE_END]) ? $a_param[self::PARAM_RANGE_END] : NULL, isset($a_param[self::PARAM_ALLOWED_VALUES]) && count($a_param[self::PARAM_ALLOWED_VALUES]) ? $a_param[self::PARAM_ALLOWED_VALUES] : NULL);
      $this->rest_args[$param][self::PARAM_VALIDATED] = TRUE;
    }

    return ($translate_bool && is_bool($this->rest_args[$param][self::PARAM_VALUE])) ? $translate_bools[$this->rest_args[$param][self::PARAM_VALUE]] : $this->rest_args[$param][self::PARAM_VALUE];
  }

  /**
   * Retrieve the original value of a supplied parameter.
   *
   * The original value is always the value that was submitted to the REST call
   * by either GET or POST. If the param value is overwritten after the GET and
   * POST is processed, this value will still contain the original value.
   *
   * @param string $param
   *   The name of the parameter.
   *
   * @return mixed
   *   The original value of the supplied parameter.
   */
  public function get_param_value_org($param) {
    if (!isset($this->rest_args[$param])) {
      throw new mediamosa_exception_program_error('Calling get_param with non-existing param (@param), should not happen; define it in allowed vars for your rest call class "@restcall"', array('@param' => $param, '@restcall' => $this->{self::CLASS_NAME}));
    }

    return $this->rest_args[$param][self::PARAM_VALUE_ORG];
  }

  /**
   * Set the value of the supplied parameter.
   *
   * Use only internally, expected to be correct value, not value checked.
   *
   * @param string $param
   *   The name of the parameter.
   * @param mixed $value
   *   The value to store.
   */
  public function set_param_value($param, $value) {
    if (isset($this->rest_args[$param]) && $this->rest_args[$param][self::PARAM_IS_READ_ONLY] == self::VAR_IS_READ_ONLY_YES) {
      throw new mediamosa_exception_program_error('Do not set param @param, its read only.', array('@param' => $param));
    }

    // Set value.
    $this->rest_args[$param][self::PARAM_VALUE] = $value;

    // Expected to ok, we cant validate anyway.
    $this->rest_args[$param][self::PARAM_VALIDATED] = TRUE;

    // Ok not set by default or URI/POST/GET.
    $this->rest_args[$param][self::PARAM_ISSET_DEFAULT] = FALSE;
    $this->rest_args[$param][self::PARAM_ISSET_GIVEN] = FALSE;
  }

  /**
   * Unset param from var listing.
   *
   * @param string $param
   *   The name of the parameter.
   */
  public function unset_param($param) {
    $this->set_param_value($param, NULL);
  }

  /**
   * Will return TRUE if the param is set (by any means set)
   *
   * @param string $param
   *   The name of the parameter.
   *
   * @return boolean
   *   Returns TRUE when value is set.
   */
  public function isset_param($param) {
    return isset($this->rest_args[$param]) && isset($this->rest_args[$param][self::PARAM_VALUE]);
  }

  /**
   * Check if the value was set by POST/GET/URI.
   *
   * If the param is set any other way (like default value or set_param), it
   * will return FALSE.
   *
   * @param string $param
   *   The name of the parameter.
   *
   * @return boolean
   *   Will return TRUE if the param was set by POST/GET/URI.
   */
  public function isset_given_param($param) {
    return isset($this->rest_args[$param]) && $this->rest_args[$param][self::PARAM_ISSET_GIVEN];
  }

  /**
   * Will return TRUE if the param was set by default and not by GET/POST/URI.
   *
   * @param string $param
   *   The name of the parameter.
   *
   * @return boolean
   *   Returns TRUE is value was set by default value.
   */
  public function isset_default_param($param) {
    return isset($this->rest_args[$param]) && $this->rest_args[$param][self::PARAM_ISSET_DEFAULT];
  }

  /**
   * Will return TRUE if the param is empty.
   *
   * @param string $param
   *   The name of the parameter.
   *
   * @return boolean
   *   Returns TRUE when value is 'empty'.
   */
  public function empty_param($param) {
    if (!self::isset_param($param)) {
      return TRUE; // Not set, is empty.
    }

    if (is_string($this->rest_args[$param][self::PARAM_VALUE])) {
      return trim($this->rest_args[$param][self::PARAM_VALUE]) == '';
    }

    if (is_int($this->rest_args[$param][self::PARAM_VALUE])) {
      return $this->rest_args[$param][self::PARAM_VALUE] == 0;
    }

    // Default empty behavour.
    return empty($this->rest_args[$param][self::PARAM_VALUE]);
  }

  /**
   * Returns the app_ids param value (always array).
   *
   * The first app_id in the array is the main application (the caller).
   *
   * @return array
   *   The app_ids.
   */
  public function get_param_value_app() {
    // Get the environment setting.
    return mediamosa::get_environment_app_ids();
  }

  /**
   * Returns the offset param value.
   *
   * @return integer
   *   The offset in the resultset.
   */
  public function get_param_value_offset() {
    return $this->get_param_value(self::OFFSET);
  }

  /**
   * Returns the limit param value.
   *
   * @return integer
   *   The maximum items returned in the resultset.
   */
  public function get_param_value_limit() {
    return $this->get_param_value(self::LIMIT);
  }

  /**
   * Returns the order_by param value.
   *
   * @return string
   *   The order by value.
   */
  public function get_param_value_order_by() {
    return $this->get_param_value(self::ORDER_BY);
  }

  /**
   * Returns the order_direction param value.
   *
   * @return string
   *   Either 'ASC' or 'DESC'.
   */
  public function get_param_value_order_direction() {
    return $this->get_param_value(self::ORDER_DIRECTION);
  }

  /**
   * Return from $_GET / $_POST the value, if found.
   *
   * Use this function if you need to go before the REST call process_rest_args.
   *
   * @param string $param
   *   The name of the parameter.
   *
   * @return mixed
   *   The value of the parameter.
   */
  public static function get_param_value_global($param) {
    // Is mediamosa_params is set, we take priority on those.
    if (isset($GLOBALS['mediamosa_params'])) {
      // If for any reason mediamosa_params is in _REQUEST, then it was supplied
      // externally.
      if (isset($_REQUEST['mediamosa_params'])) {
        throw new mediamosa_exception_error_403();
      }

      return isset($GLOBALS['mediamosa_params'][$param]) ? $GLOBALS['mediamosa_params'][$param] : NULL;
    }

    return isset($_GET[$param]) ? $_GET[$param] : (isset($_POST[$param]) ? $_POST[$param] : NULL);
  }

  /**
   * Return the method used on this REST call.
   *
   * @return string
   *   The method used.
   */
  public function get_method() {
    return $this->{self::METHOD};
  }

  /**
   * Returns TRUE / FALSE if method used is POST.
   *
   * @return boolean
   *   Returns TRUE when POST.
   */
  public function is_method_post() {
    return $this->get_method() == self::METHOD_POST;
  }

  /**
   * Returns TRUE / FALSE if method used is GET.
   *
   * @return boolean
   *   Returns TRUE when GET.
   */
  public function is_method_get() {
    return $this->get_method() == self::METHOD_GET;
  }

  /**
   * Returns TRUE / FALSE if method used is DELETE.
   *
   * @return boolean
   *   Returns TRUE when DELETE.
   */
  public function is_method_delete() {
    return $this->get_method() == self::METHOD_DELETE;
  }

  /**
   * Returns TRUE / FALSE if method used is PUT.
   *
   * @return boolean
   *   Returns TRUE when PUT.
   */
  public function is_method_put() {
    return $this->get_method() == self::METHOD_PUT;
  }

  /**
   * Returns TRUE / FALSE if method used is OPTIONS.
   *
   * @return boolean
   *   Returns TRUE when OPTIONS.
   */
  public function is_method_options() {
    return $this->get_method() == self::METHOD_OPTIONS;
  }

  // ---------------------------------------------------- Functions (protected).
  /**
   * Process the supplied parameters.
   *
   * @param array $var_setup
   */
  protected function process_rest_args(array $var_setup) {

    // Clear it.
    $this->rest_args = array();
    foreach ($this->{self::URI_PARAMS} as $key_uri => $value_uri) {
      if (!isset($var_setup[self::VARS][$key_uri]) && $this->check_for_unspecified) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_uri));
      }

      $this->rest_args[$key_uri][self::PARAM_VALUE] = $value_uri;
    }

    /**
     * If $GLOBALS['mediamosa_params'] is available, use it instead of _GET and
     * _POST. This is required if we run the REST call in a sandbox (not
     * simpletest) inside a page.
     */
    if (isset($GLOBALS['mediamosa_params'])) {
      // If for any reason mediamosa_params is in _REQUEST, then it was supplied
      // externally.
      if (isset($_REQUEST['mediamosa_params'])) {
        throw new mediamosa_exception_error_403();
      }

      // Fill the $rest_args array with rest input.
      foreach ($GLOBALS['mediamosa_params'] as $key_params => $value_params) {
        if ($key_params == 'q') {
          continue; // Ignore the q variable.
        }
        // Set by default off.

        if (!isset($var_setup[self::VARS][$key_params]) && $this->check_for_unspecified) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_params));
        }

        if (isset($this->rest_args[$key_params])) {
          if ($this->rest_args[$key_params][self::PARAM_VALUE] != $value_params) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_params, '@value1' => $this->rest_args[$key_params][self::PARAM_VALUE], '@value2' => print_r($value_params, TRUE)));
          }

          continue;
        }

        $this->rest_args[$key_params][self::PARAM_VALUE] = $value_params;
      }
    }
    else {
      // Fill the $rest_args array with rest input.
      foreach ($_GET as $key_get => $value_get) {
         // Ignore the q variable.
        if ($key_get == 'q') {
          continue;
        }

        if (!isset($var_setup[self::VARS][$key_get]) && $this->check_for_unspecified) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_get));
        }

        if (isset($this->rest_args[$key_get])) {
          if ($this->rest_args[$key_get][self::PARAM_VALUE] != $value_get) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_get, '@value1' => print_r($this->rest_args[$key_get][self::PARAM_VALUE], TRUE), '@value2' => print_r($value_get, TRUE)));
          }

          continue;
        }

        $this->rest_args[$key_get][self::PARAM_VALUE] = $value_get;
      }

      // Now do the same to _POST.
      foreach ($_POST as $key_post => $value_post) {
        if (!isset($var_setup[self::VARS][$key_post]) && $this->check_for_unspecified) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_UNSPECIFIED_VARIABLE, array('@var' => $key_post));
        }

        if (isset($this->rest_args[$key_post])) {
          if ($this->rest_args[$key_post][self::PARAM_VALUE] != $value_post) {
            throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR, array('@var' => $key_post, '@value1' => print_r($this->rest_args[$key_post][self::PARAM_VALUE], TRUE), '@value2' => print_r($value_post, TRUE)));
          }

          continue;
        }

        $this->rest_args[$key_post][self::PARAM_VALUE] = $value_post;
      }
    }

    // Copy alias values to non-existing original values.
    foreach ($var_setup[self::VARS] as $param => &$var) {

      // Any aliases?
      if (!empty($var[self::VAR_ALIAS_FOR])) {
        // If the original value is not set, then maybe an alias?
        if (!isset($this->rest_args[$param][self::PARAM_VALUE])) {
          foreach ($var[self::VAR_ALIAS_FOR] as $param_alias) {
            // Found for alias, then copy it to original. First come, first
            // serve.
            if (isset($this->rest_args[$param_alias][self::PARAM_VALUE])) {
              $this->rest_args[$param][self::PARAM_VALUE] = $this->rest_args[$param_alias][self::PARAM_VALUE];
            }

            // Unset the alias value, so we can't ever use it directly in code.
            unset($this->rest_args[$param_alias]);
          }
        }
      }
    }

    // All args are now known. Now walk through the setup and set the defaults.
    foreach ($var_setup[self::VARS] as $param => &$var) {

      // Set by default off.
      $this->rest_args[$param][self::PARAM_ISSET_DEFAULT] = FALSE; // Set by default
      $this->rest_args[$param][self::PARAM_ISSET_GIVEN] = FALSE;

      // Make sure its set, even when empty.
      if (!isset($this->rest_args[$param][self::PARAM_VALUE])) {
        $this->rest_args[$param][self::PARAM_VALUE] = NULL; // Default NULL.

        // Defaults.
        if (isset($var[self::VAR_DEFAULT_VALUE])) {
          $this->rest_args[$param][self::PARAM_VALUE] = $var[self::VAR_DEFAULT_VALUE];
          $this->rest_args[$param][self::PARAM_ISSET_DEFAULT] = TRUE; // Set by default.
        }
      }
      else {
        $this->rest_args[$param][self::PARAM_ISSET_GIVEN] = TRUE; // Was found in GET/POST/URI.
      }

      // Set not validated so we can validate when we need it.
      $this->rest_args[$param][self::PARAM_VALIDATED] = FALSE;

      // If variable is internal value only, then we set the value to default or NULL. Put it on READ ONLY.
      // If REST has tried set then we throw exception.
      if (isset($var[self::VAR_IS_INTERNAL_ONLY]) && $var[self::VAR_IS_INTERNAL_ONLY] == self::VAR_IS_INTERNAL_ONLY_YES && !$this->is_internal()) {
        if ($this->isset_given_param($param) && !self::is_internal()) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INTERNAL_ONLY, array('@param' => $param, '@type' => $var[self::PARAM_TYPE]));
        }

        $this->rest_args[$param][self::PARAM_VALUE] = isset($var[self::VAR_DEFAULT_VALUE]) ? $var[self::VAR_DEFAULT_VALUE] : NULL;
      }

      // array check.
      if (isset($this->rest_args[$param][self::PARAM_VALUE])) {
        if (!isset($var[self::VAR_IS_ARRAY]) || $var[self::VAR_IS_ARRAY] == self::VAR_IS_ARRAY_NO) {
          if (is_array($this->rest_args[$param][self::PARAM_VALUE])) {
            $this->rest_args[$param][self::PARAM_VALUE] = reset($this->rest_args[$param][self::PARAM_VALUE]);
          }
        }
        elseif (!is_array($this->rest_args[$param][self::PARAM_VALUE])) {
          $this->rest_args[$param][self::PARAM_VALUE] = $this->rest_args[$param][self::PARAM_VALUE] == '' ? array() : array($this->rest_args[$param][self::PARAM_VALUE]);
        }
      }

      // Check if we need to trim it.
      if (isset($var[self::VAR_TRIM_VALUE]) && $var[self::VAR_TRIM_VALUE] == self::VAR_TRIM_VALUE_YES) {
        $this->rest_args[$param][self::PARAM_VALUE] = trim($this->rest_args[$param][self::PARAM_VALUE]);
      }

      // Set default value for is_required.
      if (!isset($var[self::VAR_IS_REQUIRED])) {
        $var[self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;
      }

      // Set read only to no, if not set.
      if (!isset($var[self::VAR_IS_READ_ONLY])) {
        $var[self::VAR_IS_READ_ONLY] = self::VAR_IS_READ_ONLY_NO;
      }

      // If override is set, override the GET/POST/URI input.
      if (isset($var[self::VAR_OVERRIDE_VALUE])) {
        $this->rest_args[$param][self::PARAM_VALUE] = $var[self::VAR_OVERRIDE_VALUE];
         // Lets simulate input from GET/POST/URI.
        $this->rest_args[$param][self::PARAM_ISSET_GIVEN] = TRUE;
      }

      // Copy Read only status.
      $this->rest_args[$param][self::PARAM_IS_READ_ONLY] = $var[self::VAR_IS_READ_ONLY];

      // Is required.
      $this->rest_args[$param][self::PARAM_IS_REQUIRED] = $var[self::VAR_IS_REQUIRED];

      // if param is specified with range and the param is provided during rest
      // call or had defaulft value, then check range.
      $this->rest_args[$param][self::PARAM_RANGE_START] = isset($var[self::VAR_RANGE_START]) && $this->isset_param($param) ? $var[self::VAR_RANGE_START] : NULL;
      $this->rest_args[$param][self::PARAM_RANGE_END] = isset($var[self::VAR_RANGE_END]) && $this->isset_param($param) ? $var[self::VAR_RANGE_END] : NULL;

      // Type.
      $this->rest_args[$param][self::PARAM_TYPE] = $var[self::VAR_TYPE];

      // Allowed values;
      $this->rest_args[$param][self::PARAM_ALLOWED_VALUES] = isset($var[self::VAR_ALLOWED_VALUES]) ? $var[self::VAR_ALLOWED_VALUES] : array();
    }

    return $var_setup;
  }

  /**
   * Validate the supplied parameters and type validation.
   *
   * @param array $var_setup
   */
  protected function validate_rest_args(array $var_setup) {

    foreach ($this->rest_args as $name => $param) {
      if (empty($param[self::PARAM_TYPE])) {
        unset($this->rest_args[$name]); // set but not in var_set_up, unset.
        continue;
      }

      // Save the original value.
      $this->rest_args[$name][self::PARAM_VALUE_ORG] = (empty($this->rest_args[$name][self::PARAM_VALUE]) ? NULL : $this->rest_args[$name][self::PARAM_VALUE]);

      // If allowed values is set and default value is set and the given value
      // is not allowed and the value is empty, then value will get the default
      // value.
      if (!empty($this->rest_args[$name][self::PARAM_ALLOWED_VALUES]) &&
          empty($this->rest_args[$name][self::PARAM_VALUE]) &&
          isset($var_setup[self::VARS][$name][self::VAR_DEFAULT_VALUE]) &&
          !in_array($this->rest_args[$name][self::PARAM_VALUE], $this->rest_args[$name][self::PARAM_ALLOWED_VALUES])
        ) {

        // No value was given, but there is a default, then set the default.
        $this->rest_args[$name][self::PARAM_VALUE] = $var_setup[self::VARS][$name][self::VAR_DEFAULT_VALUE];

        // The default value must be allowed.
        assert(in_array($this->rest_args[$name][self::PARAM_VALUE], $this->rest_args[$name][self::PARAM_ALLOWED_VALUES]));
      }

      // check() will throw when requirements are not met.
      $this->rest_args[$name][self::PARAM_VALUE] = mediamosa_type::check($name, $param[self::PARAM_TYPE], $this->rest_args[$name][self::PARAM_VALUE], $this->rest_args[$name][self::PARAM_IS_REQUIRED] == self::VAR_IS_REQUIRED_YES, $this->rest_args[$name][self::PARAM_RANGE_START], $this->rest_args[$name][self::PARAM_RANGE_END], !empty($this->rest_args[$name][self::PARAM_ALLOWED_VALUES]) ? $this->rest_args[$name][self::PARAM_ALLOWED_VALUES] : NULL);
      $this->rest_args[$name][self::PARAM_VALIDATED] = TRUE;

      // If the type == mediamosa_sdk::RESPONSE_TYPE and not empty, then
      // change the response type of the REST call.
      if ($param[self::PARAM_TYPE] == mediamosa_sdk::TYPE_RESPONSE_TYPE && !empty($this->rest_args[$name][self::PARAM_VALUE])) {
        $this->{self::RESPONSE_TYPE} = $this->rest_args[$name][self::PARAM_VALUE];
      }
    }
  }

  /**
   * Convert incoming dates to UTC.
   *
   * Based on client application timezone, the incoming dates are converted into
   * UTC.
   */
  protected function appdate2utc_rest_args() {

    // Get the timezone.
    $timezone = mediamosa::get_environment_timezone();

    // Now loop through input and convert to UTC. Specify TYPE_DATETIME_UTC when you need to skip conversion.
    foreach ($this->rest_args as $name => $param) {
      if ($param[self::PARAM_TYPE] == mediamosa_sdk::TYPE_DATETIME && $this->isset_param($name) && !empty($this->rest_args[$name][self::PARAM_VALUE]) && $this->rest_args[$name][self::PARAM_VALUE] != '0000-00-00 00:00:00') {
        $this->rest_args[$name][self::PARAM_VALUE] = mediamosa_datetime::app_date2utc_date($param[self::PARAM_VALUE], $timezone);
      }
    }
  }
}
