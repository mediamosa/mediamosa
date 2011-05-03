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
 * Transcode profile REST calls.
 */


/**
 * URI: /transcode/profile
 *      /video/transcode_profiles (deprecated)
 *      /transcode/profiles (deprecated)
 * Method: GET
 *
 * 1.x: _transcoding_profiles_get_transcode_list.
 *
 */
class mediamosa_rest_call_transcode_profile_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Get the listing.
    $a_transcode_profiles = mediamosa_transcode_profile::search($app_id);

    foreach ($a_transcode_profiles as $a_transcode_profile) {
      $a_item = array();
      $a_item['profile_id'] = $a_transcode_profile[mediamosa_transcode_profile_db::ID];
      $a_item[mediamosa_transcode_profile_db::PROFILE] = $a_transcode_profile[mediamosa_transcode_profile_db::PROFILE];
      $a_item['default'] = $a_transcode_profile[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE];
      $a_item['global'] = $a_transcode_profile[mediamosa_transcode_profile_db::APP_ID] > 0 ? 'FALSE' : 'TRUE';

      // UTC -> App timezone.
      if (isset($a_item[mediamosa_transcode_profile_db::CREATED])) {
        $a_item[mediamosa_transcode_profile_db::CREATED] = mediamosa::utcdate2appdate($a_item[mediamosa_transcode_profile_db::CREATED]);
      }
      if (isset($a_item[mediamosa_transcode_profile_db::CHANGED])) {
        $a_item[mediamosa_transcode_profile_db::CHANGED] = mediamosa::utcdate2appdate($a_item[mediamosa_transcode_profile_db::CHANGED]);
      }

      $o_mediamosa->add_item($a_item);
    }
  }
}

/**
 * URI: /transcode/profile/$profile_id
 * Method: GET
 *
 * 1.x: _transcoding_profiles_get_transcode.
 *
 */
class mediamosa_rest_call_transcode_profile_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const PROFILE_ID = 'profile_id';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the profile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $profile_id = $this->get_param_value(self::PROFILE_ID);

    // Get the listing.
    $a_profile = mediamosa_transcode_profile::get($profile_id, $app_id);

    // Add when found.
    if ($a_profile) {
      $transcode = array();
      $transcode['profile_id'] = $a_profile[mediamosa_transcode_profile_db::ID];
      $transcode['profile_name'] = $a_profile[mediamosa_transcode_profile_db::PROFILE];
      $transcode['default'] = $a_profile[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE];
      $transcode['global'] = $a_profile[mediamosa_transcode_profile_db::APP_ID] > 0 ? 'FALSE' : 'TRUE';
      $transcode['file_extension'] = $a_profile[mediamosa_transcode_profile_db::FILE_EXTENSION];
      $transcode['created'] = mediamosa::utcdate2appdate($a_profile[mediamosa_transcode_profile_db::CREATED]);
      $transcode['changed'] = mediamosa::utcdate2appdate($a_profile[mediamosa_transcode_profile_db::CHANGED]);
      $transcode['version'] = $a_profile[mediamosa_transcode_profile_db::VERSION];
      $transcode['command'] = $a_profile[mediamosa_transcode_profile_db::COMMAND];
      $transcode['tool'] = $a_profile[mediamosa_transcode_profile_db::TOOL];

      $o_mediamosa->add_item($transcode);
    }
  }
}

/**
 * URI: /transcode/profile/create
 * Method: POST
 *
 * 1.x: _transcoding_profiles_create_transcode.
 *
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
    $a_var_setup = array();

    $a_var_setup = array(
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
    return self::get_var_setup_default($a_var_setup);
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

    $name = $this->get_param_value(self::NAME);
    $version = $this->get_param_value(self::VERSION);
    $tool = $this->get_param_value(self::TOOL);
    $file_extension = $this->get_param_value(self::FILE_EXTENSION);
    $command = $this->get_param_value(self::COMMAND);

    // Create the profile.
    $profile_id = mediamosa_transcode_profile::create($app_id, $name, $version, $default, $tool, $file_extension, $command);

    // Add to response.
    $o_mediamosa->add_item(array('profile_id' => $profile_id));
  }
}

/**
 * URI: /transcode/profile/$profile_id
 * Method: POST
 *
 * 1.x: _transcoding_profiles_update_transcode.
 *
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
    $a_var_setup = array();

    $a_var_setup = array(
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
    return self::get_var_setup_default($a_var_setup);
  }

  /**
   * Process the supplied parameters.
   *
   * @param array $a_var_setup
   */
  protected function process_rest_args(array $a_var_setup) {
    $a_var_setup = parent::process_rest_args($a_var_setup);
    $replace = $this->get_param_value(self::REPLACE_);

    if ($replace) {
      $a_var_setup[self::VARS][self::NAME][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $a_var_setup[self::VARS][self::TOOL][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $a_var_setup[self::VARS][self::VERSION][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $a_var_setup[self::VARS][self::FILE_EXTENSION][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $a_var_setup[self::VARS][self::COMMAND][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
    }

    return $a_var_setup;
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
 *
 * 1.x: _transcoding_profiles_delete_transcode.
 */
class mediamosa_rest_call_transcode_profile_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const PROFILE_ID = 'profile_id';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the profile to delete.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Get Profile ID.
    $profile_id = $this->get_param_value(self::PROFILE_ID);

    // Delete it.
    mediamosa_transcode_profile::delete($app_id, $profile_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}
