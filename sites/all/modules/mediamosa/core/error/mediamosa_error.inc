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

// FIXME: Because we introduce new module (mediamosa_sdk) this module will
//        not be found until its turned on. So after 1st release, remove this.
//        Also re-add dependency on mediamosa module -> mediamosa_sdk.
require_once DRUPAL_ROOT . '/sites/all/modules/mediamosa_sdk/mediamosa_sdk.inc';

/**
 * @file
 * The MediaMosa error class.
 */
class mediamosa_error extends mediamosa_sdk {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the listing of the error code table.
   *
   * @return array
   */
  static public function error_code_list($order_by = mediamosa_error_db::CODE, $order_direction = mediamosa_type::ORDER_DIRECTION_ASC, $errorcode = 0) {
    static $codes = NULL;

    if (!isset($codes[$order_by][$order_direction])) {
      $codes[$order_by][$order_direction] = mediamosa_db::db_select(mediamosa_error_db::TABLE_NAME, 'e')
        ->fields('e')
        ->orderBy($order_by, $order_direction)
        ->execute()->fetchAllAssoc($order_by, PDO::FETCH_ASSOC);
    }

    // If errorcode was specified, get that one specific.
    if ($errorcode && $order_by == mediamosa_error_db::CODE) {
      if (isset($codes[$order_by][$order_direction][$errorcode])) {
        return array($errorcode => (array)$codes[$order_by][$order_direction][$errorcode]);
      }

      return array();
    }
    elseif ($errorcode) {
      assert(0);
    }

    return $codes[$order_by][$order_direction];
  }

  /**
   * Find the error code.
   *
   * @param integer $code
   * @return array
   */
  static public function error_code_find($errorcode, $tags = array(), $fail = FALSE) {

    // Get the current listing.
    $code = self::error_code_list(mediamosa_error_db::CODE, mediamosa_type::ORDER_DIRECTION_ASC, $errorcode);

    // Do we find the error?
    if (!isset($code[$errorcode])) {
      if ($fail) {
        return FALSE;
      }

      return array(
        mediamosa_error_db::MESSAGE => strtr('Server error; Unable to find error code: @errorcode', array('@errorcode' => $errorcode)),
        mediamosa_error_db::CODE => self::HTTP_INTERNAL_SERVER_ERROR,
      );
    }

    // Fill message with tags.
    if (!empty($tags)) {
      $code[$errorcode][mediamosa_error_db::MESSAGE] = strtr($code[$errorcode][mediamosa_error_db::MESSAGE], $tags);
    }

    return $code[$errorcode];
  }

  /**
   * Find the error code and the description.
   *
   * @param integer $errorcode
   * @return array
   */
  static public function error_code_find_description($errorcode, $tags = array()) {
    $code = self::error_code_find($errorcode, $tags);

    return ($code ? $code[mediamosa_error_db::MESSAGE] : 'Description not available.');
  }

  /**
   * Get errorcode by row id.
   *
   * @param integer $id
   */
  static public function get($id) {
    return mediamosa_db::db_select(mediamosa_error_db::TABLE_NAME, 'e')
      ->fields('e')
      ->condition(mediamosa_error_db::ID, $id)
      ->execute()
      ->fetchAssoc();
  }
}
