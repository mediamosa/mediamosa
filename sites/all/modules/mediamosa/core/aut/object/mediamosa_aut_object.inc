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
  * The aut_object functions.
  */

class mediamosa_aut_object {
  // ------------------------------------------------------------------ Static Function.
  /**
   * Returns the join for joining with aut_object table
   *
   * @param enum $aut_type
   *  See mediamosa_aut::AUT_TYPE_*
   * @return string
   */
  static public function join_aut_object_get($aut_type) {
    switch ($aut_type) {
      case mediamosa_aut::AUT_TYPE_MEDIAFILE:
        $on = 'mf.mediafile_id';
        $alias = 'aut_obj';
        break;
      case mediamosa_aut::AUT_TYPE_COLLECTION:
        $on = 'c.coll_id';
        $alias = 'aut_obj';
        break;
      case mediamosa_aut::AUT_TYPE_ASSET:
        $on = 'a.asset_id';
        $alias = 'aut_obja';
        break;
      default:
        throw new mediamosa_exception_program_error('Unknown type used in mediamosa_aut_object::join_aut_object_get');
    }

    return strtr(
      "LEFT JOIN {#aut_object} AS #alias ON #alias.aut_object_type = '#aut_type' AND #on = #alias.aut_object_id",
      array(
        '#aut_object' => mediamosa_aut_object_db::TABLE_NAME,
        '#aut_type' => $aut_type,
        '#on' => $on,
        '#alias' => $alias,
      )
    );
  }

  /**
   * Delete all connection based on aut_type and aut_id.
   *
   * @param string $aut_type
   * @param integer $aut_id
   */
  function delete($aut_type, $aut_id) {

    // Delete all connections.
    mediamosa_db::db_delete(mediamosa_aut_object_db::TABLE_NAME)
      ->condition(mediamosa_aut_object_db::AUT_TYPE, $aut_type)
      ->condition(mediamosa_aut_object_db::AUT_ID, $aut_id)
      ->execute();
  }

  /**
   * Creates a link between object and aut_name
   *
   * @param string $aut_object_id
   * @param string $aut_type
   * @param string $aut_object_type
   * @param integer $aut_id
   * @param array $a_rights (reserved)
   */
  function link_to_object($aut_object_id, $aut_type, $aut_object_type, $aut_id, $a_rights = array(mediamosa_aut::RIGHT_ACCESS)) {
    // Here so the var is used and Zend doesn't complain).
    assert(is_array($a_rights));

    // Master-slave safe now.
    try {
      mediamosa_db::db_query("INSERT INTO {mediamosa_aut_object} SET aut_object_type = '%s', aut_object_id = '%s', aut_type = '%s', aut_id= %d", $aut_object_type, $aut_object_id, $aut_type, $aut_id);
    }
    catch (PDOException $e) {
      assert($e);
      // ignore, double row.
    }
  }

  /**
   * Get the aut information for given asset.
   */
  function get_for_asset($asset_id) {

    // Collection of aut rules on asset/mediafiles.
    $asset_aut_info = array(
      'aut_object' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'aut_app_master_slave' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'aut_domain' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'aut_realm' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'aut_realm_prefix' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'aut_user' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
      'aut_user_group' => array(
        'asset' => array(), // future.
        'mediafile' => array(),
      ),
    );

    // Get aut_object rules for all asset mediafiles.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->join(mediamosa_aut_object_db::TABLE_NAME, 'aut_obj', strtr("aut_obj.@aut_object_type = '@aut_object_type_mediafile' AND aut_obj.@aut_object_id = mf.@mediafile_id", array('@aut_object_type' => mediamosa_aut_object_db::AUT_OBJECT_TYPE, '@aut_object_type_mediafile' => mediamosa_aut_object_db::AUT_OBJECT_TYPE_MEDIAFILE, '@aut_object_id' => mediamosa_aut_object_db::AUT_OBJECT_ID, '@mediafile_id' => mediamosa_asset_mediafile_db::ID)));
    $query->condition('mf.asset_id', $asset_id);
    $query->fields('mf', array(mediamosa_asset_mediafile_db::ID));
    $query->fields('aut_obj', array(mediamosa_aut_object_db::AUT_TYPE, mediamosa_aut_object_db::AUT_ID));
    $result = $query->execute();

    $aut_name_ids = array();
    $aut_group_ids = array();

    // Now get all rules.
    foreach ($result as $aut_object) {
      $asset_aut_info['aut_object']['mediafile'][$aut_object[mediamosa_asset_mediafile_db::ID]][] = $aut_object;

      if ($aut_object[mediamosa_aut_object_db::AUT_TYPE] == mediamosa_aut_object_db::AUT_TYPE_NAME) {
        $aut_name_ids[] = $aut_object[mediamosa_aut_object_db::AUT_ID];
      }
      else {
        $aut_group_ids[] = $aut_object[mediamosa_aut_object_db::AUT_ID];
      }
    }

    if (!empty($aut_name_ids)) {
      // Now use the aut_object info and collect the data for aut_user.
      $query = mediamosa_db::db_select(mediamosa_aut_name_db::TABLE_NAME, 'an');
      $query->fields('an');
      $query->condition('an.aut_name_id', $aut_name_ids, 'IN');
      $result = $query->execute();

      foreach ($result as $aut_group) {
        switch ($aut_group[mediamosa_aut_name_db::AUT_TYPE]) {
          case mediamosa_aut_name_db::AUT_TYPE_DOMAIN:
            $asset_aut_info['aut_domain']['mediafile'][] = $aut_group[mediamosa_aut_name_db::AUT_NAME];
            break;

          case mediamosa_aut_name_db::AUT_TYPE_REALM:
            $asset_aut_info['aut_realm']['mediafile'][] = $aut_group[mediamosa_aut_name_db::AUT_NAME];
            $asset_aut_info['aut_realm_prefix']['mediafile'][] = (string) $aut_group[mediamosa_aut_name_db::AUT_PREFIX];
            break;

          case mediamosa_aut_name_db::AUT_TYPE_USER:
            $asset_aut_info['aut_user']['mediafile'][] = $aut_group[mediamosa_aut_name_db::AUT_NAME];
            break;

          case mediamosa_aut_name_db::AUT_TYPE_USER_GROUP:
            $asset_aut_info['aut_user_group']['mediafile'][] = $aut_group[mediamosa_aut_name_db::AUT_NAME];
            break;
        }
      }
    }

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->join(mediamosa_aut_app_master_slave_db::TABLE_NAME, 'aut_app', strtr("aut_app.@aut_object_type = '@aut_object_type_mediafile' AND aut_app.@aut_object_id = mf.@mediafile_id", array('@aut_object_type' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE, '@aut_object_type_mediafile' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_TYPE_MEDIAFILE, '@aut_object_id' => mediamosa_aut_app_master_slave_db::AUT_OBJECT_ID, '@mediafile_id' => mediamosa_asset_mediafile_db::ID)));
    $query->condition('mf.asset_id', $asset_id);
    $query->fields('mf', array(mediamosa_asset_mediafile_db::ID, mediamosa_asset_mediafile_db::IS_PROTECTED, mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_STILL));
    $query->fields('aut_app', array(mediamosa_aut_app_master_slave_db::APP_ID_MASTER, mediamosa_aut_app_master_slave_db::APP_ID_SLAVE));
    $result = $query->execute();

    // Now get all rules.
    foreach ($result as $aut_app) {
      $asset_aut_info['aut_app_master_slave']['mediafile'][$aut_app[mediamosa_asset_mediafile_db::ID]][] = $aut_app;
    }

    return $asset_aut_info;
  }
}
