<?php
/**
 * @file
 * The asset search class contains the search functions that are moved here
 * to keep code more clean from the normal asset code.
 */

class mediamosa_asset_search {
  // ------------------------------------------------------------------ Static Function.
  // array name for search in where
  const SEARCH = 'search';
  const APPS = 'apps';
  const ORDER_BY = 'order_by';
  const ASSET_COLLECTION = 'asset_coll';
  const COLLECTION = 'collection';
  const FTP_BATCH = 'ftp_batch';

  // ---------------------------------------------------------------- Functions.
  /**
   * Search for assets with given parameters.
   *
   * @param array $a_app_ids
   *   Application IDs.
   * @param string $cql
   *   The CQL search string.
   * @param bool $cql_store_stats
   *   Process the CQL into keywords and store it for statistics. Should be TRUE
   *   only when CQL has been provided by user input (not for internal
   *   searches).
   * @param string $acl_user_id
   *   The authentication user.
   * @param array $acl_group_ids
   *   The authentication group.
   * @param string $acl_domain
   *   The authentication domain.
   * @param string $acl_realm
   *   The authentication realm.
   * @param string $batch_id
   *   The batch ID to search through.
   * @param string $coll_id
   *   The Collection to search through.
   * @param string $fav_user_id
   *   The user ID.
   * @param string $published
   *   The published state: 'TRUE', 'FALSE', 'all'.
   * @param bool $granted
   *   When true, assets are include on which we dont have access on. The
   *   granted parameter with the asset indicates if we have access.
   * @param bool $is_public_list
   *   Used to include or exclude unappropriate assets.
   * @param bool $is_app_admin
   *   Used for unappropriate.
   * @param bool $hide_empty_assets
   *   Don't include empty assets.
   * @param bool $show_deleted
   *   Include deleted.
   * @param bool $calculate_total_count
   *   Calculate all found rows.
   * @param int $limit
   *   Maximum number of results.
   * @param int $offset
   *   Offset within the result rows.
   */
  public static function asset_search(
    array $a_app_ids,
    $cql = NULL,
    $cql_store_stats = FALSE,

    $acl_user_id = NULL,
    array $acl_group_ids = array(),
    $acl_domain = NULL,
    $acl_realm = NULL,

    $batch_id = 0,
    $coll_id = 0,

    $fav_user_id = NULL,

    $published = mediamosa_asset::METADATA_PUBLISHED_TRUE,

    $granted = FALSE,
    $is_public_list = FALSE,
    $is_app_admin = FALSE,
    $hide_empty_assets = FALSE,
    $show_deleted = FALSE,
    $calculate_total_count = TRUE,

    $limit = 10,
    $offset = 0) {
    // Do master slave depends on granted.
    $do_master_slave = $granted;

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.asset_id';
    $a_query[mediamosa_db_query::A_FROM][] = strtr('{#mediamosa_asset} AS a', array('#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME));

    $a_query[mediamosa_db_query::ALLOW_DISTINCT] = TRUE;
    $a_query[mediamosa_db_query::A_GROUP_BY][] = strtr('a.#asset_id', array('#asset_id' => mediamosa_asset_db::ID));

    // Get the slaves based on the app ids for mediafiles.
    $a_slaves_mediafiles = mediamosa_acl_app_master_slave::slave_get($a_app_ids, mediamosa_acl::ACL_TYPE_MEDIAFILE);

    // Get the slaves based on the app ids for assets.
    $a_slaves_assets = array(); // Off, not stable, mediamosa_acl_app_master_slave::slave_get($a_app_ids, mediamosa_acl::ACL_TYPE_ASSET);

    // Merge the slave app ids with my app ids.
    $a_app_ids_tmp = array_unique(array_merge($a_app_ids, array_keys($a_slaves_mediafiles), array_keys($a_slaves_assets)));
    sort($a_app_ids_tmp);

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf('a.app_id IN(%s)', mediamosa_db::escape_string($a_app_ids_tmp));

    // isprivate / unappropiate test.
    // Is outside the ACL check, else we would have problems with 'granted'.
    if (!$is_app_admin) {
      if ($is_public_list && $acl_user_id) {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(a.isprivate = 'FALSE' AND (a.is_unappropriate = 'FALSE' OR a.owner_id = '%s'))", mediamosa_db::escape_string($acl_user_id));
      }
      elseif ($is_public_list) {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "(a.isprivate = 'FALSE' AND a.is_unappropriate = 'FALSE')"; // Must both be FALSE
      }
      elseif ($acl_user_id) { // if provided, then we only have access to unappropate when owner.
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(a.isprivate = 'FALSE' OR a.isprivate = 'TRUE') AND (a.is_unappropriate = 'FALSE' OR a.owner_id = '%s')", mediamosa_db::escape_string($acl_user_id));
      }
      else {
        // No public list, no acl_user_id
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "((a.isprivate = 'FALSE' OR a.isprivate = 'TRUE') AND a.is_unappropriate = 'FALSE')"; // Ignore isprivate, is_unappropriate must be TRUE
      }
    }
    else {
      // just to match index for speed
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "(a.isprivate = 'FALSE' OR a.is_unappropriate = 'FALSE' OR a.is_unappropriate = 'TRUE')";// only 3, will match always
    }

    // Switch to hide assets that have no mediafiles
    if ($hide_empty_assets) {
      // exclude empty assets
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = "a.is_empty_asset = 'FALSE'";
    }
    else {
      // just to match index for speed
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access'][mediamosa_db_query::WHERE_AND][] = 'a.is_empty_asset IS NOT NULL'; // Never NULL, will always match index
    }

    if ($published === 'TRUE' || $published === 'FALSE') {
      $propgroup = mediamosa_asset_metadata_property_group::property_group_row_get_with_name('asset');
      $propgroup_id = $propgroup[mediamosa_asset_metadata_property_group_db::ID];
      assert($propgroup_id);

      $prop = mediamosa_asset_metadata_property::property_get_row_with_name('published', $propgroup_id);
      $prop_id = $prop[mediamosa_asset_metadata_property_db::ID];
      assert($prop_id);
      $a_query[mediamosa_db_query::A_JOIN]['asset_published'] = 'JOIN {mediamosa_asset_metadata} AS mam_asset_published ON mam_asset_published.asset_id = a.asset_id AND mam_asset_published.prop_id = ' . $prop_id . " AND mam_asset_published.val_char_lft = '" . $published . "'";
    }

    if ($cql !== '' && $cql !== NULL) {
      // Create the search CQL.
      $a_result_cql2sql = mediamosa_core_cql::parse_asset($cql, $cql_store_stats, $a_app_ids);

      if (isset($a_result_cql2sql['str_where']) && $a_result_cql2sql['str_where'] != '') {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::SEARCH] = '(' . $a_result_cql2sql['str_where'] . ')';
      }

      if (isset($a_result_cql2sql['str_having']) && $a_result_cql2sql['str_having'] !== '') {
        $a_query[mediamosa_db_query::A_HAVING][] = $a_result_cql2sql['str_having'];
        $a_query[mediamosa_db_query::ALLOW_DISTINCT] = FALSE;
      }

      if (!empty($a_result_cql2sql['a_joins'])) {

        $key = array_search('MEDIAFILE_METADATA_MIMETYPE', $a_result_cql2sql['a_joins']);
        if ($key) {
          unset($a_result_cql2sql['a_joins'][$key]);

          $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::MIME_TYPE);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";
          $a_query[mediamosa_db_query::A_JOIN]['mediafile_metadata']['mm_mime_type'] = sprintf('LEFT JOIN {mediamosa_asset_mediafile_metadata} AS mm_mime_type ON mf.mediafile_id=mm_mime_type.mediafile_id AND mm_mime_type.prop_id = %d', $prop_id);
        }

        $key = array_search('FTP_BATCH', $a_result_cql2sql['a_joins']);
        if ($key) {
          unset($a_result_cql2sql['a_joins'][$key]);

          $a_query[mediamosa_db_query::A_JOIN]['ftp_batch_asset'] = 'JOIN {mediamosa_ftp_batch_asset} AS ba ON ba.asset_id = a.asset_id';
        }

        $key = array_search('ASSET_COLLECTION', $a_result_cql2sql['a_joins']);
        if ($key) {
          unset($a_result_cql2sql['a_joins'][$key]);

          $a_query[mediamosa_db_query::A_JOIN]['asset_collection'][self::ASSET_COLLECTION] = 'JOIN {mediamosa_asset_collection} AS asset_coll USING(asset_id)';
        }

        $key = array_search('MEDIAFILE_FILENAME', $a_result_cql2sql['a_joins']);
        if ($key) {
          unset($a_result_cql2sql['a_joins'][$key]);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";
        }

        $key = array_search('ACL_NAME_USER', $a_result_cql2sql['a_joins']);
        if ($key) {
          unset($a_result_cql2sql['a_joins'][$key]);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";
          $a_query[mediamosa_db_query::A_JOIN]['acl_object'] = mediamosa_acl_object::join_acl_object_get(mediamosa_acl::ACL_TYPE_MEDIAFILE);
          $a_query[mediamosa_db_query::A_JOIN]['acl_name_user'] = sprintf("LEFT JOIN {mediamosa_acl_name} AS acl_u ON acl_u.app_id IN (%s) AND acl_u.acl_type = 'USER' AND acl_u.acl_name_id=acl_obj.acl_id", implode(',', $a_app_ids));
          $do_master_slave = TRUE;
          unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
        }

        $key = array_search('ACL_NAME_USER_GROUP', $a_result_cql2sql['a_joins']);
        if ($key) {
          unset($a_result_cql2sql['a_joins'][$key]);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";
          $a_query[mediamosa_db_query::A_JOIN]['acl_object'] = mediamosa_acl_object::join_acl_object_get(mediamosa_acl::ACL_TYPE_MEDIAFILE);
          $a_query[mediamosa_db_query::A_JOIN]['acl_name_group'] = sprintf("LEFT JOIN {mediamosa_acl_name} AS acl_ug ON acl_ug.app_id IN (%s) AND acl_ug.acl_type = 'USER_GROUP' AND acl_ug.acl_name_id=acl_obj.acl_id", implode(',', $a_app_ids));
          $do_master_slave = TRUE;
          unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
        }

        $key = array_search('ACL_GROUP_DOMAIN', $a_result_cql2sql['a_joins']);
        if ($key) {
          unset($a_result_cql2sql['a_joins'][$key]);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";
          $a_query[mediamosa_db_query::A_JOIN]['acl_object'] = mediamosa_acl_object::join_acl_object_get(mediamosa_acl::ACL_TYPE_MEDIAFILE);
          $a_query[mediamosa_db_query::A_JOIN]['acl_group_domain'] = sprintf("LEFT JOIN {mediamosa_acl_name} AS acl_gd ON acl_gd.app_id IN (%s) AND acl_gd.acl_type = 'DOMAIN' AND acl_gd.acl_name_id=acl_obj.acl_id", implode(',', $a_app_ids));
          $do_master_slave = TRUE;
          unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
        }

        $key = array_search('ACL_GROUP_REALM', $a_result_cql2sql['a_joins']);
        if ($key) {
          do {
            unset($a_result_cql2sql['a_joins'][$key]);
            $key = array_search('ACL_GROUP_REALM', $a_result_cql2sql['a_joins']);
          }
          while ($key !== FALSE);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";
          $a_query[mediamosa_db_query::A_JOIN]['acl_object'] = mediamosa_acl_object::join_acl_object_get(mediamosa_acl::ACL_TYPE_MEDIAFILE);
          $a_query[mediamosa_db_query::A_JOIN]['acl_group_realm'] = sprintf("LEFT JOIN {mediamosa_acl_name} AS acl_gr ON acl_gr.app_id IN (%s) AND acl_gr.acl_type = 'REALM' AND acl_gr.acl_name_id=acl_obj.acl_id", implode(',', $a_app_ids));
          $do_master_slave = TRUE;
          unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
        }

        $key = array_search('ACL_APP_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
        if ($key) {
          do {
            unset($a_result_cql2sql['a_joins'][$key]);
            $key = array_search('ACL_APP_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
          }
          while ($key !== FALSE);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";
          $a_query[mediamosa_db_query::A_JOIN]['acl_app_master_slave_2'] = sprintf("JOIN {mediamosa_acl_app_master_slave} AS acl_ms ON acl_ms.acl_object_type = 'MEDIAFILE' AND acl_ms.acl_object_id=mf.mediafile_id AND acl_ms.app_id_slave IN (%s)", implode(',', $a_app_ids));
          $do_master_slave = TRUE;
          unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset']['access']);
          unset($a_query[mediamosa_db_query::A_JOIN]['acl_app_master_slave']);
        }

        $key = array_search('ACL_APP_IS_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
        if ($key) {
          do {
            unset($a_result_cql2sql['a_joins'][$key]);
            $key = array_search('ACL_APP_IS_MASTER_SLAVE', $a_result_cql2sql['a_joins']);
          }
          while ($key !== FALSE);

          $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id=mf.asset_id AND mf.is_still = 'FALSE'";

          // FIXME: join on asset <-> master slave turned off, slows query down to much(!).
          $a_query[mediamosa_db_query::A_JOIN]['acl_app_is_master_slave'] = sprintf("LEFT JOIN {mediamosa_acl_app_master_slave} AS acl_is_ms ON ((acl_is_ms.acl_object_type = 'MEDIAFILE' AND acl_is_ms.acl_object_id=mf.mediafile_id))");//  OR (acl_is_ms.acl_object_type = 'ASSET' AND acl_is_ms.acl_object_id=a.asset_id))");
        }

        $a_query[mediamosa_db_query::A_JOIN]['cql'] = $a_result_cql2sql['a_joins'];
      }

      if (isset($a_result_cql2sql['a_select_expr'])) {
        $a_query[mediamosa_db_query::A_SELECT_EXPR] = array_merge($a_query[mediamosa_db_query::A_SELECT_EXPR], $a_result_cql2sql['a_select_expr']);
      }

      if (count($a_result_cql2sql['a_order_by']) > 1) {
        throw new mediamosa_exception_error_cql_error(array('@error' => 'you can not use \'sortBy\' on multiple columns, only specify one column'));
      }

      $a_order_by = reset($a_result_cql2sql['a_order_by']);

      $order_by = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_COLUMN];
      $order_direction = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_DIRECTION];
      $order_type = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE];
      $order_table = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT];
      $order_prop_id = $a_order_by[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID];
    }

    // Process possible order by.
    if (isset($order_by) && $order_by != '') {
      /**
       * 1.x to 2.x changes.
       *
       * JOIN is now JOIN instead of an LEFT JOIN.
       * We consider that all values we sort on are not empty (empty strings '').
       * Changes to the metadata creation should no longer allow creation of empty metadata.
       * Checked against production server, only 1 string was found 'empty' and will not be
       * migrated during the 1.7 to 2.0 script.
       * This makes the ordering a bit faster because we no longer need a WHERE entry.
       */
      if ($order_prop_id) {
        $join = strtr(
          'JOIN {#table} AS sort ON sort.asset_id = a.asset_id AND sort.prop_id = #prop_id',
          array(
            '#table' => $order_table,
            '#prop_id' => intval($order_prop_id),
          )
        );
      }
      else {
        $join = strtr(
         'JOIN {#table} AS sort ON sort.asset_id = a.asset_id',
          array(
            '#table' => $order_table,
          )
        );

        // We need to skip empty values
        // We skip NULL values and empty strings with <> '' (tested).
        // String types are special, we also need to skip empty '' here.
        // We include the NULL and empty '' later again in fase 2.
        switch ($order_type) {
          case mediamosa_sdk::TYPE_STRING:
            $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY] = 'sort.' . $order_by . " <> ''";
            break;

          default:
            // Only metadata table might have rows that are skipped by the JOIN.
            if ($order_table != mediamosa_asset_metadata_db::TABLE_NAME) {
              $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY] = 'sort.' . $order_by . " IS NOT NULL";
            }
            break;
        }
      }

      // 000_ will make me top join.
      $a_query[mediamosa_db_query::A_JOIN] = array_merge(array('sort' => $join), (isset($a_query[mediamosa_db_query::A_JOIN]) ? $a_query[mediamosa_db_query::A_JOIN] : array()));
      $a_query[mediamosa_db_query::A_ORDER_BY][] = 'sort.' . $order_by . ' ' . $order_direction;
    }

    // If coll_id is given, then search within the given collection(s)
    // coll_id-isprivate-is_unappropriate-owner_id
    if ($coll_id) {
      $a_query[mediamosa_db_query::A_JOIN]['asset_collection'][self::ASSET_COLLECTION] = 'JOIN {mediamosa_asset_collection} AS asset_coll USING(asset_id)';

      // is_unappropriate
      if (!$is_app_admin) {
        // Default.
        $tmp = "c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE

        if ($is_public_list && $acl_user_id) {
          $tmp = sprintf("(c.isprivate = 'FALSE' AND (c.is_unappropriate = 'FALSE' OR c.owner_id = '%s'))", mediamosa_db::escape_string($acl_user_id));
        }
        elseif ($is_public_list) {
          $tmp = "c.isprivate = 'FALSE' AND c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE
        }
        elseif ($acl_user_id) { // if provided, then we only have access to unappropate when owner.
          $tmp = sprintf("(c.is_unappropriate = 'FALSE' OR c.owner_id = '%s')", mediamosa_db::escape_string($acl_user_id));
        }

        $a_query[mediamosa_db_query::A_JOIN]['collection'][self::COLLECTION] = 'INNER JOIN {mediamosa_collection} AS c ON asset_coll.coll_id = c.coll_id AND ' . $tmp;
      }

      $a_coll_ids = is_array($coll_id) ? $coll_id : array($coll_id);
      sort($a_coll_ids);
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ASSET_COLLECTION][mediamosa_db_query::WHERE_OR][] = sprintf("asset_coll.coll_id IN ('%s')", mediamosa_db::escape_string($a_coll_ids, "','"));
    }

    // FTP Batch.
    if ($batch_id) {
      // Check if batch exists
      mediamosa_db::db_must_exists(mediamosa_ftp_batch_db::TABLE_NAME, array(mediamosa_ftp_batch_db::ID => $batch_id));

      $a_query[mediamosa_db_query::A_JOIN]['ftp_batch_asset'][self::FTP_BATCH] = 'JOIN {mediamosa_ftp_batch_asset} AS ftp_ba USING(asset_id)';

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::FTP_BATCH][] = sprintf("ftp_ba.batch_id='%s'", mediamosa_db::escape_string($batch_id));
    }

    // When fav_user_id provided, we search for this user as owner.
    if ($fav_user_id != '') {
      $a_query[mediamosa_db_query::A_JOIN]['user_favorites']['user_fav'] = "LEFT JOIN {mediamosa_user_favorite} AS user_fav ON user_fav.fav_type='" . mediamosa_user_favorite_db::FAV_TYPE_ASSET . "' AND user_fav.fav_id = a.asset_id\n";

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.name = '%s'", mediamosa_db::escape_string($fav_user_id));
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.app_id IN('%s')", mediamosa_db::escape_string($a_app_ids, "','"));
    }

    // Authentication check
    // All authentication is done as last part in the where to make the selection to authenticate as small as possible
    // Also we only do authication here when we need to filter out the results (granted == FALSE)
    // Except we need to do master/slave check, so we set switch only to include master/slave...
    // Add to the where the access check for our original app_id
    mediamosa_acl::build_access_where($a_query, mediamosa_acl::ACL_TYPE_MEDIAFILE, NULL, $a_app_ids, $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, $a_slaves_mediafiles, FALSE, $do_master_slave);

    // Asset.
    // Off, not stable.
    //mediamosa_acl::build_access_where($a_query, mediamosa_acl::ACL_TYPE_ASSET, NULL, $a_app_ids, $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, $a_slaves_assets, FALSE, $do_master_slave);

    // Limit, offset stuff
    $a_query[mediamosa_db_query::LIMIT] = $limit;
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // HACK: Unset the acl_app_master_slave when acl_app_master_slave_2 is set
    if (isset($a_query[mediamosa_db_query::A_JOIN]['acl_app_master_slave_2'])) {
      unset($a_query[mediamosa_db_query::A_JOIN]['acl_app_master_slave']);
    }

    // Deleted assets
    if (!$show_deleted) {
      // Normal behaviour
      // Do the query
      $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_QUERY::SQL_CALC_FOUND_ROWS => $calculate_total_count));

      // If we specify data2, then we do our read queries on the slave.
      global $databases;
      $olddb = NULL;
      if (isset($databases['slave'])) {
        $olddb = db_set_active('data2');
      }

      try {
        // Do the query.
        $result = mediamosa_db::db_query($query);

        // Get the number of found rows.
        $i_found_rows = mediamosa_db::db_found_rows();

        $asset_ids = array();
        foreach ($result as $a_row) {
          $asset_ids[] = $a_row[mediamosa_asset_db::ID];
        }
      }
      catch (Exception $e) {
        // Make sure we switch back(!).
        if (isset($olddb)) {
          db_set_active($olddb);
        }

        throw $e;
      }

      // Switch back.
      if (isset($olddb)) {
        db_set_active($olddb);
      }
    }
    else {
      // Oai behaviour with deleted assets

      // FIXME RBL: not touching this code to much, really needs a rewrite without the temp. table.

// How it is working:
//CREATE TEMPORARY TABLE assets_all (asset_id VARCHAR(32) NOT NULL, videotimestampmodified TIMESTAMP NULL DEFAULT NULL, PRIMARY KEY  (asset_id)) ENGINE=MEMORY;
//
//INSERT INTO assets_all SELECT DISTINCT a.asset_id, a.videotimestampmodified FROM asset AS a LEFT JOIN mediafile AS mf ON a.asset_id=mf.asset_id
//WHERE a.app_id IN(14) AND
//a.parent_id IS NULL AND
//(((a.isprivate = 'FALSE' OR a.isprivate = 'TRUE') AND a.is_unappropriate = 'FALSE') AND
//a.is_empty_asset IS NOT NULL) AND
//((a.videotimestampmodified >= '2009-06-12 00:00:00' AND a.videotimestampmodified < '2009-08-15 00:00:00')) AND
//(mf.asset_id IS NULL OR (mf.is_original_file='TRUE' AND
//mf.app_id IN(14)));
//
//INSERT INTO assets_all SELECT a.asset_id, a.videotimestampmodified FROM asset_delete AS a WHERE a.app_id IN(14) AND
//((a.videotimestampmodified >= '2009-06-12 00:00:00' AND a.videotimestampmodified < '2009-08-15 00:00:00'));
//
//SELECT SQL_CALC_FOUND_ROWS DISTINCT a.asset_id FROM assets_all AS a
//ORDER BY a.videotimestampmodified ASC LIMIT 20;
//
//DROP TABLE assets_all;

      // Copy so we can change it back.
      $tmp_a_query = $a_query;

      // Collecting the data
      $d_ordby = $a_query[mediamosa_db_query::A_ORDER_BY][0];
      $d_limit = $a_query[mediamosa_db_query::LIMIT];
      $d_offset = $a_query[mediamosa_db_query::OFFSET];
      $d_search = isset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::SEARCH]) ? $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::SEARCH] : FALSE;
      $d_where = sprintf('a.app_id IN(%s)', implode(',', $a_app_ids_tmp)) . ($d_search ? ' AND ' . $d_search : '') . ($coll_id ? ' AND (a.coll_id = \'' . implode('\' OR a.coll_id = \'', $coll_id) . '\')' : '');

      // Temporary table
      /**
       * @todo: fix this to db_query_temp or even refactor without temp.
       */

        // Modified original query
        $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.videotimestampmodified';
        unset($a_query[mediamosa_db_query::A_ORDER_BY]);
        unset($a_query[mediamosa_db_query::LIMIT]);
        unset($a_query[mediamosa_db_query::OFFSET]);
        // Clean the joins, because of performance issue.
        unset($a_query[mediamosa_db_query::A_JOIN]);

        // unset sort.videotimestampmodified.
        unset($a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['order_by']);

        $query = mediamosa_db_query::query_select($a_query);
        // Join check
        // Put back the joins, if they are (one of them) necessary
        if (stripos($query, 'mf.') !== FALSE || stripos($query, 'acl_ms.') !== FALSE || stripos($query, 'asset_coll.') !== FALSE || stripos($query, ' c.') !== FALSE) {
          $a_query[mediamosa_db_query::A_JOIN] = $tmp_a_query[mediamosa_db_query::A_JOIN];
          $query = mediamosa_db_query::query_select($a_query);
        }
        $query = str_replace('SQL_CALC_FOUND_ROWS', '', $query);

        // If we specify data2, then we do our read queries on the slave.
        global $databases;
        $olddb = NULL;
        if (isset($databases['slave'])) {
          $olddb = db_set_active('data2');
        }

        try {
          $table_name = mediamosa_db::db_query_temporary($query);

          // Deleted asset query
          mediamosa_db::db_query('INSERT INTO {' . $table_name . '} SELECT a.asset_id, a.videotimestampmodified FROM {#mediamosa_asset_delete} AS a' . ($d_where ? ' WHERE ' . $d_where : '') . ' GROUP BY a.asset_id', array(
            '#mediamosa_asset_delete' => mediamosa_asset_delete_db::TABLE_NAME,
          ));

          if (mediamosa_unicode::substr($d_ordby, 0, 5) == 'sort.') {
            $d_ordby = 'a.' . mediamosa_unicode::substr($d_ordby, 5);
          }
          $result = mediamosa_db::db_query('SELECT DISTINCT a.asset_id FROM {' . $table_name . '} AS a' . ($d_ordby ? ' ORDER BY ' . $d_ordby : '') . ($d_limit ? ($d_offset ? ' LIMIT ' . $d_offset . ',' . $d_limit : ' LIMIT ' . $d_limit) : ''));
          $asset_ids = array();
          foreach ($result as $a_row) {
            $asset_ids[] = $a_row[mediamosa_asset_db::ID];
          }

          // How much assets we have globally
          $i_found_rows = $calculate_total_count ? mediamosa_db::db_query('SELECT COUNT(*) FROM {' . $table_name . '}')->fetchField() : count($asset_ids);
        }
        catch (Exception $e) {
          // Make sure we switch back(!).
          if (isset($olddb)) {
            db_set_active($olddb);
          }

          throw $e;
        }

        // Switch back.
        if (isset($olddb)) {
          db_set_active($olddb);
        }

      // Restore.
        $a_query = $tmp_a_query;
      }

    // Now create the query that will include all other results that are not included here
    // but only when a order by was present
    // If $calculate_total_count == FALSE, then skip this when we already have the limit full.
    if (isset($order_by) && $order_by != '' && ($calculate_total_count || count($asset_ids) < $a_query[mediamosa_db_query::LIMIT])) {

      // The first part of the query got all assets that had values on its order by column. For example,
      // Not every asset has a title, so the assets without title will not be included in the 1st query.
      // But we still want the assets in our result that don't have a title.

      // This second part will now do the same query again, but on the assets that have NULL rows on the
      // previous order by column.

      // Copy the other part
      $a_query_null = $a_query; // copy 1st query

      // Remove the old order by from the query, no need to order NULLs.
      unset($a_query_null[mediamosa_db_query::A_ORDER_BY]);
      unset($a_query_null[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY]);

      // Set where we sort on, include the NULL, skip the rest.
      $a_query_null[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY] = 'sort.' . $order_by . ' IS NULL';

      if ($order_prop_id) {
        $join = strtr(
          'LEFT JOIN {#table} AS sort ON sort.asset_id = a.asset_id AND sort.prop_id = #prop_id',
          array(
            '#table' => $order_table,
            '#prop_id' => intval($order_prop_id),
          )
        );

      }
      else {
        $join = strtr(
         'LEFT JOIN {#table} AS sort ON sort.asset_id = a.asset_id',
          array(
            '#table' => $order_table,
          )
        );

        // String types are special, we also need to include empty '' here.
        if ($order_type == mediamosa_sdk::TYPE_STRING) {
          // Set where we sort on, that is empty strings and NULL.
          $a_query_null[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][self::ORDER_BY] = '(sort.' . $order_by . ' IS NULL OR sort.' . $order_by . " = '')";
        }
      }

      // 000_ will make me top join.
      $a_query_null[mediamosa_db_query::A_JOIN] = array('sort' => $join) + (isset($a_query_null[mediamosa_db_query::A_JOIN]) ? $a_query_null[mediamosa_db_query::A_JOIN] : array());


      // Correct the offset when inside the NULL range.
      if ($i_found_rows < $a_query[mediamosa_db_query::OFFSET]) {
        // Offset starts beyond the end the 1st query.
        $a_query_null[mediamosa_db_query::OFFSET] = $a_query[mediamosa_db_query::OFFSET] - $i_found_rows; // correct position
        // limit stays the same.
      }
      else {
        // offset is inside the 1st query.
        $a_query_null[mediamosa_db_query::OFFSET] = 0;
        // make sure limit is at least 1.
        $a_query_null[mediamosa_db_query::LIMIT] = max(($a_query[mediamosa_db_query::LIMIT] - count($asset_ids)), 1);
      }

      // Create the query.
      $query = mediamosa_db_query::query_select($a_query_null, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => $calculate_total_count));

      // If we specify data2, then we do our read queries on the slave.
      global $databases;
      $olddb = NULL;
      if (isset($databases['slave'])) {
        $olddb = db_set_active('data2');
      }

      try {
        // Do the query.
        $result = mediamosa_db::db_query($query);

        // Get the number of found rows.
        $i_found_rows += mediamosa_db::db_found_rows();

        foreach ($result as $a_row) {
          if (count($asset_ids) >= $a_query[mediamosa_db_query::LIMIT]) {
            break;
          }

          $asset_ids[] = $a_row['asset_id'];
        }
      }
      catch (Exception $e) {
        // Make sure we switch back(!).
        if (isset($olddb)) {
          db_set_active($olddb);
        }

        throw $e;
      }

      // Switch back.
      if (isset($olddb)) {
        db_set_active($olddb);
      }
    }

    /**
     * We return the asset_ids.
     */
    return array('asset_ids' => $asset_ids, 'total_count' => $calculate_total_count ? $i_found_rows : count($asset_ids));
  }
}
