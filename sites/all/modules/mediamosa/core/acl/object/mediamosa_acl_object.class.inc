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
  * The acl_object functions.
  */

class mediamosa_acl_object {
  // ------------------------------------------------------------------ Static Function.
  /**
   * Returns the join for joining with acl_object table
   *
   * @param enum $acl_type
   *  See mediamosa_acl::ACL_TYPE_*
   * @return string
   */
  public static function join_acl_object_get($acl_type) {
    switch ($acl_type) {
      case mediamosa_acl::ACL_TYPE_MEDIAFILE:
        $on = 'mf.mediafile_id';
        $alias = 'acl_obj';
        break;
      case mediamosa_acl::ACL_TYPE_COLLECTION:
        $on = 'c.coll_id';
        $alias = 'acl_obj';
        break;
      case mediamosa_acl::ACL_TYPE_ASSET:
        $on = 'a.asset_id';
        $alias = 'acl_obja';
        break;
      default:
        throw new mediamosa_exception_program_error('Unknown type used in mediamosa_acl_object::join_acl_object_get');
    }

    return strtr(
      "LEFT JOIN {#acl_object} AS #alias ON #alias.acl_object_type = '#acl_type' AND #on = #alias.acl_object_id",
      array(
        '#acl_object' => mediamosa_acl_object_db::TABLE_NAME,
        '#acl_type' => $acl_type,
        '#on' => $on,
        '#alias' => $alias,
      )
    );
  }

  /**
   * Delete all connection based on acl_type and acl_id.
   *
   * @param $acl_type
   * @param $acl_id
   */
  public static function delete($acl_type, $acl_id) {

    // Delete all connections.
    mediamosa_db::db_delete(mediamosa_acl_object_db::TABLE_NAME)
      ->condition(mediamosa_acl_object_db::ACL_TYPE, $acl_type)
      ->condition(mediamosa_acl_object_db::ACL_ID, $acl_id)
      ->execute();
  }

  /**
   * Creates a link between object and acl_name
   *
   * @param $acl_object_id
   * @param $acl_type
   * @param $acl_object_type
   * @param $acl_id
   * @param $rights (reserved)
   */
  public static function link_to_object($acl_object_id, $acl_type, $acl_object_type, $acl_id, array $rights = array(mediamosa_acl::RIGHT_ACCESS)) {
    // NOT master-slave safe.
    try {
      mediamosa_db::db_insert(mediamosa_acl_object_db::TABLE_NAME)
        ->fields(array(
          mediamosa_acl_object_db::ACL_OBJECT_ID => $acl_object_id,
          mediamosa_acl_object_db::ACL_TYPE => $acl_type,
          mediamosa_acl_object_db::ACL_OBJECT_TYPE => $acl_object_type,
          mediamosa_acl_object_db::ACL_ID => $acl_id,
        ))
        ->execute();
    }
    catch (PDOException $e) {
      assert($e);
      // ignore, double row.
    }
  }

  /**
   * Get the acl information for given asset.
   */
  public static function get_for_asset($asset_id) {

    // Collection of acl rules on asset/mediafiles.
    $asset_acl_info = array(
      'acl_object' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'acl_app_master_slave' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'acl_domain' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'acl_realm' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'acl_realm_prefix' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'acl_user' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'acl_user_group' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
    );

    // Get acl_object rules for all asset mediafiles.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->join(mediamosa_acl_object_db::TABLE_NAME, 'acl_obj', strtr("acl_obj.@acl_object_type = '@acl_object_type_mediafile' AND acl_obj.@acl_object_id = mf.@mediafile_id", array('@acl_object_type' => mediamosa_acl_object_db::ACL_OBJECT_TYPE, '@acl_object_type_mediafile' => mediamosa_acl_object_db::ACL_OBJECT_TYPE_MEDIAFILE, '@acl_object_id' => mediamosa_acl_object_db::ACL_OBJECT_ID, '@mediafile_id' => mediamosa_asset_mediafile_db::ID)));
    $query->condition('mf.asset_id', $asset_id);
    $query->fields('mf', array(mediamosa_asset_mediafile_db::ID));
    $query->fields('acl_obj', array(mediamosa_acl_object_db::ACL_TYPE, mediamosa_acl_object_db::ACL_ID));
    $result = $query->execute();

    $acl_name_ids = array();
    $acl_group_ids = array();

    // Now get all rules.
    foreach ($result as $acl_object) {
      $asset_acl_info['acl_object']['mediafile'][$acl_object[mediamosa_asset_mediafile_db::ID]][] = $acl_object;

      if ($acl_object[mediamosa_acl_object_db::ACL_TYPE] == mediamosa_acl_object_db::ACL_TYPE_NAME) {
        $acl_name_ids[] = $acl_object[mediamosa_acl_object_db::ACL_ID];
      }
      else {
        $acl_group_ids[] = $acl_object[mediamosa_acl_object_db::ACL_ID];
      }
    }

    if (!empty($acl_name_ids)) {
      // Now use the acl_object info and collect the data for acl_user.
      $query = mediamosa_db::db_select(mediamosa_acl_name_db::TABLE_NAME, 'an');
      $query->fields('an');
      $query->condition('an.acl_name_id', $acl_name_ids, 'IN');
      $result = $query->execute();

      foreach ($result as $acl_group) {
        switch ($acl_group[mediamosa_acl_name_db::ACL_TYPE]) {
          case mediamosa_acl_name_db::ACL_TYPE_DOMAIN:
            $asset_acl_info['acl_domain']['mediafile'][] = $acl_group[mediamosa_acl_name_db::ACL_NAME];
            break;

          case mediamosa_acl_name_db::ACL_TYPE_REALM:
            $asset_acl_info['acl_realm']['mediafile'][] = $acl_group[mediamosa_acl_name_db::ACL_NAME];
            $asset_acl_info['acl_realm_prefix']['mediafile'][] = (string) $acl_group[mediamosa_acl_name_db::ACL_PREFIX];
            break;

          case mediamosa_acl_name_db::ACL_TYPE_USER:
            $asset_acl_info['acl_user']['mediafile'][] = $acl_group[mediamosa_acl_name_db::ACL_NAME];
            break;

          case mediamosa_acl_name_db::ACL_TYPE_USER_GROUP:
            $asset_acl_info['acl_user_group']['mediafile'][] = $acl_group[mediamosa_acl_name_db::ACL_NAME];
            break;
        }
      }
    }

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->join(mediamosa_acl_app_master_slave_db::TABLE_NAME, 'acl_app', strtr("acl_app.@acl_object_type = '@acl_object_type_mediafile' AND acl_app.@acl_object_id = mf.@mediafile_id", array('@acl_object_type' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE, '@acl_object_type_mediafile' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_MEDIAFILE, '@acl_object_id' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_ID, '@mediafile_id' => mediamosa_asset_mediafile_db::ID)));
    $query->condition('mf.asset_id', $asset_id);
    $query->fields('mf', array(mediamosa_asset_mediafile_db::ID, mediamosa_asset_mediafile_db::IS_PROTECTED, mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_STILL));
    $query->fields('acl_app', array(mediamosa_acl_app_master_slave_db::APP_ID_MASTER, mediamosa_acl_app_master_slave_db::APP_ID_SLAVE));
    $result = $query->execute();

    // Now get all rules.
    foreach ($result as $acl_app) {
      $asset_acl_info['acl_app_master_slave']['mediafile'][$acl_app[mediamosa_asset_mediafile_db::ID]][] = $acl_app;
    }

    return $asset_acl_info;
  }
}
