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
  * Still style.
  */

class mediamosa_asset_mediafile_still_style {
  // ------------------------------------------------------------------ Constants.

  // ------------------------------------------------------------------ Static Function.

  /**
   * Log message.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'mediamosa_asset_mediafile_still_style');
  }

  /**
   * Log mediafile ids.
   *
   * @param string $mediafile_id
   * @param string $message
   * @param array $variables
   * @param string $asset_id
   * @param string $severity
   */
  static public function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'mediamosa_asset_mediafile_still_style');
  }

  /**
   * Log message (low prio, is only logged when mediamosa_debug is lvl 5 or higher).
   *
   * @param string $message
   * @param array $variables
   */
  static public function log_debug($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'mediamosa_asset_mediafile_still_style');
  }

  /**
   * Log message (High prio, is only logged when mediamosa_debug is lvl 10 or higher).
   *
   * @param string $message
   * @param array $variables
   */
  static public function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_high($message, $variables, 'mediamosa_asset_mediafile_still_style');
  }

  /**
   * Get the still style data.
   *
   * @param string $style
   * @param num $app_id
   */
  static public function get($style, $app_id = NULL, array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_still_style_db::TABLE_NAME, 'ss');
    $query->fields('ss', $fields);
    $query->condition('ss.' . mediamosa_asset_mediafile_still_style_db::ID, $style);
    if ($app_id) {
      $query->condition('ss.' . mediamosa_asset_mediafile_still_style_db::APP_ID, $app_id);
    }
    return $query->execute()->fetchAssoc();
  }

  /**
   * Insert the still style data.
   *
   * @param array $fields
   */
  static public function insert($fields) {
    mediamosa_db::db_insert(mediamosa_asset_mediafile_still_style_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Update the still style data.
   *
   * @param string $style
   * @param num $app_id
   * @param array $fields
   */
  static public function update($style, $app_id = NULL, array $fields) {
    $query = mediamosa_db::db_update(mediamosa_asset_mediafile_still_style_db::TABLE_NAME)
      ->fields($fields);
    $query->condition(mediamosa_asset_mediafile_still_style_db::ID, $style);
    if ($app_id) {
      $query->condition(mediamosa_asset_mediafile_still_style_db::APP_ID, $app_id);
    }
    $num_updated = $query->execute();
  }

  /**
   * Delete the still style data
   *
   * @param string $style
   * @param num $app_id
   */
  static public function delete($style, $app_id = NULL) {
    $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_still_style_db::TABLE_NAME);
    $query->condition(mediamosa_asset_mediafile_still_style_db::ID, $style);
    if ($app_id) {
      $query->condition(mediamosa_asset_mediafile_still_style_db::APP_ID, $app_id);
    }
    $num_deleted = $query->execute();
  }
}
