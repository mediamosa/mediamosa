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
 * REST call user module.
 */

/**
 * URI: /user/$user_id
 *
 * Method: GET
 *
 * 1.x: user_management_get_user
 *
 */
class mediamosa_rest_call_user_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
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

    $user_id = $this->get_param_value(self::USER_ID);

    // Get the user quota.
    $a_result = mediamosa_user::get_user_quota($app_id, $user_id);

    // Add to response.
    $o_mediamosa->add_item($a_result);
  }
}

/**
 * URI: /user
 *
 * Method: GET
 *
 * 1.x: user_management_list_user
 *
 */
class mediamosa_rest_call_user_list extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Name of the group where the user has to be a member.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
      )
    );

    // Enrich with range.
    $a_var_setup = self::get_var_setup_range($a_var_setup);

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $group_id = $this->get_param_value(self::GROUP_ID);
    $offset = $this->get_param_value_offset();
    $limit = $this->get_param_value_limit();

    // Fetch the items.
    $result = mediamosa_user::get_all(TRUE, $app_id, $offset, $limit, $group_id);

    // Fill the response.
    foreach ($result as $a_row) {
      $o_mediamosa->add_item($a_row);
    }

    // Set total count.
    $o_mediamosa->item_count_total = $result->found_rows;
  }
}

/**
 * URI: /user/create
 *
 * Method: POST
 *
 * 1.x: user_management_create_user
 *
 */
class mediamosa_rest_call_user_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user';
  const GROUP_ID = 'group_id';
  const QUOTUM = 'quotum';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User name.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Name of the group the user belongs to. A user can only be member of 1 group, and the group must exist (see /group/create [POST])',
          self::VAR_RANGE_END => mediamosa_user_db::GROUP_ID_LENGTH,
          self::VAR_DEFAULT_VALUE => '',
        ),
        self::QUOTUM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Maximum disk space of the user in mega bytes. A quotum of 0 means unlimited diskspace. Quota is is enforced with a soft-quota, a user is allowed to exceed the maximum, but the next action is rejected.',
          self::VAR_DEFAULT_VALUE => 0,
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

    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $quotum = $this->get_param_value(self::QUOTUM);

    if ($this->isset_given_param(self::GROUP_ID)) {
      mediamosa_db::db_must_exists(mediamosa_user_group_db::TABLE_NAME, array(mediamosa_user_group_db::ID => $group_id, mediamosa_user_group_db::APP_ID => $app_id));
    }

    // Make sure it does not exist.
    if (mediamosa_db::db_count_rows(mediamosa_user_db::TABLE_NAME, array(mediamosa_user_db::ID => $user_id, mediamosa_user_group_db::APP_ID => $app_id))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_USER_EXISTS);
    }

    // Create the user.
    mediamosa_user::create($app_id, $user_id, $quotum, $group_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /user/$user_id
 *
 * Method: GET
 *
 * 1.x: user_management_delete_user
 *
 */
class mediamosa_rest_call_user_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user to delete.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
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

    $user_id = $this->get_param_value(self::USER_ID);

    // Must exist.
    mediamosa_db::db_must_exists(mediamosa_user_db::TABLE_NAME, array(mediamosa_user_db::ID => $user_id, mediamosa_user_db::APP_ID => $app_id));

    // Delete the user.
    mediamosa_user::delete($app_id, $user_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /user/$user_id
 *
 * Method: POST
 *
 * 1.x: user_management_update_user
 *
 */
class mediamosa_rest_call_user_update extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const QUOTUM = 'quotum';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User name.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Name of the group where the user will be.',
          self::VAR_RANGE_END => mediamosa_user_db::GROUP_ID_LENGTH,
        ),
        self::QUOTUM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Disk space of the user in megabytes.',
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

    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $quotum = $this->get_param_value(self::QUOTUM);

    // Either one must be given.
    if (!$this->isset_given_param(self::GROUP_ID) && !$this->isset_given_param(self::QUOTUM)) {
      throw new mediamosa_exception_error_empty_result();
    }

    // Must exist.
    mediamosa_db::db_must_exists(mediamosa_user_db::TABLE_NAME, array(mediamosa_user_db::ID => $user_id, mediamosa_user_db::APP_ID => $app_id));

    // Must exist.
    if ($this->isset_given_param(self::GROUP_ID)) {
      mediamosa_db::db_must_exists(mediamosa_user_group_db::TABLE_NAME, array(mediamosa_user_group_db::ID => $group_id, mediamosa_user_group_db::APP_ID => $app_id));
    }

    // Update the user.
    mediamosa_user::update($app_id, $user_id, $quotum, $group_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /user/$user_id/set_group
 * (deprecated)
 *
 * Method: POST
 *
 * 1.x: user_management_set_group
 *
 */
class mediamosa_rest_call_user_set_group extends mediamosa_rest_call_user_update {
}

/**
 * URI: /user/$user_id/set_quotum
 * (deprecated)
 *
 * Method: POST
 *
 * 1.x: user_management_set_user_quotum
 *
 */
class mediamosa_rest_call_user_set_quotum extends mediamosa_rest_call_user_update {
}
