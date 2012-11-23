<?php
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
  public static function get($style_id, $app_id = NULL, array $fields = array()) {
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
  public static function insert($fields) {
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
  public static function update($style_id, $app_id = NULL, array $fields = array()) {
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
  public static function delete($style_id, $app_id = NULL) {
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
  public static function delete_style_files($app_id, $style_id) {
    // Remove symlinks of specific style.
    //
    // /media/app_id/[letter]/id[,style][.ext].
    mediamosa_io::exec(
      strtr(
        'find @media_app_path -maxdepth 2 -mindepth 2 -regex "^.*/[0-9]+/[a-zA-Z0-9]/[a-zA-Z0-9]+,@style_id\(\.[a-zA-Z0-9]+\)?$" -type l -delete',
        array(
          '@media_app_path' => mediamosa_io::realpath(mediamosa_storage::get_uri_media($app_id)),
          '@style_id' => $style_id,
        )
      )
    );

    // /media/ticket/app_id/[letter]/id[,style][.ext].
    mediamosa_io::exec(
      strtr(
        'find @media_app_path -maxdepth 2 -mindepth 2 -regex "^.*/[0-9]+/[a-zA-Z0-9]/[a-zA-Z0-9]+,@style_id\(\.[a-zA-Z0-9]+\)?$" -type l -delete',
        array(
          '@media_app_path' => mediamosa_io::realpath(mediamosa_storage::get_uri_media_ticket($app_id)),
          '@ticket_expiration' => (int) (mediamosa_storage::TICKET_EXPIRATION_STILL / 60),
        )
      )
    );

  }
}
