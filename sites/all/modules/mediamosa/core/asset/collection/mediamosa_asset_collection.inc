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
 * The helper functions of asset_collection module.
 */

class mediamosa_asset_collection {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Remove link between asset and collection.
   *
   * @param string $asset_id
   *  When given, remove all links from any colletion where this asset is in.
   *  In combination with coll_id, only one link might be deleted.
   * @param string $coll_id
   *  When given, remove all links from any asset in this collection.
   *  In combination with asset_id, only one link might be deleted.
   * @return boolean
   */
  static public function delete($asset_id = FALSE, $coll_id = FALSE) {

    if (!$asset_id && !$coll_id) {
      // if none is provided, none will be deleted.
      return FALSE;
    }

    // Asset must exist.
    if ($asset_id) {
      mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));
    }
    else {
      // We need the asset_ids to reindex.
      $asset_ids = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'ac')
        ->condition(mediamosa_asset_collection_db::COLL_ID, $coll_id)
        ->fields('ac', array(mediamosa_asset_collection_db::ASSET_ID))
        ->execute()
        ->fetchCol();
    }

    // Collection must exist.
    if ($coll_id) {
      mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $coll_id));
    }

    // Do the delete.
    $query = mediamosa_db::db_delete(mediamosa_asset_collection_db::TABLE_NAME);

    if ($asset_id) {
      $query->condition(mediamosa_asset_collection_db::ASSET_ID, $asset_id);
    }

    if ($coll_id) {
      $query->condition(mediamosa_asset_collection_db::COLL_ID, $coll_id);
    }

    $result = $query->execute();

    // Reindex the asset(s).
    if ($asset_id) {
      mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_COLLECTION);
    }
    elseif (!empty($asset_ids)) {
      mediamosa_asset::mediamosa_asset_reindex($asset_ids, mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_COLLECTION);
    }

    return $result;
  }

  /**
   * Delete all relations between asset and his collections.
   *
   * @param $app_id
   *  The application ID.
   * @param $user_id
   *  The possible owner.
   * @param $is_app_admin
   *  Global switch for app admin.
   * @param $asset_id
   *  The asset.
   */
  static public function delete_asset_from_all_collections($app_id, $user_id, $is_app_admin, $asset_id) {

    // Asset must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Check if we can delete the links first.
    $a_asset = mediamosa_asset::get($asset_id);

    // Get all the links of this asset and his collection(s).
    $a_coll_ids = mediamosa_db::db_query(
      'SELECT #coll_id FROM {#mediamosa_asset_collection} WHERE #asset_id = :asset_id',
      array(
        '#coll_id' => mediamosa_asset_collection_db::COLL_ID,
        '#mediamosa_asset_collection' => mediamosa_asset_collection_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_collection_db::ASSET_ID,
        ':asset_id' => $asset_id
      )
    )->fetchCol();

    // Get the collections in one go.
    $a_collections = mediamosa_collection::getAllAssoc($a_coll_ids);

    foreach ($a_collections as $a_collection) {
      // Check the user rights to delete the relation.
      try {
        mediamosa_aut::owner_check_collection_assign($app_id, $user_id, $a_asset, $a_collection, $is_app_admin);
      }
      catch (mediamosa_exception_error_access_denied $e) {
        // There is no access right, so skip it
        continue;
      }

      // Get access, remove relation.
      mediamosa_asset_collection::delete($asset_id, $a_collection[mediamosa_collection_db::ID]);
    }

    // Reindex asset.
    mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_COLLECTION);
  }

  /**
   * Get collections based on given asset_ids.
   *
   * @param array $a_asset_ids
   *  The asset ids.
   * @param array $fields (collection fields)
   *  [alias] => field
   */
  static public function get_by_asset_ids(array $a_asset_ids, array $fields = array()) {

    if (empty($a_asset_ids)) {
      return array();
    }

    $query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'ac');
    $query->leftJoin(mediamosa_collection_db::TABLE_NAME, 'c', 'ac.coll_id = c.coll_id');
    if (count($fields)) {
      foreach ($fields as $key => $field) {
        if (($pos = strpos($field, '.')) === FALSE) {
          $query->addField('c', $field, (is_numeric($key) ? $field : $key));
        }
        else {
          $pref = mediamosa_unicode::substr($field, 0, $pos);
          $field_exp = mediamosa_unicode::substr($field, $pos + 1);
          $query->addField($pref, $field_exp, (is_numeric($key) ? $field_exp : $key));
        }
      }
    }
    else {
      $query->fields();
    }
    $query->condition(mediamosa_asset_collection_db::ASSET_ID, $a_asset_ids, 'IN');
    return $query->execute();
  }

  /**
   * Get collections based on given collection ids.
   * 1.x: _media_management_return_collection_list
   *
   * 2.x: no longer return response object.
   *
   * @param array $coll_ids
   *  The collection ids.
   */
  static public function get_asset_ids_by_coll_ids(array $coll_ids, array $fields = array(mediamosa_asset_collection_db::ASSET_ID)) {
    // Build query.
    $query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'ac');
    $query->fields('ac', $fields);
    $query->condition('ac.' . mediamosa_asset_collection_db::COLL_ID, $coll_ids, 'IN');

    return $query->execute();
  }

  /**
   *
   * Create the link between asset and collection.
   *
   * @param int $app_id
   *  The application ID.
   * @param string $user_id
   *  The user ID.
   * @param bool $is_app_admin
   *  App admin.
   * @param string $asset_id
   *  The asset ID.
   * @param string $coll_id
   *  The coll ID.
   */
  static public function create($app_id, $user_id, $is_app_admin, $asset_id, $coll_id) {
    // Asset must exist.
    $asset = mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Check if relation already exist.
    if (mediamosa_db::db_exists(
      mediamosa_asset_collection_db::TABLE_NAME,
      array(
        mediamosa_asset_collection_db::ASSET_ID => $asset_id,
        mediamosa_asset_collection_db::COLL_ID => $coll_id
      )
    )) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_COLLECTION_ASSET_RELATION_ALREADY_EXISTS);
    }

    $collection = mediamosa_collection::get($coll_id);

    // Check if we are owner to add the asset to the collection.
    mediamosa_aut::owner_check_collection_assign($app_id, $user_id, $asset, $collection, $is_app_admin);

    // If we get here, we can create the link.
    $fields = array(
      mediamosa_asset_collection_db::ASSET_ID => $asset_id,
      mediamosa_asset_collection_db::COLL_ID => $coll_id,
      mediamosa_asset_collection_db::IS_EMPTY_ASSET => $asset[mediamosa_asset_db::IS_EMPTY_ASSET],
    );

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert into
    mediamosa_db::db_insert(mediamosa_asset_collection_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Reindex the asset.
    mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_COLLECTION);
  }

  /**
   * Count the number of collections of given asset.
   *
   * @param string $asset_id
   *  The asset ID.
   */
  static public function count_collections($asset_id) {

    // Asset must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Return the number of found rows.
    $query = mediamosa_db::db_select(mediamosa_asset_collection_db::TABLE_NAME, 'mac');
    $query->fields('mac', array(mediamosa_asset_collection_db::COLL_ID));
    $query->condition(mediamosa_asset_collection_db::ASSET_ID, $asset_id);
    return $query->countQuery()->execute()->fetchField();
  }

  /**
   * Find if the asset is in any collection.
   *
   * @param string $asset_id
   */
  static public function find($asset_id) {

    // return a list of collections.
    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c')
      ->fields('c')
      ->condition(mediamosa_asset_collection_db::ASSET_ID, $asset_id);
    $query->join(mediamosa_asset_collection_db::TABLE_NAME, 'ac', 'ac.coll_id = c.coll_id');
    return $query->execute();
  }

  /**
   * Get list of collections of given asset_id.
   *
   * @param $asset_id
   */
  static public function get_collections($asset_id) {
    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c');
    $query->join(mediamosa_asset_collection_db::TABLE_NAME, 'ac', 'ac.coll_id = c.coll_id');
    $query->fields('c');
    $query->condition(mediamosa_asset_collection_db::ASSET_ID, $asset_id);
    return $query->execute()->fetchAllAssoc(mediamosa_asset_collection_db::COLL_ID);
  }
}
