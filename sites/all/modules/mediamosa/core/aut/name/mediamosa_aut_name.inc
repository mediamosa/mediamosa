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
  * aut name class.
  */

class mediamosa_aut_name {
  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the aut name entry
   *
   * @param integer $app_id
   * @param string $aut_type
   * @param string $aut_name
   * @param integer $aut_group_id
   * @return array
   */
  static public function get($app_id, $aut_type, $aut_name, $aut_group_id/* can be NULL, do not use 0*/) {

    $query = mediamosa_db::db_select(mediamosa_aut_name_db::TABLE_NAME, 'an');
    $query->fields('an', array(mediamosa_aut_name_db::ID));
    $query->condition(mediamosa_aut_name_db::APP_ID, $app_id);
    $query->condition(mediamosa_aut_name_db::AUT_TYPE, $aut_type);

    // Possible NULL value test.
    is_null($aut_group_id) ? $query->isNull(mediamosa_aut_name_db::AUT_GROUP_ID) : $query->condition(mediamosa_aut_name_db::AUT_GROUP_ID, $aut_group_id);

    $query->condition(mediamosa_aut_name_db::AUT_NAME, $aut_name);

    // Return row.
    return $query->execute()->fetchAssoc();
  }

  /**
   * Remove aut name row(s).
   *
   * @param integer $app_id
   * @param string $aut_type
   * @param string $aut_name
   * @param integer $aut_group_id
   */
  static public function delete($app_id, $aut_type, $aut_name, $aut_group_id/* can be NULL, do not use 0*/) {
    $a_aut_name = self::get($app_id, $aut_type, $aut_name, $aut_group_id);
    if (!$a_aut_name) {
      throw new mediamosa_exception_error_aut_name_not_found(array('@hostname' => $aut_name));
    }

    // Delete all links to this aut_name
    mediamosa_aut_object::delete(mediamosa_aut_object_db::AUT_TYPE_NAME, $a_aut_name[mediamosa_aut_name_db::ID]);

    // Delete it.
    $query = mediamosa_db::db_delete(mediamosa_aut_name_db::TABLE_NAME);
    $query->condition(mediamosa_aut_name_db::APP_ID, $app_id);
    $query->condition(mediamosa_aut_name_db::AUT_TYPE, $aut_type);

    // Possible NULL value test.
    is_null($aut_group_id) ? $query->isNull(mediamosa_aut_name_db::AUT_GROUP_ID) : $query->condition(mediamosa_aut_name_db::AUT_GROUP_ID, $aut_group_id);

    $query->condition(mediamosa_aut_name_db::AUT_NAME, $aut_name);

    // Delete it.
    $query->execute();
  }

  /**
   * Creates a entry in the aut_name table.
   *
   * @param integer $app_id
   * @param string $s_name
   * @param string $aut_type
   * @param string $aut_group_id
   * @return integer
   *  return the last inserted ID or if already in DB, the ID of existing row
   */
  function create($app_id, $s_name, $aut_type, $aut_group_id = NULL) {

    switch ($aut_type) {
      case mediamosa_aut::AUT_NAME_TYPE_APP: // no longer in the aut_name table
        assert(0);
        throw new mediamosa_exception_error_unexpected_error();
      case mediamosa_aut::AUT_NAME_TYPE_DOMAIN:
      case mediamosa_aut::AUT_NAME_TYPE_USER:
      case mediamosa_aut::AUT_NAME_TYPE_USER_GROUP:
        $s_suffix = $s_name;
        $s_prefix = NULL;
        break;
      case mediamosa_aut::AUT_NAME_TYPE_REALM:
        if (mediamosa_unicode::substr($s_name, '@') === FALSE) {
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
    $query = mediamosa_db::db_select(mediamosa_aut_name_db::TABLE_NAME, 'an');
    $query->fields('an', array(mediamosa_aut_name_db::ID));
    $query->condition(mediamosa_aut_name_db::APP_ID, $app_id);
    $query->condition(mediamosa_aut_name_db::AUT_TYPE, $aut_type);

    // Possible NULL value test.
    is_null($aut_group_id) ? $query->isNull(mediamosa_aut_name_db::AUT_GROUP_ID) : $query->condition(mediamosa_aut_name_db::AUT_GROUP_ID, $aut_group_id);
    is_null($s_suffix) ? $query->isNull(mediamosa_aut_name_db::AUT_NAME) : $query->condition(mediamosa_aut_name_db::AUT_NAME, $s_suffix);
    is_null($s_prefix) ? $query->isNull(mediamosa_aut_name_db::AUT_PREFIX) : $query->condition(mediamosa_aut_name_db::AUT_PREFIX, $s_prefix);

    // Get the ID.
    $aut_id = $query->execute()->fetchField();

    if ($aut_id) {
      return $aut_id;
    }

    $a_fields = array(
      mediamosa_aut_name_db::APP_ID => $app_id,
      mediamosa_aut_name_db::AUT_TYPE => $aut_type,
      mediamosa_aut_name_db::AUT_GROUP_ID => $aut_group_id,
      mediamosa_aut_name_db::AUT_NAME => $s_suffix,
      mediamosa_aut_name_db::AUT_PREFIX => $s_prefix
    );

    // No enrich here.

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_aut_name_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }
}
