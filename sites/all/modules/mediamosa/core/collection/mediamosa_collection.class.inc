<?php
/**
 * @file
 * The helper functions of collection module.
 */

class mediamosa_collection {
  // Static Functions.
  /**
   * Remove collection.
   *
   * @param string $coll_id
   *   Collection id.
   *
   * @return bool
   *   True/false.
   */
  public static function delete($coll_id) {
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
   *   Collection id.
   */
  public static function must_exists($coll_id) {
    return mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $coll_id));
  }

  /**
   * Count the assets in a given collection.
   *
   * returns number of found assetes in the collection.
   *
   * @param string $coll_id
   *   Collection id.
   */
  public static function count_assets_in_collection($app_ids, $coll_id, $hide_empty_assets = FALSE) {

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
      TRUE,
      $hide_empty_assets,
      1,
      0
    )->fetchAssoc();

    return (empty($collection['numofvideos']) ? 0 : $collection['numofvideos']);
  }

  /**
   * Get multiple collections within an array.
   *
   * Key of the array is the coll_id.
   *
   * @param array $coll_ids
   *   array of collection id's.
   */
  public static function getAllAssoc(array $coll_ids) {

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
   *   Collection id.
   */
  public static function get($coll_id, $app_id = NULL) {

    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c')
      ->fields('c')
      ->condition('c.' . mediamosa_collection_db::ID, $coll_id);

    if (isset($app_id)) {
      $query->condition('c.' . mediamosa_collection_db::APP_ID, $app_id);
    }

    return $query->execute()->fetchAssoc();
  }

  /**
   * Search collections.
   *
   * @param array $fields
   *   Selection of fields to get, keep empty to get all fields.
   * @param array $options
   *   Options for query.
   *
   * @return array
   *   An associative array where key is the mediafile ID.
   */
  public static function search($fields = array(), $options = array()) {
    $options += array(
      'limit' => FALSE,
      'offset' => 0,
      'orderby' => FALSE,
      'return_resource' => FALSE,
      'app_id' => FALSE,
      'coll_id' => FALSE,
      'title' => FALSE,
      'description' => FALSE,
    );

    $query = mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c')
      ->fields('c', $fields);

    if ($options['title']) {
      $query->condition('c.title', $options['title']);
    }
    if ($options['app_id']) {
      $query->condition('c.' . mediamosa_collection_db::APP_ID, $options['app_id']);
    }
    if ($options['coll_id']) {
      $query->condition('c.' . mediamosa_collection_db::ID, $options['coll_id']);
    }
    if (($options['limit'] && is_numeric($options['limit'])) || ($options['offset'] && is_numeric($options['offset']))) {
      $query->range($options['offset'], $options['limit']);
    }
    if ($options['orderby']) {
      $query->orderBy('c.' . $options['orderby'], 'ASC');
    }

    // Return resource.
    if ($options['return_resource']) {
      return $query->execute();
    }
    return $query->execute()->fetchAllAssoc(mediamosa_collection_db::ID);
  }

  /**
   * Get all collections.
   *
   * @param array $fields
   *   Selection of fields to get, keep empty to get all fields.
   */
  public static function getAll($fields = array(), $order_by = mediamosa_collection_db::TITLE, $order_direction = mediamosa_db::ORDER_DIRECTION_ASC) {
    return mediamosa_db::db_select(mediamosa_collection_db::TABLE_NAME, 'c')
      ->fields('c', $fields)
      ->orderBy($order_by, $order_direction)
      ->execute();
  }

  /**
   * Create a collection.
   */
  public static function create($app_id, $owner_id, $title = NULL, $description = NULL, $group_id = NULL, $private = FALSE, $public = TRUE, $category = FALSE, $isprivate = FALSE, $public_assign = FALSE, $is_unappropriate = FALSE) {

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
      $fields[mediamosa_collection_db::IS_INAPPROPRIATE] = $is_unappropriate ? mediamosa_collection_db::IS_INAPPROPRIATE_TRUE : mediamosa_collection_db::IS_INAPPROPRIATE_FALSE;
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
   *   Collection id.
   * @param string $owner_id
   *   Owner id of the collection.
   * @param string $title
   *   Title of collection.
   * @param string $description
   *   Description of collection.
   * @param string $group_id
   *   Group (ownership) of collection.
   * @param bool $private
   *   Boolean to set this collection private (only visible to owner).
   * @param bool $public
   *   Makes this collection specifically public.
   * @param bool $category
   *   Optional category.
   * @param bool $isprivate
   *   needs explanation.
   * @param bool $public_assign
   *   needs explanation.
   * @param bool $is_unappropriate
   *   setting this, makes the collection disabled.
   */
  public static function update($coll_id, $owner_id = NULL, $title = NULL, $description = NULL, $group_id = NULL, $private = FALSE, $public = TRUE, $category = FALSE, $isprivate = FALSE, $public_assign = FALSE, $is_unappropriate = FALSE) {

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
      $fields[mediamosa_collection_db::IS_INAPPROPRIATE] = $is_unappropriate ? mediamosa_collection_db::IS_INAPPROPRIATE_TRUE : mediamosa_collection_db::IS_INAPPROPRIATE_FALSE;
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
