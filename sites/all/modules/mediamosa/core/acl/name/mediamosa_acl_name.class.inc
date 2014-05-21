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
  * acl name class.
  */

class mediamosa_acl_name {
  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the acl name entry
   *
   * @param int $app_id
   * @param string $acl_type
   * @param string $acl_name
   * @param int $acl_group_id
   * @return array
   */
  public static function get($app_id, $acl_type, $acl_name, $acl_group_id/* can be NULL, do not use 0*/) {

    $query = mediamosa_db::db_select(mediamosa_acl_name_db::TABLE_NAME, 'an');
    $query->fields('an', array(mediamosa_acl_name_db::ID));
    $query->condition(mediamosa_acl_name_db::APP_ID, $app_id);
    $query->condition(mediamosa_acl_name_db::ACL_TYPE, $acl_type);

    // Possible NULL value test.
    is_null($acl_group_id) ? $query->isNull(mediamosa_acl_name_db::ACL_GROUP_ID) : $query->condition(mediamosa_acl_name_db::ACL_GROUP_ID, $acl_group_id);

    $query->condition(mediamosa_acl_name_db::ACL_NAME, $acl_name);

    // Return row.
    return $query->execute()->fetchAssoc();
  }

  /**
   * Remove acl name row(s).
   *
   * @param int $app_id
   * @param string $acl_type
   * @param string $acl_name
   * @param int $acl_group_id
   */
  public static function delete($app_id, $acl_type, $acl_name, $acl_group_id/* can be NULL, do not use 0*/) {
    $a_acl_name = self::get($app_id, $acl_type, $acl_name, $acl_group_id);
    if (!$a_acl_name) {
      throw new mediamosa_exception_error_acl_name_not_found(array('@hostname' => $acl_name));
    }

    // Delete all links to this acl_name
    mediamosa_acl_object::delete(mediamosa_acl_object_db::ACL_TYPE_NAME, $a_acl_name[mediamosa_acl_name_db::ID]);

    // Delete it.
    $query = mediamosa_db::db_delete(mediamosa_acl_name_db::TABLE_NAME);
    $query->condition(mediamosa_acl_name_db::APP_ID, $app_id);
    $query->condition(mediamosa_acl_name_db::ACL_TYPE, $acl_type);

    // Possible NULL value test.
    is_null($acl_group_id) ? $query->isNull(mediamosa_acl_name_db::ACL_GROUP_ID) : $query->condition(mediamosa_acl_name_db::ACL_GROUP_ID, $acl_group_id);

    $query->condition(mediamosa_acl_name_db::ACL_NAME, $acl_name);

    // Delete it.
    $query->execute();
  }

  /**
   * Creates a entry in the acl_name table.
   *
   * @param int $app_id
   * @param string $s_name
   * @param string $acl_type
   * @param string $acl_group_id
   * @return integer
   *  return the last inserted ID or if already in DB, the ID of existing row
   */
  public static function create($app_id, $s_name, $acl_type, $acl_group_id = NULL) {

    switch ($acl_type) {
      case mediamosa_acl::ACL_NAME_TYPE_APP: // no longer in the acl_name table
        assert(0);
        throw new mediamosa_exception_error_unexpected_error();
      case mediamosa_acl::ACL_NAME_TYPE_DOMAIN:
      case mediamosa_acl::ACL_NAME_TYPE_USER:
      case mediamosa_acl::ACL_NAME_TYPE_USER_GROUP:
        $s_suffix = $s_name;
        $s_prefix = NULL;
        break;
      case mediamosa_acl::ACL_NAME_TYPE_REALM:
        if (mediamosa_unicode::strpos($s_name, '@') === FALSE) {
          $s_suffix = '@' . $s_name;
          $s_prefix = NULL;
        }
        else {
          list($s_prefix, $s_suffix) = explode('@', $s_name, 2);
          $s_suffix = '@' . $s_suffix;
        }

        if (trim($s_prefix) == "") {
          $s_prefix = NULL;
        }
        break;
      default:
        throw new mediamosa_exception_error_unexpected_error();
    }

    // Get the row (if exists).
    $query = mediamosa_db::db_select(mediamosa_acl_name_db::TABLE_NAME, 'an');
    $query->fields('an', array(mediamosa_acl_name_db::ID));
    $query->condition(mediamosa_acl_name_db::APP_ID, $app_id);
    $query->condition(mediamosa_acl_name_db::ACL_TYPE, $acl_type);

    // Possible NULL value test.
    is_null($acl_group_id) ? $query->isNull(mediamosa_acl_name_db::ACL_GROUP_ID) : $query->condition(mediamosa_acl_name_db::ACL_GROUP_ID, $acl_group_id);
    is_null($s_suffix) ? $query->isNull(mediamosa_acl_name_db::ACL_NAME) : $query->condition(mediamosa_acl_name_db::ACL_NAME, $s_suffix);
    is_null($s_prefix) ? $query->isNull(mediamosa_acl_name_db::ACL_PREFIX) : $query->condition(mediamosa_acl_name_db::ACL_PREFIX, $s_prefix);

    // Get the ID.
    $acl_id = $query->execute()->fetchField();

    if ($acl_id) {
      return $acl_id;
    }

    $a_fields = array(
      mediamosa_acl_name_db::APP_ID => $app_id,
      mediamosa_acl_name_db::ACL_TYPE => $acl_type,
      mediamosa_acl_name_db::ACL_GROUP_ID => $acl_group_id,
      mediamosa_acl_name_db::ACL_NAME => $s_suffix,
      mediamosa_acl_name_db::ACL_PREFIX => $s_prefix
    );

    // No enrich here.

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_acl_name_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }
}
