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
  * Collection search functions.
  */

class mediamosa_collection_search {

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Search on collections and return the found collections.
   *
   * 1.x: _media_management_search_collection
   *
   * @param array $app_ids
   *  The application IDs.
   * @param string $cql
   *  The search CQL string.
   * @param string $asset_id
   *  Search on this asset.
   * @param string $coll_id
   *  Only in this collection.
   * @param string $fav_user_id
   *  Search on favorites.
   * @param string $acl_user_id
   *  Search for current user.
   * @param bool $is_app_admin
   *  For searching unappropriate assets.
   * @param bool $is_public_list
   *  including/excluding private assets.
   * @param bool $calculate_total_count
   *  Calculate all found rows.
   * @param int $limit
   *  Max limit rows in result.
   * @param int $offset
   *  Offset row number with result set.
   */
  static public function collection_search(
    array $app_ids,
    $cql = NULL,
    $asset_id = NULL,
    $coll_id = NULL,
    $fav_user_id = NULL,
    $acl_user_id = NULL,

    $is_app_admin = FALSE,
    $is_public_list = FALSE,

    $calculate_total_count = TRUE,
    $calculate_total_numofvideos = TRUE,

    $hide_empty_asset = FALSE,

    $limit = 10,
    $offset = 0) {

    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'c.*';
    $a_query[mediamosa_db_query::A_FROM][] = '{mediamosa_collection} AS c';

    $a_query[mediamosa_db_query::ALLOW_DISTINCT] = FALSE;
    $a_query[mediamosa_db_query::A_GROUP_BY][] = 'c.coll_id';

    // Moved here so numofvideos is always in the result.
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = 'COUNT(DISTINCT ac.asset_id) AS numofvideos';

    // If we hide empty assets, then we check the is_empty_asset flag on asset_collection.
    if ($hide_empty_asset) {
      $a_query[mediamosa_db_query::A_JOIN]['asset_collection'][] = "LEFT JOIN {mediamosa_asset_collection} AS ac ON c.coll_id = ac.coll_id AND ac.is_empty_asset = 'FALSE'";
    }
    else { // Don't care empty or not.
      $a_query[mediamosa_db_query::A_JOIN]['asset_collection'][] = 'LEFT JOIN {mediamosa_asset_collection} AS ac ON c.coll_id = ac.coll_id';
    }

    // To select one specific collection.
    if ($coll_id) {
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['coll_id'][mediamosa_db_query::WHERE_AND][] = sprintf("c.coll_id = '%s'", mediamosa_db::escape_string($coll_id));
    }

    // Include possible asset restriction.
    if ($asset_id) {
      // Check if asset exists.
      mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['ac'][] = sprintf("ac.asset_id = '%s'", mediamosa_db::escape_string($asset_id));
    }

    // isprivate / unappropiate test.
    // Is outside the ACL check, else we would have problems with 'granted'.
    if (!$is_app_admin) {
      if ($is_public_list && $acl_user_id) {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(c.isprivate = 'FALSE' AND (c.is_unappropriate = 'FALSE' OR c.owner_id = '%s'))", mediamosa_db::escape_string($acl_user_id));
      }
      elseif ($is_public_list) {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = "c.isprivate = 'FALSE' AND c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE
      }
      elseif ($acl_user_id) { // if provided, then we only have access to unappropate when owner.
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = sprintf("(c.is_unappropriate = 'FALSE' OR c.owner_id = '%s')", mediamosa_db::escape_string($acl_user_id));
      }
      else {
        // No public list, no acl_user_id
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['collection']['access'][mediamosa_db_query::WHERE_AND][] = "c.is_unappropriate = 'FALSE'"; // is_unappropriate must be TRUE
      }
    }

    // Collect my slaves.
    $slaves_collections = mediamosa_acl_app_master_slave::slave_get($app_ids, mediamosa_acl::ACL_TYPE_COLLECTION);

    // Access selection is done here;
    mediamosa_acl::build_access_where($a_query, mediamosa_acl::ACL_TYPE_COLLECTION, NULL, $app_ids, $acl_user_id, array(), NULL, NULL, $slaves_collections, $is_app_admin);

    if ($calculate_total_numofvideos) {
      // Get the slaves based on the app ids for mediafiles.
      $slaves_mediafiles = mediamosa_acl_app_master_slave::slave_get($app_ids, mediamosa_acl::ACL_TYPE_MEDIAFILE);

      // Add access for master slave.
      mediamosa_acl::build_access_where($a_query, mediamosa_acl::ACL_TYPE_MEDIAFILE, NULL, $app_ids, NULL, array(), NULL, NULL, $slaves_mediafiles, $is_app_admin, TRUE, TRUE, 'mf');

      // Replace join with the ac alias instead of a alias (asset).
      $a_query[mediamosa_db_query::A_JOIN]['mediafile'] = "LEFT JOIN {mediamosa_asset_mediafile} AS mf ON mf.asset_id = ac.asset_id";
    }

    $order_by = '';
    if ($cql) {
      $a_result_cql2sql = mediamosa_core_cql::parse_collection($cql, $app_ids);

      if (isset($a_result_cql2sql['str_where']) && $a_result_cql2sql['str_where'] != '') {
        $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['cql'] = $a_result_cql2sql['str_where'];
      }

      if (isset($a_result_cql2sql['str_having']) && $a_result_cql2sql['str_having'] != "") {
        $a_query[mediamosa_db_query::A_HAVING][] = $a_result_cql2sql['str_having'];
      }

      if (isset($a_result_cql2sql['a_joins'])) {
        $a_query[mediamosa_db_query::A_JOIN]['cql'] = $a_result_cql2sql['a_joins'];
      }

      if (count($a_result_cql2sql['a_order_by']) > 1) {
        throw new mediamosa_exception_error_cql_error(array('@error' => 'you can not use \'sortBy\' on multiple columns, only specify one column'));
      }

      $a_order_by = reset($a_result_cql2sql['a_order_by']);

      $order_by = $a_order_by['column'];
      $order_direction = $a_order_by['direction'];
    }

    if ($order_by != '') {
      if ($order_by == 'numofvideos') {
        // Sort on numofvideos is a special case
        $a_query[mediamosa_db_query::A_ORDER_BY][] = 'numofvideos ' . mediamosa_db::escape_string($order_direction);
      }
      else {
        $a_query[mediamosa_db_query::A_ORDER_BY][] = 'ISNULL(c.' . $order_by . ') ' . mediamosa_db::escape_string($order_direction);
        $a_query[mediamosa_db_query::A_ORDER_BY][] = 'c.' . $order_by . ' ' . mediamosa_db::escape_string($order_direction);
      }
    }

    if ($fav_user_id != '') {
      $a_query[mediamosa_db_query::A_JOIN]['user_favorite']['user_fav'] = "LEFT JOIN {mediamosa_user_favorite} AS user_fav ON user_fav.fav_type = '" . mediamosa_user_favorite_db::FAV_TYPE_COLLECTION . "' AND user_fav.fav_id = c.coll_id";

      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.name = '%s'", mediamosa_db::escape_string($fav_user_id));
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['user_fav'][] = sprintf("user_fav.app_id IN(%s)", mediamosa_db::escape_string($app_ids));
    }

    // Join with the asset table.
    //$a_query[mediamosa_db_query::A_JOIN]['asset'] = 'LEFT JOIN {mediamosa_asset} AS a ON a.asset_id = ac.asset_id';

    $a_query[mediamosa_db_query::LIMIT] = $limit;
    $a_query[mediamosa_db_query::OFFSET] = $offset;

    // Build the query.
    $query = mediamosa_db_query::query_select($a_query, array(mediamosa_db_query::SQL_CALC_FOUND_ROWS => $calculate_total_count));

    // Do the query.
    $result = mediamosa_db::db_query($query);

    // Set the found_rows on the object.
    $result->found_rows = mediamosa_db_query::found_rows();

    // Return the result.
    return $result;
  }
}
