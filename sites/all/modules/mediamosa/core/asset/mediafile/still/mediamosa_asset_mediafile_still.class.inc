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
  * Stills are now mediafiles, so code for still have been moved
  * under mediafile.
  */

class mediamosa_asset_mediafile_still {
  // ------------------------------------------------------------------ Constants.
  // Watermark.
  const WATERMARK_ID = 'watermark_id';
  const WATERMARK_DST_X = 'watermark_dst_x';
  const WATERMARK_DST_Y = 'watermark_dst_y';
  const WATERMARK_PCT = 'watermark_pct';
  const WATERMARK_V_ALIGN = 'watermark_v_align';
  const WATERMARK_H_ALIGN = 'watermark_h_align';
  // Values.
  const WATERMARK_VALUE_LEFT = 'left';
  const WATERMARK_VALUE_RIGHT = 'right';
  const WATERMARK_VALUE_TOP = 'top';
  const WATERMARK_VALUE_BOTTOM = 'bottom';
  // Default values.
  const WATERMARK_DEFAULT_DST_X = 0;
  const WATERMARK_DEFAULT_DST_Y = 0;
  const WATERMARK_DEFAULT_PCT = 75;
  const WATERMARK_DEFAULT_V_ALIGN = self::WATERMARK_VALUE_LEFT;
  const WATERMARK_DEFAULT_H_ALIGN = self::WATERMARK_VALUE_TOP;

  const IMAGE_TYPE_JPG = 'JPG';
  const IMAGE_TYPE_PNG = 'PNG';

  const SAVE_QUALITY = 100;

  // ------------------------------------------------------------------ Static Function.

  /**
   * Log message.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'mediamosa_asset_mediafile_still');
  }

  /**
   * Log mediafile ids.
   *
   * @param string $mediafile_id
   * @param string $asset_id
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'mediamosa_asset_mediafile_still');
  }

  /**
   * Log message (low prio, is only logged when mediamosa_debug is lvl 5 or higher).
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log_debug($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'mediamosa_asset_mediafile_still');
  }

  /**
   * Log message (High prio, is only logged when mediamosa_debug is lvl 10 or higher).
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_high($message, $variables, 'mediamosa_asset_mediafile_still');
  }

  /**
   * Get the image types.
   *
   * @return array $types
   */
  public static function get_types() {
    // Assign usefull values for the third index of getimagesize() function.
    $types = array(
      1 => 'GIF',
      2 => 'JPG',
      3 => 'PNG',
      4 => 'SWF',
      5 => 'PSD',
      6 => 'BMP',
      7 => 'TIFF(intel byte order)',
      8 => 'TIFF(motorola byte order)',
      9 => 'JPC',
      10 => 'JP2',
      11 => 'JPX',
      12 => 'JB2',
      13 => 'SWC',
      14 => 'IFF',
      15 => 'WBMP',
      16 => 'XBM',
    );

    return $types;
  }
  /**
   * Create watermark.
   *
   * @param string $still_id
   * @param int $app_id
   * @param array $still_parameters
   * @param boolean $create_new_still
   *
   * @return boolean
   *   FALSE, in case of error, TRUE, in case of success, $still_id_new, in case of $create_new_still == TRUE
   */
  public static function create_watermark($still_id, $app_id, $still_parameters, $create_new_still = FALSE) {

    // If there isn't watermark_id, then we don't watermarking the still image.
    if (empty($still_parameters[self::WATERMARK_ID])) {
      return FALSE;
    }

    self::log_debug_high(t('Start watermarking. (still_id = @still_id, app_id = @app_id, new still = @new_still, still_params = @still_params)', array(
      '@still_id' => print_r($still_id, TRUE),
      '@app_id' => print_r($app_id, TRUE),
      '@new_still' => ($create_new_still ? t('yes') : t('no')),
      '@still_params' => print_r($still_parameters, TRUE),
    )));

    // Watermark image must exists.
    $watermark = mediamosa_asset_mediafile::get($still_parameters[self::WATERMARK_ID], $app_id, array(), mediamosa_asset_mediafile_db::IS_STILL_TRUE);
    if (empty($watermark[mediamosa_asset_mediafile_db::ID])) {
      self::log_mediafile($still_parameters[self::WATERMARK_ID], t('Watermark id is not exist.'));
      return;
    }

    // Get path to still.
    $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);

    // Get path to watermark.
    $watermark_filename = mediamosa_configuration_storage::mediafile_still_filename_get($watermark[mediamosa_asset_mediafile_db::ID]);

    $still_size = mediamosa_gd::getimagesize($still_filename);
    if (!$still_size) {
      self::log_mediafile($still_id, t('Watermark getimagesize error (still).'));
      return;
    }

    $watermark_size = mediamosa_gd::getimagesize($watermark_filename);
    if (!$watermark_size) {
      self::log_mediafile($still_parameters[self::WATERMARK_ID], t('Watermark getimagesize error (watermark).'));
      return;
    }

    // Check the image types.
    $types = self::get_types();
    if (empty($types[$still_size[2]]) || empty($types[$watermark_size[2]])) {
      self::log(t('Image type is not suitable for watermarking'));
      return;
    }
    if ($types[$still_size[2]] != self::IMAGE_TYPE_JPG && $types[$still_size[2]] != self::IMAGE_TYPE_PNG) {
      self::log_mediafile($still_id, t('Image type is not suitable for watermarking (still)'));
      return;
    }
    if ($types[$watermark_size[2]] != self::IMAGE_TYPE_JPG && $types[$watermark_size[2]] != self::IMAGE_TYPE_PNG) {
      self::log_mediafile($still_parameters[self::WATERMARK_ID], t('Image type is not suitable for watermarking (watermark)'));
      return;
    }

    // Destination.
    if ($types[$still_size[2]] == self::IMAGE_TYPE_JPG) {
      $dest = mediamosa_gd::imagecreatefromjpeg($still_filename);
    }
    else {
      $dest = mediamosa_gd::imagecreatefrompng($still_filename);
    }

    // Watermark.
    if ($types[$watermark_size[2]] == self::IMAGE_TYPE_JPG) {
      $src = mediamosa_gd::imagecreatefromjpeg($watermark_filename);
    }
    else {
      $src = mediamosa_gd::imagecreatefrompng($watermark_filename);
    }

    // Values.
    $dst_x = 0;
    $dst_y = 0;
    $src_x = 0;
    $src_y = 0;
    $src_w = mediamosa_gd::imagesx($src);
    $src_h = mediamosa_gd::imagesy($src);
    $pct = $still_parameters[self::WATERMARK_PCT];

    // Destination x calculation.
    if ($still_parameters[self::WATERMARK_V_ALIGN] == self::WATERMARK_VALUE_LEFT) {
      $dst_x = $still_parameters[self::WATERMARK_DST_X];
    }
    else {
      $dst_x = mediamosa_gd::imagesx($dest) - $src_w - $still_parameters[self::WATERMARK_DST_X];
    }

    // Destination y calculation.
    if ($still_parameters[self::WATERMARK_H_ALIGN] == self::WATERMARK_VALUE_TOP) {
      $dst_y = $still_parameters[self::WATERMARK_DST_Y];
    }
    else {
      $dst_y = mediamosa_gd::imagesy($dest) - $src_h - $still_parameters[self::WATERMARK_DST_Y];
    }

    // Copymerge.
    self::log_debug_high(t('Call imagecopymerge. (dest = @dest, src = @src, dst_x = @dst_x, dst_y = @dst_y, src_x = @src_x, src_y = @src_y, src_w = @src_w, src_h = @src_h, pct = @pct)', array(
      '@dest' => print_r($dest, TRUE),
      '@src' => print_r($src, TRUE),
      '@dst_x' => print_r($dst_x, TRUE),
      '@dst_y' => print_r($dst_y, TRUE),
      '@src_x' => print_r($src_x, TRUE),
      '@src_y' => print_r($src_y, TRUE),
      '@src_w' => print_r($src_w, TRUE),
      '@src_h' => print_r($src_h, TRUE),
      '@pct' => print_r($pct, TRUE),
    )));
    mediamosa_gd::imagecopymerge($dest, $src, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct);

    // Save image.
    if ($create_new_still) {
      // New still.
      $still_id_new = mediamosa_db::uuid($app_id);
      $still_filename_new = mediamosa_configuration_storage::mediafile_still_filename_get($still_id_new);

      // Create possible missing directories.
      mediamosa_io::mkdir(mediamosa_io::dirname($still_filename_new));
    }
    // Write still.
    if ($types[$still_size[2]] == self::IMAGE_TYPE_JPG) {
      mediamosa_gd::imagejpeg($dest, ($create_new_still ? $still_filename_new : $still_filename), self::SAVE_QUALITY);
    }
    else {
      mediamosa_gd::imagepng($dest, ($create_new_still ? $still_filename_new : $still_filename), self::SAVE_QUALITY);
    }

    // Destroye resources.
    mediamosa_gd::imagedestroy($dest);
    mediamosa_gd::imagedestroy($src);

    // Response the new still id.
    if ($create_new_still) {
      return $still_id_new;
    }

    return TRUE;
  }

  /**
   * Create a still
   *
   * @param string $asset_id
   * @param string $still_id
   * @param integer $app_id
   * @param string $owner_id
   * @param string $group_id
   * @param integer $order
   * @param boolean $still_default
   * @param array $still_parameters
   * @param integer $sec
   * @param string $mediafile_id_source
   * @param string $tag
   */
  public static function create($asset_id, $still_id, $app_id, $owner_id, $group_id, $order, $still_default = TRUE, $still_parameters = NULL, $sec = 0, $mediafile_id_source = '', $tag = '') {

    // Get path to still.
    $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);

    // Check if still exists.
    if (!mediamosa_io::file_exists($still_filename)) {
      mediamosa_job::post_create_job_failed($app_id, $owner_id, $still_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, 'File not exists.', $still_parameters);

      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    // Handle watermark.
    self::create_watermark($still_id, $app_id, $still_parameters);

    $size = mediamosa_gd::getimagesize($still_filename);
    if (!$size) {
      mediamosa_job::post_create_job_failed($app_id, $owner_id, $still_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, 'File is not an image.', $still_parameters);

      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_IMAGE, array('@asset_id' => $asset_id));
    }

    // Set width, height.
    $width = $size[0];
    $height = $size[1];

    // Mime type.
    $mime_type = $size['mime'];

    // Set file_type of still based on the mime type of the image.
    $pos = strrpos($mime_type, '/');
    $file_extension = ($pos === FALSE ? '' : mediamosa_unicode::substr($mime_type, $pos + 1));

    // Get file size of still.
    $file_size = mediamosa_io::filesize($still_filename);

    if ($still_default) {
      $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::STILL_DEFAULT);
      // Clear the earlier default mark on the video (media) file.
      mediamosa_db::db_query(
        'UPDATE {#mediamosa_asset_mediafile_metadata} AS mm
         JOIN {#mediamosa_asset_mediafile} AS m ON m.#mediafile_id = mm.#mediafile_id AND mm.#prop_id = :prop_id
         SET mm.#val_char = :still_default_false
         WHERE m.#mediafile_id_source = :mediafile_id_source AND m.#is_still = :is_still_true',
        array(
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
          ':prop_id' => $prop_id,
          '#val_char' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
          ':still_default_false' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_FALSE,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          ':mediafile_id_source' => $mediafile_id_source,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE
        )
      );
    }

    // Insert into Mediafile.
    $fields = array(
        mediamosa_asset_mediafile_db::ID => $still_id,
        mediamosa_asset_mediafile_db::ASSET_ID => $asset_id,
        mediamosa_asset_mediafile_db::APP_ID => $app_id,
        mediamosa_asset_mediafile_db::OWNER_ID => $owner_id,
        mediamosa_asset_mediafile_db::GROUP_ID => $group_id,
        mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_configuration_storage::mount_point_get(),
        mediamosa_asset_mediafile_db::FILE_EXTENSION => $file_extension,
        mediamosa_asset_mediafile_db::IS_STILL => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => $mediafile_id_source,
        mediamosa_asset_mediafile_db::TAG => $tag
    );

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Insert into Mediafile.
    $metadatas = array(
      array(
        'value' => $width,
        'prop_name' => mediamosa_asset_mediafile_metadata::WIDTH,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $height,
        'prop_name' => mediamosa_asset_mediafile_metadata::HEIGHT,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      //mediamosa_asset_mediafile_metadata_db::CONTAINER_TYPE => '',
      array(
        'value' => $file_size,
        'prop_name' => mediamosa_asset_mediafile_metadata::FILESIZE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $mime_type,
        'prop_name' => mediamosa_asset_mediafile_metadata::MIME_TYPE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'value' => $sec,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_TIME_CODE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $order,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_ORDER,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $file_extension,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_FORMAT,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'value' => (empty($still_parameters['still_type']) ? mediamosa_asset_mediafile_metadata::STILL_TYPE_NORMAL : $still_parameters['still_type']),
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_TYPE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'value' => ($still_default ? mediamosa_asset_mediafile_metadata::STILL_DEFAULT_TRUE : mediamosa_asset_mediafile_metadata::STILL_DEFAULT_FALSE),
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
    );
    mediamosa_asset_mediafile_metadata::create_mediafile_multiple_metadata($still_id, $metadatas);

    mediamosa_job::post_create_job_finished($app_id, $owner_id, $still_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, '', $still_parameters);
  }

  /**
   * Change the order of an still
   *
   * @param string $asset_id
   * @param string $mediafile_id
   * @param string $still_id
   * @param integer $order
   */
  public static function set_still_order($app_id, $user_id, $asset_id, $mediafile_id, $still_id, $order) {
    mediamosa_watchdog::log_mediafile(
      $mediafile_id,
      "Still order: asset_id = @asset_id; mediafile_id = @mediafile_id; still_id = @still_id; order = @order;",
      array(
        '@asset_id' => $asset_id,
        '@mediafile_id' => $mediafile_id,
        '@still_id' => $still_id,
        '@order' => $order
      ),
      $asset_id
    );

    // Asset must exist.
    $asset = mediamosa_asset::get($asset_id);

    // Mediafile(s) must exist.
    mediamosa_db::db_must_exists(
      mediamosa_asset_mediafile_db::TABLE_NAME,
      array(
        mediamosa_asset_mediafile_db::ASSET_ID => $asset_id,
        mediamosa_asset_mediafile_db::ID => $mediafile_id,
      ),
      mediamosa_error::ERRORCODE_STILL_NOT_FOUND,
      array('@asset_id' => $asset_id)
    );

    // Still(s) must exist.
    mediamosa_db::db_must_exists(
      mediamosa_asset_mediafile_db::TABLE_NAME,
      array(
        mediamosa_asset_mediafile_db::ASSET_ID => $asset_id,
        mediamosa_asset_mediafile_db::ID => $still_id,
      ),
      mediamosa_error::ERRORCODE_STILL_NOT_FOUND,
      array('@asset_id' => $asset_id)
    );

    // Does the user has access?
    mediamosa_acl::owner_check(
      $app_id,
      $user_id,
      $asset[mediamosa_asset_db::APP_ID],
      $asset[mediamosa_asset_db::OWNER_ID]
    );

    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::STILL_ORDER);
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset_mediafile_metadata} AS mfmd
       JOIN {#mediamosa_asset_mediafile} AS mf ON mf.#mediafile_id = mfmd.mediafile_id
       SET mfmd.#val_char = :still_order
       WHERE mfmd.#mediafile_id = :mediafile_id AND mfmd.#prop_id = :prop_id AND mf.#asset_id = :asset_id AND mf.#is_still = :is_still_true',
      array(
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#val_char' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
        ':still_order' => $order,
        ':mediafile_id' => $still_id,
        '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
        ':prop_id' => $prop_id,
        '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
        ':asset_id' => $asset_id,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE
      )
    );
  }

  /**
   * Find all stills of an asset.
   *
   * @param string $asset_id
   * @return array
   */
  public static function find_asset_all_still($asset_id) {
    $still_ids = mediamosa_db::db_query(
      "SELECT #mediafile_id FROM {#mediamosa_asset_mediafile}
      WHERE #asset_id = :asset_id AND #is_still = :is_still_true",
      array(
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
        ':asset_id' => $asset_id,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
      )
    )->fetchCol();
    return $still_ids;
  }

  /**
   * Find default still or at least the first one you find.
   *
   * @param string $asset_id
   * @return string
   */
  public static function find_default($asset_id) {

    // Try default 1st.
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::STILL_DEFAULT);
    $still_id = mediamosa_db::db_query_range(
      'SELECT m.#mediafile_id FROM {#mediamosa_asset_mediafile} AS m
      JOIN {#mediamosa_asset_mediafile_metadata} AS mm USING(#mediafile_id)
      WHERE m.#asset_id = :asset_id AND m.#is_still = :is_still_true AND mm.#still_default = :still_default_true AND mm.#prop_id = :prop_id
      ORDER BY mm.#metadata_id DESC',
      0,
      1,
      array(
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
        ':asset_id' => $asset_id,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
        ':prop_id' => $prop_id,
        '#still_default' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
        ':still_default_true' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_TRUE,
        '#metadata_id' => mediamosa_asset_mediafile_metadata_db::ID
      )
    )->fetchField();

    // Not found? Try any.
    if (!$still_id) {
      $still_id = mediamosa_db::db_query_range(
        "SELECT m.#mediafile_id FROM {#mediamosa_asset_mediafile} AS m
        JOIN {#mediamosa_asset_mediafile_metadata} AS mm USING(#mediafile_id)
        WHERE m.#asset_id = :asset_id AND m.#is_still = :is_still_true
        ORDER BY mm.#metadata_id DESC",
        0,
        1,
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
          ':asset_id' => $asset_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
          '#metadata_id' => mediamosa_asset_mediafile_metadata_db::ID
        )
      )->fetchField();
    }

    // Check if the still id is in the database
    if (!$still_id) {
      // There isn't any still
      return;
    }

    // Check if the still is ok.
    self::check($still_id);

    // Return our ID.
    return $still_id;
  }

  /**
   * Check if still file is available.
   *
   * @param string $still_id
   */
  public static function check($still_id) {
    // Check if still file exists.
    $filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);
    if (!file_exists($filename)) {
      throw new mediamosa_exception_error_file_not_found(array('@filename' => $filename));
    }
  }

  /**
   * Creates a new still image derivative based on an image style.
   *
   * The derivative still is stored next to the original, but with ,[style_id]
   * behind the file name. It does not have a extension either, like the
   * original still.
   *
   * @param $mediafile_id
   *   The still mediafile ID.
   * @param $style_id
   *   The style of the image to create.
   *
   * @return
   *   The file path of the (created) style still.
   *
   * @see image_style_create_derivative()
   *
   * @throws mediamosa_exception_error_mediafile_not_found()
   * @throws mediamosa_exception_error()
   *   mediamosa_error::ERRORCODE_CREATING_DERIVATIVE
   */
  public static function image_style_create_derivative($mediafile_id, $style_id) {
    // The original still.
    $source_still = mediamosa_configuration_storage::data_still_get_file($mediafile_id);
    if (mediamosa_io::file_exists($source_still)) {
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id)); // not found?
    }

    // Validate the style.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Make sure the mediafile is a still.
    if ($mediafile[mediamosa_asset_mediafile_db::IS_STILL] != mediamosa_asset_mediafile_db::IS_STILL_TRUE) {
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id));
    }

    // Need app_id.
    $app_id = $mediafile[mediamosa_asset_mediafile_db::APP_ID];

    // Get the style.
    $still_style = mediamosa_asset_mediafile_still_style::get($style_id, $app_id);
    if (!$still_style) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CREATING_DERIVATIVE, array('@style' => $style_id, '@path' => $source_still));
    }

    // The [mount_point]/data/still version does not have a extension (like the
    // original).
    $mediafile_str = mediamosa_asset_mediafile_media::mediafile_str_build($mediafile_id, '', $style_id);

    // The destination still.
    $destination_still = mediamosa_configuration_storage::data_still_get_file($mediafile_str);

    // Create the derivative if not already.
    if (!mediamosa_io::file_exists($destination_still)) {
      // Load the style.
      $style_array = image_style_load($style_id);
      if (!empty($style_array)) {
        // Create the resized image.
        if (!image_style_create_derivative($style_array, $source_still, $destination_still)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CREATING_DERIVATIVE, array('@style' => $style_id, '@path' => $source_still));
        }
      }
    }

    return $destination_still;
  }

  /**
   * Delete a still from database and file.
   *
   * @param $asset_id
   *  The asset_id. (will remove all stills from this asset_id).
   * @param $mediafile_id
   *  Remove specific from Mediafile parent.
   * @param $still_id
   *  Remove specific still.
   */
  public static function delete($asset_id, $mediafile_id = '', $still_id = '') {
    // Get the mediafile stills we need to remove.
    if ($still_id != '') {
      // Get specific still.
      $mediafiles = mediamosa_db::db_query(
        'SELECT #mediafile_id, #app_id, #sannas_mount_point, #mediafile_id_source FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id AND #is_still = :is_still_true AND #asset_id = :asset_id',
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#sannas_mount_point' => mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          ':mediafile_id' => $still_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
          '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
          ':asset_id' => $asset_id,
        )
      );
    }
    elseif ($mediafile_id != '') {
      // Get all stills of mediafile.
      $mediafiles = mediamosa_db::db_query(
        'SELECT s.#mediafile_id, s.#app_id, s.#sannas_mount_point, s.#mediafile_id_source FROM {#mediamosa_asset_mediafile} AS s
         INNER JOIN {#mediamosa_asset_mediafile} AS m ON m.#asset_id = s.#asset_id
         WHERE m.#mediafile_id = :mediafile_id AND m.#asset_id = :asset_id AND s.#asset_id = :asset_id AND s.#mediafile_id_source = :mediafile_id_source AND s.#is_still = :is_still_true',
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#sannas_mount_point' => mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
          ':mediafile_id' => $mediafile_id,
          ':asset_id' => $asset_id,
          ':mediafile_id_source' => $mediafile_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        )
      );
    }
    else {
      // Get all stills from asset.
      $mediafiles = mediamosa_db::db_query(
        'SELECT #mediafile_id, #app_id, #sannas_mount_point, #mediafile_id_source FROM {#mediamosa_asset_mediafile} WHERE #asset_id = :asset_id AND #is_still = :is_still_true',
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#sannas_mount_point' => mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
          ':asset_id' => $asset_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        )
      );
    }

    // Collect the mediafiles we need to delete.
    $mediafile_ids = array();
    foreach ($mediafiles as $mediafile) {
      // Build the path and filename of the still.
      $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($mediafile['mediafile_id'], $mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]);
      if (isset($mediafile_id) && $mediafile_id) {
        mediamosa_watchdog::log_mediafile($mediafile_id, 'Deleting still: @location', array('@location' => $mediafile_filename), $asset_id);
      }

      // Remove data from SAN/NAS.

      // Ticket part.
      $query = mediamosa_db::db_select(mediamosa_ticket_db::TABLE_NAME, 't');
      $query
        ->condition('t.' . mediamosa_ticket_db::MEDIAFILE_ID, $mediafile['mediafile_id'])
        ->condition('t.' . mediamosa_ticket_db::TICKET_TYPE, mediamosa_ticket_db::TICKET_TYPE_STILL)
        ->fields('t', array(mediamosa_ticket_db::ID,));
      $result = $query->execute();
      foreach ($result as $record) {
        // Ticket.
        $ticket_id = $record[mediamosa_ticket_db::ID];

        // Get the app id.
        $app_id = $record[mediamosa_ticket_db::APP_ID];

        // Remove all tickets symlinks.
        mediamosa_io::exec(
          strtr(
            'find @media_ticket_location -maxdepth 3 -mindepth 3 -name "@wildmatch*" -type l -delete',
            array(
              '@media_ticket_location' => mediamosa_configuration_storage::media_get_ticket_path(),
              '@wildmatch' => $ticket_id,
            )
          )
        );
      }

      // Remove all permanent symlinks.
      mediamosa_io::exec(
        strtr(
          'find @media_location -maxdepth 3 -mindepth 3 -name "@wildmatch*" -type l -delete',
          array(
            '@media_location' => mediamosa_configuration_storage::media_get_path(),
            '@wildmatch' => $mediafile['mediafile_id'],
          )
        )
      );

      // Remove stills.
      mediamosa_io::unlink($still_filename);

      // Collect the mediafile id of the still.
      $mediafile_ids[] = $mediafile['mediafile_id'];

      if ($mediafile_id == '') {
        $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];
      }
    }

    // Remove the stills from the database.
    if (!empty($mediafile_ids)) {
      // Remove our metadatas.
      mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
        ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_ids, 'IN')
        ->execute();

      // Remove the mediafiles.
      mediamosa_db::db_delete(mediamosa_asset_mediafile_db::TABLE_NAME)
        ->condition(mediamosa_asset_mediafile_db::ID, $mediafile_ids, 'IN')
        ->execute();

      // Remove tickets.
      mediamosa_db::db_delete(mediamosa_ticket_db::TABLE_NAME)
        ->condition(mediamosa_ticket_db::MEDIAFILE_ID, $mediafile_ids, 'IN')
        ->condition(mediamosa_ticket_db::TICKET_TYPE, mediamosa_ticket_db::TICKET_TYPE_STILL)
        ->execute();
    }

    // Happens when there where no stills to delete. (check added 2.x)
    if (!isset($mediafile_id)) {
      return;
    }

    // Is there a default still ?
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::STILL_DEFAULT);
    $mediafile_id_default = mediamosa_db::db_query_range(
      'SELECT mf.#mediafile_id FROM {#mediamosa_asset_mediafile_metadata} AS mfmd
       INNER JOIN {#mediamosa_asset_mediafile} AS mf ON mf.#mediafile_id = mfmd.#mediafile_id
       WHERE mfmd.#still_default = :still_default_true AND mf.#asset_id = :asset_id AND mf.#is_still = :is_still_true AND mf.#mediafile_id_source = :mediafile_id_source AND mfmd.#prop_id = :prop_id',
      0,
      1,
      array(
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
        ':asset_id' => $asset_id,
        '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
        ':prop_id' => $prop_id,
        '#still_default' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
        ':still_default_true' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_TRUE,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
        '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
        ':mediafile_id_source' => $mediafile_id,
      )
    )->fetchField();

    // No default, create one.
    if (!$mediafile_id_default) {
      $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::STILL_ORDER);
      $mediafile_id_default = mediamosa_db::db_query_range(
        'SELECT mf.#mediafile_id FROM {#mediamosa_asset_mediafile_metadata} AS mfmd
         INNER JOIN {#mediamosa_asset_mediafile} AS mf ON mf.#mediafile_id = mfmd.#mediafile_id
         WHERE mf.#asset_id = :asset_id AND mf.#is_still = :is_still_true AND mf.#mediafile_id_source = :mediafile_id_source AND mfmd.#prop_id = :prop_id
         ORDER BY mfmd.#still_order ASC',
        0,
        1,
        array(
          '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
          ':asset_id' => $asset_id,
          '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
          ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
          '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          ':mediafile_id_source' => $mediafile_id,
          '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
          ':prop_id' => $prop_id,
          '#still_order' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
        )
      )->fetchField();

      if ($mediafile_id_default) {
        // Found new default? Then set it as default.
        $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::STILL_DEFAULT);
        mediamosa_db::db_query(
          'UPDATE {#mediamosa_asset_mediafile_metadata} SET #still_default = :still_default_true WHERE #mediafile_id = :mediafile_id AND #prop_id = :prop_id',
          array(
            '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
            '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
            ':prop_id' => $prop_id,
            '#still_default' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
            ':still_default_true' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_TRUE,
            '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
            ':mediafile_id' => $mediafile_id_default
          )
        );
      }
    }
  }

  /**
   * Remove the style files of an specific style.
   *
   */
  public static function delete_files($app_id) {
    // Get media path, remove the symlinks.
    $media_app_path = mediamosa_configuration_storage::media_get_path($app_id);

    // Remove originals.
  }

  /**
   * Set the default still.
   * 1.x: _media_management_set_still_default
   * FIXME: the mediafile id unused.
   * 2.x: no longer returns TRUE by default.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   * The user ID for owner check.
   * @param string $asset_id
   *  The asset ID.
   * @param string $mediafile_id (unused!)
   *  The mediafile ID.
   * @param string $still_id
   *  The still ID.
   */
  public static function set_still_default($app_id, $user_id, $asset_id, $mediafile_id, $still_id) {
    mediamosa_watchdog::log_mediafile(
      $mediafile_id,
      'Setting default still: asset_id = @asset_id; mediafile_id = @mediafile_id; still_id = @still_id',
      array(
        '@asset_id' => $asset_id,
        '@mediafile_id' => $mediafile_id,
        '@still_id' => $still_id,
      ),
      $asset_id
    );

    // Asset must exist.
    $asset = mediamosa_asset::must_exists($asset_id);

    // Mediafile(s) must exist.
    mediamosa_db::db_must_exists(
      mediamosa_asset_mediafile_db::TABLE_NAME,
      array(
        mediamosa_asset_mediafile_db::ASSET_ID => $asset_id,
        mediamosa_asset_mediafile_db::ID => $mediafile_id,
      ),
      mediamosa_error::ERRORCODE_STILL_NOT_FOUND,
      array('@asset_id' => $asset_id)
    );

    // Still(s) must exist.
    mediamosa_db::db_must_exists(
      mediamosa_asset_mediafile_db::TABLE_NAME,
      array(
        mediamosa_asset_mediafile_db::ASSET_ID => $asset_id,
        mediamosa_asset_mediafile_db::ID => $still_id,
      ),
      mediamosa_error::ERRORCODE_STILL_NOT_FOUND,
      array('@asset_id' => $asset_id)
    );

    // Does the user has access?
    mediamosa_acl::owner_check(
      $app_id,
      $user_id,
      $asset[mediamosa_asset_db::APP_ID],
      $asset[mediamosa_asset_db::OWNER_ID]
    );

    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::STILL_DEFAULT);
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset_mediafile_metadata} AS mfmd
       JOIN {#mediamosa_asset_mediafile} AS mf ON mf.#mediafile_id = mfmd.#mediafile_id
       SET mfmd.#still_default = IF(mfmd.#mediafile_id = :mediafile_id, :still_default_true, :still_default_false)
       WHERE mf.#asset_id = :asset_id AND mf.#is_still = :#is_still_true AND mfmd.#prop_id = :prop_id',
      array(
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $still_id,
        '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
        ':prop_id' => $prop_id,
        '#still_default' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
        ':still_default_true' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_TRUE,
        ':still_default_false' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_FALSE,
        '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
        ':asset_id' => $asset_id,
        '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
        ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
      )
    );
  }

  /**
   * Remove the permanent symlink to a still.
   *
   * @param string $still_id
   */
  public static function remove_permanent_link($app_id, $still_id) {
    // Remove permanent stills.
    mediamosa_io::exec(
      strtr(
        'find @media_location -maxdepth 2 -mindepth 2 -name "@wildmatch*" -type l -delete',
        array(
          '@media_location' => mediamosa_configuration_storage::media_get_path($app_id),
          '@wildmatch' => $still_id,
        )
      )
    );
  }

  /**
   * Test if a permanent still link can be made.
   *
   * @param string $still_id
   *
   * @throws mediamosa_exception_error_is_unappropriate
   *         Will throw mediamosa_exception_error_is_unappropriate when your
   *         still link is not allowed.
   */
  public static function is_still_permanent($still_id, $is_app_admin = FALSE) {
    // Protected mediafiles (stills) can not be permanent.
    try {
      return !mediamosa_asset_mediafile::is_mediafile_protected($still_id, $is_app_admin);
    }
    catch (mediamosa_exception_error_is_unappropriate $e) {
      throw $e;
    }
    catch (mediamosa_exception $e) {
      return FALSE;
    }
  }
}
