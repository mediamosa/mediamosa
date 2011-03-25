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
  * Stills are now mediafiles, so code for still have been moved
  * under mediafile.
  */

class mediamosa_asset_mediafile_still {

  // ------------------------------------------------------------------ Static Function.
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
  static public function create($asset_id, $still_id, $app_id, $owner_id, $group_id, $order, $still_default = TRUE, $still_parameters = NULL, $sec = 0, $mediafile_id_source = '', $tag = '') {

    // Get path to still.
    $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);

    // Check if still exists.
    if (!mediamosa_io::file_exists($still_filename)) {
      mediamosa_job::post_create_job_failed($app_id, $owner_id, $still_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, 'File not exists.', $still_parameters);

      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    $size = mediamosa_io::getimagesize($still_filename);
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
  static public function set_still_order($app_id, $user_id, $asset_id, $mediafile_id, $still_id, $order) {
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
    mediamosa_aut::owner_check(
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
  static public function find_asset_all_still($asset_id) {
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
  static public function find_default($asset_id) {

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
  static public function check($still_id) {
    // controleer of de still op het filesystem staat
    $filename = mediamosa_configuration_storage::mediafile_still_filename_get($still_id);
    if (!file_exists($filename)) {
      throw new mediamosa_exception_error_file_not_found(array('@filename' => $filename));
    }
  }

  /**
   * Delete a still from database and file.
   * 1.x _media_management_delete_still
   *
   * @param $asset_id
   *  The asset_id. (will remove all stills from this asset_id).
   * @param $mediafile_id
   *  Remove specific from Mediafile parent.
   * @param $still_id
   *  Remove specific still.
   */
  static public function delete($asset_id, $mediafile_id = '', $still_id = '') {
    // Get the mediafile stills we need to remove.
    if ($still_id != '') {
      // Get specific still.
      $result = mediamosa_db::db_query(
        'SELECT #mediafile_id AS still_id, #app_id, #sannas_mount_point, #mediafile_id_source FROM {#mediamosa_asset_mediafile} WHERE #mediafile_id = :mediafile_id AND #is_still = :is_still_true AND #asset_id = :asset_id',
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
      $result = mediamosa_db::db_query(
        'SELECT s.#mediafile_id AS still_id, s.#app_id, s.#sannas_mount_point, s.#mediafile_id_source FROM {#mediamosa_asset_mediafile} AS s
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
      $result = mediamosa_db::db_query(
        'SELECT #mediafile_id AS still_id, #app_id, #sannas_mount_point, #mediafile_id_source FROM {#mediamosa_asset_mediafile} WHERE #asset_id = :asset_id AND #is_still = :is_still_true',
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
    $a_mediafile_ids = array();
    foreach ($result as $a_row) {
      // Build the path and filename of the still.
      $still_filename = mediamosa_configuration_storage::mediafile_still_filename_get($a_row['still_id'], $a_row[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]);
      if (isset($mediafile_id) && $mediafile_id) {
        mediamosa_watchdog::log_mediafile($mediafile_id, 'Deleting still: @location', array('@location' => $still_filename), $asset_id);
      }

      // Remove the file.
      mediamosa_io::unlink($still_filename);

      // Collect the mediafile id of the still.
      $a_mediafile_ids[] = $a_row['still_id'];

      if ($mediafile_id == '') {
        $mediafile_id = $a_row[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];
      }
    }

    // Remove the stills from the database.
    if (!empty($a_mediafile_ids)) {
      // Remove our metadatas.
      $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME);
      $query->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $a_mediafile_ids, 'IN');
      $query->execute();

      // Remove the mediafiles.
      $query = mediamosa_db::db_delete(mediamosa_asset_mediafile_db::TABLE_NAME);
      $query->condition(mediamosa_asset_mediafile_db::ID, $a_mediafile_ids, 'IN');
      $query->execute();
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
  static public function set_still_default($app_id, $user_id, $asset_id, $mediafile_id, $still_id) {
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
    mediamosa_aut::owner_check(
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
  static public function remove_permanent_link($still_id) {
    // Remove the permanent still link.
    $file = mediamosa_configuration_storage::ticket_still_location_get() . mediamosa_configuration_storage::objectid_to_location($still_id);
    mediamosa_io::unlink($file);
  }

  /**
   * Test if a permanent still link can be made.
   *
   * @param string $still_id
   *
   * @throws Will throw mediamosa_exception_error_is_unappropriate when you
   *         still link is not allowed at all.
   */
  static public function is_still_permanent($still_id, $is_app_admin = FALSE) {
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
