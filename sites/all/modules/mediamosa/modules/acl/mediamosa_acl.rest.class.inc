<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Helper to fix acl_ rename to 2.x version (aut_).

 * @param array $row
 */
function fix_vars_acl_2_aut($row) {
  // Is v3 output?
  if (mediamosa::is_v3_output()) {
    return $row;
  }

  // Its a name.
  if (!is_array($row)) {
    return (substr($row, 0, 4) == 'acl_' ? 'aut_' . substr($row, 4) : $row);
  }

  foreach ($row as $key => $value) {
    if (substr($key, 0, 4) != 'acl_') {
      continue;
    }

    $row['aut_' . substr($key, 4)] = $value;
    unset($row[$key]); // Remove ACL version.
  }

  return $row;
}


/**
 * URI: /autorisation_group
 * Method: GET
 *
 * @param array $a_args
 * @return object
 */
class mediamosa_rest_call_acl_group_list extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    // Enrich with limit and offset vars.
    $var_setup = $this->get_var_setup_range();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $offset = $this->get_param_value_offset();
    $limit = $this->get_param_value_limit();

    // Get the list.
    $result = mediamosa_acl_group::get_list($app_ids, $offset, $limit);

    // Total rows?
    $mediamosa->item_count_total = $result->found_rows;

    // Set result.
    foreach ($result as $row) {
      $mediamosa->add_item(fix_vars_acl_2_aut($row));
    }
  }
}

/**
 * URI: /autorisation_group/$group_name
 * Method: GET
 */
class mediamosa_rest_call_acl_group_get extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The group name to retrieve',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);

    // Get the list.
    $row = mediamosa_acl_group::get_by_group_name($app_ids, $group_name);
    if (!$row) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ACL_GROUP_NOT_FOUND, array('@group' => $group_name));
    }

    // Add to result.
    $mediamosa->add_item(fix_vars_acl_2_aut($row));
  }
}

/**
 *  URI: /autorisation_group/$groupname/hostname
 *  Method: GET
 */
class mediamosa_rest_call_acl_group_get_hostnames extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const LIMIT = 'limit';
  const OFFSET = 'offset';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    // Enrich with limit and offset vars.
    $var_setup = $this->get_var_setup_range();

    $var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);

    $offset = $this->get_param_value_offset();
    $limit = $this->get_param_value_limit();

    // Get the group.
    $row = mediamosa_acl_group::get_by_group_name($app_ids, $group_name);
    if (!$row) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ACL_GROUP_NOT_FOUND, array('@group' => $group_name));
    }

    // Get the group.
    $result = mediamosa_acl_group::get_hostnames_by_group_name($app_ids, $group_name, $offset, $limit);

    // Add to result.
    foreach ($result as $row) {
      $row['hostname'] = $row['acl_name'];
      unset($row['acl_name']);
      $mediamosa->add_item(fix_vars_acl_2_aut($row));
    }

    $mediamosa->item_count_total = $result->found_rows;
  }
}

/**
 * URI: /autorisation_group/$groupname/hostname/create
 * Method: POST
 */
class mediamosa_rest_call_acl_group_create_hostnames extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const HOSTNAME = 'hostname';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group, which shared the hostname(s).',
    );
    $var_setup[mediamosa_rest_call::VARS][self::HOSTNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => "The hostname that the group should be linked.\n\nIn case of multiple hostnames you can using the '[]' notation",
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $host_names = $this->get_param_value(self::HOSTNAME);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Get the group.
    $result = mediamosa_acl_group::create_hostnames_for_group($app_id, $group_name, $host_names);

    foreach ($result as $row) {
      $mediamosa->add_item(fix_vars_acl_2_aut($row));
    }
  }
}

/**
 * URI: /autorisation_group/$groupname/hostname/delete
 * Method: POST
 */
class mediamosa_rest_call_acl_group_delete_hostnames extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const HOSTNAME = 'hostname';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to delete from.',
    );
    $var_setup[mediamosa_rest_call::VARS][self::HOSTNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_IS_ARRAY => mediamosa_rest_call::VAR_IS_ARRAY_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the host to delete.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $host_names = $this->get_param_value(self::HOSTNAME);

    // First is always the main app.
    $app_id = reset($app_ids);

    $result = mediamosa_acl_group::delete_hostname_for_group($app_id, $group_name, $host_names);

    foreach ($result as $row) {
      $mediamosa->add_item(fix_vars_acl_2_aut($row));
    }
  }
}

/**
 * URI: /autorisation_group/create
 * Method: POST
 */
class mediamosa_rest_call_acl_group_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const GROUP_TYPE = 'group_type';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to create.',
    );
    $var_setup[mediamosa_rest_call::VARS][self::GROUP_TYPE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_ACL_GROUP_TYPE,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The type of the group to create.',
      mediamosa_rest_call::VAR_ALLOWED_VALUES => array(mediamosa_acl_group_db::ACL_GROUP_TYPE_DOMAIN, mediamosa_acl_group_db::ACL_GROUP_TYPE_REALM),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $group_type = $this->get_param_value(self::GROUP_TYPE);

    // First is always the main app.
    $app_id = reset($app_ids);

    if (mediamosa_db::db_exists(
      mediamosa_acl_group_db::TABLE_NAME,
      array(
        mediamosa_acl_group_db::APP_ID => $app_id,
        mediamosa_acl_group_db::ACL_GROUP_NAME => $group_name
      )
    )) {
      throw new mediamosa_exception_error_acl_group_already_exists(array('@group' => $group_name));
    }

    // Create group.
    mediamosa_acl_group::create($app_id, $group_name, $group_type);

    // Get created group.
    $row = mediamosa_acl_group::get_by_group_name($app_ids, $group_name);

    // Add created group.
    $mediamosa->add_item(fix_vars_acl_2_aut($row));
  }
}

/**
 * URI: /autorisation_group/$groupname
 * Method: POST
 */
class mediamosa_rest_call_acl_group_update extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const GROUP_TYPE = 'group_type';
  const NEW_GROUPNAME = 'new_groupname';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to update.',
    );
    $var_setup[mediamosa_rest_call::VARS][self::GROUP_TYPE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_ACL_GROUP_TYPE,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The type of the group.',
    );
    $var_setup[mediamosa_rest_call::VARS][self::NEW_GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The new name of the group.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $new_group_type = $this->get_param_value(self::GROUP_TYPE);
    $new_group_name = $this->get_param_value(self::NEW_GROUPNAME);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Test of de groep bestaat
    mediamosa_db::db_must_exists(mediamosa_acl_group_db::TABLE_NAME, array(mediamosa_acl_group_db::APP_ID => $app_id, mediamosa_acl_group_db::ACL_GROUP_NAME => $group_name));

    // Fields to update.
    $a_fields = array();

    // If we rename, then the renamed group must not exist
    if ($this->isset_given_param(self::NEW_GROUPNAME) && $new_group_name != '') {
      if (mediamosa_db::db_exists(mediamosa_acl_group_db::TABLE_NAME, array(mediamosa_acl_group_db::APP_ID => $app_id, mediamosa_acl_group_db::ACL_GROUP_NAME => $new_group_name))) {
        throw new mediamosa_exception_error_acl_group_already_exists(array('@group' => $new_group_name));
      }

      $a_fields[mediamosa_acl_group_db::ACL_GROUP_NAME] = $new_group_name;
    }

    // Change of type.
    if ($this->isset_given_param(self::GROUP_TYPE) && $new_group_type != '') {

      $a_acl_group = mediamosa_acl_group::get_by_group_name($app_ids, $group_name);
      assert($a_acl_group);

      if (mediamosa_unicode::strtoupper($a_acl_group[mediamosa_acl_group_db::ACL_GROUP_TYPE]) != mediamosa_unicode::strtoupper($new_group_type)) {
        // If we change group type, then the group must be empty.
        if (mediamosa_db::db_count_rows(mediamosa_acl_name_db::TABLE_NAME, array(mediamosa_acl_name_db::ACL_GROUP_ID => $a_acl_group[mediamosa_acl_group_db::ID]))) {
          throw new mediamosa_exception_error_cant_retype_group_must_be_empty(array('@group' => $group_name));
        }

        $a_fields[mediamosa_acl_group_db::ACL_GROUP_TYPE] = $new_group_type;
      }
    }

    // If not empty, we update the row.
    if (!empty($a_fields)) {
      mediamosa_db::db_update(mediamosa_acl_group_db::TABLE_NAME)
        ->fields($a_fields)
        ->condition(mediamosa_acl_group_db::ACL_GROUP_NAME, $group_name)
        ->execute();
    }

    // All ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /autorisation_group/$groupname/delete
 * Method: POST
 */
class mediamosa_rest_call_acl_group_delete extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const GROUPNAME = 'groupname';
  const DELETE = 'delete';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[mediamosa_rest_call::VARS][self::GROUPNAME] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => 'The name of the group to delete.',
    );

    $var_setup[mediamosa_rest_call::VARS][self::DELETE] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_NO,
      mediamosa_rest_call::VAR_DESCRIPTION => 'Provide value "cascade" to delete also contents of group. Will fail if group is not empty and this value is not set to "cascade".',
      mediamosa_rest_call::VAR_ALLOWED_VALUES => array('', 'cascade'),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $group_name = $this->get_param_value(self::GROUPNAME);
    $delete = $this->get_param_value(self::DELETE);

    // First is always the main app.
    $app_id = reset($app_ids);

    mediamosa_db::db_must_exists(mediamosa_acl_group_db::TABLE_NAME, array(mediamosa_acl_group_db::APP_ID => $app_id, mediamosa_acl_group_db::ACL_GROUP_NAME => $group_name));

    // Get the Id of the group.
    $group_id = mediamosa_db::db_query(
      'SELECT acl_group_id FROM {mediamosa_acl_group} WHERE app_id = :app_id AND (acl_group_type IS NULL OR acl_group_type IS NOT NULL) AND acl_group_name = :acl_group_name',
      array(
        ':app_id' => $app_id,
        ':acl_group_name' => $group_name,
      )
    )->fetchField();
    assert($group_id);

    // Only with cascade will all be deleted without checks.
    if ($delete != 'cascade') {
      if (mediamosa_db::db_count_rows(mediamosa_acl_name_db::TABLE_NAME, array(mediamosa_acl_name_db::ACL_GROUP_ID => $group_id))) {
        throw new mediamosa_exception_error_group_not_empty(array('@group' => $group_name));
      }
    }

    // Remove link between acl_object and acl_name/group
    $a_acl_ids = mediamosa_db::db_query(
      'SELECT #acl_name_id FROM {#mediamosa_acl_name} WHERE #acl_group_id = :acl_group_id',
      array(
        '#mediamosa_acl_name' => mediamosa_acl_name_db::TABLE_NAME,
        '#acl_name_id' => mediamosa_acl_name_db::ID,
        '#acl_group_id' => mediamosa_acl_name_db::ACL_GROUP_ID,
        ':acl_group_id' => $group_id
      )
    )->fetchCol();

    // Remove all links between acl_object -> acl_name.
    if (!empty($a_acl_ids)) {
      mediamosa_db::db_query(
        'DELETE FROM {#mediamosa_acl_object} WHERE acl_type = :acl_type_name AND #acl_id IN(:acl_id)',
        array(
          '#mediamosa_acl_object' => mediamosa_acl_object_db::TABLE_NAME,
          '#acl_type' => mediamosa_acl_object_db::ACL_TYPE,
          ':acl_type_name' => mediamosa_acl_object_db::ACL_TYPE_NAME,
          '#acl_id' => mediamosa_acl_object_db::ACL_ID,
          ':acl_id' => $a_acl_ids,
        )
      );
    }

    // Remove all links from acl_object -> acl_group.
    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_acl_object} WHERE #acl_type = :acl_type_group AND #acl_id = :acl_id',
      array(
        '#mediamosa_acl_object' => mediamosa_acl_object_db::TABLE_NAME,
        '#acl_type' => mediamosa_acl_object_db::ACL_TYPE,
        ':acl_type_group' => mediamosa_acl_object_db::ACL_TYPE_GROUP,
        '#acl_id' => mediamosa_acl_object_db::ACL_ID,
        ':acl_id' => $group_id,
      )
    );

    // Remove all from acl_name.
    mediamosa_db::db_query(
      "DELETE FROM {#mediamosa_acl_name} WHERE #acl_group_id = :acl_group_id",
      array(
        '#mediamosa_acl_name' => mediamosa_acl_name_db::TABLE_NAME,
        '#acl_group_id' => mediamosa_acl_name_db::ACL_GROUP_ID,
        ':acl_group_id' => $group_id
      )
    );

    // Remove group.
    mediamosa_db::db_query(
      "DELETE FROM {#mediamosa_acl_group} WHERE #acl_group_id = :acl_group_id",
      array(
        '#mediamosa_acl_group' => mediamosa_acl_group_db::TABLE_NAME,
        '#acl_group_id' => mediamosa_acl_group_db::ID,
        ':acl_group_id' => $group_id
      )
    );

    // All ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/acl
 * Method: POST
 */
class mediamosa_rest_call_acl_mediafile_set_multi_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const ACL_APP = 'acl_app';
  const ACL_USER = 'acl_user';
  const ACL_GROUP = 'acl_group';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const ACL_USER_ID = 'acl_user_id';
  const ACL_GROUP_ID = 'acl_group_id';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_APP = 'aut_app';
  const ALIAS_AUT_USER = 'aut_user';
  const ALIAS_AUT_GROUP = 'aut_group';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::ASSET_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The asset IDs of mediafiles which to change the rights.',
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'Must match the owner fo the mediafile.',
      );

    $var_setup[self::VARS][self::REPLACE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 'TRUE',
      self::VAR_DESCRIPTION => "'TRUE' of 'FALSE' if the current right has to be replaced. Standard is TRUE. Define if 'FALSE' if you want to keep the old rights.",
    );

    $var_setup[self::VARS][self::ACL_APP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Application(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_APP),
    );

    $var_setup[self::VARS][self::ACL_USER] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'User(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER),
    );

    $var_setup[self::VARS][self::ACL_GROUP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Group(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP),
    );

    $var_setup[self::VARS][self::ACL_DOMAIN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Domain(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
    );

    $var_setup[self::VARS][self::ACL_REALM] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Realm(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Alias of acl_user.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_GROUP_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Alias of acl_group.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $asset_ids = $this->get_param_value('asset_id');
    $user_id = $this->get_param_value('user_id');

    // First is always the main app.
    $app_id = reset($app_ids);

    // Validate the asset_ids.
    $a_mediafile_ids = array();
    foreach ($asset_ids as $asset_id) {
      // Collect all the mediafile_id belonging to the asset_id.
      $a_mediafile_ids[$asset_id] = mediamosa_asset_mediafile::mediafiles_search($asset_id);
    }

    if (mediamosa::is_v3_output()) {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'acl_app',
        mediamosa_acl::ACL_NAME_TYPE_USER => 'acl_user',
        mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'acl_group',
        mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'acl_domain',
        mediamosa_acl::ACL_NAME_TYPE_REALM => 'acl_realm',
      );
    }
    else {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'aut_app',
        mediamosa_acl::ACL_NAME_TYPE_USER => 'aut_user',
        mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'aut_group',
        mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'aut_domain',
        mediamosa_acl::ACL_NAME_TYPE_REALM => 'aut_realm',
      );
    }

    // Set the rights.
    foreach ($asset_ids as $asset_id) {
      foreach ($a_mediafile_ids[$asset_id] as $mediafile_id) {
        // Change the ACL rule for the mediafile.

        $app_ids = $this->get_param_value('acl_app');
        $a_acl_user_ids = $this->get_param_value('acl_user');
        $a_acl_group_ids = $this->get_param_value('acl_group');
        $a_acl_domains = $this->get_param_value('acl_domain');
        $a_acl_realms = $this->get_param_value('acl_realm');
        $b_replace = $this->get_param_value('replace');

        // Get the media file.
        $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id, $app_id);

        // Fix for difference in acl_user and acl_user_id
        if (!count($a_acl_user_ids)) {
          // Hmmm no a_acl_user_ids, then try acl_user_id and set user_id if we find one
          $a_acl_user_ids = $this->get_param_value('acl_user_id');
          $this->set_param_value('acl_user', $a_acl_user_ids);
        }
        else {
          $this->set_param_value('acl_user_id', $a_acl_user_ids);
        }

        if (!count($a_acl_group_ids)) {
          // Hmmm no a_acl_group_ids, then try acl_group_id and set group_id if we find one
          $a_acl_group_ids = $this->get_param_value('acl_group_id');
          $this->set_param_value('acl_group', $a_acl_group_ids);
        }
        else {
          $this->set_param_value('acl_group_id', $a_acl_group_ids);
        }

        // Set access rights
        $result_rights_set = mediamosa_acl::rights_set($app_id, $user_id, mediamosa_acl::ACL_TYPE_MEDIAFILE, $a_mediafile, $app_ids, $a_acl_user_ids, $a_acl_group_ids, $a_acl_domains, $a_acl_realms, $b_replace);

        foreach ($result_rights_set as $error) {
          $type = (isset($types[$error['type']]) ? $types[$error['type']] : 'unknown');

          if ($error['a_error'] === FALSE) {
            $mediamosa->add_item(
              array(
                $type => array(
                  'asset_id' => $asset_id,
                  'mediafile_id' => $mediafile_id,
                  'value' => $error['value'],
                  'result' => mediamosa_response::SUCCESS,
                  'result_id' => mediamosa_error::ERRORCODE_OKAY,
                  'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
                )
              )
            );
          }
          else {
            $mediamosa->add_item(
              array(
                $type => array(
                  'asset_id' => $asset_id,
                  'mediafile_id' => $mediafile_id,
                  'value' => $error['value'],
                  'result' => mediamosa_response::ERROR,
                  'result_id' => $error['a_error']['code'],
                  'result_description' => $error['a_error']['message'],
                )
              )
            );
          }
        }
      }
    }
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl
 * Method: POST
 */
class mediamosa_rest_call_acl_mediafile_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const ACL_APP = 'acl_app';
  const ACL_USER = 'acl_user';
  const ACL_GROUP = 'acl_group';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const ACL_USER_ID = 'acl_user_id';
  const ACL_GROUP_ID = 'acl_group_id';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_APP = 'aut_app';
  const ALIAS_AUT_USER = 'aut_user';
  const ALIAS_AUT_GROUP = 'aut_group';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::MEDIAFILE_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The mediafile ID of which to change the rights.',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'Must match the owner fo the mediafile.',
      );

    $var_setup[self::VARS][self::REPLACE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 'TRUE',
      self::VAR_DESCRIPTION => "'TRUE' of 'FALSE' if the current right has to be replaced. Standard is TRUE. Define if 'FALSE' if you want to keep the old rights.",
    );

    $var_setup[self::VARS][self::ACL_APP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Application(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_APP),
    );

    $var_setup[self::VARS][self::ACL_USER] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'User(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER),
    );

    $var_setup[self::VARS][self::ACL_GROUP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Group(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP),
    );

    $var_setup[self::VARS][self::ACL_DOMAIN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Domain(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
    );

    $var_setup[self::VARS][self::ACL_REALM] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Realm(s) that gain access to the mediafile.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Alias of acl_user.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_GROUP_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Alias of acl_group.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $mediafile_id = $this->get_param_value('mediafile_id');
    $user_id = $this->get_param_value('user_id');

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the media file exists.
    mediamosa_asset_mediafile::must_exists_cached($mediafile_id, $app_id);

    $app_ids = $this->get_param_value('acl_app');
    $a_acl_user_ids = $this->get_param_value('acl_user');
    $a_acl_group_ids = $this->get_param_value('acl_group');
    $a_acl_domains = $this->get_param_value('acl_domain');
    $a_acl_realms = $this->get_param_value('acl_realm');
    $b_replace = $this->get_param_value('replace');

    // Get the media file.
    $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id, $app_id);

    // Fix for difference in acl_user and acl_user_id
    if (!count($a_acl_user_ids)) {
      // Hmmm no a_acl_user_ids, then try acl_user_id and set user_id if we find one
      $a_acl_user_ids = $this->get_param_value('acl_user_id');
      $this->set_param_value('acl_user', $a_acl_user_ids);
    }
    else {
      $this->set_param_value('acl_user_id', $a_acl_user_ids);
    }

    if (!count($a_acl_group_ids)) {
      // Hmmm no a_acl_group_ids, then try acl_group_id and set group_id if we find one
      $a_acl_group_ids = $this->get_param_value('acl_group_id');
      $this->set_param_value('acl_group', $a_acl_group_ids);
    }
    else {
      $this->set_param_value('acl_group_id', $a_acl_group_ids);
    }

    // Set access rights
    $result_rights_set = mediamosa_acl::rights_set($app_id, $user_id, mediamosa_acl::ACL_TYPE_MEDIAFILE, $a_mediafile, $app_ids, $a_acl_user_ids, $a_acl_group_ids, $a_acl_domains, $a_acl_realms, $b_replace);

    if (mediamosa::is_v3_output()) {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'acl_app',
        mediamosa_acl::ACL_NAME_TYPE_USER => 'acl_user',
        mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'acl_group',
        mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'acl_domain',
        mediamosa_acl::ACL_NAME_TYPE_REALM => 'acl_realm',
      );
    }
    else {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'aut_app',
        mediamosa_acl::ACL_NAME_TYPE_USER => 'aut_user',
        mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'aut_group',
        mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'aut_domain',
        mediamosa_acl::ACL_NAME_TYPE_REALM => 'aut_realm',
      );
    }

    foreach ($result_rights_set as $error) {
      $type = (isset($types[$error['type']]) ? $types[$error['type']] : 'unknown');

      if ($error['a_error'] === FALSE) {
        $mediamosa->add_item(
          array(
            $type => array(
              'value' => $error['value'],
              'result' => mediamosa_response::SUCCESS,
              'result_id' => mediamosa_error::ERRORCODE_OKAY,
              'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
            )
          )
        );
      }
      else {
        $mediamosa->add_item(
          array(
            $type => array(
              'value' => $error['value'],
              'result' => mediamosa_response::ERROR,
              'result_id' => $error['a_error']['code'],
              'result_description' => $error['a_error']['message'],
            )
          )
        );
      }
    }
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl/delete
 * Method: POST
 */
class mediamosa_rest_call_acl_mediafile_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::MEDIAFILE_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The mediafile ID where from to remove the ACL rights',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The owner of the mediafile.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the media file exists.
    mediamosa_asset_mediafile::must_exists_cached($mediafile_id, $app_id);

    // Get the media file
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id, $app_id);

    // Remove rights.
    mediamosa_acl::rights_clear($app_id, $user_id, mediamosa_acl::ACL_TYPE_MEDIAFILE, $mediafile);

    // All ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/acl
 * Method: GET
 */
class mediamosa_rest_call_acl_mediafile_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::MEDIAFILE_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The mediafile ID from which to receive the ACL rights',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'Must be the owner of the mediafile object to alter rights.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $mediafile_id = $this->get_param_value('mediafile_id');
    $user_id = $this->get_param_value('user_id');
    $is_app_admin = $this->get_param_value('is_app_admin');

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the media file exists.
    // Removed the app_id so the mediafile can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));

    // Get the media file
    // Removed the app_id so the mediafile can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    // Get rights.
    $rights = mediamosa_acl::rights_get($app_id, $user_id, mediamosa_acl::ACL_TYPE_MEDIAFILE, $mediafile, $is_app_admin);

    // Fill response.
    if (isset($rights[mediamosa_acl::OBJ_LINK_TYPE_NAME])) {
      foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_NAME] as $right) {
        $right['type'] = ($right['type'] == 'acl_user_group' ? 'acl_group' : $right['type']);
        $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['name']));
      }
    }

    if (isset($rights[mediamosa_acl::OBJ_LINK_TYPE_GROUP])) {
      foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_GROUP] as $right) {
        $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['name']));
      }
    }

    if (isset($rights[mediamosa_acl::OBJ_LINK_TYPE_APP])) {
      foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_APP] as $right) {
        $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['app_id_master']));
      }
    }
  }
}

/**
 * URI: asset/$asset_id/acl
 * Method: POST
 */
class mediamosa_rest_call_acl_asset_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const ACL_APP = 'acl_app';
  const ACL_USER = 'acl_user';
  const ACL_GROUP = 'acl_group';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const ACL_USER_ID = 'acl_user_id';
  const ACL_GROUP_ID = 'acl_group_id';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_APP = 'aut_app';
  const ALIAS_AUT_USER = 'aut_user';
  const ALIAS_AUT_GROUP = 'aut_group';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::ASSET_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The asset_id.',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'User ID, owner of the asset.',
    );

    $var_setup[self::VARS][self::REPLACE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 'TRUE',
      self::VAR_DESCRIPTION => '\'TRUE\' or \'FALSE\'. Is the rest call overwrite the existing rules? Default is TRUE. Defined as \'FALSE\' means the old rules will not be overwritten.',
    );

    $var_setup[self::VARS][self::ACL_APP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Application(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_APP),
    );

    $var_setup[self::VARS][self::ACL_USER] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'User(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER),
    );

    $var_setup[self::VARS][self::ACL_GROUP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Group(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP),
    );

    $var_setup[self::VARS][self::ACL_DOMAIN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Domain(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
    );

    $var_setup[self::VARS][self::ACL_REALM] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Realm(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Same as acl_user.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_GROUP_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Same as acl_group.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the asset exists
    $a_asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::APP_ID => $app_id, mediamosa_asset_db::ID => $asset_id));

    $a_acl_app_ids = $this->get_param_value(self::ACL_APP);
    $a_acl_user_ids = $this->get_param_value(self::ACL_USER);
    $a_acl_group_ids = $this->get_param_value(self::ACL_GROUP);
    $a_acl_domains = $this->get_param_value(self::ACL_DOMAIN);
    $a_acl_realms = $this->get_param_value(self::ACL_REALM);
    $b_replace = $this->get_param_value(self::REPLACE);

    // Fix for difference in acl_user and acl_user_id
    if (!count($a_acl_user_ids)) {
      // Hmmm no a_acl_user_ids, then try acl_user_id and set user_id if we find one
      $a_acl_user_ids = $this->get_param_value(self::ACL_USER_ID);
      $this->set_param_value(self::ACL_USER, $a_acl_user_ids);
    }
    else {
      $this->set_param_value(self::ACL_USER_ID, $a_acl_user_ids);
    }

    if (!count($a_acl_group_ids)) {
      // Hmmm no a_acl_group_ids, then try acl_group_id and set group_id if we find one
      $a_acl_group_ids = $this->get_param_value(self::ACL_GROUP_ID);
      $this->set_param_value(self::ACL_GROUP, $a_acl_group_ids);
    }
    else {
      $this->set_param_value(self::ACL_GROUP_ID, $a_acl_group_ids);
    }

    // Set access rights
    $result_rights_set = mediamosa_acl::rights_set($app_id, $user_id, mediamosa_acl::ACL_TYPE_ASSET, $a_asset, $a_acl_app_ids, $a_acl_user_ids, $a_acl_group_ids, $a_acl_domains, $a_acl_realms, $b_replace);

    if (mediamosa::is_v3_output()) {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'acl_app',
        mediamosa_acl::ACL_NAME_TYPE_USER => 'acl_user',
        mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'acl_group',
        mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'acl_domain',
        mediamosa_acl::ACL_NAME_TYPE_REALM => 'acl_realm',
      );
    }
    else {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'aut_app',
        mediamosa_acl::ACL_NAME_TYPE_USER => 'aut_user',
        mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'aut_group',
        mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'aut_domain',
        mediamosa_acl::ACL_NAME_TYPE_REALM => 'aut_realm',
      );
    }

    foreach ($result_rights_set as $error) {
      $type = (isset($types[$error['type']]) ? $types[$error['type']] : 'unknown');

      if ($error['a_error'] === FALSE) {
        $mediamosa->add_item(
          array(
            $type => array(
              'value' => $error['value'],
              'result' => mediamosa_response::SUCCESS,
              'result_id' => mediamosa_error::ERRORCODE_OKAY,
              'result_description' => '',
            )
          )
        );
      }
      else {
        $mediamosa->add_item(
          array(
            $type => array(
              'value' => $error['value'],
              'result' => mediamosa_response::ERROR,
              'result_id' => $error['a_error']['code'],
              'result_description' => $error['a_error']['message'],
            )
          )
        );
      }
    }
  }
}

/**
 * URI: asset/mediafile/acl
 * Method: POST
 */
class mediamosa_rest_call_acl_asset_mediafile_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const ACL_APP = 'acl_app';
  const ACL_USER = 'acl_user';
  const ACL_GROUP = 'acl_group';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const ACL_USER_ID = 'acl_user_id';
  const ACL_GROUP_ID = 'acl_group_id';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_APP = 'aut_app';
  const ALIAS_AUT_USER = 'aut_user';
  const ALIAS_AUT_GROUP = 'aut_group';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::ASSET_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'The asset_ids.',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'User ID, owner of the asset.',
    );

    $var_setup[self::VARS][self::REPLACE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 'TRUE',
      self::VAR_DESCRIPTION => '\'TRUE\' or \'FALSE\'. Is the rest call overwrite the existing rules? Default is TRUE. Defined as \'FALSE\' means the old rules will not be overwritten.',
    );

    $var_setup[self::VARS][self::ACL_APP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Application(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_APP),
    );

    $var_setup[self::VARS][self::ACL_USER] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'User(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER),
    );

    $var_setup[self::VARS][self::ACL_GROUP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Group(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP),
    );

    $var_setup[self::VARS][self::ACL_DOMAIN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Domain(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
    );

    $var_setup[self::VARS][self::ACL_REALM] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Realm(s) that get acces to the asset.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Alias of acl_user. If both are set, acl_user has priority.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
    );

    // alias.
    $var_setup[self::VARS][self::ACL_GROUP_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Alias of acl_group. If both are set, acl_group has priority.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $asset_ids = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($app_ids);

    $app_ids = $this->get_param_value('acl_app');
    $a_acl_user_ids = $this->get_param_value('acl_user');
    $a_acl_group_ids = $this->get_param_value('acl_group');
    $a_acl_domains = $this->get_param_value('acl_domain');
    $a_acl_realms = $this->get_param_value('acl_realm');
    $b_replace = $this->get_param_value('replace');

    // Fix for difference in acl_user and acl_user_id
    if (!count($a_acl_user_ids)) {
      // Hmmm no a_acl_user_ids, then try acl_user_id and set user_id if we find one
      $a_acl_user_ids = $this->get_param_value('acl_user_id');
      $this->set_param_value('acl_user', $a_acl_user_ids);
    }
    else {
      $this->set_param_value('acl_user_id', $a_acl_user_ids);
    }

    if (!count($a_acl_group_ids)) {
      // Hmmm no a_acl_group_ids, then try acl_group_id and set group_id if we find one
      $a_acl_group_ids = $this->get_param_value('acl_group_id');
      $this->set_param_value('acl_group', $a_acl_group_ids);
    }
    else {
      $this->set_param_value('acl_group_id', $a_acl_group_ids);
    }

    // Go through the asset ids.
    foreach ($asset_ids as $asset_id) {
      // Make sure the asset exists
      $a_asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::APP_ID => $app_id, mediamosa_asset_db::ID => $asset_id));

      // Get all of the mediafiles belonging to the asset.
      $result = mediamosa_db::db_query("
        SELECT m.#mediafile_id
        FROM {#mediamosa_asset} a
        JOIN {#mediamosa_asset_mediafile} m USING (#asset_id)
        WHERE a.#asset_id = :asset_id
      ", array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
      ));

      if (mediamosa::is_v3_output()) {
        $types = array(
          mediamosa_acl::ACL_NAME_TYPE_APP => 'acl_app',
          mediamosa_acl::ACL_NAME_TYPE_USER => 'acl_user',
          mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'acl_group',
          mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'acl_domain',
          mediamosa_acl::ACL_NAME_TYPE_REALM => 'acl_realm',
        );
      }
      else {
        $types = array(
          mediamosa_acl::ACL_NAME_TYPE_APP => 'aut_app',
          mediamosa_acl::ACL_NAME_TYPE_USER => 'aut_user',
          mediamosa_acl::ACL_NAME_TYPE_USER_GROUP => 'aut_group',
          mediamosa_acl::ACL_NAME_TYPE_DOMAIN => 'aut_domain',
          mediamosa_acl::ACL_NAME_TYPE_REALM => 'aut_realm',
        );
      }

      foreach ($result as $mediafile) {
        $mediafile_id = $mediafile['mediafile_id'];
        // Get the media file.
        $a_mediafile = mediamosa_asset_mediafile::get($mediafile_id, $app_id);

        // Set access rights
        $a_result_rights_set = mediamosa_acl::rights_set($app_id, $user_id, mediamosa_acl::ACL_TYPE_MEDIAFILE, $a_mediafile, $app_ids, $a_acl_user_ids, $a_acl_group_ids, $a_acl_domains, $a_acl_realms, $b_replace);

        foreach ($a_result_rights_set as $error) {
          $type = (isset($types[$error['type']]) ? $types[$error['type']] : 'unknown');

          if ($error['a_error'] === FALSE) {
            $mediamosa->add_item(
              array(
                'asset_id' => $asset_id,
                'mediafile_id' => $mediafile_id,
                'result' => mediamosa_response::SUCCESS,
                'result_id' => mediamosa_error::ERRORCODE_OKAY,
                'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
                $type => array(
                  'value' => $error['value'],
                  'result' => mediamosa_response::SUCCESS,
                  'result_id' => mediamosa_error::ERRORCODE_OKAY,
                  'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
                )
              )
            );
          }
          else {
            $mediamosa->add_item(
              array(
                'asset_id' => $asset_id,
                'mediafile_id' => $mediafile_id,
                'result' => mediamosa_response::ERROR,
                'result_id' => $error['a_error']['code'],
                'result_description' => $error['a_error']['message'],
                $type => array(
                  'value' => $error['value'],
                  'result' => mediamosa_response::ERROR,
                  'result_id' => $error['a_error']['code'],
                  'result_description' => $error['a_error']['message'],
                )
              )
            );
          }
        }
      }
    }
  }
}

/**
 * URI: /asset/$asset_id/acl/delete
 * Method: POST
 */
class mediamosa_rest_call_acl_asset_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::ASSET_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The asset ID where from to remove the ACL rights',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The owner of the asset for access.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the asset exists.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Get the media file.
    $asset = mediamosa_asset::get($asset_id, $app_id);

    // Remove rights.
    mediamosa_acl::rights_clear($app_id, $user_id, mediamosa_acl::ACL_TYPE_ASSET, $asset);

    // All ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/acl
 * Method: GET
 */
class mediamosa_rest_call_acl_asset_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::ASSET_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The ID of the asset to set the rights on.',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'This must be the owner ID of the asset.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the asset exists.
    // Removed the app_id so the asset can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $a_asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Get the rights on asset.
    $rights = mediamosa_acl::rights_get($app_id, $user_id, mediamosa_acl::ACL_TYPE_ASSET, $a_asset, $is_app_admin);

    foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_NAME] as $right) {
      $right['type'] = ($right['type'] == 'acl_user_group' ? 'acl_group' : $right['type']);
      $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['name']));
    }

    foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_GROUP] as $right) {
      $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['name']));
    }

    foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_APP] as $right) {
      $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['app_id_master']));
    }

    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /collection/$coll_id/acl/delete
 * Method: POST
 */
class mediamosa_rest_call_acl_collection_delete_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::COLL_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'Collection id, unique identifier of the collection.',

    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'User id, to determine whether the applicant has the right to remove authorization or not.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $coll_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the media file exists.
    mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::APP_ID => $app_id, mediamosa_collection_db::ID => $coll_id));

    // Get the media file
    $collection = mediamosa_collection::get($coll_id, $app_id);

    // Remove rights.
    mediamosa_acl::rights_clear($app_id, $user_id, mediamosa_acl::ACL_TYPE_COLLECTION, $collection);

    // All ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /collection/$coll_id/acl
 * Method: GET
 */
class mediamosa_rest_call_acl_collection_get_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::COLL_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The coll id.',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'The user id.',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $collection_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the collection exists.
    // Removed the app_id so the collection can be accessed by other app.
    // vpx_acl_rights_get() will check if its in the right app or you have access through slave.
    $collection = mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $collection_id));

    // Get the rights.
    $rights = mediamosa_acl::rights_get($app_id, $user_id, mediamosa_acl::ACL_TYPE_COLLECTION, $collection, $is_app_admin);

    foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_NAME] as $right) {
      $right['type'] = ($right['type'] == 'acl_user_group' ? 'acl_group' : $right['type']);
      $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['name']));
    }

    foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_GROUP] as $right) {
      $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['name']));
    }

    foreach ($rights[mediamosa_acl::OBJ_LINK_TYPE_APP] as $right) {
      $mediamosa->add_item(array(fix_vars_acl_2_aut($right['type']) => $right['app_id_master']));
    }
  }
}


/**
 * URI: collection/$coll_id/acl
 * Method: POST
 */
class mediamosa_rest_call_acl_collection_set_rights extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';
  const REPLACE = 'replace';
  const ACL_APP = 'acl_app';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_APP = 'aut_app';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::COLL_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'Collection ID',
    );

    $var_setup[self::VARS][self::USER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_DESCRIPTION => 'User id, for authorization the changing  process.',
    );

    $var_setup[self::VARS][self::REPLACE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 'true',
      self::VAR_DESCRIPTION => "'TRUE' or 'FALSE'. Whether the existing rules overwritten or not. Default is 'TRUE'. Defining it as 'FALSE', if you want to keep the old rights."
    );

    $var_setup[self::VARS][self::ACL_APP] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
      self::VAR_DESCRIPTION => 'Application(s) that access to the collection.',
      self::VAR_DEFAULT_VALUE => array(),
      self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_APP),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $collection_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // First is always the main app.
    $app_id = reset($app_ids);

    // Make sure the media file exists
    $a_collection = mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $collection_id));

    $a_acl_app_ids = $this->get_param_value(self::ACL_APP);
    $b_replace = $this->get_param_value(self::REPLACE);

    // Set access rights
    $result_rights_set = mediamosa_acl::rights_set(
      $app_id,
      $user_id,
      mediamosa_acl::ACL_TYPE_COLLECTION,
      $a_collection,
      $a_acl_app_ids,
      array(),
      array(),
      array(),
      array(),
      $b_replace,
      array(mediamosa_acl::RIGHT_ACCESS),
      $is_app_admin
    );

    if (mediamosa::is_v3_output()) {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'acl_app',
      );
    }
    else {
      $types = array(
        mediamosa_acl::ACL_NAME_TYPE_APP => 'aut_app',
      );
    }

    foreach ($result_rights_set as $error) {
      $type = (isset($types[$error['type']]) ? $types[$error['type']] : 'unknown');

      if ($error['a_error'] === FALSE) {
        $mediamosa->add_item(
          array(
            $type => array(
              'value' => $error['value'],
              'result' => mediamosa_response::SUCCESS,
              'result_id' => mediamosa_error::ERRORCODE_OKAY,
              'result_description' => '',
            )
          )
        );
      }
      else {
        $mediamosa->add_item(
          array(
            $type => array(
              'value' => $error['value'],
              'result' => mediamosa_response::ERROR,
              'result_id' => $error['a_error']['code'],
              'result_description' => $error['a_error']['message'],
            )
          )
        );
      }
    }
  }
}

/**
 * URI:
 *    acl/app
 *    acl/get_foreign_apps (deprecated)
 *
 * Method: GET
 */
class mediamosa_rest_call_acl_get_apps extends mediamosa_rest_call {

  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const IS_OAI = 'is_oai';
  const NAME = 'name';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::IS_OAI] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
      self::VAR_DESCRIPTION => 'The OAI is allowed to see all applications. However, the call must be made internally to work.',
      self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
      self::VAR_DEFAULT_VALUE => 'FALSE',
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app id(s).
    $app_ids = $this->get_param_value_app();
    $is_oai = $this->get_param_value(self::IS_OAI);

    // First is always the main app.
    $app_id = reset($app_ids);
    if ($is_oai) {
      $result = mediamosa_db::db_query(
        "SELECT #id AS app_id, #name, #play_proxy_url, #view_asset_url, #download_url
        FROM {#mediamosa_app}
        ORDER BY #name ASC", array(
          '#id' => mediamosa_app_db::APP_ID,
          '#name' => mediamosa_app_db::APP_NAME,
          '#play_proxy_url' => mediamosa_app_db::PLAY_PROXY_URL,
          '#view_asset_url' => mediamosa_app_db::VIEW_ASSET_URL,
          '#download_url' => mediamosa_app_db::DOWNLOAD_URL,
          '#mediamosa_app' => mediamosa_app_db::TABLE_NAME,
      ));

      foreach ($result as $app) {
        $mediamosa->add_item(array(
          mediamosa_app_db::APP_ID => $app[mediamosa_app_db::APP_ID],
          self::NAME => $app[mediamosa_app_db::APP_NAME],
          mediamosa_app_db::PLAY_PROXY_URL => $app[mediamosa_app_db::PLAY_PROXY_URL],
          mediamosa_app_db::VIEW_ASSET_URL => $app[mediamosa_app_db::VIEW_ASSET_URL],
          mediamosa_app_db::DOWNLOAD_URL => $app[mediamosa_app_db::DOWNLOAD_URL],
        ));
      }
    }
    else {
      // @todo: Fix serialize speed when there are many apps.

      // Restricted listing.
      $result = mediamosa_db::db_query(
        "SELECT #id AS app_id, #name, #allow_masterslave_apps
        FROM {#mediamosa_app}
        ORDER BY #name ASC", array(
          '#id' => mediamosa_app_db::APP_ID,
          '#name' => mediamosa_app_db::APP_NAME,
          '#allow_masterslave_apps' => mediamosa_app_db::ALLOW_MASTERSLAVE_APPS,
          '#mediamosa_app' => mediamosa_app_db::TABLE_NAME,
      ));

      foreach ($result as $app) {
        $allowed_apps = unserialize($app[mediamosa_app_db::ALLOW_MASTERSLAVE_APPS]);
        $allowed_apps = is_array($allowed_apps) ? $allowed_apps : array();

        // Get
        $app_ids = array_keys($allowed_apps);

        // Am I allowed?
        if (in_array($app_id, $app_ids)) {
          $mediamosa->add_item(array(
            mediamosa_app_db::APP_ID => $app[mediamosa_app_db::APP_ID],
            self::NAME => $app[mediamosa_app_db::APP_NAME],
          ));
        }
      }
    }
  }
}
