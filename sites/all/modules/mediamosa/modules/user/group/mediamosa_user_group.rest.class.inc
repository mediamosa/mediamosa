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
 * User group specific REST calls.
 */
/**
 * URI: /group/$group_id
 *
 * Method: GET
 *
 * 1.x: user_management_get_group
 *
 */
class mediamosa_rest_call_group_get extends mediamosa_rest_call {
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
          self::VAR_DESCRIPTION => 'The group to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
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

    $group_id = $this->get_param_value(self::GROUP_ID);

    // Get the group quota.
    $result = mediamosa_user_group::get_user_group_quota($app_id, $group_id);

    if (isset($result[mediamosa_user_group_db::CREATED])) {
      $result[mediamosa_user_group_db::CREATED] = mediamosa::utcdate2appdate($result[mediamosa_user_group_db::CREATED]);
    }

    if (isset($result[mediamosa_user_group_db::CHANGED])) {
      $result[mediamosa_user_group_db::CHANGED] = mediamosa::utcdate2appdate($result[mediamosa_user_group_db::CHANGED]);
    }

    // Add to response.
    $o_mediamosa->add_item($result);
  }
}

/**
 * URI: /group
 *
 * Method: GET
 *
 * 1.x: user_management_list_group
 *
 */
class mediamosa_rest_call_group_list extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

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

    $offset = $this->get_param_value_offset();
    $limit = $this->get_param_value_limit();

    // Fetch the items.
    $result = mediamosa_user::get_all(FALSE, $app_id, $offset, $limit);

    // Fill the response.
    foreach ($result as $row) {
      $o_mediamosa->add_item($row);
    }

    // Set total count.
    $o_mediamosa->item_count_total = $result->found_rows;
  }
}

/**
 * URI: /group/create
 *
 * Method: POST
 *
 * 1.x: user_management_create_group
 *
 */
class mediamosa_rest_call_group_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const QUOTUM = 'quotum';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group name.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::QUOTUM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Disk space of the group in mega bytes. 0 means unlimited.',
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

    $group_id = $this->get_param_value(self::GROUP_ID);
    $quotum = $this->get_param_value(self::QUOTUM);

    // Make sure it does not exist.
    if (mediamosa_db::db_count_rows(mediamosa_user_group_db::TABLE_NAME, array(mediamosa_user_group_db::ID => $group_id, mediamosa_user_group_db::APP_ID => $app_id))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_GROUP_EXISTS);
    }

    // Create the group.
    mediamosa_user_group::create($app_id, $group_id, $quotum);

    // Set ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /group/$group_id/delete
 *
 * Method: POST
 *
 * 1.x: user_management_delete_group
 *
 */
class mediamosa_rest_call_group_delete extends mediamosa_rest_call {
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
          self::VAR_DESCRIPTION => 'The group name.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
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

    $group_id = $this->get_param_value(self::GROUP_ID);

    // Must exist to delete.
    mediamosa_db::db_must_exists(mediamosa_user_group_db::TABLE_NAME, array(mediamosa_user_group_db::ID => $group_id, mediamosa_user_group_db::APP_ID => $app_id));

    $group_key = mediamosa_user_group::key($app_id, $group_id);

    // Must be empty.
    if (mediamosa_db::db_count_rows(mediamosa_user_in_group_db::TABLE_NAME,
            array(mediamosa_user_in_group_db::GROUP_KEY => $group_key))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_GROUP_NOT_EMPTY);
    }

    // Delete the group.
    mediamosa_user_group::delete($app_id, $group_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /group/$group_id
 *      /group/$group_id/set_quotum
 * Method: POST
 *
 * 1.x: user_management_set_group_quotum
 *
 */
class mediamosa_rest_call_group_update extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const QUOTUM = 'quotum';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group name.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::QUOTUM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Disk space of the user in megabytes.',
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

    $group_id = $this->get_param_value(self::GROUP_ID);
    $quotum = $this->get_param_value(self::QUOTUM);

    // Must exist.
    mediamosa_db::db_must_exists(mediamosa_user_group_db::TABLE_NAME, array(mediamosa_user_group_db::ID => $group_id, mediamosa_user_group_db::APP_ID => $app_id));

    // Update the group.
    mediamosa_user_group::update($app_id, $group_id, $quotum);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}
