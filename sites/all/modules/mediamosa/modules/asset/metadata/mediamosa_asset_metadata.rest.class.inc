<?php
 /*
 * @file
 * Asset metadata related REST calls.
 */

/**
 * URI: /asset/$asset_id/metadata
 *
 * Create the metadata for the asset.
 */
class mediamosa_rest_call_asset_metadata_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const ACTION = 'action';
  const ACTION_REPLACE = 'replace';
  const ACTION_APPEND = 'append';
  const ACTION_UPDATE = 'update';

  private $metadata_definitions_full = NULL;

  /**
   * A list of metadata fields I created during var_setup. These are unset
   * during process_rest_args() because var_setup can be called for quering the
   * vars only f.e. documentation.
   *
   * @var array
   */
  private $used_metadata_definitions = array();

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to create the metadata for.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID, must be owner of asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::REPLACE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Replace all metadata or append given metadata values. Do not specify both action and replace parameters.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::ACTION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => "Action to choose for given metadata values, choose either 'replace', 'append' or 'update'. Do not specify both action and replace parameters.",
          self::VAR_ALLOWED_VALUES => array(self::ACTION_REPLACE, self::ACTION_APPEND, self::ACTION_UPDATE),
          self::VAR_DEFAULT_VALUE => self::ACTION_REPLACE,
        ),
      ),
    );

    /**
     * var_setup is just to build the setup. If its required input, then the
     * input is also optional. Meaning, you need to fall back to the situation
     * where no input is available and STILL able to generate the var_setup.
     *
     * In our case we at least supply default. However. Because in 1 case where
     * master slave metadata is set. The user is only allowed to create metadata
     * within his app and not the metadata of the original app.
     *
     * To fix this black hole, I will store the names I set here and unset them
     * in process_rest_args. This will correctly create var_setup when run and
     * create the var_setup in default when run only for documentation.
     */

    // Get full definitions.
    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full();

    // We need to add the allowed definitions to the $var_setup.
    foreach ($metadata_definitions_full as $name => $definitions) {

      switch ($definitions['propdef_type']) {
        case mediamosa_asset_metadata_property_db::TYPE_INT:
          $type = mediamosa_sdk::TYPE_INT;
          break;

        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          $type = mediamosa_sdk::TYPE_DATETIME;
          break;

        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
        default:
          // Default is string.
          $type = mediamosa_sdk::TYPE_STRING;
          break;
      }

      // FIXME Don't like this hack. Rather see extension on metadata property
      //       with a allowed values.
      if ($name == 'language') {
        $type = mediamosa_sdk::TYPE_LANGUAGE_CODE;
      }

      // Store the name, so we can unset them if we run process_rest_args.
      $this->used_metadata_definitions[] = $name;

      // Do not setup a default value, for any value(!).
      $var_setup[self::VARS][$name] = array(
        self::VAR_TYPE => $type,
        self::VAR_DESCRIPTION => 'A metadata property.',
        self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES, // always array.
      );
    }

    return self::get_var_setup_app_admin($var_setup);
  }

  // ----------------------------------------------- Override process_rest_args.
  protected function process_rest_args(array $var_setup) {
    // process current args so we can use them.
    $var_setup = parent::process_rest_args($var_setup);

    // 1st unset what we might not need from var_setup. Situation may change
    // during this call.
    foreach ($this->used_metadata_definitions as $name) {
      unset($var_setup[self::VARS][$name]);
    }

    // Now we can add the metadata definition input to our var setup.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Asset must exist.
    $asset = mediamosa_asset::must_exists($asset_id);

    // If the app match, we check ownership or else a master slave record must exist
    if ($app_id == $asset[mediamosa_asset_db::APP_ID]) {
      mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

      // Get full definitions.
      $this->metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id);
    }
    else {
      // Check if there is a master / slave record on the asset / mediafile we are trying to add metadata to.
      mediamosa_acl::read_single_object(mediamosa_acl::ACL_TYPE_ASSET, $asset_id, $app_ids);

      // Only allow own definitions, not dc, qdc etc.
      $this->metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full($app_id, array());
    }

    // We need to add the allowed definitions to the $var_setup.
    foreach ($this->metadata_definitions_full as $name => $definitions) {
      switch ($definitions['propdef_type']) {
        case mediamosa_asset_metadata_property_db::TYPE_INT:
          $type = mediamosa_sdk::TYPE_INT;
          break;

        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          $type = mediamosa_sdk::TYPE_DATETIME;
          break;

        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
        default:
          // Default is string.
          $type = mediamosa_sdk::TYPE_STRING;
          break;
      }

      // FIXME Don't like this hack. Rather see extension on metadata property
      //       with a allowed values.
      if ($name === 'language') {
        $type = mediamosa_sdk::TYPE_LANGUAGE_CODE;
      }

      // Do not setup a default value, for any value(!).
      $var_setup[self::VARS][$name] = array(
        self::VAR_TYPE => $type,
        self::VAR_DESCRIPTION => 'A metadata property.',
        self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES, // always array.
      );
    }

    // Now again process the args with the metadata properties.
    return parent::process_rest_args($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Access is checked in process_rest_args().

    $asset_id = $this->get_param_value(self::ASSET_ID);

    // Can't be set both.
    if ($this->isset_given_param(self::ACTION) && $this->isset_given_param(self::REPLACE)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ACTION_AND_REPLACE);
    }

    // Make sure replace is set (at least to default).
    if (!$this->isset_param(self::REPLACE)) {
      $this->set_param_value(self::REPLACE, TRUE);
    }

    // Will use $action, so if not given, use value of replace to get value for it.
    if (!$this->isset_given_param(self::ACTION)) {
      $replace = $this->get_param_value(self::REPLACE);
      $this->set_param_value(self::ACTION, $replace ? 'replace' : 'append');
    }

    $action = $this->get_param_value(self::ACTION);

    // Fill the values.
    $params = array();
    foreach ($this->metadata_definitions_full as $name => $definitions) {
      if ($this->isset_given_param($name)) {
        $params[$name] = $this->get_param_value($name);
      }
    }

    // Create the metadata.
    $value_set = mediamosa_asset_metadata::metadata_create($asset_id, $this->metadata_definitions_full, $params, $action);

    // Return what has been changed/inserted.
    foreach ($value_set as $name => $values) {
      $definitions = $this->metadata_definitions_full[$name];

      // Datetime values shall be converted to user timezone.
      if ($definitions['propdef_type'] == mediamosa_asset_metadata_property_db::TYPE_DATETIME) {
        if (is_array($values)) {
          foreach ($values as &$value) {
            $value = mediamosa::utcdate2appdate($value);
          }
        }
      }

      $o_mediamosa->add_item(array($name => $values));
    }
  }
}
