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
  * The Asset REST calls.
  */

/**
 * URI: /change_ownership
 * Method: POST
 *
 * 1.x: media_management_change_ownership
 *
 */
class mediamosa_rest_call_change_ownership extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const OLD_GROUP_ID = 'old_group_id';
  const NEW_GROUP_ID = 'new_group_id';
  const OLD_OWNER_ID = 'old_owner_id';
  const NEW_OWNER_ID = 'new_owner_id';
  const OLD_APP_ID = 'old_app_id';
  const NEW_APP_ID = 'new_app_id';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::OLD_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The current group ID to replace.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::NEW_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The new group ID to replace with.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::OLD_OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The current owner to replace.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::NEW_OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The new owner to replace with.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::OLD_APP_ID => array( // Unused.
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The current app to replace.',
        ),
        self::NEW_APP_ID => array( // Unused.
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The new app to replace with.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    /*
     * Because its to dangerous, new_app_id/old_app_id has been turned off (#774).
     */
    $old_owner_id = $this->get_param_value(self::OLD_OWNER_ID);
    $new_owner_id = $this->get_param_value(self::NEW_OWNER_ID);
    $old_group_id = $this->get_param_value(self::OLD_GROUP_ID);
    $new_group_id = $this->get_param_value(self::NEW_GROUP_ID);
    $old_app_id = reset($app_ids);
    $new_app_id = 0;

    // One (or more) of each set, and its ok.
    $set_old = ($old_owner_id == '' && $old_group_id == '' && !$old_app_id);
    $set_new = ($new_owner_id == '' && $new_group_id == '' && !$new_app_id);
    if ($set_old || $set_new) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CHANGE_OWNERSHIP_MISSING_PARAMETERS);
    }

    // All ok.
    $mediamosa->set_result_okay();

    // The tables.
    $a_table = array(
      mediamosa_asset_mediafile_db::TABLE_NAME,
      mediamosa_asset_db::TABLE_NAME,
      mediamosa_collection_db::TABLE_NAME,
    );

    // Go through each table and do our stuff.
    foreach ($a_table as $table) {
      $query = mediamosa_db::db_update($table);

      // Conditions.
      if ($old_owner_id != '') {
        $query->condition('owner_id', $old_owner_id);
      }

      if ($old_group_id != '') {
        $query->condition('group_id', $old_group_id);
      }

      if ($old_app_id) {
        $query->condition('app_id', $old_app_id);
      }

      // Changes.
      $a_fields = array();
      if ($new_owner_id != '') {
        $a_fields['owner_id'] = $new_owner_id;
      }

      if ($new_group_id != '') {
        $a_fields['group_id'] = $new_group_id;
      }

      if ($new_app_id) {
        $a_fields['app_id'] = $new_app_id;
      }

      assert(count($a_fields));

      // Add changed field.
      $a_fields = mediamosa_db::db_update_enrich($a_fields);

      // Add sets.
      $query->fields($a_fields);

      $rename = array(
        mediamosa_asset_db::TABLE_NAME => 'asset',
        mediamosa_asset_mediafile_db::TABLE_NAME => 'mediafile',
        mediamosa_collection_db::TABLE_NAME => 'collection',
      );

      // Add affected rows to output.
      $mediamosa->add_item(array($rename[$table] => $query->execute()));
    }
  }
}

/**
 * URI: /asset/count
 * Method: GET
 *
 * 1.x: media_management_asset_count
 *
 */
class mediamosa_rest_call_count_asset extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const OWNER_ID = 'owner_id';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($app_ids);

    $group_id = $this->get_param_value(self::GROUP_ID);
    $owner_id = $this->get_param_value(self::OWNER_ID);

    $mediamosa->set_result_okay();

    // Set the total count.
    $mediamosa->item_count_total = mediamosa_db::db_count_items(mediamosa_asset_db::TABLE_NAME, $app_id, $owner_id, $group_id);
  }
}

/**
 * URI: /asset
 * Method: GET
 */
class mediamosa_rest_call_asset_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const RETURN_ASSET_IDS = 'return_asset_ids';
  const FAV_USER_ID = 'fav_user_id';
  const USER_ID = 'user_id';
  const ACL_USER_ID = 'acl_user_id'; // Alias for user_id.
  const ACL_GROUP_ID = 'acl_group_id';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const GRANTED = 'granted';
  const OPERATOR = 'operator';
  const HIDE_EMPTY_ASSETS = 'hide_empty_assets';
  const COLL_ID = 'coll_id';
  const IS_PUBLIC_LIST = 'is_public_list';
  const BATCH_ID = 'batch_id';
  const CQL = 'cql';
  const SHOW_DELETED = 'show_deleted';
  const SHOW_STILLS = 'show_stills';
  const SHOW_COLLECTIONS = 'show_collections';
  const CALCULATE_TOTAL_COUNT = 'calculate_total_count';
  const VIEW_HIDDEN_METADATA = 'view_hidden_metadata';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';

  // ------------------------------------------------------------------ Public Functions.
  public static function get_search_params() {
    return array(
      'asset_id' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'owner_id' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'group_id' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'provider_id' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'reference_id' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'videotimestamp' => mediamosa_sdk::TYPE_SEARCH_DATETIME,
      'videotimestampmodified' => mediamosa_sdk::TYPE_SEARCH_DATETIME,
      'mediafile_duration' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'mediafile_container_type' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'changed' => mediamosa_sdk::TYPE_SEARCH_DATETIME,
      'app_id_search' => mediamosa_sdk::TYPE_SEARCH_INT,
      'mime_type' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'filename' => mediamosa_sdk::TYPE_SEARCH_STRING,
      'numofviews' => mediamosa_sdk::TYPE_SEARCH_INT,
      'numofplays' => mediamosa_sdk::TYPE_SEARCH_INT,
    );
  }

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::RETURN_ASSET_IDS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Return only the found asset ids.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::FAV_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Search within the favorites of this supplied user ID.',
        ),
        self::HIDE_EMPTY_ASSETS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Do not include assets that do not have mediafiles.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'Search within these collections.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::GRANTED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Include a flag on each asset to indicate access. When providing with TRUE, the result will include assets where no access is available. The 'granted' value in the result, indicates if access was granted for the asset.",
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::IS_PUBLIC_LIST => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include or hide private assets in result. TRUE: exclude private assets, FALSE: include private assets.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_APP_ADMIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Allows seeing unappropriate assets.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Alias for acl_user_id parameter, is chosen above acl_user_id.',
        ),
        self::ACL_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
        ),
        self::ACL_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DEFAULT_VALUE => array(),
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
        ),
        self::ACL_DOMAIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
        ),
        self::ACL_REALM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
          self::VAR_DESCRIPTION => 'Authentication parameter.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
        ),
        self::BATCH_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BATCH_ID,
          self::VAR_DESCRIPTION => 'Search within batch.',
        ),
        self::CQL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_CQL_ASSET,
          self::VAR_DESCRIPTION => 'The CQL search string, used for extended and complex search options.',
        ),
        self::SHOW_DELETED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include deleted assets for OAI. The deleted assets are not complete assets, these assets are empty and are only an indication that they are deleted.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include stills in output.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::SHOW_COLLECTIONS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include collection information of each assets, when available.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::CALCULATE_TOTAL_COUNT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Calculate the number of rows in the complete scope of the search and store it in the header parameter 'item_count_total'. Performance will drasticly improve with value 'FALSE' if you don't need to know the total number of assets in your scope result. If you provide 'FALSE', then 'item_count_total' value in the header will always be the number of items returned, same as 'item_count'.",
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::VIEW_HIDDEN_METADATA => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "View hidden metadata.",
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
    $a_var_setup = self::get_var_setup_range($a_var_setup);

    // Include order by.
    $a_var_setup = self::get_var_setup_order_by($a_var_setup);

    // Include default.
    $a_var_setup = self::get_var_setup_default($a_var_setup);

    // Process what we have.
    $a_var_setup = parent::process_rest_args($a_var_setup);

    // If we return only asset ids, then we differ from limit_max.
    $return_asset_ids = $this->get_param_value(self::RETURN_ASSET_IDS);

    if ($return_asset_ids) {
      $a_var_setup = self::get_var_setup_range($a_var_setup, mediamosa_settings::LIMIT_MAX_IDS_ONLY);
    }

    $app_ids = $this->get_param_value_app();

    // Now include metadata search stuff.
    $a_var_setup = mediamosa_core_cql::get_var_setup_search_asset($app_ids, $a_var_setup, self::get_search_params());

    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Override Process Rest Args.
  // Fix for alias of user_id / acl_user_id.
  protected function process_rest_args(array $a_var_setup) {

    // Process input.
    $a_var_setup = parent::process_rest_args($a_var_setup);

    // If user_id is set, override acl_user_id, dont care if acl_user_id is set.
    $user_id = $this->get_param_value(self::USER_ID);

    if ($user_id != '') {
      $this->set_param_value(self::ACL_USER_ID, $user_id);
    }

    // Return the var_setup.
    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Override Validate Rest Args.
  // Override for checking CQL string.
  protected function validate_rest_args(array $var_setup) {

    // Validate first.
    parent::validate_rest_args($var_setup);

    // CQL string needs validation. We do this here.

    // Get app ids.
    $app_ids = $this->get_param_value_app();

    // Now convert the old search parameters to CQL.
    $var_setup_search = mediamosa_core_cql::get_var_setup_search_asset($app_ids, array(), self::get_search_params());

    $a_cql = array();
    $translate_to = array();
    foreach ($var_setup_search[mediamosa_rest_call::VARS] as $param_name => $param) {
      if ($param[mediamosa_rest_call::VAR_TYPE] == mediamosa_sdk::TYPE_SEARCH_MATCH) {
        continue;
      }

      // When provided, then convert.
      if (!self::empty_param($param_name)) {
        $value = $this->get_param_value($param_name);
        $match = $this->get_param_value($param_name . '_match');
        $a_cql[] = mediamosa_core_cql::convert_searchparam_to_cql($param_name, $match, $value, $param[mediamosa_rest_call::PARAM_TYPE], $translate_to);
      }
    }

    $cql = '';
    if (!empty($a_cql)) {

      // CQL must not be set.
      if (!self::empty_param(self::CQL)) {
        throw new mediamosa_exception_error_cql_exclusive();
      }

      // Get the glue operator.
      $operator = $this->get_param_value(self::OPERATOR);

      $cql = implode(' ' . mediamosa_unicode::strtoupper($operator) . ' ', $a_cql);
    }

    // Non CQL parameters.
    $order_by = $this->get_param_value(self::ORDER_BY);
    $order_direction = $this->get_param_value(self::ORDER_DIRECTION);

    // Add possible order by to the CQL.
    if ($order_by != '') {
      if (!self::empty_param(self::CQL)) {
        throw new mediamosa_exception_error_cql_exclusive();
      }

      $cql .= (!empty($cql) ? ' ' : '') . 'sortby ' . $order_by;
      $cql .= (empty($order_direction) ? '' : '/' . (mediamosa_unicode::strtolower($order_direction) == mediamosa_type::ORDER_DIRECTION_ASC ? 'ascending' : 'descending'));
    }
    elseif (empty($a_cql)) {
      // Now normal search params, then take CQL (if any).
      $cql = $this->get_param_value(self::CQL);
    }
    $app_ids = $this->get_param_value_app();
    $error_text = '';

    if (!empty($cql) && !mediamosa_core_cql::verify_asset($cql, $app_ids, $error_text)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CQL_ERROR, array('@error' => $error_text));
    }

    // Save the possible changed CQL.
    $this->set_param_value(self::CQL, $cql);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Calculate the total.
    $calculate_total_count = $this->get_param_value(self::CALCULATE_TOTAL_COUNT);

    // View hidden metadata.
    $view_hidden_metadata = $this->get_param_value(self::VIEW_HIDDEN_METADATA);

    // Get CQL.
    $cql = $this->get_param_value(self::CQL);

    // Now search.
    $asset_ids = mediamosa_search::asset(array(
      'app_ids' => $app_ids,
      'cql' => $cql,
      'cql_store_stats' => TRUE,
      'acl_user_id' => $this->get_param_value(self::ACL_USER_ID),
      'acl_group_ids' => $this->get_param_value(self::ACL_GROUP_ID),
      'acl_domain' => $this->get_param_value(self::ACL_DOMAIN),
      'acl_realm' => $this->get_param_value(self::ACL_REALM),

      'batch_id' => $this->get_param_value(self::BATCH_ID),
      'coll_id' => $this->get_param_value(self::COLL_ID),

      'fav_user_id' => $this->get_param_value(self::FAV_USER_ID),

      'granted' => $this->get_param_value(self::GRANTED),
      'is_public_list' => $this->get_param_value(self::IS_PUBLIC_LIST),
      'is_app_admin' => $this->get_param_value(self::IS_APP_ADMIN),
      'hide_empty_assets' => $this->get_param_value(self::HIDE_EMPTY_ASSETS),
      'show_deleted' => $this->get_param_value(self::SHOW_DELETED),
      'calculate_total_count' => $calculate_total_count,

      'limit' => $this->get_param_value_limit(),
      'offset' => $this->get_param_value_offset()
    ));

    // Set the total amount found.
    $mediamosa->item_count_total = $asset_ids['total_count'];

    // Fill response.
    if ($this->get_param_value(self::RETURN_ASSET_IDS)) {
      foreach ($asset_ids['asset_ids'] as $asset_id) {
        $mediamosa->add_item(array('asset_id' => $asset_id));
      }
    }
    else {
      $add_has_streamable_mediafiles = TRUE;

      $items = mediamosa_asset::asset_collect(
        $asset_ids['asset_ids'],
        $app_ids,
        $this->get_param_value(self::ACL_USER_ID),
        $this->get_param_value(self::GRANTED),
        $this->get_param_value(self::ACL_GROUP_ID),
        $this->get_param_value(self::ACL_DOMAIN),
        $this->get_param_value(self::ACL_REALM),
        $this->get_param_value(self::IS_APP_ADMIN),
        $this->get_param_value(self::SHOW_STILLS),
        $this->get_param_value(self::SHOW_COLLECTIONS),
        $add_has_streamable_mediafiles,
        $view_hidden_metadata
      );

      foreach ($items as $item) {
        $mediamosa->add_item($item);
      }
    }
  }
}

/**
 * URI: /asset/$asset_id/favorites/count
 *      /asset/$asset_id/count_favorites (deprecated)
 * Method: GET
 *
 * 1.x: media_management_favorites_count_asset
 */
class mediamosa_rest_call_asset_favorites_count extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const FAV_TYPE= 'fav_type';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to find its favorites.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $asset_id = $this->get_param_value(self::ASSET_ID);

    // FIXME:
    // Any app can get any count from any asset....

    // Set total count.
    $mediamosa->item_count_total = mediamosa_user_favorite::count_items(mediamosa_user_favorite_db::FAV_TYPE_ASSET, $asset_id);

    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/delete
 *
 * Method: POST
 *
 * 1.x: media_management_delete_asset
 */
class mediamosa_rest_call_asset_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';

  // ------------------------------------------------------------------ Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to delete.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User ID, must be the owner to delete asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::DELETE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'Delete mode.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array('', 'cascade'),
        ),
      ),
    );

    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $asset_id = $this->get_param_value(self::ASSET_ID);

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $delete = $this->get_param_value(self::DELETE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Asset must exist to be deleted.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Get asset.
    $a_asset = mediamosa_db::db_query(
      'SELECT #app_id, #owner_id FROM {#mediamosa_asset} WHERE #asset_id  = :asset_id',
      array(
        '#app_id' => mediamosa_asset_db::APP_ID,
        '#owner_id' => mediamosa_asset_db::OWNER_ID,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
      )
    )->fetchAssoc();

    // Must be owner / app_admin to delete.
    mediamosa_acl::owner_check($app_id, $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    // Delete the asset (and all other stuff related).
    mediamosa_asset::delete($asset_id, $delete == 'cascade');

    // All ok.
    $mediamosa->set_result_okay();
  }
}


/**
 * URI: /asset/$asset_id
 * Method: GET
 *
 * Get a asset.
 *
 * 1.x: media_management_get_asset
 */
class mediamosa_rest_call_asset_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const ACL_USER_ID = 'acl_user_id'; // Alias for user_id.
  const ACL_GROUP_ID = 'acl_group_id';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const SHOW_STILLS = 'show_stills';
  const SHOW_COLLECTIONS = 'show_collections';
  const TAG = 'tag';
  const IS_OAI = 'is_oai';
  const VIEW_HIDDEN_METADATA = 'view_hidden_metadata';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the asset to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Include tag to search on mediafiles of the specified asset.',
          self::VAR_RANGE_END => mediamosa_asset_mediafile_db::TAG_LENGTH,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Alias for acl_user_id parameter. Warning: is chosen above acl_user_id when both are used.',
        ),
        self::ACL_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The current user is used for authorization. The acl_* fields affect the [granted] field in the output, and the parameter "granted" was given the number of records.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
        ),
        self::ACL_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group(s), where the current user is in, is used for authorization.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DEFAULT_VALUE => array(), // NULL is bad when not provided.
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
        ),
        self::ACL_DOMAIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'The domain of the user for autohorisation.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
        ),
        self::ACL_REALM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
          self::VAR_DESCRIPTION => 'The realm of the user for autohorisation.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
        ),
        self::SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, then include all of the still information.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::SHOW_COLLECTIONS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, then include all of the collection information.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_OAI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Generate additonal asset data for OAI output.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::VIEW_HIDDEN_METADATA => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "View hidden metadata.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      )
    );

    // Include default.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);

    if ($this->isset_given_param(self::USER_ID)) {
      $this->set_param_value(self::ACL_USER_ID, $this->get_param_value(self::USER_ID));
    }

    // View hidden metadata.
    $view_hidden_metadata = $this->get_param_value(self::VIEW_HIDDEN_METADATA);

    $acl_user_id = $this->get_param_value(self::ACL_USER_ID);
    $a_acl_group_id = $this->get_param_value(self::ACL_GROUP_ID);
    $acl_domain = $this->get_param_value(self::ACL_DOMAIN);
    $acl_realm = $this->get_param_value(self::ACL_REALM);
    $tag = $this->get_param_value(self::TAG);
    $is_oai = $this->get_param_value(self::IS_OAI);

    $show_stills = $this->get_param_value(self::SHOW_STILLS);
    $show_collections = $this->get_param_value(self::SHOW_COLLECTIONS);

    // Hotfix, we dont provide this, we always return the asset, so always supply the granted flag
    $granted = TRUE;

    // Asset must exits.
    $asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id), NULL, array(), FALSE);

    // Fix app_id.
    if (in_array($asset['app_id'], $app_ids)) {
      $app_id = $asset['app_id'];
    }

    // ACL check.
    mediamosa_acl::read_single_object(mediamosa_acl::ACL_TYPE_ASSET, $asset_id, $app_ids);

    // Check un/inappropriate flag.
    mediamosa_asset::is_unappropriate($asset_id, $app_ids, $acl_user_id, $is_app_admin);

    // Collect.
    $add_has_streamable_mediafiles = FALSE;
    $items = mediamosa_asset::asset_collect(
      array($asset_id),
      $app_ids,
      $acl_user_id,
      $granted,
      $a_acl_group_id,
      $acl_domain,
      $acl_realm,
      $is_app_admin,
      $show_stills,
      $show_collections,
      $add_has_streamable_mediafiles,
      $view_hidden_metadata
    );

    // Add mediafiles.
    $mediafile_ids = mediamosa_asset_mediafile::mediafiles_search($asset_id, $tag, FALSE, ($asset['app_id'] == $app_id ? array() : $app_ids));

    // Is app client at 1.6 ?
    $env_active_version = mediamosa::get_environment_setting(mediamosa::ENV_MEDIAMOSA_VERSION);

    // Show in old format? Yes, if the app client < 1.6.
    $old_output = $env_active_version !== FALSE && ($env_active_version[mediamosa_version::MAJOR] == 1 && $env_active_version[mediamosa_version::MINOR] < 6);

    // Now enrich the output with mediafiles.
    $items[0]['mediafiles'] = mediamosa_asset_mediafile::enrich_response_mediafile($mediafile_ids, $app_ids, $is_oai, $show_stills, $old_output);

    // Add streamable setting on asset.
    $items[0]['has_streamable_mediafiles'] = mediamosa_asset::enrich_response_has_streamable_mediafiles($items[0]['mediafiles']) ? 'TRUE' : 'FALSE';

    // Add to response.
    foreach ($items as $item) {
      $mediamosa->add_item($item);
    }

    // Increase viewed.
    mediamosa_asset::asset_viewed($asset_id);
  }
}

/**
 * URI: /assets
 * Method: GET
 *
 * Get multiple assets.
 */
class mediamosa_rest_call_assets_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const ACL_USER_ID = 'acl_user_id'; // Alias for user_id.
  const ACL_GROUP_ID = 'acl_group_id';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const SHOW_STILLS = 'show_stills';
  const SHOW_COLLECTIONS = 'show_collections';
  const TAG = 'tag';
  const IS_OAI = 'is_oai';
  const VIEW_HIDDEN_METADATA = 'view_hidden_metadata';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';

  // Unlimited.
  const LIMIT = 99999;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the assets to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Include tag to search on mediafiles of the specified asset.',
          self::VAR_RANGE_END => mediamosa_asset_mediafile_db::TAG_LENGTH,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Alias for acl_user_id parameter. Warning: is chosen above acl_user_id when both are used.',
        ),
        self::ACL_USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The current user is used for authorization. The acl_* fields affect the [granted] field in the output, and the parameter "granted" was given the number of records.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_USER_ID),
        ),
        self::ACL_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group(s), where the current user is in, is used for authorization.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DEFAULT_VALUE => array(), // NULL is bad when not provided.
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
        ),
        self::ACL_DOMAIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'The domain of the user for autohorisation.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
        ),
        self::ACL_REALM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
          self::VAR_DESCRIPTION => 'The realm of the user for autohorisation.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
        ),
        self::SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, then include all of the still information.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::SHOW_COLLECTIONS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, then include all of the collection information.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_OAI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Generate additonal asset data for OAI output.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::VIEW_HIDDEN_METADATA => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "View hidden metadata.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      )
    );

    // Include default.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_ids = $this->get_param_value(self::ASSET_ID);

    // Transform asset_ids to cql search.
    $cql = array();

    foreach ($asset_ids as $asset_id) {
      // Will fix some notices when we supply array in array.
      if (is_array($asset_id)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => self::ASSET_ID, '@type' => mediamosa_sdk::TYPE_ASSET_ID));
      }

      $cql[] = 'asset_id=="^' . $asset_id . '^"';
    }

    // Create the CQL.
    $cql = implode(' OR ', $cql);

    // View hidden metadata.
    $view_hidden_metadata = $this->get_param_value(self::VIEW_HIDDEN_METADATA);

    $acl_user_id = $this->get_param_value(self::ACL_USER_ID);
    $a_acl_group_id = $this->get_param_value(self::ACL_GROUP_ID);
    $acl_domain = $this->get_param_value(self::ACL_DOMAIN);
    $acl_realm = $this->get_param_value(self::ACL_REALM);
    $tag = $this->get_param_value(self::TAG);
    $is_oai = $this->get_param_value(self::IS_OAI);

    $acl_user_id = $this->get_param_value(self::ACL_USER_ID);
    $acl_group_id = $this->get_param_value(self::ACL_GROUP_ID);
    $acl_domain = $this->get_param_value(self::ACL_DOMAIN);
    $acl_realm = $this->get_param_value(self::ACL_REALM);

    $tag = $this->get_param_value(self::TAG);
    $is_oai = $this->get_param_value(self::IS_OAI);

    $show_stills = $this->get_param_value(self::SHOW_STILLS);
    $show_collections = $this->get_param_value(self::SHOW_COLLECTIONS);

    // Hotfix, we dont provide this, we always return the asset, so always
    // supply the granted flag.
    $granted = TRUE;

    // Now search.
    $asset_ids = mediamosa_search::asset(array(
      'app_ids' => $app_ids,
      'cql' => $cql,
      'cql_store_stats' => FALSE,
      'acl_user_id' => $acl_user_id,
      'acl_group_ids' => $acl_group_id,
      'acl_domain' => $acl_domain,
      'acl_realm' => $acl_realm,

      'batch_id' => 0,
      'coll_id' => array(),

      'fav_user_id' => NULL,

      'granted' => $granted,
      'is_public_list' => FALSE,
      'is_app_admin' => $is_app_admin,
      'hide_empty_assets' => FALSE,
      'show_deleted' => FALSE,
      'calculate_total_count' => FALSE,

      'limit' => self::LIMIT,
      'offset' => 0,
    ));

    if (!empty($asset_ids['asset_ids'])) {

      // Fix app_id.
      $app_id = reset($app_ids);

      // Get app version.
      $env_active_version = mediamosa::get_environment_setting(mediamosa::ENV_MEDIAMOSA_VERSION);

      // Show in old format? Yes, if the app client < 1.6.
      $old_output = $env_active_version !== FALSE && ($env_active_version[mediamosa_version::MAJOR] == 1 && $env_active_version[mediamosa_version::MINOR] < 6);

      // Collect.
      $add_has_streamable_mediafiles = FALSE;
      $items = mediamosa_asset::asset_collect(
        $asset_ids['asset_ids'],
        $app_ids,
        $acl_user_id,
        $granted,
        $acl_group_id,
        $acl_domain,
        $acl_realm,
        $is_app_admin,
        $show_stills,
        $show_collections,
        $add_has_streamable_mediafiles,
        $view_hidden_metadata
      );

      // Add to response.
      foreach ($items as $item) {
        $asset_id = $item['asset_id'];

        // Add mediafiles.
        $mediafile_ids = mediamosa_asset_mediafile::mediafiles_search($asset_id, $tag, FALSE, array());

        // Now enrich the output with mediafiles.
        $item['mediafiles'] = mediamosa_asset_mediafile::enrich_response_mediafile($mediafile_ids, $app_ids, $is_oai, $show_stills, $old_output);

        // Add streamable setting on asset.
        $item['has_streamable_mediafiles'] = mediamosa_asset::enrich_response_has_streamable_mediafiles($old_output ?  $item['mediafiles'] : $item['mediafiles']) ? 'TRUE' : 'FALSE';

        // Increase viewed.
        mediamosa_asset::asset_viewed($asset_id);

        // Add to output.
        $mediamosa->add_item($item);
      }
    }
  }
}

/**
 * URI: /asset/delete
 * Method: POST
 *
 * Delete assets.
 *
 * 1.x: media_management_multi_delete_asset
 */
class mediamosa_rest_call_asset_delete_all extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset IDs to delete.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User ID, must be owner to delete asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::DELETE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Delete mode.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array('', 'cascade'),
        ),
      )
    );

    // Include default.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Get the asset ids we need to delete.
    $a_asset_ids = $this->get_param_value(self::ASSET_ID);
    // User id
    $user_id = $this->get_param_value(self::USER_ID);
    // Delete
    $delete = $this->get_param_value(self::DELETE);

    foreach ($a_asset_ids as $asset_id) {
      try {
        // Get asset.
        $a_asset = mediamosa_db::db_query(
          'SELECT #app_id, #owner_id FROM {#mediamosa_asset} WHERE #asset_id  = :asset_id',
          array(
            '#app_id' => mediamosa_asset_db::APP_ID,
            '#owner_id' => mediamosa_asset_db::OWNER_ID,
            '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
            '#asset_id' => mediamosa_asset_db::ID,
            ':asset_id' => $asset_id,
          )
        )->fetchAssoc();

        // Must be owner / app_admin to delete.
        mediamosa_acl::owner_check($app_id, $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

        mediamosa_asset::delete($asset_id, $delete == 'cascade');

        $mediamosa->add_item(
          array(
            'asset_id' => $asset_id,
            'result' => mediamosa_response::SUCCESS,
            'result_id' => mediamosa_error::ERRORCODE_OKAY,
            'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY)
          )
        );
      }
      catch (mediamosa_exception_error $e) {
        $mediamosa->add_item(
          array(
            'asset_id' => $asset_id,
            'result' => mediamosa_response::ERROR,
            'result_id' => $e->getCode(),
            'result_description' => $e->getMessage()
          )
        );
      }
    }
  }
}

/**
 * URI: /asset/create
 * Method: POST
 *
 * Create asset.
 *
 * 1.x: media_management_create_asset
 */
class mediamosa_rest_call_asset_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const REFERENCE_ID = 'reference_id';
  const PROVIDER_ID = 'provider_id';
  const PARENT_ID = 'parent_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The user group ID.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::REFERENCE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The reference ID.',
        ),
        self::PROVIDER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'The provider ID.',
        ),
        self::PARENT_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'The parent ID. Is deprecated, do not supply, value will be ignored.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
      )
    );

    // Include default.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $owner_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $reference_id = $this->get_param_value(self::REFERENCE_ID);
    $provider_id = $this->get_param_value(self::PROVIDER_ID);
    assert($this->get_param_value(self::PARENT_ID) == NULL); // Not allowed anymore.
    $parent_id = NULL; // $this->get_param_value(self::PARENT_ID);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Create the asset.
    $asset_id = mediamosa_asset::create($app_id, $owner_id, $group_id, $reference_id, $provider_id, $parent_id);

    $mediamosa->add_item(array('asset_id' => $asset_id));
  }
}

/**
 * URI: /asset/$asset_id
 * Method: POST
 *
 * Update asset.
 */
class mediamosa_rest_call_asset_update extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const PLAY_RESTRICTION_START = 'play_restriction_start';
  const PLAY_RESTRICTION_END = 'play_restriction_end';
  const ISPRIVATE = 'isprivate';
  const ISPRIVATE_TRUE = 'TRUE';
  const ISPRIVATE_FALSE = 'FALSE';
  const IS_UNAPPROPRIATE = 'is_unappropriate';
  const IS_INAPPROPRIATE = 'is_inappropriate';
  const OWNER_ID = 'owner_id';
  const GROUP_ID = 'group_id';
  const REFERENCE_ID = 'reference_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the asset to update.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID used for authorization.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::PLAY_RESTRICTION_START => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,// Will skip UTC conversion.
          self::VAR_DESCRIPTION => "The play restriction range start position. Provide value '' (empty string) to clear this field. If play_restriction_start is beyond the play_restriction_end date, then the values of play_restriction_start and play_restriction_end parameters are swapped. Setting this value to '' and setting the play_restriction_end, then play_restriction_start is considered infinite.",
        ),
        self::PLAY_RESTRICTION_END => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,// Will skip UTC conversion.
          self::VAR_DESCRIPTION => "The play restriction range end position. Provide value '' (empty string) to clear this field. If play_restriction_end is before the play_restriction_start date, then the values of play_restriction_start and play_restriction_end parameters are swapped. Setting this value to '' and setting the play_restriction_start, then play_restriction_end is considered infinite.",
        ),
        self::ISPRIVATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If isprivate = true, the asset is not displayed in public lists, such as top 10, last uploaded, etc.',
        ),
        self::IS_INAPPROPRIATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If is_inappropriate = true then only the owner or the app admin (and underlying media files) can see this asset. Only the app manager can change this flag(is_app_admin=true).',
        ),
        self::IS_UNAPPROPRIATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Alias, same as is_inappropriate.',
          self::VAR_DEFAULT_VALUE => NULL,
          self::VAR_IS_HIDDEN => self::VAR_IS_HIDDEN_YES,
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The possible new owner of the asset; Only the app manager can change this flag (is_app_admin=true).',
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The possible new group of the asset; Only the app manager can change this flag(is_app_admin=true).',
        ),
        self::REFERENCE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The reference ID.',
        ),
      )
    );

    // Include default.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    $owner_id = $this->get_param_value(self::OWNER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);

    $play_restriction_start = $this->get_param_value(self::PLAY_RESTRICTION_START);
    $play_restriction_end = $this->get_param_value(self::PLAY_RESTRICTION_END);
    $isprivate = $this->get_param_value(self::ISPRIVATE);

    $reference_id = $this->get_param_value(self::REFERENCE_ID);

    // If is_inappropriate is set, we'll use that one.
    if ($this->isset_given_param(self::IS_INAPPROPRIATE)) {
      $this->set_param_value(self::IS_UNAPPROPRIATE, $this->get_param_value(self::IS_INAPPROPRIATE));
    }

    $is_unappropriate =  $this->get_param_value(self::IS_UNAPPROPRIATE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Update the asset.
    mediamosa_asset::update($app_id, $user_id, $is_app_admin, $asset_id, $owner_id, $group_id, $play_restriction_start, $play_restriction_end, $isprivate, $is_unappropriate, $reference_id);

    // Default OK.
    $mediamosa->set_result_okay();
  }
}
