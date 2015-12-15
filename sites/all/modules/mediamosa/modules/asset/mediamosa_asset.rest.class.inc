<?php
/**
 * @file
 * The Asset REST calls.
 */

/**
 * URI: /change_ownership
 * Method: POST
 */
class mediamosa_rest_call_change_ownership extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const OLD_GROUP_ID = 'old_group_id';
  const NEW_GROUP_ID = 'new_group_id';
  const OLD_OWNER_ID = 'old_owner_id';
  const NEW_OWNER_ID = 'new_owner_id';
  const OLD_APP_ID = 'old_app_id';
  const NEW_APP_ID = 'new_app_id';

  // ---------------------------------------------------------------- Functions.
  /**
   * Variables setup.
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
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
        // Unused.
        self::OLD_APP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The current app to replace.',
        ),
        // Unused.
        self::NEW_APP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The new app to replace with.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  /**
   * Perform restcall.
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Because its to dangerous, new_app_id/old_app_id is turned off (#774).
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
 */
class mediamosa_rest_call_count_asset extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const OWNER_ID = 'owner_id';

  // ------------------------------------------------------------------ Var Setup.
  /**
   * Variables setup.
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
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
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
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
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const RETURN_ASSET_IDS = 'return_asset_ids';
  const FAV_USER_ID = 'fav_user_id';
  const USER_ID = 'user_id';
  // Alias for user_id.
  const ACL_USER_ID = 'acl_user_id';
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
  const PUBLISHED = 'published';
  const TRASHCAN = 'trashcan';

  // Search engine specific.
  const RELATED = 'related';
  const RELATED_FIELD = 'related_field';
  const RELATED_COUNT = 'related_count';
  const RELATED_BOOST = 'related_boost';
  const RELATED_SHOW_STILLS = 'related_show_stills';
  const RELATED_SHOW_VIEWED_ASSET = 'related_show_viewed_asset';
  const SOLR_FACET = 'solr_facet';
  const SOLR_FACET_FIELD = 'solr_facet_field';
  const SOLR_FACET_PREFIX = 'solr_facet_prefix';
  const SOLR_FACET_SORT = 'solr_facet_sort';
  const SOLR_FACET_LIMIT = 'solr_facet_limit';
  const SOLR_FACET_OFFSET = 'solr_facet_offset';
  const SOLR_FACET_MINCOUNT = 'solr_facet_mincount';
  const SOLR_FACET_MISSING = 'solr_facet_missing';
  const SOLR_FACET_METHOD = 'solr_facet_method';
  const SOLR_FACET_DATE = 'solr_facet_date';
  const SOLR_FACET_DATE_START = 'solr_facet_date_start';
  const SOLR_FACET_DATE_END = 'solr_facet_date_end';
  const SOLR_FACET_DATE_GAP = 'solr_facet_date_gap';
  const SOLR_FACET_DATE_HARDEND = 'solr_facet_date_hardend';
  const SOLR_FACET_DATE_OTHER = 'solr_facet_date_other';
  const SOLR_FACET_DATE_INCLUDE = 'solr_facet_date_include';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';

  // ---------------------------------------------------------------- Functions.
  /**
   * Get search parameters().
   */
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
    $var_setup = array();

    $var_setup = array(
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

        self::SOLR_FACET => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Return facets.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::SOLR_FACET_FIELD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DESCRIPTION => "This param allows you to specify a field which should be treated as a facet. F.e. dc.title or dc.subject. May contain multple fields as array. It will iterate over each Term in the field and generate a facet count using that Term as the constraint.",
          self::VAR_DEFAULT_VALUE => array('dc.title'),
        ),
        self::SOLR_FACET_PREFIX => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DESCRIPTION => "Limits the terms on which to facet to those starting with the given string prefix. Note that this does not change the search results -- it merely reduces the facet values returned to those beginning with the specified prefix.",
        ),
        self::SOLR_FACET_SORT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "This param determines the ordering of the facet field constraints.",
        ),
        self::SOLR_FACET_LIMIT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => "This param indicates the maximum number of constraint counts that should be returned for the facet fields. A negative value means unlimited.",
        ),
        self::SOLR_FACET_OFFSET => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => "This param indicates an offset into the list of constraints to allow paging.",
        ),
        self::SOLR_FACET_MINCOUNT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => "This param indicates the minimum counts for facet fields should be included in the response.",
        ),
        self::SOLR_FACET_MISSING => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Set to true this param indicates that in addition to the Term based constraints of a facet field, a count of all matching results which have no value for the field should be computed",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::SOLR_FACET_METHOD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "This parameter indicates what type of algorithm/method to use when faceting a field.",
        ),
        self::SOLR_FACET_DATE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DESCRIPTION => "This param allows you to specify names of fields (of type DateField) which should be treated as date facets.",
        ),
        self::SOLR_FACET_DATE_START => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "The lower bound for the first date range for all Date Faceting on this field. This should be a single date expression which may use the DateMathParser syntax.",
        ),
        self::SOLR_FACET_DATE_END => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "The minimum upper bound for the last date range for all Date Faceting on this field (see facet.date.hardend for an explanation of what the actual end value may be greater). This should be a single date expression which may use the DateMathParser syntax.",
        ),
        self::SOLR_FACET_DATE_GAP => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "The size of each date range expressed as an interval to be added to the lower bound using the DateMathParser syntax.",
        ),
        self::SOLR_FACET_DATE_HARDEND => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "A Boolean parameter instructing Solr what to do in the event that facet.date.gap does not divide evenly between facet.date.start and facet.date.end. If this is true, the last date range constraint will have an upper bound of facet.date.end; if false, the last date range will have the smallest possible upper bound greater then facet.date.end such that the range is exactly facet.date.gap wide.",
        ),
        self::SOLR_FACET_DATE_OTHER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "This param indicates that in addition to the counts for each date range constraint between facet.date.start and facet.date.end, counts should also be computed for...",
        ),
        self::SOLR_FACET_DATE_INCLUDE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "By default, the ranges used to compute date faceting between facet.date.start and facet.date.end are all inclusive of both endpoints, while the the before and after ranges are not inclusive. This behavior can be modified by the facet.date.include param, which can be any combination of the following options...",
        ),
        self::RELATED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include related search results. Related search is only available for Solr or other external MediaMosa search engines. Not supported by MySQL.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_FIELD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DESCRIPTION => "This param allows you to specify a field which should be treated for relation calculation. F.e. dc.title or dc.description. Related search is only available for Solr or other external MediaMosa search engines. Not supported by MySQL.",
          self::VAR_DEFAULT_VALUE => array('dc.description'),
        ),
        self::RELATED_COUNT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'The number of similar assets to return for each result. Related search is only available for Solr or other external MediaMosa search engines. Not supported by MySQL.',
          self::VAR_DEFAULT_VALUE => 10,
        ),
        self::RELATED_BOOST => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Set if the query will be boosted by the interesting term relevance. Related search is only available for Solr or other external MediaMosa search engines. Not supported by MySQL.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, generate a default still for related asset.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_SHOW_VIEWED_ASSET => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, include the view count of the asset in the related asset data.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),

        // Old fasion search params.
        self::OPERATOR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_OPERATOR,
          self::VAR_DESCRIPTION => 'For non-CQL search only; provide either OR or AND for search parameters..',
          self::VAR_DEFAULT_VALUE => 'and',
        ),
      ),
    );

    // Include limit, offset vars.
    $var_setup = self::get_var_setup_range($var_setup);

    // Include order by.
    $var_setup = self::get_var_setup_order_by($var_setup);

    // Include default.
    $var_setup = self::get_var_setup_default($var_setup);

    // Process what we have.
    $var_setup = parent::process_rest_args($var_setup);

    // If we return only asset ids, then we differ from limit_max.
    $return_asset_ids = $this->get_param_value(self::RETURN_ASSET_IDS);

    if ($return_asset_ids) {
      $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX_IDS_ONLY);
    }

    $app_ids = $this->get_param_value_app();

    // Now include metadata search stuff.
    $var_setup = mediamosa_core_cql::get_var_setup_search_asset($app_ids, $var_setup, self::get_search_params());

    $app_id = reset($app_ids);

    $publish_default = mediamosa_asset::METADATA_PUBLISHED_TRUE;
    $trashcan_default = mediamosa_asset::METADATA_TRASHCAN_FALSE;
    if ($app_id) {
      $app = mediamosa_app::get_by_appid($app_id);
      if (!empty($app)) {
        $publish_default = $app[mediamosa_app_db::PUBLISHED_ASSET_SEARCH_DEFAULT];
        $trashcan_default = $app[mediamosa_app_db::TRASHCAN_ASSET_SEARCH_DEFAULT];
      }
    }

    // Overrule the publish metadata autocreate.
    $var_setup[self::VARS][self::PUBLISHED] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      self::VAR_DESCRIPTION => "Filtering on published state. The default published states are 'TRUE' and 'FALSE'. To search within all published assets, supply 'TRUE'. For all unpublished, supply 'FALSE'. To search on all assets supply 'ALL'. If you want to use CQL, make sure you supply 'ALL' as value for 'published' on the REST call besides CQL. Once you supply 'ALL', use in CQL asset.published == \"^TRUE^\", asset.published == \"^FALSE^\" or for both asset.published = \"^TRUE^ ^FALSE^\". The default value for published parameter can be overridden per client application on its settings page.",
      self::VAR_DEFAULT_VALUE => $publish_default,
      self::VAR_ALLOWED_VALUES => array('TRUE', 'FALSE', 'ALL'),
    );
    unset($var_setup[self::VARS][self::PUBLISHED . '_match']);

    $var_setup[self::VARS][self::TRASHCAN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      self::VAR_DESCRIPTION => "Include or skip assets in the trashcan. Assets that where put in the trashcan will not show up by default in the asset search. The value 'FALSE' will hide all assets that where stored in the trashcan. Supply 'TRUE' to search the trashcan, or supply 'ALL' to search on all assets, regardless if its in the trashcan. If you u want to use CQL, make sure you supply 'ALL' as value for 'trashcan' on the REST call besides CQL. Once you supply 'ALL', use in CQL asset.trashcan == \"^TRUE^\", asset.trashcan == \"^FALSE^\" or for both asset.trashcan = \"^TRUE^ ^FALSE^\".",
      self::VAR_DEFAULT_VALUE => $trashcan_default,
      self::VAR_ALLOWED_VALUES => array('TRUE', 'FALSE', 'ALL'),
    );
    unset($var_setup[self::VARS][self::TRASHCAN . '_match']);

    // Allow modules to alter the search params.
    drupal_alter('mediamosa_asset_rest_call_vars', $var_setup);

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

    // CQL string needs validation. We do this here.

    // Get app ids.
    $app_ids = $this->get_param_value_app();

    // Now convert the old search parameters to CQL.
    $var_setup_search = mediamosa_core_cql::get_var_setup_search_asset($app_ids, array(), self::get_search_params());

    $a_cql = array();
    $translate_to = array();
    foreach ($var_setup_search[mediamosa_rest_call::VARS] as $param_name => $param) {
      if ($param[mediamosa_rest_call::VAR_TYPE] === mediamosa_sdk::TYPE_SEARCH_MATCH) {
        continue;
      }

      // When provided, then convert.
      if (!self::empty_param($param_name) && !self::empty_param($param_name . '_match')) {
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
    else {
      $cql = $this->get_param_value(self::CQL);
    }

    // Add order by to CQL (unless sortby is already specified).
    $order_by = $this->get_param_value(self::ORDER_BY);

    $error_text = '';
    if ($order_by != '') {
      $order_direction = $this->get_param_value(self::ORDER_DIRECTION);

      // Make sure CQL is valid before we attach order by.
      if (!empty($cql) && !mediamosa_search::asset_verify($cql, $app_ids, $error_text)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CQL_ERROR, array('@error' => $error_text));
      }

      // Recheck if we detected 'sortby' in current CQL. But 'sortby' can also
      // be used in search text, so attach it and verify cql.
      $has_sortby = mediamosa_unicode::stripos($cql, 'sortby ');
      $cql .= (!empty($cql) ? ' ' : '') . 'sortby ' . $order_by;
      $cql .= (empty($order_direction) ? '' : '/' . (mediamosa_unicode::strtolower($order_direction) == mediamosa_type::ORDER_DIRECTION_ASC ? 'ascending' : 'descending'));
      if ($has_sortby && !mediamosa_search::asset_verify($cql, $app_ids, $error_text)) {
        throw new mediamosa_exception_error_cql_exclusive();
      }
    }
    else {
      if (!empty($cql) && !mediamosa_search::asset_verify($cql, $app_ids, $error_text)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CQL_ERROR, array('@error' => $error_text));
      }
    }

    // Save the possible changed CQL.
    $this->set_param_value(self::CQL, $cql);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $acl_user_id = $this->get_param_value(self::ACL_USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Calculate the total.
    $calculate_total_count = $this->get_param_value(self::CALCULATE_TOTAL_COUNT);

    // View hidden metadata.
    $view_hidden_metadata = $this->get_param_value(self::VIEW_HIDDEN_METADATA);

    // Get CQL.
    $cql = $this->get_param_value(self::CQL);

    // Now search.
    $asset_search = mediamosa_search::asset(array(
      'app_ids' => $app_ids,
      'cql' => $cql,
      'cql_store_stats' => TRUE,
      'acl_user_id' => $acl_user_id,
      'acl_group_ids' => $this->get_param_value(self::ACL_GROUP_ID),
      'acl_domain' => $this->get_param_value(self::ACL_DOMAIN),
      'acl_realm' => $this->get_param_value(self::ACL_REALM),

      'batch_id' => $this->get_param_value(self::BATCH_ID),
      'coll_id' => $this->get_param_value(self::COLL_ID),

      'fav_user_id' => $this->get_param_value(self::FAV_USER_ID),

      'published' => $this->get_param_value(self::PUBLISHED),
      'trashcan' => $this->get_param_value(self::TRASHCAN),

      'granted' => $this->get_param_value(self::GRANTED),
      'is_public_list' => $this->get_param_value(self::IS_PUBLIC_LIST),
      'is_app_admin' => $is_app_admin,
      'hide_empty_assets' => $this->get_param_value(self::HIDE_EMPTY_ASSETS),
      'show_deleted' => $this->get_param_value(self::SHOW_DELETED),
      'calculate_total_count' => $calculate_total_count,

      'limit' => $this->get_param_value_limit(),
      'offset' => $this->get_param_value_offset(),

      'related' => $this->get_param_value(self::RELATED),
      'related_field' => $this->get_param_value(self::RELATED_FIELD),
      'related_count' => $this->get_param_value(self::RELATED_COUNT),
      'related_boost' => $this->get_param_value(self::RELATED_BOOST),

      'solr_facet' => $this->get_param_value(self::SOLR_FACET),
      'solr_facet_field' => $this->get_param_value(self::SOLR_FACET_FIELD),
      'solr_facet_prefix' => $this->get_param_value(self::SOLR_FACET_PREFIX),
      'solr_facet_sort' => $this->get_param_value(self::SOLR_FACET_SORT),
      'solr_facet_limit' => $this->get_param_value(self::SOLR_FACET_LIMIT),
      'solr_facet_offset' => $this->get_param_value(self::SOLR_FACET_OFFSET),
      'solr_facet_mincount' => $this->get_param_value(self::SOLR_FACET_MINCOUNT),
      'solr_facet_missing' => $this->get_param_value(self::SOLR_FACET_MISSING),
      'solr_facet_method' => $this->get_param_value(self::SOLR_FACET_METHOD),
      'solr_facet_date' => $this->get_param_value(self::SOLR_FACET_DATE),
      'solr_facet_date_start' => $this->get_param_value(self::SOLR_FACET_DATE_START),
      'solr_facet_date_end' => $this->get_param_value(self::SOLR_FACET_DATE_END),
      'solr_facet_date_gap' => $this->get_param_value(self::SOLR_FACET_DATE_GAP),
      'solr_facet_date_hardend' => $this->get_param_value(self::SOLR_FACET_DATE_HARDEND),
      'solr_facet_date_other' => $this->get_param_value(self::SOLR_FACET_DATE_OTHER),
      'solr_facet_date_include' => $this->get_param_value(self::SOLR_FACET_DATE_INCLUDE),
    ));

    // Set the total amount found.
    $mediamosa->item_count_total = $asset_search['total_count'];

    // Fill response.
    if ($this->get_param_value(self::RETURN_ASSET_IDS)) {
      foreach ($asset_search['asset_ids'] as $asset_id) {
        $mediamosa->add_item(array('asset_id' => $asset_id));
      }
    }
    else {
      $add_has_streamable_mediafiles = TRUE;

      $items = mediamosa_asset::asset_collect(
        $asset_search['asset_ids'],
        $app_ids,
        $acl_user_id,
        $this->get_param_value(self::GRANTED),
        $this->get_param_value(self::ACL_GROUP_ID),
        $this->get_param_value(self::ACL_DOMAIN),
        $this->get_param_value(self::ACL_REALM),
        $is_app_admin,
        $this->get_param_value(self::SHOW_STILLS),
        $this->get_param_value(self::SHOW_COLLECTIONS),
        $add_has_streamable_mediafiles,
        $view_hidden_metadata
      );

      if (!empty($asset_search['related'])) {
        mediamosa_asset::asset_collect_related(
          $app_id,
          $is_app_admin,
          $items,
          $asset_search['related'],
          array(
            'show_stills' => $this->get_param_value(self::RELATED_SHOW_STILLS),
            'show_viewed_asset' => $this->get_param_value(self::RELATED_SHOW_VIEWED_ASSET),
          )
        );
      }

      if (!empty($asset_search['solr_facet'])) {
        $solr_facet = mediamosa_asset::asset_collect_solr_facet($asset_search['solr_facet']);
        $mediamosa->set_result_header_extra($solr_facet, 'solr_facet');
      }

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
 */
class mediamosa_rest_call_asset_favorites_count extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const FAV_TYPE = 'fav_type';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to find its favorites.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    return self::get_var_setup_default($var_setup);
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
 */
class mediamosa_rest_call_asset_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';
  const TRASHCAN = 'trashcan';

  // ---------------------------------------------------------------- Functions.
  public function get_var_setup() {
    $var_setup = array();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $trashcan_default = 'TRUE';
    if ($app_id) {
      $app = mediamosa_app::get_by_appid($app_id);
      if (!empty($app)) {
        $trashcan_default = $app[mediamosa_app_db::TRASHCAN_ASSET_DEFAULT];
      }
    }

    $var_setup = array(
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
        self::TRASHCAN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Assets can be deleted trashcanly (default) or kept in the database with an trashcan flag. To remove the asset trashcanly, supply 'TRUE' (default). With 'FALSE', the asset is kept and will get the flag asset.trashcan 'TRUE'. Asset search will exclude these 'trashcanned' assets automaticly and can also be used to retrieve and search on trashcanned assets. The default value for this parameter can be overridden by the client application setting, which by default is 'TRUE'.",
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_DEFAULT_VALUE => $trashcan_default,
        ),
      ),
    );

    return self::get_var_setup_default($var_setup);
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
    $trashcan = $this->get_param_value(self::TRASHCAN);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Asset must exist to be deleted.
    // Get asset.
    $asset = mediamosa_asset::must_exists($asset_id, NULL, TRUE);

    // Must be owner / app_admin to delete.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    // Delete the asset (and all other stuff related).
    mediamosa_asset::delete($asset_id, $delete === 'cascade', $trashcan);

    // All ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/restore
 *
 * Method: POST
 */
class mediamosa_rest_call_asset_restore extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ---------------------------------------------------------------- Functions.
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to restore.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User ID, must be the owner of trashcanned asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $asset_id = $this->get_param_value(self::ASSET_ID);

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    $asset = mediamosa_asset::must_exists($asset_id, NULL, TRUE);

    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    mediamosa_asset::restore($asset_id);

    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/restore
 * Method: POST
 *
 * Undelete assets.
 */
class mediamosa_rest_call_asset_restore_all extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset IDs to restore.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User ID, must be owner of the trashcanned asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
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

    // Get the asset ids we need to delete.
    $asset_ids = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    $assets = mediamosa_asset::getAllAssoc($asset_ids);
    foreach ($asset_ids as $asset_id) {
      try {
        mediamosa_asset::must_exists($asset_id, NULL, TRUE);

        if (empty($assets[$asset_id])) {
          throw new mediamosa_exception_error_404;
        }

        mediamosa_acl::owner_check($app_id, $user_id, $assets[$asset_id][mediamosa_asset_db::APP_ID], $assets[$asset_id][mediamosa_asset_db::OWNER_ID], $is_app_admin);

        mediamosa_asset::restore($asset_id);

        $mediamosa->add_item(
          array(
            'asset_id' => $asset_id,
            'result' => mediamosa_response::SUCCESS,
            'result_id' => mediamosa_error::ERRORCODE_OKAY,
            'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
          )
        );
      }
      catch (mediamosa_exception_error $e) {
        $mediamosa->add_item(
          array(
            'asset_id' => $asset_id,
            'result' => mediamosa_response::ERROR,
            'result_id' => $e->getCode(),
            'result_description' => $e->getMessage(),
          )
        );
      }
    }

    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id
 * Method: GET
 *
 * Get a asset.
 */
class mediamosa_rest_call_asset_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  // Alias for user_id.
  const ACL_USER_ID = 'acl_user_id';
  const ACL_GROUP_ID = 'acl_group_id';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const SHOW_STILLS = 'show_stills';
  const SHOW_COLLECTIONS = 'show_collections';
  const TAG = 'tag';
  const IS_OAI = 'is_oai';
  const VIEW_HIDDEN_METADATA = 'view_hidden_metadata';
  const COUNT_VIEW = 'count_view';
  const TRASHCAN = 'trashcan';

  // Search engine specific.
  const RELATED = 'related';
  const RELATED_FIELD = 'related_field';
  const RELATED_COUNT = 'related_count';
  const RELATED_BOOST = 'related_boost';
  const RELATED_SHOW_STILLS = 'related_show_stills';
  const RELATED_SHOW_VIEWED_ASSET = 'related_show_viewed_asset';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
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
        self::COUNT_VIEW => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Specifing 'FALSE' will prevent increasing the view count on the asset. Default 'TRUE'.",
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::RELATED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include related search results.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_FIELD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DESCRIPTION => "This param allows you to specify a field which should be treated for relation calculation. F.e. dc.title or dc.description.",
          self::VAR_DEFAULT_VALUE => array('dc.description'),
        ),
        self::RELATED_COUNT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'The number of similar assets to return for each result.',
          self::VAR_DEFAULT_VALUE => 10,
        ),
        self::RELATED_BOOST => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Set if the query will be boosted by the interesting term relevance.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, generate a default still for related asset.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_SHOW_VIEWED_ASSET => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, include the view count of the asset in the related asset data.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::TRASHCAN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Allow retrieval of the asset when in the trashcan.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      ),
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

    if ($this->isset_given_param(self::USER_ID)) {
      $this->set_param_value(self::ACL_USER_ID, $this->get_param_value(self::USER_ID));
    }

    // View hidden metadata.
    $view_hidden_metadata = $this->get_param_value(self::VIEW_HIDDEN_METADATA);

    $acl_user_id = $this->get_param_value(self::ACL_USER_ID);
    $acl_group_id = $this->get_param_value(self::ACL_GROUP_ID);
    $acl_domain = $this->get_param_value(self::ACL_DOMAIN);
    $acl_realm = $this->get_param_value(self::ACL_REALM);
    $tag = $this->get_param_value(self::TAG);
    $is_oai = $this->get_param_value(self::IS_OAI);

    $show_stills = $this->get_param_value(self::SHOW_STILLS);
    $show_collections = $this->get_param_value(self::SHOW_COLLECTIONS);

    $granted = TRUE;

    $trashcan = $this->get_param_value(self::TRASHCAN);

    // Asset must exits.
    $asset = mediamosa_asset::must_exists($asset_id, NULL, $trashcan);

    // Fix app_id.
    if (in_array($asset['app_id'], $app_ids)) {
      $app_id = $asset['app_id'];
    }

    // ACL check.
    mediamosa_acl::read_single_object(mediamosa_acl::ACL_TYPE_ASSET, $asset_id, $app_ids);

    // Check un/inappropriate flag.
    mediamosa_asset::is_inappropriate($app_ids, $asset_id, $acl_user_id, $is_app_admin);

    // If any additional search options is on, we need to do extra search engine
    // call.
    $related = $this->get_param_value(self::RELATED);

    if ($related && mediamosa_search::support_related()) {
      // Now search.
      $asset_search = mediamosa_search::asset(array(
        'app_ids' => $app_ids,
        'cql' => 'asset_id = "^' . mediamosa_sdk::escape_cql($asset_id) .  '^"',
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

        'limit' => 1,
        'offset' => 0,

        // Additional search options.
        'related' => $this->get_param_value(self::RELATED),
        'related_field' => $this->get_param_value(self::RELATED_FIELD),
        'related_count' => $this->get_param_value(self::RELATED_COUNT),
        'related_boost' => $this->get_param_value(self::RELATED_BOOST),
      ));
    }
    else {
      $asset_search = array('asset_ids' => array($asset_id), 'total_count' => 1);
    }

    // Collect.
    $add_has_streamable_mediafiles = FALSE;
    $items = mediamosa_asset::asset_collect(
      $asset_search['asset_ids'],
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

    if (isset($asset_search['related'])) {
      mediamosa_asset::asset_collect_related(
        $app_id,
        $is_app_admin,
        $items,
        $asset_search['related'],
        array(
          'show_stills' => $this->get_param_value(self::RELATED_SHOW_STILLS),
          'show_viewed_asset' => $this->get_param_value(self::RELATED_SHOW_VIEWED_ASSET),
        )
      );
    }

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

    // Increase view count if not the owner (or admin).
    if ($this->get_param_value(self::COUNT_VIEW) &&
      (!$is_app_admin) &&
      ($acl_user_id != $asset['owner_id'])) {
      // Increase viewed.
      mediamosa_asset::asset_viewed($asset_id);
    }
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
  const COUNT_VIEW = 'count_view';
  const TRASHCAN = 'trashcan';

  // Search engine specific.
  const RELATED = 'related';
  const RELATED_FIELD = 'related_field';
  const RELATED_COUNT = 'related_count';
  const RELATED_BOOST = 'related_boost';
  const RELATED_SHOW_STILLS = 'related_show_stills';
  const RELATED_SHOW_VIEWED_ASSET = 'related_show_viewed_asset';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_USER_ID = 'aut_user_id';
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
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
        self::COUNT_VIEW => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Specifing 'FALSE' will prevent increasing the view count on the asset. Default 'TRUE'.",
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::RELATED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include related search results.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_FIELD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DESCRIPTION => "This param allows you to specify a field which should be treated for relation calculation. F.e. dc.title or dc.description.",
          self::VAR_DEFAULT_VALUE => array('dc.description'),
        ),
        self::RELATED_COUNT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'The number of similar assets to return for each result.',
          self::VAR_DEFAULT_VALUE => 10,
        ),
        self::RELATED_BOOST => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Set if the query will be boosted by the interesting term relevance.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, generate a default still for related asset.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::RELATED_SHOW_VIEWED_ASSET => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'If true, include the view count of the asset in the related asset data.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      ),
    );

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $trashcan_default = mediamosa_asset::METADATA_TRASHCAN_FALSE;
    if ($app_id) {
      $app = mediamosa_app::get_by_appid($app_id);
      if (!empty($app)) {
        $trashcan_default = $app[mediamosa_app_db::TRASHCAN_ASSET_SEARCH_DEFAULT];
      }
    }
    $var_setup[self::VARS][self::TRASHCAN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      self::VAR_DESCRIPTION => "Include or skip assets in the trashcan. Assets that where put in the trashcan will not show up by default in the asset search. The value 'FALSE' will hide all assets that where stored in the trashcan. Supply 'TRUE' to search the trashcan, or supply 'ALL' to search on all assets, regardless of its in the trashcan. If you u want to use CQL, make sure you supply 'ALL' as value for 'trashcan' on the REST call besides CQL. Once you supply 'ALL', use in CQL asset.trashcan == \"^TRUE^\", asset.trashcan == \"^FALSE^\" or for both asset.trashcan = \"^TRUE^ ^FALSE^\".",
      self::VAR_DEFAULT_VALUE => $trashcan_default,
      self::VAR_ALLOWED_VALUES => array('TRUE', 'FALSE', 'ALL'),
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

    $asset_ids = $this->get_param_value(self::ASSET_ID);

    // Transform asset_ids to cql search.
    $cql = array();

    foreach ($asset_ids as $asset_id) {
      // Will fix some notices when we supply array in array.
      if (is_array($asset_id)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => self::ASSET_ID, '@type' => mediamosa_sdk::TYPE_ASSET_ID));
      }

      $cql[] = '^' . mediamosa_sdk::escape_cql($asset_id) . '^';
    }

    // Create the CQL.
    $cql = 'asset_id any/keeporder "' . implode(' ', $cql) . '"';

    // View hidden metadata.
    $view_hidden_metadata = $this->get_param_value(self::VIEW_HIDDEN_METADATA);

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
    $asset_search = mediamosa_search::asset(array(
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

      'published' => 'ALL',
      'trashcan' => $this->get_param_value(self::TRASHCAN),

      'granted' => $granted,
      'is_public_list' => FALSE,
      'is_app_admin' => $is_app_admin,
      'hide_empty_assets' => FALSE,
      'show_deleted' => FALSE,
      'calculate_total_count' => FALSE,

      'limit' => count($asset_ids),
      'offset' => 0,

      'related' => $this->get_param_value(self::RELATED),
      'related_field' => $this->get_param_value(self::RELATED_FIELD),
      'related_count' => $this->get_param_value(self::RELATED_COUNT),
      'related_boost' => $this->get_param_value(self::RELATED_BOOST),
    ));

    if (!empty($asset_search['asset_ids'])) {
      // Get app version.
      $env_active_version = mediamosa::get_environment_setting(mediamosa::ENV_MEDIAMOSA_VERSION);

      // Show in old format? Yes, if the app client < 1.6.
      $old_output = $env_active_version !== FALSE && ($env_active_version[mediamosa_version::MAJOR] == 1 && $env_active_version[mediamosa_version::MINOR] < 6);

      // Collect.
      $add_has_streamable_mediafiles = FALSE;
      $items = mediamosa_asset::asset_collect(
        $asset_search['asset_ids'],
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

      if (!empty($asset_search['related'])) {
        mediamosa_asset::asset_collect_related(
          $app_id,
          $is_app_admin,
          $items,
          $asset_search['related'],
          array(
            'show_stills' => $this->get_param_value(self::RELATED_SHOW_STILLS),
            'show_viewed_asset' => $this->get_param_value(self::RELATED_SHOW_VIEWED_ASSET),
          )
        );
      }

      // Add to response.
      foreach ($items as $item) {
        $asset_id = $item['asset_id'];

        // Add mediafiles.
        $mediafile_ids = mediamosa_asset_mediafile::mediafiles_search($asset_id, $tag, FALSE, array());

        // Now enrich the output with mediafiles.
        $item['mediafiles'] = mediamosa_asset_mediafile::enrich_response_mediafile($mediafile_ids, $app_ids, $is_oai, $show_stills, $old_output);

        // Add streamable setting on asset.
        $item['has_streamable_mediafiles'] = mediamosa_asset::enrich_response_has_streamable_mediafiles($old_output ? $item['mediafiles'] : $item['mediafiles']) ? 'TRUE' : 'FALSE';

        // Increase view count if not the owner (or admin).
        if (($this->get_param_value(self::COUNT_VIEW) &&
             (!$is_app_admin) &&
             ($acl_user_id != $item['owner_id']))) {
          mediamosa_asset::asset_viewed($asset_id);
        }

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
 */
class mediamosa_rest_call_asset_delete_all extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';
  const TRASHCAN = 'trashcan';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $trashcan_default = mediamosa_app_db::TRASHCAN_ASSET_DEFAULT_FALSE;
    if ($app_id) {
      $app = mediamosa_app::get_by_appid($app_id);
      if (!empty($app)) {
        $trashcan_default = $app[mediamosa_app_db::TRASHCAN_ASSET_DEFAULT];
      }
    }

    $var_setup = array(
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
        self::TRASHCAN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Assets can be deleted (default) or kept in the database in the trashcan. To remove the asset completly, supply 'FALSE' (default). With 'TRUE', the asset is kept in the trashcan and will get the flag asset.trashcan = TRUE. Asset search will exclude these 'trashcanned' assets automaticly and can also be used to retrieve and search on assets in the trashcan. The default value for this parameter can be overridden by the client application setting, which by default is 'FALSE'.",
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_DEFAULT_VALUE => $trashcan_default,
        ),
      ),
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

    // Get the asset ids we need to delete.
    $a_asset_ids = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $delete = $this->get_param_value(self::DELETE);
    $trashcan = $this->get_param_value(self::TRASHCAN);

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

        mediamosa_asset::delete($asset_id, $delete === 'cascade', $trashcan);

        $mediamosa->add_item(
          array(
            'asset_id' => $asset_id,
            'result' => mediamosa_response::SUCCESS,
            'result_id' => mediamosa_error::ERRORCODE_OKAY,
            'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY),
          )
        );
      }
      catch (mediamosa_exception_error $e) {
        $mediamosa->add_item(
          array(
            'asset_id' => $asset_id,
            'result' => mediamosa_response::ERROR,
            'result_id' => $e->getCode(),
            'result_description' => $e->getMessage(),
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
 */
class mediamosa_rest_call_asset_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const REFERENCE_ID = 'reference_id';
  const PROVIDER_ID = 'provider_id';
  const PARENT_ID = 'parent_id';
  const PUBLISHED = 'published';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $publish_default = 'TRUE';
    if ($app_id) {
      $app = mediamosa_app::get_by_appid($app_id);
      if (!empty($app)) {
        $publish_default = $app[mediamosa_app_db::PUBLISHED_ASSET_DEFAULT];
      }
    }

    $var_setup = array(
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
          self::VAR_RANGE_END => mediamosa_asset_db::REFERENCE_ID_LENGTH,
        ),
        self::PROVIDER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The provider ID.',
          self::VAR_RANGE_END => mediamosa_asset_db::PROVIDER_ID_LENGTH,
        ),
        self::PUBLISHED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "The default published value of the asset. This default is overridden by the client application default, which is by default 'TRUE'.",
          self::VAR_DEFAULT_VALUE => $publish_default,
        ),
      ),
    );

    // The default value is depended on the value set on the client application.

    // Include default.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $owner_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $reference_id = $this->get_param_value(self::REFERENCE_ID);
    $provider_id = $this->get_param_value(self::PROVIDER_ID);
    $published = $this->get_param_value(self::PUBLISHED);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Create the asset.
    $asset_id = mediamosa_asset::create($app_id, $owner_id, $group_id, $reference_id, $provider_id, $published);

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
  const PROVIDER_ID = 'provider_id';
  const ASSET_TYPE = 'asset_type';

  // ------------------------------------------------------------ Get Var Setup.
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
        self::IS_INAPPROPRIATE => array(
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
          self::VAR_RANGE_END => mediamosa_asset_db::REFERENCE_ID_LENGTH,
        ),
        self::PROVIDER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The provider ID.',
          self::VAR_RANGE_END => mediamosa_asset_db::PROVIDER_ID_LENGTH,
        ),
        self::ASSET_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Asset type.',
          self::VAR_RANGE_END => mediamosa_asset_db::ASSET_TYPE_LENGTH,
        ),
      ),
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
    $provider_id = $this->get_param_value(self::PROVIDER_ID);
    $asset_type = $this->get_param_value(self::ASSET_TYPE);

    // If is_inappropriate is set, we'll use that one.
    if ($this->isset_given_param(self::IS_INAPPROPRIATE)) {
      $this->set_param_value(self::IS_INAPPROPRIATE, $this->get_param_value(self::IS_INAPPROPRIATE));
    }

    $is_unappropriate = $this->get_param_value(self::IS_INAPPROPRIATE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Update the asset.
    mediamosa_asset::update($app_id, $user_id, $is_app_admin, $asset_id, $owner_id, $group_id, $play_restriction_start, $play_restriction_end, $isprivate, $is_unappropriate, $reference_id, $provider_id, $asset_type);

    // Default OK.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/deleted
 * Method: POST
 *
 * Update asset.
 */
class mediamosa_rest_call_asset_get_deleted extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Include limit, offset vars.
    $var_setup = self::get_var_setup_range($var_setup);

    // Include order by.
    $var_setup = self::get_var_setup_order_by($var_setup, mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED, array(mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED, mediamosa_asset_delete_db::ID));

    // Include default.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $asset_deleted = mediamosa_asset::get_deleted(
      $app_id,
      array(mediamosa_asset_delete_db::ID, mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED),
      $this->get_param_value_offset(),
      $this->get_param_value_limit(),
      $this->get_param_value_order_by(),
      $this->get_param_value_order_direction()
    );

    foreach ($asset_deleted['asset_deleted'] as $asset_delete) {
      $coll_ids = $asset_delete['coll_ids'];
      $asset_delete['coll_ids'] = array();
      foreach ($coll_ids as $coll_id) {
        $asset_delete['coll_ids']['coll_id'][] = $coll_id;
      }

      $mediamosa->add_item($asset_delete, array('id' => $asset_delete[mediamosa_asset_delete_db::ID]));
    }

    $mediamosa->item_count_total = $asset_deleted['item_count_total'];
  }
}

/**
 * URI: /asset/$asset_id/publish
 * Method: POST
 */
class mediamosa_rest_call_asset_publish extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ---------------------------------------------------------------- Functions.
  /**
   * The var setup contains the properties description of the rest call.
   *
   * @return array
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the asset to publish.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID used for authorization.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_app_admin($var_setup);
  }

  /**
   * Execute the rest call.
   *
   * @throws mediamosa_exception_error
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    $asset = mediamosa_asset::must_exists($asset_id);

    // Must be owner / app_admin to publish.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    // Publish.
    mediamosa_asset::publish($asset_id);

    // Default OK.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/publish
 * Method: POST
 */
class mediamosa_rest_call_asset_publish_multi extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ---------------------------------------------------------------- Functions.
  /**
   * The var setup contains the properties description of the rest call.
   *
   * @return array
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the assets to publish.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID used for authorization, must be the owner of all assets to publish. Use is_app_admin=TRUE to skip owner check, if you do not require per asset owner testing.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_app_admin($var_setup);
  }

  /**
   * Execute the rest call.
   *
   * @throws mediamosa_exception_error
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_ids = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    $result = array();
    $failed = 0;
    foreach ($asset_ids as $asset_id) {
      try {
        $asset = mediamosa_asset::must_exists($asset_id);

        // Must be owner / app_admin to publish.
        mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

        // Publish.
        mediamosa_asset::publish($asset_id);

        $result[$asset_id] = array(
          'code' => mediamosa_sdk::ERRORCODE_OKAY,
        );
      }
      catch (mediamosa_exception $e) {
        $result[$asset_id] = array(
          'code' => $e->getCode(),
          'error' => $e->getMessage(),
        );
        $failed++;
      }
    }

    $mediamosa->set_result(array(
      'total' => count($result),
      'failed' => $failed,
      'result' => $result,
      ),
      mediamosa_sdk::ERRORCODE_OKAY
    );

    // Default OK.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/unpublish
 * Method: POST
 */
class mediamosa_rest_call_asset_unpublish extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ---------------------------------------------------------------- Functions.
  /**
   * The var setup contains the properties description of the rest call.
   *
   * @return array
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the asset to publish.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID used for authorization.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_app_admin($var_setup);
  }

  /**
   * Execute the rest call.
   *
   * @throws mediamosa_exception_error
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    $asset = mediamosa_asset::must_exists($asset_id);

    // Must be owner / app_admin to publish.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    // Publish.
    mediamosa_asset::unpublish($asset_id);

    // Default OK.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/unpublish
 * Method: POST
 */
class mediamosa_rest_call_asset_unpublish_multi extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ---------------------------------------------------------------- Functions.
  /**
   * The var setup contains the properties description of the rest call.
   *
   * @return array
   *
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the assets to unpublish.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID used for authorization, must be the owner of all assets to unpublish. Use is_app_admin=TRUE to skip owner check, if you do not require per asset owner testing.',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_app_admin($var_setup);
  }

  /**
   * Execute the rest call.
   *
   * @throws mediamosa_exception_error
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $asset_ids = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    $result = array();
    $failed = 0;
    foreach ($asset_ids as $asset_id) {
      try {
        $asset = mediamosa_asset::must_exists($asset_id);

        // Must be owner / app_admin to publish.
        mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

        // Publish.
        mediamosa_asset::unpublish($asset_id);

        $result[$asset_id] = array(
          'code' => mediamosa_sdk::ERRORCODE_OKAY,
        );
      }
      catch (mediamosa_exception $e) {
        $result[$asset_id] = array(
          'code' => $e->getCode(),
          'error' => $e->getMessage(),
        );
        $failed++;
      }
    }

    $mediamosa->set_result(array(
      'total' => count($result),
      'failed' => $failed,
      'result' => $result,
      ),
      mediamosa_sdk::ERRORCODE_OKAY
    );

    // Default OK.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/cleanup
 *
 * Method: POST
 */
class mediamosa_rest_call_asset_cleanup extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const LIFETIME_HOURS = 'lifetime_hours';

  // ---------------------------------------------------------------- Functions.
  /**
   * The var setup contains the properties description of the rest call.
   *
   * @return array
   *
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::LIFETIME_HOURS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The number of hours ago when an empty asset as been created to be deleted.',
          self::VAR_DEFAULT_VALUE => variable_get('mediamosa_asset_garbage_lifetime', mediamosa_settings::ASSET_GARBAGE_LIFETIME),
          self::VAR_RANGE_START => mediamosa_settings::ASSET_GARBAGE_LIFETIME_MIN,
        ),
      ),
    );

    return self::get_var_setup_default($var_setup, FALSE);
  }

  /**
   * Execute the rest call.
   *
   * @throws mediamosa_exception_error
   */
  public function do_call() {
    $mediamosa = mediamosa::get();
    $lifetime_hours = $this->get_param_value(self::LIFETIME_HOURS);

    $result = mediamosa_asset::garbage_cleanup($lifetime_hours);
    $result['failed_ids'] = array('failed_id' => $result['failed_ids']);

    $mediamosa->item_count = $result['cleaned'];
    $mediamosa->item_count_total = $result['cleaned'] + $result['failed'];
    $mediamosa->add_item($result);
  }
}

/**
 * URI: /asset/duplicates
 *
 * Method: POST
 */
class mediamosa_rest_call_asset_duplicates extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const TRASHCAN = 'trashcan';

  // ---------------------------------------------------------------- Functions.
  /**
   * The var setup contains the properties description of the rest call.
   *
   * @return array
   *   The var setup array.
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup[self::VARS][self::TRASHCAN] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      self::VAR_DESCRIPTION => 'Include searching assets that are in or outside the trashcan. Use "TRUE" to search in the trashcan only, "FALSE" to search only on assets outside the trashcan and "ALL" when searching on all assets regardless of their trashcan state.',
      self::VAR_DEFAULT_VALUE => 'FALSE',
      self::VAR_ALLOWED_VALUES => array('TRUE', 'FALSE', 'ALL'),
    );

    $var_setup = static::get_var_setup_range($var_setup);
    return static::get_var_setup_default($var_setup, FALSE);
  }

  /**
   * Execute the rest call.
   *
   * @throws mediamosa_exception_error
   */
  public function do_call() {
    $mediamosa = mediamosa::get();
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $limit = $this->get_param_value_limit();
    $offset = $this->get_param_value_offset();

    switch ($this->get_param_value(self::TRASHCAN)) {
      default:
      case 'FALSE':
        $trashcan = 0;
        break;

      case 'TRUE':
        $trashcan = 1;
        break;

      case 'ALL':
        $trashcan = 2;
        break;
    }

    $result = mediamosa_asset::find_duplicate_mediafiles($app_id, $trashcan, $offset, $limit);
    foreach ($result['duplicates'] as $md5 => $asset_ids) {
      $mediamosa->add_item(array('asset_id' => $asset_ids), array('md5' => $md5));
    }

    $mediamosa->item_count_total = $result['item_count_total'];
  }
}
