<?php
/**
 * @file
 * Transcode profile REST calls.
 */

/**
 * URI: /transcode/profile
 *      /video/transcode_profiles (deprecated)
 *      /transcode/profiles (deprecated)
 * Method: GET
 */
class mediamosa_rest_call_transcode_profile_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // ---------------------------------------------------------------- Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Get the listing.
    $transcode_profiles = mediamosa_transcode_profile::search($app_id);

    foreach ($transcode_profiles as $transcode_profile) {
      $item = array();
      $item['profile_id'] = $transcode_profile[mediamosa_transcode_profile_db::ID];
      $item[mediamosa_transcode_profile_db::PROFILE] = $transcode_profile[mediamosa_transcode_profile_db::PROFILE];
      $item['default'] = $transcode_profile[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE];
      $item['global'] = $transcode_profile[mediamosa_transcode_profile_db::APP_ID] > 0 ? 'FALSE' : 'TRUE';

      // UTC -> App timezone.
      if (isset($item[mediamosa_transcode_profile_db::CREATED])) {
        $item[mediamosa_transcode_profile_db::CREATED] = mediamosa::utcdate2appdate($item[mediamosa_transcode_profile_db::CREATED]);
      }
      if (isset($item[mediamosa_transcode_profile_db::CHANGED])) {
        $item[mediamosa_transcode_profile_db::CHANGED] = mediamosa::utcdate2appdate($item[mediamosa_transcode_profile_db::CHANGED]);
      }

      $mediamosa->add_item($item);
    }
  }
}

/**
 * URI: /transcode/profile/$profile_id
 * Method: GET
 */
class mediamosa_rest_call_transcode_profile_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const PROFILE_ID = 'profile_id';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the profile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $profile_id = $this->get_param_value(self::PROFILE_ID);

    // Get the listing.
    $profile = mediamosa_transcode_profile::get($profile_id, $app_id);

    // Add when found.
    if ($profile) {
      $transcode = array();
      $transcode['profile_id'] = $profile[mediamosa_transcode_profile_db::ID];
      $transcode['profile_name'] = $profile[mediamosa_transcode_profile_db::PROFILE];
      $transcode['default'] = $profile[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE];
      $transcode['global'] = $profile[mediamosa_transcode_profile_db::APP_ID] > 0 ? 'FALSE' : 'TRUE';
      $transcode['file_extension'] = $profile[mediamosa_transcode_profile_db::FILE_EXTENSION];
      $transcode['created'] = mediamosa::utcdate2appdate($profile[mediamosa_transcode_profile_db::CREATED]);
      $transcode['changed'] = mediamosa::utcdate2appdate($profile[mediamosa_transcode_profile_db::CHANGED]);
      $transcode['version'] = $profile[mediamosa_transcode_profile_db::VERSION];
      $transcode['command'] = $profile[mediamosa_transcode_profile_db::COMMAND];
      $transcode['tool'] = $profile[mediamosa_transcode_profile_db::TOOL];

      $mediamosa->add_item($transcode);
    }
  }
}

/**
 * URI: /transcode/profile/create
 * Method: POST
 */
class mediamosa_rest_call_transcode_profile_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const NAME = 'name';
  const TOOL = 'tool';
  const DEFAULT_ = 'default';
  const VERSION = 'version';
  const FILE_EXTENSION = 'file_extension';
  const COMMAND = 'command';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The name of the profile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::PROFILE_LENGTH,
        ),
        self::TOOL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The tool used for the profile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::TOOL_LENGTH,
        ),
        self::DEFAULT_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Is the default profile.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::VERSION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The version number.',
        ),
        self::FILE_EXTENSION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The file extension.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::FILE_EXTENSION_LENGTH,
        ),
        self::COMMAND => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The command.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::COMMAND_LENGTH,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Hotfix: Only global app can set default.
    // FIXME: is this ok? 2.x: added is_app_admin here.
    $default = ($is_app_admin ? $this->get_param_value(self::DEFAULT_) : FALSE);

    $name = $this->get_param_value(self::NAME);
    $version = $this->get_param_value(self::VERSION);
    $tool = $this->get_param_value(self::TOOL);
    $file_extension = $this->get_param_value(self::FILE_EXTENSION);
    $command = $this->get_param_value(self::COMMAND);

    // Create the profile.
    $profile_id = mediamosa_transcode_profile::create($app_id, $name, $version, $default, $tool, $file_extension, $command);

    // Add to response.
    $mediamosa->add_item(array('profile_id' => $profile_id));
  }
}

/**
 * URI: /transcode/profile/$profile_id
 * Method: POST
 */
class mediamosa_rest_call_transcode_profile_update extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const PROFILE_ID = 'profile_id';
  const REPLACE_ = 'replace';
  const NAME = 'name';
  const TOOL = 'tool';
  const DEFAULT_ = 'default';
  const VERSION = 'version';
  const FILE_EXTENSION = 'file_extension';
  const COMMAND = 'command';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the profile to update.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::REPLACE_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Is replace.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::NAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The name of the profile.',
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::PROFILE_LENGTH,
        ),
        self::TOOL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The tool used for the profile.',
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::TOOL_LENGTH,
        ),
        self::DEFAULT_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Is the default profile.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::VERSION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The version number.',
        ),
        self::FILE_EXTENSION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The file extension.',
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::FILE_EXTENSION_LENGTH,
        ),
        self::COMMAND => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The command.',
          self::VAR_RANGE_END => mediamosa_transcode_profile_db::COMMAND_LENGTH,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  /**
   * Process the supplied parameters.
   *
   * @param array $var_setup
   *   The variable description of the REST call.
   */
  protected function process_rest_args(array $var_setup) {
    $var_setup = parent::process_rest_args($var_setup);
    $replace = $this->get_param_value(self::REPLACE_);

    if ($replace) {
      $var_setup[self::VARS][self::NAME][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $var_setup[self::VARS][self::TOOL][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $var_setup[self::VARS][self::VERSION][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $var_setup[self::VARS][self::FILE_EXTENSION][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $var_setup[self::VARS][self::COMMAND][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
    }

    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Hotfix: Only global app can set default.
    // FIXME: is this ok? 2.x: added is_app_admin here.
    $default = ($is_app_admin ? $this->get_param_value(self::DEFAULT_) : FALSE);

    $profile_id = $this->get_param_value(self::PROFILE_ID);
    $name = $this->get_param_value(self::NAME);
    $version = $this->get_param_value(self::VERSION);
    $tool = $this->get_param_value(self::TOOL);
    $file_extension = $this->get_param_value(self::FILE_EXTENSION);
    $command = $this->get_param_value(self::COMMAND);

    // Create the profile.
    mediamosa_transcode_profile::update($app_id, $profile_id, $name, $version, $default, $tool, $file_extension, $command);

    // Add to response.
    $o_mediamosa->add_item(array('profile_id' => $profile_id));
  }
}

/**
 * URI: /transcode/profile/$profile_id/delete
 * Method: POST
 */
class mediamosa_rest_call_transcode_profile_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const PROFILE_ID = 'profile_id';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the profile to delete.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Get Profile ID.
    $profile_id = $this->get_param_value(self::PROFILE_ID);

    // Delete it.
    mediamosa_transcode_profile::delete($app_id, $profile_id);

    // Set Ok.
    $mediamosa->set_result_okay();
  }
}
