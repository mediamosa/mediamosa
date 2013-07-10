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
  * User management module.
  */

class mediamosa_user {

  // ------------------------------------------------------------------ Functions static.
  /**
   * Get the user.
   *
   * @param string $user_id
   *  The user to retrieve.
   * @param integer $app_id
   *  The application ID.
   * @param boolean $with_groups
   *  Shall we populate ['group_id'] with groups array?
   *
   * @return
   *  array with info about particular user if found, FALSE otherwise.
   */
  public static function get($user_id, $app_id, $with_groups = TRUE) {
    $user_info = mediamosa_db::db_select(mediamosa_user_db::TABLE_NAME, 'u')
      ->fields('u')
      ->condition(mediamosa_user_db::NAME, $user_id)
      ->condition(mediamosa_user_db::APP_ID, $app_id)
      ->execute()
      ->fetchAssoc();

    if (!empty($user_info) && $with_groups) {
      $user_info['group_id'] = mediamosa_user::get_groups($app_id, $user_id);
    }

    return $user_info;
  }

  /**
   * Get the user's group(s).
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $user_id
   *  The user name.
   * @param bool $serialize_keys
   *  If TRUE, keys in array will be prepared for
   *  processing with mediamosa_response::generate_plain_xml_add_child().
   *
   * @return
   *  array with group IDs.
   */
  public static function get_groups($app_id, $user_id, $serialize_keys = FALSE) {

  /* We're building SQL like
   SELECT g.group_key, g.group_id FROM mediamosa_user AS u
        JOIN mediamosa_user_in_group as i ON u.user_key = i.user_key
        JOIN mediamosa_user_group AS g ON i.group_key=g.group_key
             WHERE name='dave' AND u.app_id=99    */

    $query = mediamosa_db::db_select(mediamosa_user_db::TABLE_NAME, 'u');
    $query->fields('g', array('group_key', 'group_id'));
    $query->condition('u.'. mediamosa_user_db::NAME, $user_id);
    $query->condition('u.'. mediamosa_user_db::APP_ID, $app_id);
    $query->join(mediamosa_user_in_group_db::TABLE_NAME, 'i', 'u.user_key = i.user_key');
    $query->join(mediamosa_user_group_db::TABLE_NAME, 'g', 'i.group_key = g.group_key');

    $groups = array();
    $items = $query->execute()->fetchAll();
    foreach ($items as $item) {
      $key = $item['group_key'];
      if ($serialize_keys) {
        $key = '#'. serialize(array('id' => $key));
      }
      $groups[$key] = $item['group_id'];
    }

    return $groups;
  }

  /**
   * Add user's group membership identifying user by key.
   *
   * @param integer $app_id
   *  Application ID.
   * @param integer $user_key
   *  User key.
   * @param string $group_id
   *  A particular Group ID to include user to.
   */
  public static function add_group_by_key($app_id, $user_key, $group_id) {
    $query = mediamosa_db::db_select(mediamosa_user_group_db::TABLE_NAME, 'g');
    $query->addField('g', mediamosa_user_group_db::KEY);
    $query->condition(mediamosa_user_group_db::ID, $group_id);
    $query->condition(mediamosa_user_db::APP_ID, $app_id);

    $result = $query->execute()->fetchAssoc();
    if (!$result) {
      // Group not found
      mediamosa_watchdog::log('Group lookup unsuccesful for app/group #app/#group.',
        array('#app' => $app_id, '#group' => $group_id), WATCHDOG_DEBUG);
      return;
    }

    $group_key = $result[mediamosa_user_group_db::KEY];

    $user_key = mediamosa_db::db_insert(mediamosa_user_in_group_db::TABLE_NAME)
      ->fields(array(
          mediamosa_user_in_group_db::USER_KEY => $user_key,
          mediamosa_user_in_group_db::GROUP_KEY => $group_key,
         ))
      ->execute();

    return $group_key;
  }

  /**
   * Update the user.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  User ID.
   * @param integer $quotum
   *  The allowed disk usage.
   * @param array $group_ids
   *  Group ID.
   */
  public static function update($app_id, $user_id, $quotum, $group_ids) {

    if (isset($quotum)) {
      $user_key = mediamosa_user::key($app_id, $user_id);

      // Add changed value.
      $fields[mediamosa_user_db::QUOTUM] = $quotum;
      $fields = mediamosa_db::db_update_enrich($fields);

      mediamosa_db::db_update(mediamosa_user_db::TABLE_NAME)
        ->fields($fields)
        ->condition(mediamosa_user_db::KEY, $user_key)
        ->execute();
    }

    if (!empty($group_ids)) {
      // If we need to update groups...

      // Clear old data first.
      mediamosa_db::db_delete(mediamosa_user_in_group_db::TABLE_NAME)
        ->condition(mediamosa_user_in_group_db::USER_KEY, $user_key)
        ->execute();

      // And set renewed.
      foreach ($group_ids as $group_id) {
        mediamosa_user::add_group_by_key($app_id, $user_key, $group_id);
      }
    }

  }

  /**
   * Delete the user.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  User ID.
   */
  public static function delete($app_id, $user_id) {

    $user_key = mediamosa_user::key($app_id, $user_id);

    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_user_in_group} WHERE #key = :user_key',
      array(
        '#mediamosa_user_in_group' => mediamosa_user_in_group_db::TABLE_NAME,
        '#key' => mediamosa_user_in_group_db::USER_KEY,
        ':user_key' => $user_key
      )
    );

    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_user} WHERE #key = :user_key',
      array(
        '#mediamosa_user' => mediamosa_user_db::TABLE_NAME,
        '#key' => mediamosa_user_db::KEY,
        ':user_key' => $user_key
      )
    );
  }

  /**
   * Create a user.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  User ID.
   * @param integer $quotum
   *  The allowed disk usage.
   * @param array $group_ids
   *  Group IDs.
   */
  public static function create($app_id, $user_id, $quotum, $group_ids) {

    // Values to insert.
    $fields = array(
      mediamosa_user_db::APP_ID => $app_id,
      mediamosa_user_db::NAME => $user_id,
      mediamosa_user_db::QUOTUM => $quotum,
    );

    // Enrich with created/changed.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert.
    $user_key = mediamosa_db::db_insert(mediamosa_user_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    if (!empty($group_ids)) {
      foreach ($group_ids as $group_id) {
        mediamosa_user::add_group_by_key($app_id, $user_key, $group_id);
      }
    }

    return $user_key;
  }

  /**
   * Get quota of user.
   *
   * 1.x: _user_management_get_quota
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $group_id
   */
  public static function get_quota($app_id, $user_id = '', $group_id = '') {

    return mediamosa_db::db_query(
      'SELECT app.#quota * 1024 AS app_quota_mb, g.#quotum AS group_quota_mb, u.#quotum AS owner_quota_mb FROM {#mediamosa_app} AS app
       LEFT JOIN {#mediamosa_user_group} AS g ON g.#group_id = :group_id AND g.#app_id = app.#app_id
       LEFT JOIN {#mediamosa_user} AS u ON u.#name = :user_id AND u.#app_id = app.#app_id
       WHERE app.#app_id = :app_id',
      array(
        '#mediamosa_app' => mediamosa_app_db::TABLE_NAME,
        '#mediamosa_user_group' => mediamosa_user_group_db::TABLE_NAME,
        '#mediamosa_user' => mediamosa_user_db::TABLE_NAME,
        '#quota' => mediamosa_app_db::QUOTA,
        '#quotum' => mediamosa_user_group_db::QUOTUM,
        '#group_id' => mediamosa_user_group_db::ID,
        ':group_id' => $group_id,
        '#app_id' => mediamosa_app_db::APP_ID,
        ':app_id' => $app_id,
        '#name' => mediamosa_user_db::NAME,
        ':user_id' => $user_id,
      )
    )->fetchAssoc();
  }

  /**
   * Get the used diskspace of user/group.
   *
   * 1.x:_user_management_get_diskspace
   *
   * @param string $subject
   * @param string $id
   * @param integer $app_id
   *
   * @return array
   */
  public static function get_diskspace($subject, $id, $app_id) {

    $prop_id_filesize = mediamosa_asset_mediafile_metadata_property::get_property_id(mediamosa_asset_mediafile_metadata::FILESIZE, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
    $mbytes = mediamosa_db::db_query(
      'SELECT SUM(mfmd.#val_int) / 1048576 FROM {#mediamosa_asset_mediafile} AS mf
       JOIN {#mediamosa_asset_mediafile_metadata} AS mfmd ON mfmd.#mediafile_id = mf.#mediafile_id
       WHERE mf.#subject_id = :id AND mf.#app_id = :app_id AND mfmd.#prop_id_filesize = :prop_id_filesize',
      array(
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#val_int' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
        '#subject_id' => $subject . '_id',
        '#prop_id_filesize' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
        ':id' => $id,
        ':app_id' => $app_id,
        ':prop_id_filesize' => $prop_id_filesize,
      )
    )->fetchField();

    return array(
      'diskspace_used_mb' => (int)round($mbytes),
    );
  }

  /**
   * Check if the user has enough quota.
   *
   * 1.x: _user_management_check_user_quota
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  The user ID / Owner ID.
   * @param string $group_id
   *  The group ID.
   *
   * @param bool $return_free
   */
  public static function check_user_quota($app_id, $user_id = '', $group_id = '', $return_free = FALSE) {

    // Quota is reported in MB.
    $a_quota = self::get_quota($app_id, $user_id, $group_id);

    if (!empty($a_quota)) {
      foreach (array('app' => $app_id, 'group' => $group_id, 'owner' => $user_id) as $subject => $var) {
        if ($var != '' && !is_null($var)) {
          // Get the diskspace used of subject.
          $a_diskspace_used = self::get_diskspace($subject, $var, $app_id);

          $a_quota[$subject . '_used_mb'] = $a_diskspace_used['diskspace_used_mb'];
          $a_quota[$subject . '_free_mb'] = $a_quota[$subject . '_quota_mb'] - $a_diskspace_used['diskspace_used_mb'];

          // Unlimited quota ?
          if ($a_quota[$subject . '_quota_mb'] != 0) {
            !isset($a_quota['quota_available_mb']) ? $a_quota['quota_available_mb'] = $a_quota[$subject . '_free_mb'] : $a_quota['quota_available_mb'] = min($a_quota['quota_available_mb'], $a_quota[$subject . '_free_mb']);
          }
        }
      }
    }

    if (!isset($a_quota['quota_available_mb'])) {
      $a_quota['quota_available_mb'] = 0;
    }

    if ($return_free) {
      return (int)$a_quota['quota_available_mb'];
    }

    return ($a_quota['quota_available_mb'] >= 0);
  }

  /**
   * Check if the user has enough quota and throw exception when failed.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  The user ID / Owner ID.
   * @param string $group_id
   *  The group ID.
   */
  public static function must_have_user_quota($app_id, $user_id = '', $group_id = '') {
    if (!self::check_user_quota($app_id, $user_id, $group_id)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NOT_ENOUGH_FREE_QUOTA);
    }
  }

  /**
   * Collect and calculate users quota.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   *  User / Owner ID.
   */
  public static function get_user_quota($app_id, $user_id) {

    $total = mediamosa_db::db_query(
      'SELECT COUNT(*) AS total FROM
       (
         (SELECT #name FROM {#mediamosa_user} WHERE #name = :user_id AND #app_id = :app_id)
         UNION
         (SELECT #owner_id FROM {#mediamosa_collection} WHERE #owner_id = :user_id AND #app_id = :app_id)
         UNION
         (SELECT #owner_id FROM {#mediamosa_asset} AS a WHERE #owner_id = :user_id AND #app_id = :app_id)
         UNION
         (SELECT #owner_id FROM {#mediamosa_asset_mediafile} WHERE #owner_id = :user_id AND #app_id = :app_id)
       ) tmp',
      array(
        '#mediamosa_user' => mediamosa_user_db::TABLE_NAME,
        '#mediamosa_collection' => mediamosa_collection_db::TABLE_NAME,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#name' => mediamosa_user_db::NAME,
        '#owner_id' => mediamosa_collection_db::OWNER_ID,
        ':user_id' => $user_id,
        '#app_id' => mediamosa_collection_db::APP_ID,
        ':app_id' => $app_id,
      )
    )->fetchField();

    // Removed: new users that havent created anything will not be able to see
    // its quota.
    if (!$total) {
      throw new mediamosa_exception_error_userman_invalid_user(array('@user_id' => $user_id));
    }

    // Get the user.
    $a_user = mediamosa_user::get($user_id, $app_id);
    if (!$a_user) {
      $a_user = array(
        'group_id' => '',
        'created' => '',
        'changed' => '',
      );
    }

    $primary_group_id = mediamosa_user::get_primary_group($app_id, $user_id);

    // Get the quota.
    $a_quota = mediamosa_user::get_quota($app_id, $user_id, '');

    // owner is unclear: use user for output
    $a_quota['user_quota_mb'] = (int)$a_quota['owner_quota_mb'];
    $a_quota['group_quota_mb'] = (int)$a_quota['group_quota_mb']; // int so we have '0' as value when empty
    unset($a_quota['owner_quota_mb']);

    // Get free diskspace.
    $diskspace_free = mediamosa_user::check_user_quota($app_id, $user_id, $primary_group_id, TRUE);

    // Collection diskspace used on group/user/app.
    $a_diskspace_used = array(
      'user' => mediamosa_user::get_diskspace('owner', $user_id, $app_id),
      'group' => mediamosa_user::get_diskspace('group', $primary_group_id, $app_id),
      'app' => mediamosa_user::get_diskspace('app', $app_id, $app_id)
    );

    // Make one array.
    return array_merge(
      $a_quota,
      array(
        'app_diskspace_used_mb' => $a_diskspace_used['app']['diskspace_used_mb'],
        'group_diskspace_used_mb' => $a_diskspace_used['group']['diskspace_used_mb'],
        'user_diskspace_used_mb' => $a_diskspace_used['user']['diskspace_used_mb'],
        'quota_available_mb' => $diskspace_free,
        'user_over_quota' => ($diskspace_free < 0 ? 'true' : 'false')
      ),
      $a_user
    );
  }

  /**
   * Helper function for collecting data.
   *
   * @param $is_users
   *  TRUE for mediamosa_user or FALSE for mediamosa_user_group
   * @param $app_id
   *  Application ID.
   * @param $offset
   *  Offset in result set.
   * @param $limit
   *  Limit of items in result set.
   */
  public static function get_all($is_users, $app_id, $offset, $limit) {

    // Do the query.
    $result = mediamosa_db::db_query_range(
      'SELECT SQL_CALC_FOUND_ROWS DISTINCT tmp.#select, g.#quotum, g.#created, g.#changed FROM
        (
          (SELECT DISTINCT #element FROM {#mediamosa_asset} WHERE #app_id = :app_id and #element IS NOT NULL)
          UNION
          (SELECT DISTINCT #element FROM {#mediamosa_asset_mediafile} WHERE #app_id = :app_id and #element IS NOT NULL)
          UNION
          (SELECT DISTINCT #element FROM {#mediamosa_collection} WHERE #app_id = :app_id and #element IS NOT NULL)
          UNION
          (SELECT DISTINCT #select2 FROM {#table} WHERE #app_id = :app_id)
        ) AS tmp
       LEFT JOIN {#table} AS g ON g.#column1 = tmp.#column2 AND g.app_id = :app_id
       LEFT JOIN {#mediamosa_app} AS app ON app.#app_id = :app_id',
      $offset,
      $limit,
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediamosa_collection' => mediamosa_collection_db::TABLE_NAME,
        '#mediamosa_app' => mediamosa_app_db::TABLE_NAME,
        '#select' => $is_users ? 'owner_id AS user_id' : 'group_id',
        '#select2' => $is_users ? 'name AS owner_id' : 'group_id',
        '#table' => $is_users ? mediamosa_user_db::TABLE_NAME : mediamosa_user_group_db::TABLE_NAME,
        '#element' => $is_users ? 'owner_id' : 'group_id',
        '#column1' => $is_users ? 'name' : 'group_id',
        '#column2' => $is_users ? 'owner_id' : 'group_id',
        '#app_id' => mediamosa_user_db::APP_ID,
        '#quotum' => mediamosa_user_db::QUOTUM,
        '#created' => mediamosa_user_db::CREATED,
        '#changed' => mediamosa_user_db::CHANGED,
        ':app_id' => $app_id,
      )
    );

    // Set found rows.
    $result->found_rows = mediamosa_db::db_found_rows();

    // Return the result.
    return $result;
  }

  /**
   * Helper function to get user key by user ID.
   *
   * @param $user_id
   *
   * @return integer
   *  key value if user exists.
   */
  public static function key($app_id, $user_id) {
    $user_key = mediamosa_db::db_select(mediamosa_user_db::TABLE_NAME, 'u')
      ->fields('u', array(mediamosa_user_db::KEY))
      ->condition(mediamosa_user_db::NAME, $user_id)
      ->condition(mediamosa_user_db::APP_ID, $app_id)
      ->execute()
      ->fetchField();
    return $user_key;
  }

  /**
   * Retrieve ID of user's primary group. If user is not a member of any group
   * empty string returned.
   *
   * If primary group depends on soze of data to store - just add another
   * parameter here.
   *
   * @param $app_id
   * @param $user_id
   *
   * @return string with group ID. Empty string if user is not a member of any.
   */
  public static function get_primary_group($app_id, $user_id) {
    $group_ids = mediamosa_user::get_groups($app_id, $user_id);

    if (empty($group_ids)) {
      return '';
    }

    // Make the query to detect most available space from given group.
    // '0' as a quota means special case - no limit.
    $query = mediamosa_db::db_select(mediamosa_user_group_db::TABLE_NAME, 'g');
    $query->fields('g', array(mediamosa_user_group_db::ID));

    // Zero means 'no limit'
    $query->addExpression(mediamosa_user_group_db::QUOTUM . ' = 0', 'is_unlimited');

    $query->condition(mediamosa_user_group_db::APP_ID, $app_id);
    $query->condition(mediamosa_user_group_db::ID, $group_ids, 'IN');

    // Zero means 'no limit' so those groups are going first.
    $query->orderBy('is_unlimited', 'DESC');

    $query->orderBy(mediamosa_user_group_db::QUOTUM, 'DESC');
    $query->range(0, 1);

    $result = $query->execute();
    $group_with_highest_quotum = $result->fetchField();

    return $group_with_highest_quotum;
  }
}
