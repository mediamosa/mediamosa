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
 * The helper functions of collection module.
 */

class mediamosa_collection {
  // ------------------------------------------------------------------ Static Functions.
  /**
   * Remove collection.
   *
   * @param string $coll_id
   * @return boolean
   */
  static public function delete($coll_id) {
    $query = mediamosa_db::db_delete(mediamosa_collection_db::TABLE_NAME);
    $query->condition(mediamosa_collection_db::ID, $coll_id);
    return $query->execute() ? TRUE : FALSE;
  }

  /**
   * Collection must exists, will throw exception.
   *
   * returns found collection.
   *
   * @param string $coll_id
   */
  static public function must_exists($coll_id) {
    return mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $coll_id));
  }

  /**
   * Count the assets in a given collection.
   *
   * returns number of found assetes in the collection.
   *
   * @param string $coll_id
   */
  static public function count_assets_in_collection($app_ids, $coll_id, $hide_empty_assets = FALSE) {

    // Now search.
    $collection = mediamosa_collection_search::collection_search(
      $app_ids,
      NULL,
      NULL,
      $coll_id,
      NULL,
      NULL,
      FALSE,
      FALSE,
      TRUE,
      $hide_empty_assets,
      1,
      0
    )->fetchAssoc();

    return (empty($collection['numofvideos']) ? 0 : $collection['numofvideos']);
  }

  /**
   * Get multiple collections within an array, with key
   * as coll_id.
   *
   * @param array $a_coll_ids
   */
  static public function getAllAssoc(array $coll_ids) {

    // No coll_ids? Return FALSE.
    if (empty($coll_ids)) {
      return FALSE;
    }

    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c');
    $query->fields('c');
    $query->condition('c.' . mediamosa_collection_db::ID, $coll_ids, 'IN');
    return $query->execute()->fetchAllAssoc(mediamosa_collection_db::ID, PDO::FETCH_ASSOC);
  }

  /**
   * Get the collection.
   *
   * @param string $coll_id
   */
  static public function get($coll_id, $app_id = NULL) {

    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c');
    $query->fields('c');
    $query->condition('c.' . mediamosa_collection_db::ID, $coll_id);

    if (isset($app_id)) {
      $query->condition('c.' . mediamosa_collection_db::APP_ID, $app_id);
    }

    return $query->execute()->fetchAssoc();
  }

  /**
   * Get all collections.
   *
   * @param $fields
   */
  static public function getAll($fields = array(), $order_by = mediamosa_collection_db::TITLE, $order_direction = mediamosa_db::ORDER_DIRECTION_ASC) {
    return mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c')
      ->fields('c', $fields)
      ->orderBy($order_by, $order_direction)
      ->execute();
  }

  /**
   * Create a collection.
   */
  static public function create($app_id, $owner_id, $title = NULL, $description = NULL, $group_id = NULL, $private = FALSE, $public  = TRUE, $category = FALSE, $isprivate = FALSE, $public_assign = FALSE, $is_unappropriate = FALSE) {

    // New ID.
    $coll_id = mediamosa_db::uuid($app_id);

    // Required.
    $fields = array();
    $fields[mediamosa_collection_db::ID] = $coll_id;
    $fields[mediamosa_collection_db::APP_ID] = $app_id;
    $fields[mediamosa_collection_db::OWNER_ID] = $owner_id;

    if (isset($title)) {
      $fields[mediamosa_collection_db::TITLE] = $title;
    }
    if (isset($description)) {
      $fields[mediamosa_collection_db::DESCRIPTION] = $description;
    }
    if (isset($group_id)) {
      $fields[mediamosa_collection_db::GROUP_ID] = $group_id;
    }
    if (isset($private)) {
      $fields[mediamosa_collection_db::PRIVATE_] = $private ? mediamosa_collection_db::PRIVATE_TRUE : mediamosa_collection_db::PRIVATE_FALSE;
    }
    if (isset($public)) {
      $fields[mediamosa_collection_db::PUBLIC_] = $public ? mediamosa_collection_db::PUBLIC_TRUE : mediamosa_collection_db::PUBLIC_FALSE;
    }
    if (isset($category)) {
      $fields[mediamosa_collection_db::CATEGORY] = $category ? mediamosa_collection_db::CATEGORY_TRUE : mediamosa_collection_db::CATEGORY_FALSE;
    }
    if (isset($isprivate)) {
      $fields[mediamosa_collection_db::ISPRIVATE] = $isprivate ? mediamosa_collection_db::ISPRIVATE_TRUE : mediamosa_collection_db::ISPRIVATE_FALSE;
    }
    if (isset($public_assign)) {
      $fields[mediamosa_collection_db::PUBLIC_ASSIGN] = $public_assign ? mediamosa_collection_db::PUBLIC_ASSIGN_TRUE : mediamosa_collection_db::PUBLIC_ASSIGN_FALSE;
    }
    if (isset($is_unappropriate)) {
      $fields[mediamosa_collection_db::IS_UNAPPROPRIATE] = $is_unappropriate ? mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE : mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE;
    }

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_collection_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Return self made ID.
    return $coll_id;
  }

  /**
   * Update a collection.
   *
   * @param string $coll_id
   * @param string $owner_id
   * @param string $title
   * @param string $description
   * @param string $group_id
   * @param bool $private
   * @param bool $public
   * @param bool $category
   * @param bool $isprivate
   * @param bool $public_assign
   * @param bool $is_unappropriate
   */
  static public function update($coll_id, $owner_id = NULL, $title = NULL, $description = NULL, $group_id = NULL, $private = FALSE, $public  = TRUE, $category = FALSE, $isprivate = FALSE, $public_assign = FALSE, $is_unappropriate = FALSE) {

    // Required.
    $fields = array();

    if (isset($owner_id)) {
      $fields[mediamosa_collection_db::OWNER_ID] = $owner_id;
    }
    if (isset($title)) {
      $fields[mediamosa_collection_db::TITLE] = $title;
    }
    if (isset($description)) {
      $fields[mediamosa_collection_db::DESCRIPTION] = $description;
    }
    if (isset($group_id)) {
      $fields[mediamosa_collection_db::GROUP_ID] = $group_id;
    }
    if (isset($private)) {
      $fields[mediamosa_collection_db::PRIVATE_] = $private ? mediamosa_collection_db::PRIVATE_TRUE : mediamosa_collection_db::PRIVATE_FALSE;
    }
    if (isset($public)) {
      $fields[mediamosa_collection_db::PUBLIC_] = $public ? mediamosa_collection_db::PUBLIC_TRUE : mediamosa_collection_db::PUBLIC_FALSE;
    }
    if (isset($category)) {
      $fields[mediamosa_collection_db::CATEGORY] = $category ? mediamosa_collection_db::CATEGORY_TRUE : mediamosa_collection_db::CATEGORY_FALSE;
    }
    if (isset($isprivate)) {
      $fields[mediamosa_collection_db::ISPRIVATE] = $isprivate ? mediamosa_collection_db::ISPRIVATE_TRUE : mediamosa_collection_db::ISPRIVATE_FALSE;
    }
    if (isset($public_assign)) {
      $fields[mediamosa_collection_db::PUBLIC_ASSIGN] = $public_assign ? mediamosa_collection_db::PUBLIC_ASSIGN_TRUE : mediamosa_collection_db::PUBLIC_ASSIGN_FALSE;
    }
    if (isset($is_unappropriate)) {
      $fields[mediamosa_collection_db::IS_UNAPPROPRIATE] = $is_unappropriate ? mediamosa_collection_db::IS_UNAPPROPRIATE_TRUE : mediamosa_collection_db::IS_UNAPPROPRIATE_FALSE;
    }

    if (empty($fields)) {
      return FALSE;
    }

    // Add created value.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Insert it.
    mediamosa_db::db_update(mediamosa_collection_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_collection_db::ID, $coll_id)
      ->execute();

    // Return true that we have updated.
    return TRUE;
  }
}
