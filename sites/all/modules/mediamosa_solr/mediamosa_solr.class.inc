<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Our extended classes for Solr connections.
 */

class mediamosa_solr {

  // ------------------------------------------------------------------- Consts.
  // In seconds, how long to process the queue before we stop for next run.
  const TIME_TO_PROCESS_QUEUE = 54;
  // when having x number assets, then commit. Be careful with memory when
  // increasing.
  const COMMIT_BULK_PER = 50;
  // Only store 50 chars max for sorting text.
  const VAR_CHAR_SORT_LENGTH = 50;
  // Number of runs we remember the # index during run.
  const TAKE_AVERAGE_LAST_X_MIN = 5;

  const WHERE_AND = 'AND';
  const WHERE_OR = 'OR';

  // array name for search in where
  const CQL = 'cql';
  const M_APPS = 'm_apps';
  const APPS = 'apps';
  const SLAVE_APPS = 's_apps';
  const MF = 'mf';
  const ASSET = 'asset';
  const NO_MF = 'no_mf';
  const COLLS = 'colls';
  const BATCH = 'batch';
  const USER_FAV = 'user_fav';
  const ORG = 'original';
  const IS_PROTECTED = 'is_protected';
  const IS_PROTECTED_FALSE = 'is_protected_false';
  const IS_PROTECTED_TRUE = 'is_protected_true';
  const ACL = 'acl';
  const ACL_NAME = 'name';
  const ACL_GRP = 'group';

  // ---------------------------------------------------------------- Functions.
  static public function asset_search($parameters) {

    // Make sure its unique.
    $parameters['app_ids'] = array_unique($parameters['app_ids']);

    $solr_query = array();

    // Get the slaves based on the app ids for mediafiles.
    $slave_mediafiles = mediamosa_acl_app_master_slave::slave_get($parameters['app_ids'], mediamosa_acl::ACL_TYPE_MEDIAFILE);

    // Get the slaves based on the app ids for assets.
    $slave_assets = array(); // Off, not stable, mediamosa_acl_app_master_slave::slave_get($a_app_ids, mediamosa_acl::ACL_TYPE_ASSET);

    // Merge the slave app ids with my app ids.
    $app_ids_all = array_unique(array_merge($parameters['app_ids'], array_keys($slave_mediafiles), array_keys($slave_assets)));
    sort($app_ids_all);

    // Need these.
    foreach ($app_ids_all as $app_id) {
      $solr_query[self::WHERE_AND][self::ASSET][self::APPS][self::WHERE_OR][] = 'app_id:' . (int) $app_id;
    }

    // isprivate / unappropiate test.
    // Is outside the ACL check, else we would have problems with 'granted'.
    if (!$parameters['is_app_admin']) {
      if ($parameters['is_public_list'] && $parameters['acl_user_id']) {
        $solr_query[self::WHERE_AND][self::ASSET][self::ACL][self::WHERE_AND][] = strtr('(isprivate:FALSE AND (is_unappropriate:FALSE OR owner_id:@owner_id))', array('@owner_id' => mediamosa_solr_apache_solr_service::escapeForSpace($parameters['acl_user_id'])));
      }
      elseif ($parameters['is_public_list']) {
        $solr_query[self::WHERE_AND][self::ASSET][self::ACL][self::WHERE_AND][] = '(isprivate:FALSE AND is_unappropriate:FALSE)'; // Must both be FALSE
      }
      elseif ($parameters['acl_user_id']) { // if provided, then we only have access to unappropate when owner.
        $solr_query[self::WHERE_AND][self::ASSET][self::ACL][self::WHERE_AND][] = strtr('(is_unappropriate:FALSE OR owner_id:@owner_id)', array('@owner_id' => mediamosa_solr_apache_solr_service::escapeForSpace($parameters['acl_user_id'])));
      }
      else {
        // No public list, no acl_user_id
        $solr_query[self::WHERE_AND][self::ASSET][self::ACL][self::WHERE_AND][] = 'is_unappropriate:FALSE'; // Ignore isprivate, is_unappropriate must be TRUE
      }
    }

    // Switch to hide assets that have no mediafiles and no metadata.
    if ($parameters['hide_empty_assets']) {
      // exclude empty assets
      $solr_query[self::WHERE_AND][self::ASSET][self::ACL][self::WHERE_AND][] = 'is_empty_asset:FALSE';
    }

    if (!empty($parameters['cql'])) {
      $result = mediamosa_solr_cql::parse_asset($parameters['cql'], $parameters['cql_store_stats'], $parameters['app_ids']);

      // When searching with ACL search rule set, we need special case.
      if (!empty($result['is_acl_search'])) {
        // Granted = TRUE, see all.
        $parameters['granted'] = TRUE;

        // Unset access where.
        unset($solr_query[self::WHERE_AND][self::ASSET][self::ACL]);

        // Special case.
        if (in_array('ACL_APP_MASTER_SLAVE', $result['a_joins'])) {
          // Get all assets of our app(s) and slave apps.
          foreach ($parameters['app_ids'] as $slave_app_id) {
            $solr_query[self::WHERE_AND][self::ASSET][self::SLAVE_APPS][self::WHERE_OR][] = 'mf_app_id_slave:' . (int) $slave_app_id;
          }
        }
      }

      // We got something from cql?
      if (!empty($result['str_where'])) {
        $solr_query[self::WHERE_AND][self::CQL] = $result['str_where'];
      }

      $order_bys = reset($result['a_order_by']);

      $order_by = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT];
      $order_direction = drupal_strtolower($order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_DIRECTION]);
      $order_type = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_TYPE];
      $order_table = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_TABLE_FOR_SORT];
      $is_metadata = $order_bys[MEDIAMOSA_CQL_CONTEXT_KEY_PROP_ID] > 0;
    }

    // If coll_id is given, then search within the given collection(s)
    if (!empty($parameters['coll_id'])) {
      assert(is_array($parameters['coll_id']));

      foreach ($parameters['coll_id'] as $coll_id) {
        // Not a app_admin, then rules apply.
        // coll_id _ isprivate _ is_unappropriate _ owner_id
        // coll_id _ owner_id
        if (!$parameters['is_app_admin']) {
          $isprivate_false_false = mediamosa_solr_apache_solr_service::escapeForSpace(strtr('@coll_id_FALSE_FALSE', array('@coll_id' => $coll_id)));
          $isprivate_false_true = mediamosa_solr_apache_solr_service::escapeForSpace(strtr('@coll_id_FALSE_TRUE', array('@coll_id' => $coll_id)));
          $is_unappropriate_false_false = mediamosa_solr_apache_solr_service::escapeForSpace(strtr('@coll_id_FALSE_FALSE', array('@coll_id' => $coll_id)));
          $is_unappropriate_true_false = mediamosa_solr_apache_solr_service::escapeForSpace(strtr('@coll_id_TRUE_FALSE', array('@coll_id' => $coll_id)));
          $owner = mediamosa_solr_apache_solr_service::escapeForSpace(strtr('@coll_id_@owner', array('@coll_id' => $coll_id, '@owner' => mediamosa_solr_apache_solr_service::filterDelimeters($parameters['acl_user_id']))));

          if ($parameters['is_public_list'] && $parameters['acl_user_id']) {
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('((coll_id_ext:@isprivate_1 OR coll_id_ext:@isprivate_2) AND (coll_id_ext:@unappropriate_1 OR coll_id_ext:@unappropriate_2 OR coll_id_owner:@owner))' , array('@isprivate_1' => $isprivate_false_false, '@isprivate_2' => $isprivate_false_true, '@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false, '@owner' => $owner));
          }
          elseif ($parameters['is_public_list']) {
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('((coll_id_ext:@isprivate_1 OR coll_id_ext:@isprivate_2) AND (coll_id_ext:@unappropriate_1 OR coll_id_ext:@unappropriate_2))' , array('@isprivate_1' => $isprivate_false_false, '@isprivate_2' => $isprivate_false_true, '@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false));
          }
          elseif ($parameters['acl_user_id']) { // if provided, then we only have access to unappropate when owner.
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('(coll_id_ext:@unappropriate_1 OR coll_id_ext:@unappropriate_2 OR coll_id_owner:@owner)' , array('@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false, '@owner' => $owner));
          }
          else {
            $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR][] = strtr('(coll_id_ext:@unappropriate_1 OR coll_id_ext:@unappropriate_2)' , array('@unappropriate_1' => $is_unappropriate_false_false, '@unappropriate_2' => $is_unappropriate_true_false));
          }
        }
        else {
          $coll_id = mediamosa_solr_apache_solr_service::escape(strtr('@coll_id_', array('@coll_id' => $coll_id)));
          $solr_query[self::WHERE_AND][self::COLLS][self::WHERE_OR] = strtr('coll_id_ext:@coll_id*', array('@coll_id' => $coll_id));
        }
      }
    }

    // FTP Batch.
    if (!empty($parameters['batch_id'])) {
      $solr_query[self::WHERE_AND][self::BATCH] = strtr('batch_id:@batch_id', array('@batch_id' => mediamosa_solr_apache_solr_service::escapeForSpace($parameters['batch_id'])));
    }

    if (!empty($parameters['fav_user_id'])) {
      foreach ($parameters['app_ids'] as $app_id) {
        $user_fav = mediamosa_solr_apache_solr_service::escapeForSpace(strtr('@app_id_@name', array('@app_id' => $app_id, '@name' => mediamosa_solr_apache_solr_service::filterDelimeters($parameters['fav_user_id']))));
        $solr_query[self::WHERE_AND][self::USER_FAV][self::WHERE_OR][] = strtr('user_fav:@user_fav', array('@user_fav' => $user_fav));
      }
    }

    // ACL layer.
    self::build_access_where($solr_query, mediamosa_acl::ACL_TYPE_MEDIAFILE, NULL, $parameters['app_ids'], $parameters['acl_user_id'], $parameters['acl_group_ids'], $parameters['acl_domain'], $parameters['acl_realm'], $slave_mediafiles, FALSE, $parameters['granted']);

    // Build query.
    $query = self::where($solr_query);

    // Setup sorting.
    $query_params = array();
    if (!empty($order_by)) {
      $query_params['sort'] = $order_by . ($is_metadata ? ($order_direction == 'ASC' ? '_lo' : '_hi') : '') . ' ' . $order_direction;
    }

    // Do query.
    $mediamosa_solr_apache_solr_response = mediamosa_solr::search($query, $parameters['offset'], $parameters['limit'], $query_params);

    // Done.
    return array('asset_ids' => $mediamosa_solr_apache_solr_response->getAssetIds(), 'total_count' => $mediamosa_solr_apache_solr_response->getNumFound());
  }

  /**
   * Our solr version of mediamosa_acl::build_access_where().
   */
  static public function build_access_where(&$solr_query, $acl_type, $object_id, array $master_app_ids, $acl_user_id, array $acl_group_ids, $acl_domain, $acl_realm, array $slaves = array(), $is_app_admin = FALSE, $granted = FALSE, $do_master_slave_null = FALSE, $app_id_table_prefix = 'a') {

    // Only supports mediamosa_acl::ACL_TYPE_MEDIAFILE.
    if ($acl_type != mediamosa_acl::ACL_TYPE_MEDIAFILE) {
      return mediamosa_acl::build_access_where($solr_query, $acl_type, $object_id, $master_app_ids, $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, $slaves, $is_app_admin, $granted, $do_master_slave_null, $app_id_table_prefix);
    }

    if (empty($slaves)) {
      $slaves = mediamosa_acl_app_master_slave::slave_get($master_app_ids, $acl_type);
    }

    // These must be true.
    assert(is_null($object_id));
    assert($do_master_slave_null === FALSE);
    assert($app_id_table_prefix == 'a');

    // First app is considered the main app(!).
    $build_access_app_ids = array_unique(array_merge($master_app_ids, array_keys($slaves)));

    // Get of this current user the acl_name/acl_group ids so we know his/her access.
    $acl_ids = mediamosa_acl::build_access($build_access_app_ids, $acl_user_id, $acl_group_ids, $acl_domain, $acl_realm, $slaves);

    if (!empty($acl_user_id) || !empty($acl_group_ids) || count($acl_ids) || count($slaves)) {
      // Even if we return all stuff where we have access to, we must still grant only access to master/slave apps

      if (!$granted) {
        // Include all assets without original mediafile for master app_id
        foreach ($master_app_ids as $app_id) {
          $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::NO_MF][$app_id][self::WHERE_AND][] = 'app_id:' . (int) $app_id;
          $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::NO_MF][$app_id][self::WHERE_AND][] = 'mf:FALSE';
        }

        // If we have acl id for the master app, check them
        foreach ($master_app_ids as $app_id) {
          // Master app_id checks
          $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND][] = 'app_id:' . (int) $app_id;
          $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND][self::IS_PROTECTED][self::WHERE_OR][] = 'mf_org_is_protected:0';

          if (!empty($acl_ids[$app_id])) {

            // App_id is the same AND [is_protected != FALSE AND] one of the acl_rules match.
            foreach ($acl_ids[$app_id]['a_name'] as $acl_id) {
              $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND][self::IS_PROTECTED][self::WHERE_OR][self::IS_PROTECTED_TRUE][self::WHERE_AND][self::WHERE_OR]['name'][] = 'mf_org_acl_obj_acl_name:' . mediamosa_solr_apache_solr_service::escape($acl_id);
            }

            // App_id is the same AND [is_protected != FALSE AND] one of the acl_rules match.
            foreach ($acl_ids[$app_id]['a_group'] as $acl_id) {
              $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND][self::IS_PROTECTED][self::WHERE_OR][self::IS_PROTECTED_TRUE][self::WHERE_AND][self::WHERE_OR]['group'][] = 'mf_org_acl_obj_acl_group:' . mediamosa_solr_apache_solr_service::escape($acl_id);
            }
          }

          if (!empty($acl_user_id)) {
            $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND][self::IS_PROTECTED][self::WHERE_OR]['owner'] = strtr('mf_org_owner:@owner_id', array('@owner_id' => mediamosa_solr_apache_solr_service::escapeForSpace($acl_user_id)));
          }

          // group_id
          foreach ($acl_group_ids as $acl_group_id) {
            if (!empty($acl_group_id)) {
              $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id][self::WHERE_AND][self::IS_PROTECTED][self::WHERE_OR]['group'][] = strtr('mf_org_group:@group_id', array('@group_id' => mediamosa_solr_apache_solr_service::escapeForSpace($acl_group_id)));
            }
          }
        }
      }
      else {
        // When $do_master_slave_only is TRUE, we have always access to media of our own app_id(s)
        foreach ($master_app_ids as $app_id) {
          $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR]['master_app_id'][self::WHERE_AND]['ms'][] = 'app_id:' . (int) $app_id;
        }
      }

      // possible slaves
      foreach (array_keys($slaves) as $app_id_slave) {
        assert($app_id_slave);// should not happen

        // Must be same app_id for slaves.
        $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][] = 'app_id:' . (int) $app_id_slave;

        // Check if master/slave exists.
        if ($granted) {
          foreach ($master_app_ids as $master_app_id) {
            $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][self::WHERE_OR][] = 'mf_acl_master_slave:' . mediamosa_solr_apache_solr_service::escape(implode('_', array($app_id_slave, $master_app_id)) . '_') . '*';
          }

          continue;
        }

        // 2 cases;
        // 1. The object is protected only app_id (is_protected == FALSE)
        //   then only access if app_id == slave_app_id
        // 2. The object is protected by other rights (is_protected == TRUE)
        //   then only access if app_id == slave_app_id and rights match with other rights

        foreach ($master_app_ids as $master_app_id) {
          // Allow any master/slave without ACL rights.
          $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][self::WHERE_OR][] = 'mf_acl_master_slave:' . mediamosa_solr_apache_solr_service::escape(implode('_', array($app_id_slave, $master_app_id, mediamosa_acl::MEDIAFILE_IS_PROTECTED_FALSE)));

          // Only DOMAIN_REALM require ACL.
          if (!empty($acl_ids[$app_id_slave]['a_name'])) {
            foreach ($acl_ids[$app_id_slave]['a_name'] as $acl_id) {
              $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][self::WHERE_OR][] = 'mf_acl_master_slave:' . mediamosa_solr_apache_solr_service::escape(implode('_', array($app_id_slave, $master_app_id, mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl_object_db::ACL_TYPE_NAME, $acl_id)));
            }
          }

          if (!empty($acl_ids['slaves_convert']['a_group'][$app_id_slave])) {
            foreach ($acl_ids['slaves_convert']['a_group'][$app_id_slave] as $acl_id) {
              $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::MF][self::WHERE_AND][self::ORG][self::WHERE_OR][$app_id_slave][self::WHERE_AND][self::WHERE_OR][] = 'mf_acl_master_slave:' . mediamosa_solr_apache_solr_service::escape(implode('_', array($app_id_slave, $master_app_id, mediamosa_acl::MEDIAFILE_IS_PROTECTED_TRUE, mediamosa_acl_object_db::ACL_TYPE_GROUP, $acl_id)));
            }
          }
        }
      }
    }
    else {
      $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::NO_MF] = 'mf:FALSE';
      // Include all assets with original mediafile for master app_id and no protection.
      foreach ($master_app_ids as $app_id) {
        $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::APPS][$app_id][self::WHERE_AND][] = 'app_id:' . (int) $app_id;
        $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::APPS][$app_id][self::WHERE_AND][] = 'mf_org:TRUE';

        if (!$granted) {
          $solr_query[self::WHERE_AND][self::ACL][self::WHERE_OR][self::APPS][$app_id][self::WHERE_AND][] = 'mf_org_is_protected:0';
        }
      }
    }
  }

  /**
   * Check access on assets using given array asset_ids
   *
   * @param array $asset_ids
   * @param integer $app_id
   * @param string $acl_user_id
   * @param array $acl_group_ids
   * @param string $acl_domain
   * @param string $acl_realm
   * @return array
   *  returns the array with asset_id(s) on which we have access on
   */
  static public function access_check_assets(array $parameters) {

    $asset_ids = array();

    $solr_query = array();

    // Assets to process.
    foreach ($parameters['asset_ids'] as $asset_id) {
      $solr_query[self::WHERE_AND][self::ASSET][self::WHERE_OR][] = 'asset_id:' . mediamosa_solr_apache_solr_service::escapeForSpace($asset_id);
    }

    // ACL layer.
    self::build_access_where($solr_query, mediamosa_acl::ACL_TYPE_MEDIAFILE, NULL, $parameters['app_ids'], $parameters['acl_user_id'], $parameters['acl_group_ids'], $parameters['acl_domain'], $parameters['acl_realm'], array(), $parameters['is_app_admin']);

    // Build query.
    $query = self::where($solr_query);

    // Do query.
    $mediamosa_solr_apache_solr_response = mediamosa_solr::search($query);

    // Get the asset_ids.
    $result = $mediamosa_solr_apache_solr_response->getAssetIds();

    foreach ($result as $asset_id) {
      $asset_ids[$asset_id] = $asset_id;
    }

    return $asset_ids;
  }

  /**
   * Instant update on metadata?
   */
  static public function settingInstantlyMetadata() {
    return variable_get('mediamosa_solr_metadata_instantly', TRUE);
  }


  /**
   * Instant update on asset update?
   */
  static public function settingInstantlyAssetCreate() {
    return variable_get('mediamosa_solr_asset_create_instantly', TRUE);
  }

  /**
   * Instant update on asset update?
   */
  static public function settingInstantlyAssetUpdate() {
    return variable_get('mediamosa_solr_asset_update_instantly', TRUE);
  }

  /**
   * Instant update on mediafile update?
   */
  static public function settingInstantlyMediafileUpdate() {
    return variable_get('mediamosa_solr_mediafile_update_instantly', TRUE);
  }

  /**
   * Instant update on normalize asset?
   */
  static public function settingInstantlyAssetNormalize() {
    return self::settingInstantlyAssetUpdate(); // The same setting for now.
  }

  /**
   * Instant update on normalize asset?
   */
  static public function settingInstantlyUserFav() {
    return variable_get('mediamosa_solr_user_fav_instantly', TRUE);
  }

  /**
   * Instant update on normalize asset?
   */
  static public function settingInstantlyMasterSlave() {
    return variable_get('mediamosa_solr_master_slave_instantly', TRUE);
  }

  /**
   * Instant update on ACL rules?
   */
  static public function settingInstantlyACL() {
    return variable_get('mediamosa_solr_acl_instantly', TRUE);
  }

  /**
   * Instant update on asset collection relations?
   */
  static public function settingInstantlyAssetCollection() {
    return variable_get('mediamosa_solr_assetcollection_instantly', TRUE);
  }

  /**
   * Common log function.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   * @param string $link
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE, $link = NULL) {
    mediamosa_watchdog::log($message, $variables, $severity, 'Solr');
  }

  /**
   * Reindex with given asset_ids. Stores it in table, will be reindexed asap.
   *
   * @param string $asset_id
   */
  static public function queue_assets(array $asset_ids) {

    foreach ($asset_ids as $asset_id) {
      try {
        if (mediamosa_db::db_select(mediamosa_solr_queue_db::TABLE_NAME, 'sq')
          ->condition(mediamosa_solr_queue_db::ID, $asset_id)
          ->countQuery()->execute()->fetchField() == 0) {

            // If it fails because of existing primary key, then ignore.
            mediamosa_db::db_insert(mediamosa_solr_queue_db::TABLE_NAME)
              ->fields(array(
                  mediamosa_solr_queue_db::ID => $asset_id,
                  mediamosa_solr_queue_db::CREATED => mediamosa_datetime::db_current_timestamp_now()
              ))
              ->execute();
          }
      }
      catch (PDOException $e) {
        // Ignore, we have no way to lock the table in drupal db layer, so no
        // select or update will not work here.
        mediamosa_watchdog::log($e->getMessage());
      }
      catch (Exception $e) {
        mediamosa_watchdog::log($e->getMessage());
      }
    }
  }

  /**
   * Remove the asset(s) from solr.
   *
   * @param string $asset_id
   */
  static public function delete_assets(array $asset_ids) {

    foreach ($asset_ids as $asset_id) {
      try {
        mediamosa_solr::delete_asset_document($asset_id, FALSE);
      }
      catch (Exception $e) {
        mediamosa_watchdog::log('Unable to delete asset from Solr.');
      }
    }

    // Commit all.
    self::commit();
  }

  /**
   * Insert asset data without delay.
   *
   * @param string $asset_id
   */
  static public function insert_assets(array $asset_ids) {

    $documents = array();
    foreach ($asset_ids as $asset_id) {
      try {
        $document = self::create_asset_document($asset_id);
        if (empty($document)) {
          $documents['asset_ids'][] = $asset_id; // So its removed from queue.
          continue; // Not found.
        }

        // Create the document.
        $documents['documents'][] = $document;
        $documents['asset_ids'][] = $asset_id;

        if (count($documents) == self::COMMIT_BULK_PER) {
          self::commit_documents($documents);
          $documents = array();
        }
      }
      catch (Exception $e) {
        // If one fails, then we can't fail the rest too.
       self::log($e->getMessage());
      }
    }

    // Commit if anything left.
    try {
      self::commit_documents($documents);
    }
    catch (Exception $e) {
      // Ignore.
      self::log($e->getMessage());
    }
  }

  /**
   * Return the count of the number of queued assets.
   */
  static public function queue_get_count() {
    return mediamosa_db::db_select(mediamosa_solr_queue_db::TABLE_NAME, 'sq')->countQuery()->execute()->fetchField();
  }

  /**
   * Triggered by REST call. Should not be run DURING the CRON!
   *
   * Will reindex the solr database as much as its allowed during this call.
   */
  static public function process_queue() {

    $mediamosa_solr_enable_queue = variable_get('mediamosa_solr_enable_queue', TRUE);
    if (empty($mediamosa_solr_enable_queue)) {
      // Always Optimize.
      self::optimize();

      return;
    }

    // Number of seconds the reindex is running max.
    $set_time_limit = variable_get('mediamosa_solr_reindex_time_limit', mediamosa_solr::TIME_TO_PROCESS_QUEUE);

    // Get possible current running reindex. Make sure its not running too.
    $run_last = variable_get('mediamosa_solr_queue_cron_last', NULL);

    // Make sure we don't run the queue process twice.
    // We will give it enough time to timeout.
    if ($run_last && round(REQUEST_TIME - $run_last) < (mediamosa_solr::TIME_TO_PROCESS_QUEUE * 3)) {
      // Only if its running about twice when we expected we should watchdog it.
      if (round(REQUEST_TIME - $run_last) > (mediamosa_solr::TIME_TO_PROCESS_QUEUE * 2)) {
        self::log('Unexpected; Solr reindex was still / already running when cron was triggered.');
      }

      return;
    }

    // If we get here then we can go.
    variable_set('mediamosa_solr_queue_cron_last', REQUEST_TIME);

    try {
      // 10 seconds more than we think we need.
      set_time_limit($set_time_limit + 10);

      // At where
      $end_at = REQUEST_TIME + $set_time_limit;

      $indexed = 0;

      // Get the now time, as long as we under end_at then continue.
      while (floor(microtime(TRUE)) < $end_at) {
        // Do our stuff until we are done or till we run out of time.
        $query = mediamosa_db::db_select(mediamosa_solr_queue_db::TABLE_NAME, 'sq');
        $query->fields('sq', array(mediamosa_solr_queue_db::ID));
        $query->range(0, self::COMMIT_BULK_PER);
        $query->orderBy(mediamosa_solr_queue_db::CREATED, 'ASC');
        $asset_ids = $query->execute()->fetchCol();

        // None left?
        if (empty($asset_ids)) {
          break;
        }

        $documents = array();
        foreach ($asset_ids as $asset_id) {
          $document = self::create_asset_document($asset_id);
          if (empty($document)) {
            // Delete from Solr index.
            self::delete_assets(array($asset_id));

            $documents['asset_ids'][] = $asset_id; // So its removed from queue.
            continue; // Not found.
          }

          // Create the document.
          $documents['documents'][] = $document;
          $documents['asset_ids'][] = $asset_id;

          $indexed++;
        }

        // Commit will also remove the asset_ids from the queue.
        self::commit_documents($documents);
      }

      if ($indexed) {
        // Optimize.
        self::optimize();

        mediamosa_debug::log('Solr index run done, indexed @count assets', array('@count' => $indexed));
      }
    }
    catch (Exception $e) {
      // Exception, turn off that we are processing.
      variable_set('mediamosa_solr_queue_cron_last', NULL);
      variable_set('mediamosa_solr_queue_failures', $e->getMessage());
      throw $e;
    }

    // Done.
    variable_del('mediamosa_solr_queue_cron_last');
    variable_del('mediamosa_solr_queue_failures');

    // Save number of indexes for averages calc.
    $stats = variable_get('mediamosa_solr_queue_stats', array());
    $stats[] = $indexed;
    while (count($stats) > self::TAKE_AVERAGE_LAST_X_MIN) {
      array_shift($stats);
    }
    variable_set('mediamosa_solr_queue_stats', $stats);
  }

  /**
   * Will throw away index in Solr database, be careful with this function!
   *
   * @throws mediamosa_solr_exception
   */
  static public function purge_solr_index() {
    // Now purge, don't commit yet.
    self::deleteByQuery('*:*');

    // Commit.
    self::commit();
  }

  /**
   * Do search query.
   *
   * @param string $query
   *
   * @return
   *  Returns the Solr Response object.
   */
  static public function search($query, $offset = 0, $limit = 10, $params = array(), $method = 'POST') {
    $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();
    return new mediamosa_solr_apache_solr_response($mediamosa_apache_solr_service->search($query, $offset, $limit, $params, $method));
  }

  /**
   * Delete documents based on query. Don't forget to call commit.
   *
   * @param $query
   *
   * @return
   *  Returns the Solr Response object.
   */
  static public function deleteByQuery($query) {
    $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();

    return new mediamosa_solr_apache_solr_response($mediamosa_apache_solr_service->deleteByQuery($query));
  }

  /**
   * All must be commited.
   */
  static public function commit() {
    try {
      $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();
      $mediamosa_apache_solr_service->commit();
    }
    catch (Exception $e) {
      self::log($e->getMessage());
    }
  }

  /**
   * Delete documents of test data of the given app_ids.
   *
   * @param array $app_ids
   */
  static public function delete_simpletest_documents(array $app_ids = array()) {
    try {
      if (empty($app_ids)) {
        $solr_response = self::search('app_id:[' . (mediamosa_settings::APP_ID_MAX + 1) . ' TO *]');

        // How many to delete?
        $found = $solr_response->getNumFound();

        if ($found) {
          self::deleteByQuery('app_id:[' . (mediamosa_settings::APP_ID_MAX + 1) . ' TO *]');
          self::commit();
        }

        return $found;
      }
      else {
        $found_total = 0;

        // Delete metadata using the app_ids.
        foreach ($app_ids as $app_id) {
          $solr_response = self::search('app_id:' . $app_id);

          // How many to delete?
          $found = $solr_response->getNumFound();

          if ($found) {
            $found_total += $found;
            self::deleteByQuery('app_id:' . $app_id);
          }

        }

        // Commit.
        self::commit();

        return $found_total;
      }
    }
    catch (Exception $e) {
      // Cant react on exception, we might be in destructor.

      self::commit();
      return 0;
    }
  }

  /**
   * Delete one asset with data.
   */
  static public function delete_asset_document($asset_id, $commit = TRUE) {
    self::deleteByQuery('asset_id:' . mediamosa_solr_apache_solr_service::phrase($asset_id));

    if ($commit) {
      self::commit();
    }
  }

  /**
   * Insert one asset with data.
   */
  static public function create_asset_document($asset_id) {

    // Get all the data needed for indexing.
    $asset_for_index = mediamosa_asset::get_asset_for_index($asset_id);
    if (empty($asset_for_index)) {
      return FALSE;
    }

    // Create empty document object asset;
    $document = new Apache_Solr_Document();

    // Asset data.
    $document->app_id = $asset_for_index['asset'][mediamosa_asset_db::APP_ID];
    $document->asset_id = $asset_for_index['asset'][mediamosa_asset_db::ID];

    $document->group_id = $asset_for_index['asset'][mediamosa_asset_db::GROUP_ID];
    $document->owner_id = $asset_for_index['asset'][mediamosa_asset_db::OWNER_ID];

    $document->viewed = $asset_for_index['asset'][mediamosa_asset_db::VIEWED];
    $document->played = $asset_for_index['asset'][mediamosa_asset_db::PLAYED];

    $asset_optionals = array(
      mediamosa_asset_db::PROVIDER_ID,
      mediamosa_asset_db::REFERENCE_ID,
      mediamosa_asset_db::MEDIAFILE_DURATION,
      mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE,
    );

    foreach ($asset_optionals as $optional) {
      if (!empty($asset_for_index['asset'][$optional])) {
        $document->{$optional} = $asset_for_index['asset'][$optional];
      }
    }

    // Some metadata of mediafiles.
    foreach ($asset_for_index['mediamosa_asset_mediafile_metadata'] as $mediafile_id => $mediafile_metadata) {
      // Technical metadata.
      foreach (array(mediamosa_asset_mediafile_metadata::MIME_TYPE) as $name) {
        if (!empty($mediafile_metadata['metadata'][$name])) {
          $document->{$name} = $mediafile_metadata['metadata'][$name];
        }
      }
    }

    // Get a mediafiles.
    foreach ($asset_for_index['mediafiles'] as $mediafile_id => $mediafile) {
      // Skip stills.
      if ($mediafile[mediamosa_asset_mediafile_db::IS_STILL] == mediamosa_asset_mediafile_db::IS_STILL_TRUE) {
        continue;
      }

      // Mediafile.
      foreach (array(
        mediamosa_asset_mediafile_db::FILENAME,
        mediamosa_asset_mediafile_db::ID,
        mediamosa_asset_mediafile_db::URI,
        ) as $name) {
        if (!empty($mediafile[$name])) {
          $document->{$name} = $mediafile[$name];
        }
      }
    }

    $asset_optionals_dates = array(
      mediamosa_asset_db::VIDEOTIMESTAMP,
      mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED,
      mediamosa_asset_db::CREATED,
      mediamosa_asset_db::CHANGED,
    );

    foreach ($asset_optionals_dates as $optional) {
      if (!empty($asset_for_index['asset'][$optional]) && $asset_for_index['asset'][$optional] != '0000-00-00 00:00:00') {
        $asset_for_index['asset'][$optional] = drupal_substr($asset_for_index['asset'][$optional], 0, 10) . 'T' . drupal_substr($asset_for_index['asset'][$optional], 11, 8) . 'Z';

        // Do not index invalid dates(!).
        if (!preg_match('/\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z/', $asset_for_index['asset'][$optional])) {
          continue;
        }

        $document->{$optional} = $asset_for_index['asset'][$optional];
      }
    }

    // Optional.
    if (!empty($asset_for_index['batch_ids'])) {
      $document->batch_id = $asset_for_index['batch_ids'];
    }

    $document->is_empty_asset = $asset_for_index['asset'][mediamosa_asset_db::IS_EMPTY_ASSET] == mediamosa_asset_db::IS_EMPTY_ASSET_FALSE ? mediamosa_asset_db::IS_EMPTY_ASSET_FALSE : mediamosa_asset_db::IS_EMPTY_ASSET_TRUE;
    $document->isprivate = $asset_for_index['asset'][mediamosa_asset_db::ISPRIVATE] == mediamosa_asset_db::ISPRIVATE_TRUE ? mediamosa_asset_db::ISPRIVATE_TRUE : mediamosa_asset_db::ISPRIVATE_FALSE;
    $document->is_unappropriate = $asset_for_index['asset'][mediamosa_asset_db::IS_UNAPPROPRIATE] == mediamosa_asset_db::IS_UNAPPROPRIATE_TRUE ? mediamosa_asset_db::IS_UNAPPROPRIATE_TRUE : mediamosa_asset_db::IS_UNAPPROPRIATE_FALSE;
    $document->is_external = $asset_for_index['asset'][mediamosa_asset_db::IS_EXTERNAL] == mediamosa_asset_db::IS_EXTERNAL_TRUE ? mediamosa_asset_db::IS_EXTERNAL_TRUE : mediamosa_asset_db::IS_EXTERNAL_FALSE;

    // Collection ids. collid_isprivate_isunappropriate_
    if (!empty($asset_for_index['collections'])) {
      foreach ($asset_for_index['collections'] as $coll_id => $collection) {
        $isprivate = $collection[mediamosa_collection_db::ISPRIVATE] == mediamosa_collection_db::ISPRIVATE_TRUE ? mediamosa_collection_db::ISPRIVATE_TRUE : mediamosa_collection_db::ISPRIVATE_FALSE;
        $is_unappropriate = $collection[mediamosa_collection_db::IS_UNAPPROPRIATE] == mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE ? mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE : mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE;
        $coll_ids[] = $coll_id;
        $coll_ids_ext[] = implode('_', array($coll_id, $isprivate, $is_unappropriate));
        $coll_id_owners[] = implode('_', array($coll_id, mediamosa_solr_apache_solr_service::filterDelimeters($collection[mediamosa_collection_db::OWNER_ID])));
      }

      $document->coll_id = $coll_ids;
      $document->coll_id_ext = $coll_ids_ext;
      $document->coll_id_owner = $coll_id_owners;
    }

    // User fav.
    if (!empty($asset_for_index['user_favorites'])) {
      foreach ($asset_for_index['user_favorites'] as $user_favorite) {
        $user_favorites[] = implode('_', array($user_favorite[mediamosa_user_favorite_db::APP_ID], mediamosa_solr_apache_solr_service::filterDelimeters($user_favorite[mediamosa_user_favorite_db::NAME])));
      }

      $document->user_fav = $user_favorites;
    }

    // Asset metadata.
    // [groupname][propertyname] = array('values' => array(value, value), 'type' => type).
    foreach ($asset_for_index['asset_metadata'] as $name => $metadata_group) {
      foreach ($metadata_group as $name => $metadata) {

        switch ($metadata['type']) {
          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            // Sort from low to high.
            sort($metadata['values'], SORT_STRING);

            $document->{$name . '_vc'} = $metadata['values'];
            $document->{$name . '_vc_exact'} = $metadata['values'];
            $document->{$name . '_svc_lo'} = drupal_substr(reset($metadata['values']), 0, self::VAR_CHAR_SORT_LENGTH);
            $document->{$name . '_svc_hi'} = drupal_substr(array_pop($metadata['values']), 0, self::VAR_CHAR_SORT_LENGTH);
            break;

          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            // Dates from mediamosa are not completely comp. with Solr.
            foreach ($metadata['values'] as $key => $date) {
              if ($date == '0000-00-00 00:00:00' || empty($date)) {
                continue;
              }

              $metadata['values'][$key] = drupal_substr($metadata['values'][$key], 0, 10) . 'T' . drupal_substr($metadata['values'][$key], 11, 8) . 'Z';

              // Do not index invalid dates(!).
              if (!preg_match('/\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z/', $metadata['values'][$key])) {
                unset($metadata['values'][$key]);
              }
            }

            // Values left?
            if (empty($metadata['values'][$key])) {
              continue;
            }

            // Sort from low to high.
            sort($metadata['values'], SORT_STRING);

            $document->{$name . '_vd'} = $metadata['values'];
            $document->{$name . '_svd_lo'} = reset($metadata['values']);
            $document->{$name . '_svd_hi'} = array_pop($metadata['values']);
            break;

          case mediamosa_asset_metadata_property_db::TYPE_INT:
            // Sort from low to high.
            sort($metadata['values'], SORT_NUMERIC);

            $document->{$name . '_vi'} = $metadata['values'];
            $document->{$name . '_svi_lo'} = reset($metadata['values']);
            $document->{$name . '_svi_hi'} = array_pop($metadata['values']);
            break;

          default:
            throw new mediamosa_solr_exception('Unsupported metadata type detected!');
        }
      }
    }

    // No original mediafiles?
    $document->mf_org = FALSE;

    // Has any mediafiles?
    $document->mf = !empty($asset_for_index['mediafiles']);

    // Walk through the mediafiles.
    foreach ($asset_for_index['mediafiles'] as $mediafile_id => $mediafile) {
      if (!empty($asset_for_index['acl']['acl_app_master_slave']['mediafile'][$mediafile_id])) {
        $mf_acl_master_slave = array();

        // Add master/slave rules.
        foreach ($asset_for_index['acl']['acl_app_master_slave']['mediafile'][$mediafile_id] as $acl_app_master_slave) {

          // Is protected has 2 values here.
          $is_protected = in_array($mediafile[mediamosa_asset_mediafile_db::IS_PROTECTED], array(mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE, mediamosa_asset_mediafile_db::IS_PROTECTED_DOMAIN_REALM)) ? mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE : mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE;

          $master_slave = array();
          $master_slave[] = $acl_app_master_slave[mediamosa_acl_app_master_slave_db::APP_ID_SLAVE];
          $master_slave[] = $acl_app_master_slave[mediamosa_acl_app_master_slave_db::APP_ID_MASTER];
          $master_slave[] = $is_protected;

          if ($is_protected == mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE) {
            // Make sure is_protected and acl rules are matching.
            assert(!empty($asset_for_index['acl']['acl_object']['mediafile'][$mediafile_id]));
            foreach ($asset_for_index['acl']['acl_object']['mediafile'][$mediafile_id] as $acl_object) {
              $master_slave_2 = $master_slave;
              $master_slave_2[] = $acl_object[mediamosa_acl_object_db::ACL_TYPE];
              $master_slave_2[] = $acl_object[mediamosa_acl_object_db::ACL_ID];
              $mf_acl_master_slave[] = implode('_', $master_slave_2);
            }

          }
          else {
            $mf_acl_master_slave[] = implode('_', $master_slave);
          }
        }

        if (!empty($mf_acl_master_slave)) {
          $document->mf_acl_master_slave = $mf_acl_master_slave;
        }
      }

      // Only originals.
      if ($mediafile[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE) {
        continue;
      }

      $document->mf_org = TRUE;

      // Owner / group.
      $mf_org_owner[] = $mediafile[mediamosa_asset_mediafile_db::OWNER_ID];

      // Group ID is optional.
      if (!empty($mediafile[mediamosa_asset_mediafile_db::GROUP_ID])) {
        $mf_org_group[] = $mediafile[mediamosa_asset_mediafile_db::GROUP_ID];
      }

      // Is protected.
      switch ($mediafile[mediamosa_asset_mediafile_db::IS_PROTECTED]) {
        case mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE:
          $mf_org_is_protected[] = 1;
          break;
        case mediamosa_asset_mediafile_db::IS_PROTECTED_DOMAIN_REALM:
          $mf_org_is_protected[] = 2;
          break;
        case mediamosa_asset_mediafile_db::IS_PROTECTED_USER_USERGROUP:
          $mf_org_is_protected[] = 3;
          break;

        default:
          assert(0);
        case mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE:
          $mf_org_is_protected[] = 0;
          break;
      }

      // Has acl_object rules for mediafile?
      if (!empty($asset_for_index['acl']['acl_object']['mediafile'][$mediafile_id])) {
        $mf_org_acl_obj_acl_name = $mf_org_acl_obj_acl_group = array();

        foreach ($asset_for_index['acl']['acl_object']['mediafile'][$mediafile_id] as $acl_object) {
          // Is protected has 2 values here.
          $is_protected = in_array($mediafile[mediamosa_asset_mediafile_db::IS_PROTECTED], array(mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE, mediamosa_asset_mediafile_db::IS_PROTECTED_DOMAIN_REALM)) ? mediamosa_asset_mediafile_db::IS_PROTECTED_TRUE : mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE;

          switch ($acl_object[mediamosa_acl_object_db::ACL_TYPE]) {
            case mediamosa_acl_object_db::ACL_TYPE_NAME:
              $mf_org_acl_obj_acl_name[] = (int) $acl_object[mediamosa_acl_object_db::ACL_ID];
              break;

            case mediamosa_acl_object_db::ACL_TYPE_GROUP:
              $mf_org_acl_obj_acl_group[] = (int) $acl_object[mediamosa_acl_object_db::ACL_ID];
              break;

            default:
              assert(0);
              break;
          }
        }

        if (!empty($mf_org_acl_obj_acl_name)) {
          $document->mf_org_acl_obj_acl_name = array_unique($mf_org_acl_obj_acl_name);
        }
        if (!empty($mf_org_acl_obj_acl_group)) {
          $document->mf_org_acl_obj_acl_group = array_unique($mf_org_acl_obj_acl_group);
        }
      }
    }

    if (!empty($mf_org_owner)) {
      $document->mf_org_owner = array_unique($mf_org_owner);
    }
    if (!empty($mf_org_group)) {
      $document->mf_org_group = array_unique($mf_org_group);
    }
    if (!empty($mf_org_is_protected)) {
      $document->mf_org_is_protected = array_unique($mf_org_is_protected);
    }

    // Default is not slaved.
    $is_master_slaved = FALSE;
    $is_protected = array();

    // Walk through the mediafiles, including all non-originals.
    foreach ($asset_for_index['mediafiles'] as $mediafile_id => $mediafile) {

      $is_protected[] = $mediafile[mediamosa_asset_mediafile_db::IS_PROTECTED];

      // Has acl_object rules for mediafile?
      if (!empty($asset_for_index['acl']['acl_app_master_slave']['mediafile'][$mediafile_id])) {
        $is_master_slaved = TRUE;

        // Add master/slave rules.
        foreach ($asset_for_index['acl']['acl_app_master_slave']['mediafile'][$mediafile_id] as $acl_app_master_slave) {
          // For ACL CQL context.
          $app_id_masters[] = $acl_app_master_slave[mediamosa_acl_app_master_slave_db::APP_ID_MASTER];
          $app_id_slaves[] = $acl_app_master_slave[mediamosa_acl_app_master_slave_db::APP_ID_SLAVE];
        }
      }
    }

    // Either the asset is master slaved, or not.
    $document->asset_is_master_slaved = $is_master_slaved;

    if (!empty($asset_for_index['acl']['acl_user']['mediafile'])) {
      $document->mf_acl_user = array_unique($asset_for_index['acl']['acl_user']['mediafile']);
    }
    if (!empty($asset_for_index['acl']['acl_user_group']['mediafile'])) {
      $document->mf_acl_user_group = array_unique($asset_for_index['acl']['acl_user_group']['mediafile']);
    }
    if (!empty($asset_for_index['acl']['acl_domain']['mediafile'])) {
      $document->mf_acl_domain = array_unique($asset_for_index['acl']['acl_domain']['mediafile']);
    }
    if (!empty($asset_for_index['acl']['acl_realm']['mediafile'])) {
      $document->mf_acl_realm = array_unique($asset_for_index['acl']['acl_realm']['mediafile']);
    }
    if (!empty($asset_for_index['acl']['acl_realm_prefix']['mediafile'])) {
      $document->mf_acl_realm_prefix = array_unique($asset_for_index['acl']['acl_realm_prefix']['mediafile']);
    }

    // ACL CQL.
    if (!empty($app_id_masters)) {
      $document->mf_app_id_master = array_unique($app_id_masters);
    }
    if (!empty($app_id_slaves)) {
      $document->mf_app_id_slave = array_unique($app_id_slaves);
    }

    if (!empty($is_protected)) {
      $document->mf_is_protected = array_unique($is_protected);
    }

    // Done.
    return $document;
  }

  /**
   * Commit the collected documents.
   *
   * @param array $documents
   */
  static public function commit_documents(array $documents, $optimize = FALSE) {
    if (!empty($documents['documents'])) {

      // Get the service object.
      $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();

      if (count($documents['documents']) == 1) {
        $mediamosa_apache_solr_service->addDocument(reset($documents['documents']));
      }
      else {
        $mediamosa_apache_solr_service->addDocuments($documents['documents']);
      }

      // Commit the documents.
      $mediamosa_apache_solr_service->commit();
    }

    // Optimize when indicated.
    if ($optimize) {
      // Optimize.
      $mediamosa_apache_solr_service->optimize();
    }

    // Remove from queue.
    mediamosa_db::db_delete(mediamosa_solr_queue_db::TABLE_NAME)->condition(mediamosa_solr_queue_db::ID, $documents['asset_ids'], 'IN')->execute();
  }

  /**
   * Do a optimize call.
   */
  static public function optimize() {
    // Get the service object.
    $mediamosa_apache_solr_service = mediamosa_solr_apache_solr_service::mediamosaGetSolrObject();

    // Optimize.
    $mediamosa_apache_solr_service->optimize();
  }

  /**
   * Create the WHERE syntax
   *
   * @param array $where
   * @return string
   */
  static public function where($where) {
    return implode(' AND ', self::where_2($where));
  }

  /**
   * self::where helper function
   *
   * @param array $where
   * @param string $l
   * @param string $r
   * @param string $glue
   * @return array
   */
  static public function where_2($where, $l = '', $r = '', $glue = ' AND ') {
    if (!is_array($where)) {
      return array($where);
    }

    $result = array();
    foreach ($where as $type => $statements) {
      if ($type == self::WHERE_AND) {
        $glue = " AND ";
        $statements = self::where_2($statements, '(', ')', " AND ");
        $result[] = $l . implode(" AND ", $statements) . $r;
      }
      elseif ($type == self::WHERE_OR) {
        $statements = self::where_2($statements, '(', ')', ' OR ');
        $result[] = $l . implode(' OR ', $statements) . $r;
      }
      else {
        $result[] = implode($glue, self::where_2($statements, '(', ')', $glue));
      }
    }

    return $result;
  }

  /**
   * Returns TRUE/FALSE to indicate we are active for updates etc.
   */
  static public function is_enabled() {
    return variable_get('mediamosa_search_engine', 'mediamosa_search') == 'mediamosa_solr';
  }
}
