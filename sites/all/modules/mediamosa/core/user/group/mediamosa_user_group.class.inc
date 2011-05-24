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
  * The user group module.
  */
class mediamosa_user_group {

  // ------------------------------------------------------------------ Functions static.
  /**
   * Get the user group.
   *
   * @param string $group_id
   *  Group to retrieve.
   * @param integer $app_id
   *  Application ID.
   */
  static public function get($group_id, $app_id, array $a_fields = NULL) {
    return mediamosa_db::db_select(mediamosa_user_group_db::TABLE_NAME, 'g')
      ->fields('g', $a_fields)
      ->condition(mediamosa_user_group_db::ID, $group_id)
      ->condition(mediamosa_user_group_db::APP_ID, $app_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Collect and calculate group quota.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $group_id
   *  Group ID.
   */
  static public function get_user_group_quota($app_id, $group_id) {
    $total = mediamosa_db::db_query(
      'SELECT COUNT(*) AS total FROM
       (
         (SELECT #group_id FROM {#mediamosa_user_group} WHERE #group_id = :group_id AND #app_id = :app_id)
         UNION
         (SELECT #group_id FROM {#mediamosa_collection} WHERE #group_id = :group_id AND #app_id = :app_id)
         UNION
         (SELECT #group_id FROM {#mediamosa_asset} AS a WHERE #group_id = :group_id AND #app_id = :app_id)
         UNION
         (SELECT #group_id FROM {#mediamosa_asset_mediafile} WHERE #group_id = :group_id AND #app_id = :app_id)
       ) tmp',
      array(
        '#mediamosa_user_group' => mediamosa_user_group_db::TABLE_NAME,
        '#mediamosa_collection' => mediamosa_collection_db::TABLE_NAME,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#group_id' => mediamosa_user_group_db::ID,
        '#owner_id' => mediamosa_collection_db::OWNER_ID,
        ':group_id' => $group_id,
        '#app_id' => mediamosa_collection_db::APP_ID,
        ':app_id' => $app_id,
      )
    )->fetchField();

    if (!$total) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_USERMAN_INVALID_GROUP, array('@group_id' => $group_id));
    }

    // Get the group.
    $a_group = mediamosa_user_group::get($group_id, $app_id, array(mediamosa_user_group_db::CREATED, mediamosa_user_group_db::CHANGED));
    if (!$a_group) {
      $a_group = array();
    }

    // Get the quota.
    $a_quota = mediamosa_user::get_quota($app_id, '', $group_id);
    unset($a_quota['owner_quota_mb']);

    // Get free diskspace.
    $diskspace_free = mediamosa_user::check_user_quota($app_id, '', $group_id, TRUE);

    // Collection diskspace used on group/user/app.
    $a_diskspace_used = array(
      'group' => mediamosa_user::get_diskspace('group', $group_id, $app_id),
      'app' => mediamosa_user::get_diskspace('app', $app_id, $app_id)
    );

    // Make one array.
    return array_merge(
      $a_quota,
      array(
        'app_diskspace_used_mb' => $a_diskspace_used['app']['diskspace_used_mb'],
        'group_diskspace_used_mb' => $a_diskspace_used['group']['diskspace_used_mb'],
        'quota_available_mb' => $diskspace_free,
        'group_over_quota' => ($diskspace_free < 0) ? 'true' : 'false'
      ),
      $a_group
    );
  }

  /**
   * Create a group.
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $group_id
   *  The group ID.
   * @param integer $quotum
   *  The quotum.
   */
  static public function create($app_id, $group_id, $quotum) {
    $a_fields = array(
      mediamosa_user_group_db::APP_ID => $app_id,
      mediamosa_user_group_db::ID => $group_id,
      mediamosa_user_group_db::QUOTUM => $quotum,
    );

    // Add changed/created.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert into database.
    return mediamosa_db::db_insert(mediamosa_user_group_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Delete a group.
   *
   * @param integer $app_id
   *  The Application ID.
   * @param string $group_id
   *  The Group ID.
   */
  static public function delete($app_id, $group_id) {

    // Please note that group cannot be deleted while it contains users.
    // In case such operation become allowed one day we need to mimic code from
    // mediamosa_user::delete()

    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_user_group} WHERE #app_id = :app_id AND #group_id = :group_id',
      array(
        '#mediamosa_user_group' => mediamosa_user_group_db::TABLE_NAME,
        '#app_id' => mediamosa_user_group_db::APP_ID,
        ':app_id' => $app_id,
        '#group_id' => mediamosa_user_group_db::ID,
        ':group_id' => $group_id
      )
    );
  }

  /**
   * Update the group.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $group_id
   *  The group ID.
   * @param integer $quotum
   *  Quotum to change.
   */
  static public function update($app_id, $group_id, $quotum) {

    // Fields to set.
    $a_fields = array(
      mediamosa_user_group_db::QUOTUM => $quotum,
    );

    // Add changed value.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    $query = mediamosa_db::db_update(mediamosa_user_group_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_user_group_db::ID, $group_id)
      ->condition(mediamosa_user_group_db::APP_ID, $app_id)
      ->execute();
  }

  /**
   * Helper function to get group key by group ID.
   *
   * @param $group_id
   *
   * @return integer
   *  key value if group exists.
   */
  function key($app_id, $group_id) {
    $group_key = mediamosa_db::db_select(mediamosa_user_group_db::TABLE_NAME, 'g')
      ->fields('g', array(mediamosa_user_group_db::KEY))
      ->condition(mediamosa_user_group_db::ID, $group_id)
      ->condition(mediamosa_user_group_db::APP_ID, $app_id)
      ->execute()
      ->fetchField();
    return $group_key;
  }

}
