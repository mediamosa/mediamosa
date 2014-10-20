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
  * The user favorite module.
  */
class mediamosa_user_favorite {

  // ------------------------------------------------------------------ Functions static.
  /**
   * Return the number of rows found.
   *
   * @param string $fav_type
   * @param string $fav_id
   * @return integer
   */
  public static function count_items($fav_type, $fav_id) {

    // Return the number of found rows.
    $query = mediamosa_db::db_select(mediamosa_user_favorite_db::TABLE_NAME, 'muf');
    $query->fields('muf', array(mediamosa_user_favorite_db::FAV_ID));
    $query->condition(mediamosa_user_favorite_db::FAV_TYPE, $fav_type);
    $query->condition(mediamosa_user_favorite_db::FAV_ID, $fav_id);
    return $query->countQuery()->execute()->fetchField();
  }

  /**
   * Get a fav. relation.
   *
   * @param int $app_id
   *  Application ID.
   * @param string $user_id
   *  User ID.
   * @param string $fav_type
   *  Favorite type.
   * @param int $fav_id
   *  ID asset / collection.
   */
  public static function get($app_id, $user_id, $fav_type, $fav_id) {
    return mediamosa_db::db_select(mediamosa_user_favorite_db::TABLE_NAME, 'uf')
      ->fields('uf')
      ->condition(mediamosa_user_favorite_db::NAME, $user_id)
      ->condition(mediamosa_user_favorite_db::APP_ID, $app_id)
      ->condition(mediamosa_user_favorite_db::FAV_TYPE, $fav_type)
      ->condition(mediamosa_user_favorite_db::FAV_ID, $fav_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Delete a fav. relation.
   *
   * @param int $app_id
   *  Application ID.
   * @param string $user_id
   *  User ID.
   * @param string $fav_type
   *  Favorite type.
   * @param int $fav_id
   *  ID asset / collection.
   */
  public static function delete($app_id, $user_id, $fav_type, $fav_id) {
    return mediamosa_db::db_delete(mediamosa_user_favorite_db::TABLE_NAME)
      ->condition(mediamosa_user_favorite_db::NAME, $user_id)
      ->condition(mediamosa_user_favorite_db::APP_ID, $app_id)
      ->condition(mediamosa_user_favorite_db::FAV_TYPE, $fav_type)
      ->condition(mediamosa_user_favorite_db::FAV_ID, $fav_id)
      ->execute();

    if ($fav_type == mediamosa_user_favorite_db::FAV_TYPE_ASSET) {
      // Trigger the reindex hook for external search engines.
      mediamosa_asset::mediamosa_asset_reindex(array($fav_id), mediamosa_settings::SEARCH_INDEX_TYPE_FAV_USER);
    }
  }

  /**
   * Create a fav. relation.
   *
   * @param int $app_id
   *  Application ID.
   * @param string $user_id
   *  User ID.
   * @param string $fav_type
   *  Favorite type.
   * @param int $fav_id
   *  ID asset / collection.
   */
  public static function create($app_id, $user_id, $fav_type, $fav_id) {

    switch ($fav_type) {
      case mediamosa_user_favorite_db::FAV_TYPE_ASSET:
        // Must exist.
        mediamosa_asset::must_exists($fav_id);
        break;

      case mediamosa_user_favorite_db::FAV_TYPE_COLLECTION:
        // Must exist.
        mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $fav_id));
        break;

      default:
        // Any other type is not allowed
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_FAV_TYPE, array('@type' => $fav_type, '@valid_types' => implode(', ', array(mediamosa_user_favorite_db::FAV_TYPE_ASSET, mediamosa_user_favorite_db::FAV_TYPE_COLLECTION))));
    }

    // Get possible link.
    $fav_relations = mediamosa_user_favorite::get($app_id, $user_id, $fav_type, $fav_id);

    if (!$fav_relations) {
      // Set these fields.
      $fields = array(
        mediamosa_user_favorite_db::NAME => $user_id,
        mediamosa_user_favorite_db::APP_ID => $app_id,
        mediamosa_user_favorite_db::FAV_TYPE => $fav_type,
        mediamosa_user_favorite_db::FAV_ID => $fav_id,
      );

      // FIXME not master-master save.
      mediamosa_db::db_insert(mediamosa_user_favorite_db::TABLE_NAME)
        ->fields($fields)
        ->execute();
    }

    if ($fav_type == mediamosa_user_favorite_db::FAV_TYPE_ASSET) {
      // Trigger the reindex hook for external search engines.
      mediamosa_asset::mediamosa_asset_reindex(array($fav_id), mediamosa_settings::SEARCH_INDEX_TYPE_FAV_USER);
    }
  }

  /**
   * Get list of fav. of this asset.
   *
   * @param $asset_id
   */
  public static function get_fav_of_asset($asset_id) {
    return mediamosa_db::db_select(mediamosa_user_favorite_db::TABLE_NAME, 'uf')
      ->fields('uf')
      ->condition(mediamosa_user_favorite_db::FAV_TYPE, mediamosa_user_favorite_db::FAV_TYPE_ASSET)
      ->condition(mediamosa_user_favorite_db::FAV_ID, $asset_id)
      ->execute();
  }
}
