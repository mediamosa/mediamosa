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
   * Get the still style data.
   *
   * @param $style_id
   *   The ID of the style.
   * @param $app_id
   *   Application ID.
   */
  static public function get($style_id, $app_id = NULL, array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_still_style_db::TABLE_NAME, 'ss');
    $query->fields('ss', $fields);
    $query->condition('ss.' . mediamosa_asset_mediafile_still_style_db::ID, $style_id);
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
   * @param string $style_id
   * @param num $app_id
   * @param array $fields
   */
  static public function update($style_id, $app_id = NULL, array $fields = array()) {
    $query = mediamosa_db::db_update(mediamosa_asset_mediafile_still_style_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_mediafile_still_style_db::ID, $style_id);

    if ($app_id) {
      $query->condition(mediamosa_asset_mediafile_still_style_db::APP_ID, $app_id);
    }

    $query->execute();
  }

  /**
   * Delete the still style data
   *
   * @param string $style_id
   * @param num $app_id
   */
  static public function delete($style_id, $app_id = NULL) {
    // Remove from our db.
    $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_still_style_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_still_style_db::ID, $style_id);

    if ($app_id) {
      $query->condition(mediamosa_asset_mediafile_still_style_db::APP_ID, $app_id);
    }
    $query->execute();

    // Remove style image files.
    self::delete_style_files($app_id, $style_id);
  }

  /**
   * Remove the style files of an specific style.
   *
   */
  static public function delete_style_files($app_id, $style_id) {
    // Remove symlinks of specific style.
    //
    // /media/app_id/[letter]/id[,style][.ext].
    mediamosa_io::exec(
      strtr(
        'find @media_app_path -maxdepth 2 -mindepth 2 -regex "^.*/[0-9]+/[a-zA-Z0-9]/[a-zA-Z0-9]+,@style_id\(\.[a-zA-Z0-9]+\)?$" -type l -delete',
        array(
          '@media_app_path' => mediamosa_configuration_storage::media_get_path($app_id),
          '@style_id' => $style_id,
        )
      )
    );

    // /media/ticket/app_id/[letter]/id[,style][.ext].
    mediamosa_io::exec(
      strtr(
        'find @media_app_path -maxdepth 2 -mindepth 2 -regex "^.*/[0-9]+/[a-zA-Z0-9]/[a-zA-Z0-9]+,@style_id\(\.[a-zA-Z0-9]+\)?$" -type l -delete',
        array(
          '@media_app_path' => mediamosa_configuration_storage::media_get_ticket_path($app_id),
          '@ticket_expiration' => (int)(mediamosa_settings::TICKET_EXPIRATION_STILL / 60),
        )
      )
    );

  }
}
