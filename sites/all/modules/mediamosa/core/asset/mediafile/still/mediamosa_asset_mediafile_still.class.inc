<?php
/**
 * @file
 * Stills are now mediafiles, so code for still have been moved
 * under mediafile.
 */

class mediamosa_asset_mediafile_still {
  // ------------------------------------------------------------------- Consts.
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

  // ---------------------------------------------------------------- Functions.

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
   * The still file and db entry must exist.
   *
   * @param string $still_id
   *   The mediafile ID of the still to check.
   * @param string $app_id
   *   (optional) The app ID of the still.
   *
   * @param array
   *   The still (mediafile) array.
   *
   * @throws mediamosa_exception_error
   */
  public static function still_must_exist($still_id, $app_id = NULL) {
    // Get the still mediafile.
    $still = mediamosa_asset_mediafile::must_exists($still_id, $app_id);

    // Get path to still.
    $still_uri = mediamosa_storage::get_uri_mediafile($still);

    if (!mediamosa_io::file_exists($still_uri)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_FILE_NOT_FOUND, array('@mediafile_id' => $still_id, '@path' => $still_uri));
    }

    return $still;
  }

  /**
   * Get the default image size setting.
   *
   * Provide application ID when possible, client applications can override
   * default.
   *
   * @param int $app_id
   *   (optional) The client application ID.
   *
   * @return string
   *   The default image size in WxH format.
   */
  public static function get_default_image_size($app_id) {
    $default_image_size = mediamosa_app::get_still_default_size($app_id);

    // Either return the application settings or the default.
    return $default_image_size ? $default_image_size : mediamosa_settings::STILL_DEFAULT_SIZE;
  }

  /**
   * Returns the best width and height, based on default and input.
   *
   * @param string $size
   *   (optional) In WxH format.
   * @param int $width
   *   (optional) The width.
   * @param int $height
   *   (optional) The height.
   * @param string $default_size
   *   In WxH format.
   *
   * @return array
   *   returns array(width, height), use list($width, $height).
   */
  public static function process_image_size($size = '', $width = 0 , $height = 0, $default_size = mediamosa_settings::STILL_DEFAULT_SIZE) {
    // If width and height is not set, try to use size.
    if (empty($width) && empty($height)) {
      if (empty($size)) {
        $size = $default_size;
      }

      $matches = array();
      preg_match('/^([0-9]+)x([0-9]+)$/', $size, $matches);
      if (!empty($matches)) {
        $size_width = $matches[1];
        $size_height = $matches[2];

        if ($size_width >= 0 && $size_height >= 0) {
          $width = $size_width;
          $height = $size_height;
        }
      }
    }

    return array($width, $height);
  }

  /**
   * Process and return the order and selection range.
   *
   * @param string $range
   *   The range value, like 1,4-5 or 3-5
   *
   * @return array
   *   The order array, generated from $range.
   */
  public static function process_still_range($range) {
    $order = array();

    // Any value?
    if (empty($range)) {
      // Return NULL here so order is ignored later.
      return NULL;
    }

    // Single number?
    if (is_numeric($range)) {
      return array($range);
    }

    foreach (explode(',', $range) as $range) {
      if (is_numeric($range)) {
        $order[] = $range;
      }
      else {
        list($low, $high) = explode('-', $range, 2);
        if (isset($low) && isset($high)) {
          list($low, $high) = array(min(abs($low), abs($high)), max(abs($low), abs($high)));
          for ($i = $low; $i <= $high; $i++) {
            $order[] = $i;
          }
        }
      }
    }

    return empty($order) ? NULL : $order;
  }

  /**
   * Create watermark.
   *
   * @param string $still_id
   *   The still ID to create watermark on.
   * @param int $app_id
   *   The application ID.
   * @param array $still_parameters
   *   The still parameters.
   * @param bool $create_new_still
   *   The create new still file, instead of overwriting on current.
   *
   * @return string
   *   The ID of the still with the watermark.
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
    try {
      $watermark = self::still_must_exist($still_parameters[self::WATERMARK_ID], $app_id);
    }
    catch (mediamosa_exception $e) {
      self::log('Watermark MediaFile (ID: @id) was not found in database, exception message; @message.', array('@id' => $still_parameters[self::WATERMARK_ID], '@message' => $e->getMessage()));
      return;
    }

    // Get the still mediafile.
    $still = self::still_must_exist($still_id);

    // Get path to still.
    $still_uri = mediamosa_storage::get_uri_mediafile($still);

    // Get path to watermark.
    $watermark_uri = mediamosa_storage::get_uri_mediafile($watermark);

    // Create still surface.
    $image_result = mediamosa_gd::imagecreate($still_uri);
    if (!is_resource($image_result)) {
      self::log_mediafile($still_id, 'Watermark not created, unable to create PNG or JPG surface from still image (@filename).', array('@filename' => mediamosa_io::realpath_safe($still_uri)));
      return;
    }

    $image_watermark = mediamosa_gd::imagecreate($watermark_uri);
    if (!is_resource($image_result)) {
      self::log_mediafile($still_parameters[self::WATERMARK_ID], 'Watermark not created, unable to create PNG or JPG surface from watermark image (@filename).', array('@filename' => mediamosa_io::realpath_safe($watermark_uri)));
      return;
    }

    // Values.
    $dst_x = 0;
    $dst_y = 0;
    $image_watermark_x = 0;
    $image_watermark_y = 0;
    $image_watermark_w = mediamosa_gd::imagesx($image_watermark);
    $image_watermark_h = mediamosa_gd::imagesy($image_watermark);
    $pct = $still_parameters[self::WATERMARK_PCT];

    // Destination x calculation.
    if ($still_parameters[self::WATERMARK_V_ALIGN] == self::WATERMARK_VALUE_LEFT) {
      $dst_x = $still_parameters[self::WATERMARK_DST_X];
    }
    else {
      $dst_x = mediamosa_gd::imagesx($image_result) - $image_watermark_w - $still_parameters[self::WATERMARK_DST_X];
    }

    // Destination y calculation.
    if ($still_parameters[self::WATERMARK_H_ALIGN] == self::WATERMARK_VALUE_TOP) {
      $dst_y = $still_parameters[self::WATERMARK_DST_Y];
    }
    else {
      $dst_y = mediamosa_gd::imagesy($image_result) - $image_watermark_h - $still_parameters[self::WATERMARK_DST_Y];
    }

    // Copymerge.
    self::log_debug_high(t('Calling imagecopymerge. (dest = @dest, src = @src, dst_x = @dst_x, dst_y = @dst_y, src_x = @src_x, src_y = @src_y, src_w = @src_w, src_h = @src_h, pct = @pct)', array(
      '@dest' => print_r($image_result, TRUE),
      '@src' => print_r($image_watermark, TRUE),
      '@dst_x' => print_r($dst_x, TRUE),
      '@dst_y' => print_r($dst_y, TRUE),
      '@src_x' => print_r($image_watermark_x, TRUE),
      '@src_y' => print_r($image_watermark_y, TRUE),
      '@src_w' => print_r($image_watermark_w, TRUE),
      '@src_h' => print_r($image_watermark_h, TRUE),
      '@pct' => print_r($pct, TRUE),
    )));

    mediamosa_gd::imagecopymerge($image_result, $image_watermark, $dst_x, $dst_y, $image_watermark_x, $image_watermark_y, $image_watermark_w, $image_watermark_h, $pct);

    // Save image.
    if ($create_new_still) {
      // New still.
      $still_id_new = mediamosa_db::uuid($app_id);
      $still_new_uri = mediamosa_storage::create_local_mediafile_uri($app_id, $still_id_new, TRUE);

      // Create possible missing directories.
      mediamosa_io::mkdir(mediamosa_io::dirname($still_new_uri));
    }

    // Create new image file.
    assert(mediamosa_gd::image($image_result, mediamosa_gd::get_image_type($still_uri), ($create_new_still ? $still_new_uri : $still_uri), self::SAVE_QUALITY));

    // New file?
    if (!mediamosa_io::file_exists($create_new_still ? $still_new_uri : $still_uri)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_FILE_NOT_FOUND, array('@mediafile_id' => $create_new_still ? $still_id_new : $still_id, '@path' => $create_new_still ? $still_new_uri : $still_uri));
    }

    // Destroy resources.
    mediamosa_gd::imagedestroy($image_result);
    mediamosa_gd::imagedestroy($image_watermark);

    // Return the new still ID or TRUE.
    return $create_new_still ? $still_id_new : $still_id;
  }

  /**
   * Create a still
   *
   * @param string $asset_id
   * @param string $still_id
   * @param int $app_id
   * @param string $owner_id
   * @param string $group_id
   * @param int $order
   * @param bool $still_default
   * @param array $still_parameters
   * @param int $sec
   * @param string $mediafile_id_source
   * @param string $tag
   */
  public static function create($asset_id, $still_id, $app_id, $owner_id, $group_id, $order, $still_default = TRUE, $still_parameters = NULL, $sec = 0, $mediafile_id_source = '', $tag = '') {

    // Get path to still.
    $still_new_uri = mediamosa_storage::create_local_mediafile_uri($app_id, $still_id, TRUE);

    // Check if still exists.
    if (!mediamosa_io::file_exists($still_new_uri)) {
      mediamosa_job::post_create_job_failed($app_id, $owner_id, $still_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, 'File does not exists.', $still_parameters);

      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    // Handle watermark.
    self::create_watermark($still_id, $app_id, $still_parameters);

    $size = mediamosa_gd::get_image_size($still_new_uri);
    if (!$size) {
      mediamosa_job::post_create_job_failed($app_id, $owner_id, $still_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, 'File is not an image.', $still_parameters);

      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_IMAGE, array('@asset_id' => $asset_id));
    }

    // Set width, height.
    $width = $size[0];
    $height = $size[1];

    // Mime type.
    $mime_type = $size['mime'];

    // Get file extension based on the mime type of the image.
    $file_extension = mediamosa_mimetype::mimetype2extension($mime_type);

    // Get file size of still.
    $file_size = mediamosa_io::filesize($still_new_uri);

    // Get parent filename.
    $filename = mediamosa_io::get_base_filename(mediamosa_asset_mediafile::get_parent_filename($still_id));
    $filename = $file_extension == '' ? $filename : ($filename . '.' . $file_extension);

    // Get MD5.
    $md5 = mediamosa_io::md5_file($still_new_uri);

    if ($still_default) {
      assert($mediafile_id_source != '');
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
        mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_storage::create_local_mount_point_uri($app_id, TRUE),
        mediamosa_asset_mediafile_db::FILENAME => $filename,
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
      array(
        'value' => $file_size,
        'prop_name' => mediamosa_asset_mediafile_metadata::FILESIZE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      ),
      array(
        'value' => $md5,
        'prop_name' => mediamosa_asset_mediafile_metadata::MD5,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
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

    //  Relocate still if final storage location is not local.
    mediamosa_storage::relocate_mediafile($app_id, $still_id, TRUE);
  }

  /**
   * Change the order of an still
   *
   * @param string $asset_id
   * @param string $mediafile_id
   * @param string $still_id
   * @param int $order
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
   * Find default still or at least the first one you find.
   *
   * @param string $asset_id
   *   The asset ID to get default still from.
   *
   * @return array
   *   The found mediafile still or FALSE.
   */
  public static function find_default($asset_id) {

    // Try default 1st.
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::STILL_DEFAULT);

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfmd', 'mfmd.mediafile_id = mf.mediafile_id');
    $still = $query
      ->fields('mf')
      ->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE)
      ->condition('mfmd.' . mediamosa_asset_mediafile_metadata_db::PROP_ID, $prop_id)
      ->condition('mfmd.' . mediamosa_asset_mediafile_metadata_db::VAL_CHAR, mediamosa_asset_mediafile_metadata::STILL_DEFAULT_TRUE)
      ->orderBy('mfmd.' . mediamosa_asset_mediafile_metadata_db::ID, 'DESC')
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();

    // Not found? Try any.
    if (!$still) {
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfmd', 'mfmd.mediafile_id = mf.mediafile_id');
      $still = $query
        ->fields('mf')
        ->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
        ->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE)
        ->orderBy('mfmd.' . mediamosa_asset_mediafile_metadata_db::ID, 'DESC')
        ->range(0, 1)
        ->execute()
        ->fetchAssoc();
    }

    // Check if the still id is in the database
    if (!$still || !mediamosa_storage::file_must_exist($still, FALSE, FALSE)) {
      // There isn't any still
      return FALSE;
    }

    // Return our mediafile still array.
    return $still;
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
    // Validate the style.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // The original still.
    $still_uri = mediamosa_storage::get_uri_mediafile($mediafile);
    if (!mediamosa_io::file_exists($still_uri)) {
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id));
    }

    // The [mount_point]/data/still version does not have a extension (like the
    // original).
    $filename = mediamosa_media::filename_build($mediafile_id, '', $style_id);

    // The destination still.
    $destination_still_uri = mediamosa_storage::create_local_mediafile_uri($mediafile[mediamosa_asset_mediafile_db::APP_ID], $filename, TRUE);

    // Create the derivative if not already.
    if (!mediamosa_io::file_exists($destination_still_uri)) {
      // Load the style.
      $style_array = image_style_load($style_id);
      if (!empty($style_array)) {
        // Create the resized image.
        if (!image_style_create_derivative($style_array, mediamosa_io::realpath($still_uri), $destination_still_uri)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CREATING_DERIVATIVE, array('@style' => $style_id, '@path' => mediamosa_io::realpath_safe($still_uri)));
        }
      }
    }

    return mediamosa_io::realpath($destination_still_uri);
  }

  /**
   * Delete a still from database and file.
   *
   * @param string $asset_id
   *  The asset_id. (will remove all stills from this asset_id).
   * @param string $mediafile_id
   *  Remove specific from Mediafile parent.
   * @param string $still_id
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
      $still_uri = mediamosa_storage::get_uri_mediafile($mediafile);
      if (isset($mediafile_id) && $mediafile_id) {
        mediamosa_watchdog::log_mediafile($mediafile_id, 'Deleting still: @filename', array('@filename' => $still_uri), $asset_id);
      }

      // Remove data from SAN/NAS.

      // Ticket part.
      $query = mediamosa_db::db_select(mediamosa_media_ticket_db::TABLE_NAME, 't');
      $query
        ->condition('t.' . mediamosa_media_ticket_db::MEDIAFILE_ID, $mediafile['mediafile_id'])
        ->condition('t.' . mediamosa_media_ticket_db::TICKET_TYPE, mediamosa_media_ticket_db::TICKET_TYPE_STILL)
        ->fields('t', array(mediamosa_media_ticket_db::ID,));
      $result = $query->execute();
      foreach ($result as $record) {
        // Ticket.
        $ticket_id = $record[mediamosa_media_ticket_db::ID];

        // Remove all tickets symlinks.
        mediamosa_io::exec(
          strtr(
            'find @media_ticket_location -maxdepth 4 -mindepth 4 -name "@wildmatch*" -type l -delete',
            array(
              '@media_ticket_location' => mediamosa_storage::trim_uri(mediamosa_storage::get_realpath_media_ticket()) . 'still',
              '@wildmatch' => $ticket_id,
            )
          )
        );

        mediamosa_io::clearstatcache();
      }

      // Remove public (permenant) link.
      mediamosa_media::remove_public_link($mediafile['app_id'], $mediafile['mediafile_id']);

      // Remove stills.
      mediamosa_io::unlink($still_uri);

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
      mediamosa_db::db_delete(mediamosa_media_ticket_db::TABLE_NAME)
        ->condition(mediamosa_media_ticket_db::MEDIAFILE_ID, $mediafile_ids, 'IN')
        ->condition(mediamosa_media_ticket_db::TICKET_TYPE, mediamosa_media_ticket_db::TICKET_TYPE_STILL)
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
   * @todo: needs work.
   */
  public static function delete_files($app_id) {
  }

  /**
   * Set the default still.
   *
   * FIXME: the mediafile id unused.
   *
   * @param int $app_id
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
}
