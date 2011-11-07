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
  *
  */

/**
 * @file
 * FTP users module
 */

/* FTP settings
 */
define('MEDIAMOSA_FTP_CREDENTIAL_LENGTH', 8);

/**
 * URI: /ftp
 *      /ftp/list (deprecated)
 *
 * Method: GET
 *
 * 1.x: ftp_user_list
 *
 */
class mediamosa_rest_call_ftp_user_list extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_BATCH_UPLOAD, $a_app_ids);

    $result = mediamosa_db::db_query('SELECT * FROM {mediamosa_ftp_user} WHERE app_id = :app_id', array(':app_id' => $app_id));

    foreach ($result as $a_user) {
      $mediamosa->add_item(
        array(
          'userid' => $a_user['userid'],
          'active' => intval($a_user['active']) ? 'true' : 'false',
          'modified' => mediamosa::utcdate2appdate($a_user['modified']),
        )
      );
    }
  }
}

/**
 * URI: /ftp/$user/create
 *      /ftp/create
 *
 * Method: POST
 *
 * 1.x: ftp_user_create
 *
 */
class mediamosa_rest_call_ftp_user_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER = 'user';
  const PASSWORD = 'password';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::PASSWORD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The password.',
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

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $username = $this->get_param_value(self::USER);
    $password = $this->get_param_value(self::PASSWORD);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_BATCH_UPLOAD, $a_app_ids);

    $password = '{md5}' . base64_encode(md5($password, TRUE));

    $eua_name = sprintf("%03d", $app_id);
    if (mediamosa_unicode::substr($username, -3) != $eua_name) {
      $username .= $eua_name;
    }

    if ((mediamosa_unicode::strlen($username) < MEDIAMOSA_FTP_CREDENTIAL_LENGTH) || (mediamosa_unicode::strlen($password) < MEDIAMOSA_FTP_CREDENTIAL_LENGTH)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FTP_CREDENTIAL_LENGTH);
    }

    // Create FTP dir.
    $user_home_dir = mediamosa_configuration_storage::mount_point_get() . '/' . mediamosa_configuration_storage::ftp_location_get() . '/' . $username;

    try {
      mediamosa_io::mkdir($user_home_dir, TRUE, 0775);
    }
    catch (mediamosa_exception $e) {
      // Ignore.
    }
    // Create entry.
    mediamosa_db::db_query(
      'INSERT INTO {mediamosa_ftp_user} SET app_id = :app_id, userid = :username, passwd = :password, homedir = :homedir, active = 1, count = 0, accessed = UTC_TIMESTAMP(), modified = UTC_TIMESTAMP()',
      array(
        ':app_id' => $app_id,
        ':username' => $username,
        ':password' => $password,
        ':homedir' => $user_home_dir,
      )
    );

    // Set OK
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /ftp/$user/delete
 *
 * Method: POST
 *
 * 1.x: ftp_user_delete
 *
 */
class mediamosa_rest_call_ftp_user_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER = 'user';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user.',
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

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $user = $this->get_param_value(self::USER);

    $eua_name = sprintf("%03d", $app_id);
    if (mediamosa_unicode::substr($user, -3) != $eua_name) {
      $user .= $eua_name;
    }

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_BATCH_UPLOAD, $a_app_ids);

    mediamosa_db::db_query(
      'DELETE FROM {mediamosa_ftp_user} WHERE userid = :userid && app_id = :app_id',
      array(
        ':userid' => $user,
        ':app_id' => $app_id
      )
    );

    // Set OK
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /ftp/$user
 *      /ftp/$user/update
 *
 * Method: POST
 *
 * 1.x: ftp_user_update
 *
 */
class mediamosa_rest_call_ftp_user_update extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER = 'user';
  const PASSWORD = 'password';
  const ACTIVE = 'active';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::PASSWORD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The password.',
        ),
        self::ACTIVE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Active state.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $user = $this->get_param_value(self::USER);
    $eua_name = sprintf("%03d", $app_id);
    if (mediamosa_unicode::substr($user, -3) != $eua_name) {
      $user .= $eua_name;
    }

    $password = $this->get_param_value(self::PASSWORD);
    $active = $this->get_param_value(self::ACTIVE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_BATCH_UPLOAD, $a_app_ids);

    if (!$this->isset_given_param(self::PASSWORD) && !$this->isset_given_param(self::ACTIVE)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_CHANGES);
    }

    // Must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_user_db::TABLE_NAME, array('userid' => $user));

    $query = mediamosa_db::db_update(mediamosa_ftp_user_db::TABLE_NAME);
    $query->condition('userid', $user);
    $query->condition('app_id', $app_id);

    $a_fields = array(
      'modified' => mediamosa_datetime::utc_current_timestamp_now()
    );

    if (is_bool($active)) {
      $a_fields['active'] = $active ? 1 : 0;
    }

    if ($password) {
      $a_fields['passwd'] = '{md5}' . base64_encode(md5($password, TRUE));
    }

    // Add changed field.
    $a_fields['modified'] = mediamosa_datetime::utc_current_timestamp_now();

    $query->fields($a_fields);
    $query->execute();

    // Set OK
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /ftp/$user
 *
 *
 * Method: GET
 *
 * 1.x: ftp_user_get
 *
 */
class mediamosa_rest_call_ftp_user_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER = 'user';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user.',
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

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $user = $this->get_param_value(self::USER);

    $eua_name = sprintf("%03d", $app_id);
    if (mediamosa_unicode::substr($user, -3) != $eua_name) {
      $user .= $eua_name;
    }

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_BATCH_UPLOAD, $a_app_ids);

    $query = mediamosa_db::db_select(mediamosa_ftp_user_db::TABLE_NAME, 'fu');
    $query->fields('fu', array('userid', 'active', 'modified'));
    $query->condition('fu.userid', $user);
    $query->condition('fu.app_id', $app_id);
    $a_user = $query->execute()->fetchAssoc();

    if (!$a_user) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FTP_UNKNOWN_USER);
    }

    $o_mediamosa->add_item(array(
      'userid' => $a_user['userid'],
      'active' => intval($a_user['active']) ? 'true' : 'false',
      'modified' => mediamosa::utcdate2appdate($a_user['modified']),
    ));
  }
}

/**
 * URI: /ftp/$user/change_password
 * (deprecated)
 *
 * Method: POST
 *
 * 1.x: ftp_user_change_password
 *
 */
class mediamosa_rest_call_ftp_user_update_password extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER = 'user';
  const PASSWORD = 'password';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::PASSWORD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The password.',
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

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $user = $this->get_param_value(self::USER);
    $eua_name = sprintf("%03d", $app_id);
    if (mediamosa_unicode::substr($user, -3) != $eua_name) {
      $user .= $eua_name;
    }

    $password = $this->get_param_value(self::PASSWORD);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_BATCH_UPLOAD, $a_app_ids);

    // Must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_user_db::TABLE_NAME, array('userid' => $user));

    $query = mediamosa_db::db_update(mediamosa_ftp_user_db::TABLE_NAME);
    $query->condition('userid', $user);
    $query->condition('app_id', $app_id);

    $a_fields = array(
      'modified' => mediamosa_datetime::utc_current_timestamp_now(),
      'passwd' => '{md5}' . base64_encode(md5($password, TRUE)),
    );

    $query->fields($a_fields);
    $query->execute();

    // Set OK
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /ftp/$user/change_status
 * (deprecated)
 *
 * Method: POST
 *
 * 1.x: ftp_user_change_status
 *
 */
class mediamosa_rest_call_ftp_user_update_status extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER = 'user';
  const ACTIVE = 'active';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ACTIVE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Active state.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $user = $this->get_param_value(self::USER);
    $eua_name = sprintf("%03d", $app_id);
    if (mediamosa_unicode::substr($user, -3) != $eua_name) {
      $user .= $eua_name;
    }

    $active = $this->get_param_value(self::ACTIVE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_BATCH_UPLOAD, $a_app_ids);

    // Must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_user_db::TABLE_NAME, array('userid' => $user));

    $query = mediamosa_db::db_update(mediamosa_ftp_user_db::TABLE_NAME);
    $query->condition('userid', $user);
    $query->condition('app_id', $app_id);

    $a_fields = array(
      'modified' => mediamosa_datetime::utc_current_timestamp_now()
    );

    if (is_bool($active)) {
      $a_fields['active'] = $active ? 1 : 0;
    }

    $query->fields($a_fields);
    $query->execute();

    // Set OK
    $o_mediamosa->set_result_okay();
  }
}
