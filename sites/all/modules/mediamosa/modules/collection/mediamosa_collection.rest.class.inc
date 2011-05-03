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
  * The Collection REST calls.
  */

/**
 * URI: /collection/count
 * Method: GET
 *
 * 1.x: media_management_get_collection_search
 */
class mediamosa_rest_call_count_collection extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const OWNER_ID = 'owner_id';

  // --------------------------------------------------------- Public Functions.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($app_ids);

    $group_id = $this->get_param_value(self::GROUP_ID);
    $owner_id = $this->get_param_value(self::OWNER_ID);

    $o_mediamosa->set_result_okay();

    // Set the total count.
    $o_mediamosa->item_count_total = mediamosa_db::db_count_items(mediamosa_collection_db::TABLE_NAME, $app_id, $owner_id, $group_id);
  }
}

/**
 * URI: /collection
 * Method: GET
 */
class mediamosa_rest_call_collection_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const FAV_USER_ID = 'fav_user_id';
  const ASSET_ID = 'asset_id';
  const PRIVATE_ = 'private';
  const PUBLIC_ = 'public';
  const CATEGORY = 'category';
  const PUBLIC_ASSIGN = 'public_assign';
  const CQL = 'cql';
  const USER_ID = 'user_id'; // alias of acl_user_id.
  const ACL_USER_ID = 'acl_user_id';
  const IS_PUBLIC_LIST = 'is_public_list';
  const NUMOFVIDEOS = 'numofvideos';
  const OPERATOR = 'operator';
  const CALCULATE_TOTAL_COUNT = 'calculate_total_count';
  const HIDE_EMPTY_ASSETS = 'hide_empty_assets';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_USER_ID = 'aut_user_id';

  // --------------------------------------------------------- Public Functions.
  static public function get_search_params() {
    return array(
      'title' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'description' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'owner_id' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'group_id' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'created' => mediamosa_sdk::TYPE_SEARCH_DATETIME,
      'changed' => mediamosa_sdk::TYPE_SEARCH_DATETIME,
      'private' => mediamosa_sdk::TYPE_SEARCH_BOOL,
      'public' => mediamosa_sdk::TYPE_SEARCH_BOOL,
      'category' => mediamosa_sdk::TYPE_SEARCH_BOOL,
    );
  }

  // ---------------------------------------------------------------- Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'Search within the collection of this supplied asset ID.',
        ),
        self::CQL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_CQL_COLLECTION,
          self::VAR_DESCRIPTION => 'The CQL search string, used for extended and complex search options.',
        ),
        self::FAV_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Search within the favorites of this supplied user ID.',
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Alias for acl_user_id parameter, is chosen above acl_user_id.',
        ),
        self::ACL_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Authentication parameter, for including the collections that are private.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
        ),
        self::PRIVATE_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the private flag status of the collection.',
        ),
        self::PUBLIC_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the public flag status of the collection.',
        ),
        self::CATEGORY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the category flag status of the collection.',
        ),
        self::PUBLIC_ASSIGN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Search on the public assign flag status of the collection.',
        ),
        self::IS_APP_ADMIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Allows seeing unappropriate assets.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_PUBLIC_LIST => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'TRUE = include private assets, FALSE = exclude private assets (default),',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::CALCULATE_TOTAL_COUNT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Calculate the number of rows in the complete scope of the search. Performance wise to use this parameter with value 'FALSE' if you don't need to know the total number of rows in your scope result.",
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::HIDE_EMPTY_ASSETS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Do not include assets that do not have mediafiles.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),

        // Old fasion search params.
        self::OPERATOR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OPERATOR,
          self::VAR_DESCRIPTION => 'For non-CQL search only; provide either OR or AND for search parameters..',
          self::VAR_DEFAULT_VALUE => 'and',
        )
      )
    );

    // Include limit, offset vars.
    $var_setup = self::get_var_setup_range($var_setup);

    // Include order by.
    $var_setup = self::get_var_setup_order_by($var_setup);

    // Include default.
    $var_setup = self::get_var_setup_default($var_setup);

    // Now include metadata search stuff.
    $var_setup = mediamosa_core_cql::get_var_setup_search($var_setup, self::get_search_params());

    return $var_setup;
  }

  // ----------------------------------------------- Override Process Rest Args.
  // Fix for alias of user_id / acl_user_id.
  protected function process_rest_args(array $var_setup) {
    // Process input.
    $var_setup = parent::process_rest_args($var_setup);

    // If user_id is set, override acl_user_id, dont care if acl_user_id is set.
    $user_id = $this->get_param_value(self::USER_ID);

    if ($user_id != '') {
      $this->set_param_value(self::ACL_USER_ID, $user_id);
    }

    // Return the var_setup.
    return $var_setup;
  }

  // ---------------------------------------------- Override Validate Rest Args.
  // Override for checking CQL string.
  protected function validate_rest_args(array $var_setup) {

    // Validate first.
    parent::validate_rest_args($var_setup);

    $cql = $this->get_param_value(self::CQL);
    $app_ids = $this->get_param_value_app();
    $error_text = '';

    if ($cql != '' && !mediamosa_core_cql::verify_collection($cql, $app_ids, $error_text)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CQL_ERROR, array('@error' => $error_text));
    }
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Now convert the old search parameters to CQL.
    $var_setup = mediamosa_core_cql::get_var_setup_search(array(), self::get_search_params());

    // Non CQL parameters.
    $order_by = $this->get_param_value(self::ORDER_BY);
    $order_direction = $this->get_param_value(self::ORDER_DIRECTION);

    $cqls = array();
    $a_translate_to = array();
    foreach ($var_setup[mediamosa_rest_call::VARS] as $param => $a_param) {
      if ($a_param[mediamosa_rest_call::VAR_TYPE] == mediamosa_sdk::TYPE_SEARCH_MATCH) {
        continue;
      }
      // When provided, then convert.
      if (!self::empty_param($param)) {
        $a_value = $this->get_param_value($param);
        $match = $this->get_param_value($param . '_match');

        $cqls[] = mediamosa_core_cql::convert_searchparam_to_cql($param, $match, $a_value, $a_param[mediamosa_rest_call::PARAM_TYPE], $a_translate_to);
      }
    }

    // Build either the CQL from scratch or build from the parameters.
    $cql = '';
    if (count($cqls)) {

      // CQL must not be set.
      if (!self::empty_param(self::CQL)) {
        throw new mediamosa_exception_error_cql_exclusive();
      }

      // Get the glue operator.
      $operator = $this->get_param_value(self::OPERATOR);

      $cql = implode(' ' . mediamosa_unicode::strtoupper($operator) . ' ', $cqls);
    }

    // Add possible order by to the CQL.
    if ($order_by != '') {
      if (!self::empty_param(self::CQL)) {
        throw new mediamosa_exception_error_cql_exclusive();
      }

      $cql .= ($cql == '' ? ' ' : '') . 'sortby ' . $order_by . mediamosa_lib::order_direction2cql($order_direction);
    }
    elseif ($cql == '') {
      // Now normal search params, then take CQL (if any).
      $cql = $this->get_param_value(self::CQL);
    }

    // If order by is set on NUMOFVIDEOS and no value for order_direction was given, then set it to other default.
    if ($this->get_param_value_order_by() == self::NUMOFVIDEOS && $this->isset_default_param(self::ORDER_DIRECTION)) {
      self::set_param_value(self::ORDER_DIRECTION, mediamosa_db::ORDER_DIRECTION_DESC);
    }

    // Now search.
    $result = mediamosa_collection_search::collection_search(
      $app_ids,
      $cql,
      $this->get_param_value(self::ASSET_ID),
      NULL,
      $this->get_param_value(self::FAV_USER_ID),
      $this->get_param_value(self::ACL_USER_ID),
      $this->get_param_value(self::IS_APP_ADMIN),
      $this->get_param_value(self::IS_PUBLIC_LIST),
      $this->get_param_value(self::CALCULATE_TOTAL_COUNT),
      $this->get_param_value(self::HIDE_EMPTY_ASSETS),
      $this->get_param_value_limit(),
      $this->get_param_value_offset()
    );

    // Total rows.
    $o_mediamosa->item_count_total = $result->found_rows;

    // Set result.
    foreach ($result as $row) {
      // Convert UTC to app timezone.
      $row[mediamosa_collection_db::CREATED] = mediamosa::utcdate2appdate($row[mediamosa_collection_db::CREATED]);
      $row[mediamosa_collection_db::CHANGED] = mediamosa::utcdate2appdate($row[mediamosa_collection_db::CHANGED]);

      $o_mediamosa->add_item($row);
    }
  }
}

/**
 * URI: /asset/$asset_id/collection
 * Method: GET
 */
class mediamosa_rest_call_collection_search_asset extends mediamosa_rest_call_collection_search {
  // ---------------------------------------------------------------- Var Setup.
  public function get_var_setup() {
    // Get var setup.
    $var_setup = parent::get_var_setup();

    // Make asset_id required.
    $var_setup[self::VARS][self::ASSET_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;

    return $var_setup;
  }
}

/**
 * URI: /collection/$coll_id/favorites/count
 *      /collection/$coll_id/count_favorites (deprecated)
 * Method: GET
 *
 * 1.x: media_management_favorites_count_collection
 */
class mediamosa_rest_call_collection_favorites_count extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const FAV_TYPE= 'fav_type';

  // -------------------------------------------------------------Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'The collection ID to find its favorites.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $collection_id = $this->get_param_value(self::COLL_ID);

    // FIXME:
    // Any app can get any count from any asset.... behavor copied from 1.x.

    // Set total count.
    $o_mediamosa->item_count_total = mediamosa_user_favorite::count_items(mediamosa_user_favorite_db::FAV_TYPE_COLLECTION, $collection_id);

    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /collection/$coll_id
 * Method: GET
 *
 * 1.x: media_management_get_collection
 */
class mediamosa_rest_call_collection_get extends mediamosa_rest_call {
// --------------------------------------------------------------------- Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';
  const HIDE_EMPTY_ASSETS = 'hide_empty_assets';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The User ID requesting the details, used for unappropriate checks.',
        ),
        self::HIDE_EMPTY_ASSETS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Do not include assets that do not have mediafiles.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);
    $coll_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Collection must exist.
    $collection = mediamosa_collection::must_exists($coll_id);

    if ($collection[mediamosa_collection_db::IS_UNAPPROPRIATE] == mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE) {
      // Must be owner or admin.
      try {
        // Check the rights
        mediamosa_acl::owner_check($app_id, $user_id, $collection[mediamosa_collection_db::APP_ID], $collection[mediamosa_collection_db::OWNER_ID], $is_app_admin);
      }
      catch (mediamosa_exception_error_access_denied $e) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_IS_UNAPPROPRIATE);
      }
    }

    // UTC conversion to app timezone.
    $collection[mediamosa_collection_db::CREATED] = mediamosa::utcdate2appdate($collection[mediamosa_collection_db::CREATED]);
    $collection[mediamosa_collection_db::CHANGED] = mediamosa::utcdate2appdate($collection[mediamosa_collection_db::CHANGED]);

    $collection['numofvideos'] = mediamosa_collection::count_assets_in_collection($app_ids, $coll_id, $this->get_param_value(self::HIDE_EMPTY_ASSETS));

    $o_mediamosa->add_item($collection);
  }
}

/**
 * URI: /collection/$coll_id/asset/count
 * Method: GET
 *
 */
class mediamosa_rest_call_collection_count_relation extends mediamosa_rest_call {
// --------------------------------------------------------------------- Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';
  const HIDE_EMPTY_ASSETS = 'hide_empty_assets';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The User ID requesting the details, used for unappropriate checks.',
        ),
        self::HIDE_EMPTY_ASSETS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Do not include assets that do not have mediafiles.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);
    $coll_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Collection must exist.
    $collection = mediamosa_collection::must_exists($coll_id);

    if ($collection[mediamosa_collection_db::IS_UNAPPROPRIATE] == mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE) {
      // Must be owner or admin.
      try {
        // Check the rights
        mediamosa_acl::owner_check($app_id, $user_id, $collection[mediamosa_collection_db::APP_ID], $collection[mediamosa_collection_db::OWNER_ID], $is_app_admin);
      }
      catch (mediamosa_exception_error_access_denied $e) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_IS_UNAPPROPRIATE);
      }
    }

    $collection = array();
    $collection['numofvideos'] = mediamosa_collection::count_assets_in_collection($app_ids, $coll_id, $this->get_param_value(self::HIDE_EMPTY_ASSETS));

    $o_mediamosa->add_item($collection);
  }
}

/**
 * URI: /collection/create
 * Method: POST
 *
 * 1.x: media_management_create_collection
 */
class mediamosa_rest_call_collection_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const TITLE = 'title';
  const DESCRIPTION = 'description';
  const PRIVATE_ = 'private';
  const PUBLIC_ = 'public';
  const CATEGORY = 'category';
  const ISPRIVATE = 'isprivate';
  const PUBLIC_ASSIGN = 'public_assign';
  const IS_UNAPPROPRIATE = 'is_unappropriate';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID, will become the owner of the collection.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID, will be used for the group_id of the collection.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::TITLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The title of the new collection.',
          self::VAR_RANGE_END => mediamosa_collection_db::TITLE_LENGTH,
        ),
        self::DESCRIPTION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Description of the new collection.',
        ),
        self::PRIVATE_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Used to identify private collections, use [GET] /collection?private=true to get these collections.',
          self::VAR_DEFAULT_VALUE => 'false',
        ),
        self::PUBLIC_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Used to identify public collections, use [GET] /collection?public=true to get these collections.',
          self::VAR_DEFAULT_VALUE => 'true',
        ),
        self::CATEGORY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Used to identify main category collections, use [GET] /collection?category=true to get these collections.',
          self::VAR_DEFAULT_VALUE => 'false',
        ),
        self::ISPRIVATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => t('If isprivate = true, then the collction is not displayed in public lists, such as top 10, last uploaded, etc.'),
          self::VAR_DEFAULT_VALUE => 'false',
        ),
        self::PUBLIC_ASSIGN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "A 'true' value indicates that all users have access to the collection entitled to assets.",
          self::VAR_DEFAULT_VALUE => 'false',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $this->unset_param(self::IS_UNAPPROPRIATE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Get the user ID (required).
    $user_id = $this->get_param_value(self::USER_ID);

    // Optionals.
    $title = $this->get_param_value(self::TITLE);
    $description = $this->get_param_value(self::DESCRIPTION);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $private = $this->get_param_value(self::PRIVATE_);
    $public = $this->get_param_value(self::PUBLIC_);
    $category = $this->get_param_value(self::CATEGORY);
    $isprivate = $this->get_param_value(self::ISPRIVATE);
    $public_assign = $this->get_param_value(self::PUBLIC_ASSIGN);

    // Now create the collection.
    $coll_id = mediamosa_collection::create($app_id, $user_id, $title, $description, $group_id, $private, $public, $category, $isprivate, $public_assign);

    // Unique ID to response.
    $o_mediamosa->add_item(array(mediamosa_collection_db::ID => $coll_id));
  }
}

/**
 * URI: /collection/$coll_id
 * Method: POST
 *
 * 1.x: media_management_update_collection
 */
class mediamosa_rest_call_collection_update extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';
  const OWNER_ID = 'owner_id';
  const GROUP_ID = 'group_id';
  const PRIVATE_ = 'private';
  const PUBLIC_ = 'public';
  const CATEGORY = 'category';
  const ISPRIVATE = 'isprivate';
  const PUBLIC_ASSIGN = 'public_assign';
  const IS_UNAPPROPRIATE = 'is_unappropriate';
  const TITLE = 'title';
  const DESCRIPTION = 'description';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    // Do not set default values for update REST calls.
    $var_setup = array(
      self::VARS => array(
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'The collection ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID, must match the owner of the collection, otherwise the update will fail.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::TITLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The title of the collection.',
        ),
        self::DESCRIPTION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Description of the collection.',
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'An user who is the current owner can reassign a new owner; after changing the owner, the old owner has no rights to modify the collection.',
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::PRIVATE_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Used to identify private collections, use [GET] /collection?private=true to get these collections.',
        ),
        self::PUBLIC_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Used to identify public collections, use [GET] /collection?public=true to get these collections.',
        ),
        self::CATEGORY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Used to identify main category collections, use [GET] /collection?category=true to get these collections.',
        ),
        self::ISPRIVATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => '',
        ),
        self::PUBLIC_ASSIGN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => '',
        ),
        self::IS_UNAPPROPRIATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'A field to flag the collection as inappropriate. Only the is_app_admin is allowed to set this parameter. When set, the collection will not be shown except to the user and the is_app_admin. It does not affect assets belonging to the collection.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_app_admin($var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $coll_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Only when app admin.
    if (!$is_app_admin) {
      $this->unset_param(self::IS_UNAPPROPRIATE);
      $this->unset_param(self::OWNER_ID);
      $this->unset_param(self::GROUP_ID);
    }

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Collection must exist.
    $collection = mediamosa_collection::must_exists($coll_id);

    // Must be owner / app_admin to update.
    mediamosa_acl::owner_check($app_id, $user_id, $collection[mediamosa_collection_db::APP_ID], $collection[mediamosa_collection_db::OWNER_ID], $is_app_admin);

    // Optionals.
    $owner_id = $this->get_param_value(self::OWNER_ID);
    $title = $this->get_param_value(self::TITLE);
    $description = $this->get_param_value(self::DESCRIPTION);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $private = $this->get_param_value(self::PRIVATE_);
    $public = $this->get_param_value(self::PUBLIC_);
    $category = $this->get_param_value(self::CATEGORY);
    $isprivate = $this->get_param_value(self::ISPRIVATE);
    $public_assign = $this->get_param_value(self::PUBLIC_ASSIGN);
    $is_unappropriate = $this->get_param_value(self::IS_UNAPPROPRIATE);

    // Now update the collection.
    if (!mediamosa_collection::update($coll_id, $owner_id, $title, $description, $group_id, $private, $public, $category, $isprivate, $public_assign, $is_unappropriate)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_CHANGES);
    }

    // Set all OK.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /collection/$coll_id/delete
 * Method: POST
 *
 * 1.x: media_management_delete_collection
 */
class mediamosa_rest_call_collection_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'The collection ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::DELETE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Delete mode. Use this parameter with value \'cascade\' to delete *all* of the underlying items: assets with metadata, mediafiles, stills.

The collection is deleted only if all underlying assets successfully removed.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array('', 'cascade'),
          self::VAR_DEFAULT_VALUE => '',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $coll_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $delete = $this->get_param_value(self::DELETE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Collection must exist.
    $collection = mediamosa_collection::must_exists($coll_id);

    // Must be owner / app_admin to update.
    mediamosa_acl::owner_check($app_id, $user_id, $collection[mediamosa_collection_db::APP_ID], $collection[mediamosa_collection_db::OWNER_ID], $is_app_admin);

    // Count the links between this collection and possible assets.
    $asset_ids = mediamosa_db::db_query(
      'SELECT #asset_id FROM {#mediamosa_asset_collection} WHERE #coll_id = :coll_id',
      array(
        '#asset_id' => mediamosa_asset_collection_db::ASSET_ID,
        '#mediamosa_asset_collection' => mediamosa_asset_collection_db::TABLE_NAME,
        '#coll_id' => mediamosa_asset_collection_db::COLL_ID,
        ':coll_id' => $coll_id
      )
    )->fetchCol();

    // Check if there are assets in our collection.
    if (count($asset_ids)) {
      if ($delete != 'cascade') {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_NOT_EMPTY, array('@asset_count' => count($asset_ids)));
      }

      foreach ($asset_ids as $asset_id) {
        $result = mediamosa_response::SUCCESS;
        $result_id = mediamosa_error::ERRORCODE_OKAY;
        $result_description = mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY);

        try {
          mediamosa_asset::delete($asset_id, TRUE);
        }
        catch (mediamosa_exception $e) {
          $result = mediamosa_response::ERROR;
          $result_id = $e->getCode();
          $result_description = $e->getMessage();
        }

        // Add result of asset deletion.
        $o_mediamosa->add_item(
          array(
            'asset_id' => $asset_id,
            'result' => $result,
            'result_id' => $result_id,
            'result_description' => $result_description
          )
        );
      }

      // Now count if any assets are still left in collection.
      $count_assets_in_collection = mediamosa_db::db_count_rows(mediamosa_asset_collection_db::TABLE_NAME, array(mediamosa_asset_collection_db::COLL_ID => $coll_id));
      if ($count_assets_in_collection) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_NOT_EMPTY, array('@asset_count' => $count_assets_in_collection));
      }

      // Now delete the collection.
      mediamosa_collection::delete($coll_id);

      // All ok.
      $o_mediamosa->set_result_okay();
    }
    else {
      // Now delete the collection.
      mediamosa_collection::delete($coll_id);

      // All ok.
      $o_mediamosa->set_result_okay();
    }
  }
}


/**
 * URI: /collection/$coll_id/asset_relation/delete
 * Method: POST
 *
 * 1.x: media_management_delete_collection_relation
 */
class mediamosa_rest_call_collection_delete_relation extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'The collection ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID\'s to be removed from the collection. This parameter accepts an array of assets.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID, must be owner of the collection.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $coll_id = $this->get_param_value(self::COLL_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $asset_ids = $this->get_param_value(self::ASSET_ID);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Collection must exist.
    $collection = mediamosa_collection::must_exists($coll_id);

    // Get the assets.
    $a_assets = mediamosa_asset::getAllAssoc($asset_ids);

    // Now go through each asset.
    foreach ($asset_ids as $asset_id) {

      if (!isset($a_assets[$asset_id])) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $asset_id));
      }

      // Test link of collection to asset.
      mediamosa_db::db_must_exists(
        mediamosa_asset_collection_db::TABLE_NAME,
        array(
          mediamosa_asset_collection_db::ASSET_ID => $asset_id,
          mediamosa_asset_collection_db::COLL_ID => $coll_id,
        )
      );

      mediamosa_acl::owner_check_collection_assign($app_id, $user_id, $a_assets[$asset_id], $collection, $is_app_admin);
    }

    // If we get to this point, then all was passed and we can remove them from the collection.

    // Now go through each asset.
    foreach ($asset_ids as $asset_id) {
      // Delete the relation.
      mediamosa_asset_collection::delete($asset_id, $coll_id);

      // Add deletion to output. (new 2.0).
      $o_mediamosa->add_item(array('asset_id' => $asset_id, 'coll_id' => $coll_id));
    }

    $o_mediamosa->set_result_okay();
  }
}
