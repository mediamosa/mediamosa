<?php
/**
 * @file
 * Transition storage class module.
 *
 * The transition storage is where files are stored temporarily.
 *
 * Note:
 * Some abstract functions can not be used and will throw exceptions when tried.
 */

class mediamosa_storage_transition/* extends mediamosa_storage*/ {
  // ------------------------------------------------------------------- Consts.
  // -------------------------------------------------------------- Constructor.
  /**
   * Overrule constructor, as we are not really an storage object.
   *
   * @param object|integer $storage_profile
   *   Storage profile node or nid.
   *
   * @param $uri
   *   Optional uri to the storage object
   */
  public function __construct(array $storage_profile) {
    $this->storage_profile = FALSE;
    $this->storage_profile_apps = FALSE;
    $this->storage_profile_apps_stills = FALSE;
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Return with uri of the profile mount point.
   *
   * @param integer $storage_profile_id
   *   (ignored) The storage profile ID.
   * @param string $path
   *   (ignored) The storage path.
   * @param boolean $is_still
   *   Is mount point for still?
   *
   * @return string
   *   The uri of the storage profile.
   */
  public function build_mount_point_uri($storage_profile_id, $path, $is_still = FALSE) {
    return self::get_scheme($is_still) . '://';
  }

  /**
   * Return the storage options available for the storage type
   *
   * @return
   *   Array of available storage options
   */
  public function get_storage_options() {
    throw new mediamosa_exception_program_error('The usage of ' . __CLASS__ . '::' . __FUNCTION__ . '() is not supported in this class.');
  }

  public function get_upload_parameters($app_id, $user_id, $group_id) {
    throw new mediamosa_exception_program_error('The usage of ' . __CLASS__ . '::' . __FUNCTION__ . '() is not supported in this class.');
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the default scheme.
   *
   * @param boolean $is_still
   *   (ignored) Is mount point for still?
   *
   * @return string
   *   The uri of the moint point to store.
   */
  public static function get_scheme($is_still = FALSE) {
    return 'mediamosa-storage-transition';
  }

  /**
   * Register the file in our database so we know what is stored and how long
   * to keep it.
   *
   * Only register when file is downloaded and is the latest version.
   *
   * Will return the transition_uri. Make sure to use this uri instead of
   * provided. It will make sure the MD5 matches the uri.
   *
   * @param string $transition_uri
   *   The URI of the transition file.
   * @param boolean $trust_md5
   *   Trust the file as valid. In some cases we have to assume the file is ok.
   *
   * @return string
   *   The correct transition file.
   */
  public static function register_transition_file($transition_uri, $trust_md5 = FALSE) {
    // Some cache.
    $registered = &drupal_static('mediamosa_transition_registered', array());

    // Do not go through this process again if it was registered.
    if (isset($registered[$transition_uri])) {
      return $registered[$transition_uri];
    }

    // Need streamwrapper.
    $mediamosa_io_streamwrapper_transition = mediamosa_io::require_stream_wrapper_instance_by_uri($transition_uri);

    // Must be transition location.
    if (!($mediamosa_io_streamwrapper_transition instanceof mediamosa_io_streamwrapper_local_transition)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("transition uri (@uri) is not valid.", array('@uri' => $transition_uri))));
    }

    // Must exist.
    if (!mediamosa_io::file_exists($transition_uri)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("file (@uri) is not found in transition.", array('@uri' => $transition_uri))));
    }

    // Get md5 from uri.
    $md5_uri = $mediamosa_io_streamwrapper_transition->get_md5_from_uri();

    // Get mediafile_id.
    $mediafile_id = $mediamosa_io_streamwrapper_transition->get_mediafile_id();

    // Trust it unless its default MD5.
    if (!$trust_md5 || $md5_uri = mediamosa_io_streamwrapper::MD5_DEFAULT) {
      // Need md5.
      $md5 = mediamosa_io::md5_file($transition_uri);

      // Can not accept files on the wrong location. This could happen when file
      // has been assigned default md5 (at download was md5 unknown).
      if ($md5 != $md5_uri) {
        // Correct location.
        $transition_new_uri = self::build_transition_uri($mediafile_id, $md5_uri);

        // Does not already exists?
        if (!mediamosa_io::file_exists($transition_new_uri)) {
          // Move file.
          if (!mediamosa_io::rename($transition_uri, $transition_new_uri)) {
            throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("Failed to copy file from @source to @destination.", array('@source' => mediamosa_io::realpath_safe($transition_uri), '@destination' => mediamosa_io::realpath_safe($transition_new_uri)))));
          }
        }

        // Remove transition uri file in case its in the database. Its either
        // moved or was already present on it correct md5 location.
        mediamosa_io::unregister_transition_file($transition_uri);

        // Store in static cache.
        $registered[$transition_uri] = $transition_new_uri;

        $md5 = $md5_uri;
        $transition_uri = $transition_new_uri;
      }
    }
    else {
      // Store in static cache.
      $registered[$transition_uri] = $transition_uri;
    }

    // Create symlink from data dir to transition location.
    $mediamosa_io_streamwrapper_transition->create_transition_symlink_data();

    // Now register or update the accessed date.
    $filesize = mediamosa_io::filesize($transition_uri);

    // Either update accessed or insert a new row.
    mediamosa_db::db_merge(mediamosa_storage_transition_db::TABLE_NAME)
      ->key(array(
        mediamosa_storage_transition_db::MEDIAFILE_ID => $mediafile_id,
        mediamosa_storage_transition_db::MD5 => $md5,
      ))
      ->fields(array(
        mediamosa_storage_transition_db::ACCESSED => mediamosa_datetime::db_current_timestamp_now(),
        mediamosa_storage_transition_db::CREATED => mediamosa_datetime::db_current_timestamp_now(),
        mediamosa_storage_transition_db::FILESIZE => $filesize,
      ))
      ->updateFields(array(
        mediamosa_storage_transition_db::ACCESSED => mediamosa_datetime::db_current_timestamp_now(),
      ))
      ->execute();

    return $transition_uri;
  }

  /**
   * Try to register access to the transition file.
   *
   * Will throw exception when transition file is not found or can not be
   * trusted.
   *
   * @param string $transition_uri
   *   The URI of the transition file.
   *
   * @throws mediamosa_exception_error
   */
  public static function register_transition_file_access($transition_uri) {
    // Need streamwrapper.
    $mediamosa_io_streamwrapper_transition = mediamosa_io::require_stream_wrapper_instance_by_uri($transition_uri);

    // Get md5 from uri.
    $md5_uri = $mediamosa_io_streamwrapper_transition->get_md5_from_uri();

    // Can not register the default MD5 version.
    if ($md5_uri == mediamosa_io_streamwrapper::MD5_DEFAULT) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_EXTERNAL_FILE_NOT_FOUND_LOCAL, array(array('@uri' => $transition_uri)));
    }

    // Get mediafile_id.
    $mediafile_id = $mediamosa_io_streamwrapper_transition->get_mediafile_id();

    // We must already been registered (then we are verified), if we are not
    // then we the file can not be trusted to be valid.
    $verified = mediamosa_db::db_count_rows(
      mediamosa_storage_transition_db::TABLE_NAME,
      array(
        mediamosa_storage_transition_db::MEDIAFILE_ID => $mediafile_id,
        mediamosa_storage_transition_db::MD5 => $md5_uri,
      )
    );

    // Not in our table, then we can not trust the file.
    if (!$verified) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_EXTERNAL_FILE_NOT_FOUND_LOCAL, array('@uri' => $transition_uri));
    }

    // Either update accessed or insert a new row.
    mediamosa_db::db_update(mediamosa_storage_transition_db::TABLE_NAME)
      ->fields(array(
        mediamosa_storage_transition_db::ACCESSED => mediamosa_datetime::db_current_timestamp_now(),
      ))
      ->condition(mediamosa_storage_transition_db::MEDIAFILE_ID, $mediafile_id)
      ->condition(mediamosa_storage_transition_db::MD5, $md5_uri)
      ->execute();
  }

  /**
   * Delete entry from database.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $md5
   *   The MD5.
   */
  public static function unregister_transition_file($transition_uri) {
    // Need streamwrapper.
    $mediamosa_io_streamwrapper_transition = mediamosa_io::require_stream_wrapper_instance_by_uri($transition_uri);

    // Must be transition location.
    if (!($mediamosa_io_streamwrapper_transition instanceof mediamosa_io_streamwrapper_local_transition)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("transition uri (@uri) is not valid.", array('@uri' => $transition_uri))));
    }

    // Need data from uri.
    $mediafile_id = $mediamosa_io_streamwrapper_transition->get_mediafile_id();
    $md5 = $mediamosa_io_streamwrapper_transition->get_md5_from_uri();

    // Remove from DB.
    mediamosa_db::db_delete(mediamosa_storage_transition_db::TABLE_NAME)
      ->condition(mediamosa_storage_transition_db::MEDIAFILE_ID, $mediafile_id)
      ->condition(mediamosa_storage_transition_db::MD5, $md5)
      ->execute();

    // Remove file.
    mediamosa_io::unlink($transition_uri);
  }

  /**
   * Build the transition uri.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $md5
   *   The mediafile MD5.
   *
   * @return string
   *   The transition uri.
   */
  public static function build_transition_uri($mediafile_id, $md5 = NULL) {
    // Need md5.
    if (empty($md5)) {
      // Get the md5.
      $md5 = mediamosa_asset_mediafile::get_md5($mediafile_id);

      // If for some reason no md5 is present, default md5 will do.
      if (empty($md5)) {
        $md5 = mediamosa_io_streamwrapper::MD5_DEFAULT;
      }
    }

    return self::get_scheme() . '://' . mediamosa_io_streamwrapper::create_uri_mediafile_id_path($mediafile_id) . '/' . $md5;
  }

  /**
   * Check if transition file md5 matches the md5 directory its in.
   *
   * @param string $transition_uri
   *   The file to check.
   * @param boolean $allow_md5_default
   *   In some cases (when downloading), the MD5 is unknown. In that case we
   *   will ignore the MD5 if its default MD5 value.
   *
   * @return integer
   *   Returns;
   *   0 - failed.
   *   1 - md5 matched file.
   *   2 - md5 in uri matched default md5.
   *
   * @throws mediamosa_exception_error
   */
  public static function verify_transition_file($transition_uri, $allow_md5_default = FALSE) {
    // Need streamwrapper.
    $mediamosa_io_streamwrapper_transition = mediamosa_io::require_stream_wrapper_instance_by_uri($transition_uri);

    // Must be transition location.
    if (!($mediamosa_io_streamwrapper_transition instanceof mediamosa_io_streamwrapper_local_transition)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("transition uri (@uri) is not valid.", array('@uri' => $transition_uri))));
    }

    // Get md5 from uri.
    $transition_md5_from_uri = $mediamosa_io_streamwrapper_transition->get_md5_from_uri();

    // Allow invalid md5 default value for now.
    if (($allow_md5_default && $transition_md5_from_uri == mediamosa_io_streamwrapper::MD5_DEFAULT)) {
      return 2;
    }

    // Need md5.
    $transition_md5 = mediamosa_io::md5_file($transition_uri);

    // Must match.
    return $transition_md5 == $transition_md5_from_uri ? 1 : 0;
  }

  /**
   * Find possible transition file.
   *
   * @param mediamosa_io_streamwrapper_external $mediamosa_io_streamwrapper_external
   *   The external streamwrapper to translate to transition.
   *
   * @return string|FALSE
   *   Either valid transition file or FALSE.
   */
  public static function find_transition_file($mediamosa_io_streamwrapper_external) {
    // Must be transition location.
    if (!($mediamosa_io_streamwrapper_external instanceof mediamosa_io_streamwrapper_external)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t('streamwrapper object must be external')));
    }

    // Get mediafile ID, if present in uri.
    $mediafile_id = $mediamosa_io_streamwrapper_external->get_mediafile_id();
    if (empty($mediafile_id)) {
      return FALSE;
    }

    // Get MD5.
    $md5 = $mediamosa_io_streamwrapper_external->getMD5();

    // Do not accept default MD5. That is only used during downloading.
    if (empty($md5) || $md5 == mediamosa_io_streamwrapper::MD5_DEFAULT) {
      return FALSE;
    }

    // Rebuild uri.
    $transition_uri = self::build_transition_uri($mediafile_id, $md5);

    // Must exist.
    if (!mediamosa_io::file_exists($transition_uri)) {
      return FALSE;
    }

    try {
      // Register so it kept for a while (its being accessed). If this call
      // fails, then we can not trust the file.
      self::register_transition_file_access($transition_uri);
    }
    catch (mediamosa_exception $e) {
      return FALSE;
    }

    return $transition_uri;
  }

  /**
   * Get and build the transition uri.
   *
   * This function will use the md5 of the mediafile (either physical md5 or
   * stored md5) to build the transition uri. However, it does not garantee that
   * the file exists.
   *
   * @param string $source_uri
   *   The transition uri expected for mediafile.
   *
   * @return string
   *   The transition location.
   */
  public static function get_transition_uri($source_uri) {
    // Build wrapper.
    $mediamosa_io_streamwrapper_source = mediamosa_io::require_stream_wrapper_instance_by_uri($source_uri);

    // Get the mediafile ID.
    $mediafile_id = $mediamosa_io_streamwrapper_source->get_mediafile_id();

    // Get the md5 file value.
    $source_md5 = $mediamosa_io_streamwrapper_source->getMD5();

    // Build it.
    return self::build_transition_uri($mediafile_id, $source_md5);
  }

  /**
   * Move mediafile from any 'local' location to transition.
   *
   * @param string $mediafile_id
   *   The ID of mediafile to move.
   *
   * @return string
   *   The transition URI.
   */
  public static function mediafile_local_move_to_transition($mediafile_id) {
    // Get current location.
    $source_uri = mediamosa_storage::get_uri_mediafile($mediafile_id);

// Find the stream wrapper.
    $mediamosa_io_streamwrapper_source = mediamosa_io::require_stream_wrapper_instance_by_uri($source_uri);
// @todo: remove from release.
    mediamosa_debug::log_export('debug in ' . __FUNCTION__ . '() on line ' . __LINE__ . ' in ' . __FILE__);
    mediamosa_debug::log_export($source_uri);
// @todo: remove from release.

    // Only local files.
    if (!$mediamosa_io_streamwrapper_source->is_local()) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("Will not move external file; @uri.", array('@uri' => $source_uri))));
    }

    // Get the md5 file value.
    $source_md5 = $mediamosa_io_streamwrapper_source->getMD5();

    // Build it.
    $transition_uri = self::build_transition_uri($mediafile_id, $source_md5);

    // Create directory.
    mediamosa_io::mkdir(mediamosa_io::dirname($transition_uri));

    // Ok move it, will replace any file already there.
    mediamosa_io::rename($source_uri, $transition_uri);

    // Move done, register the new file so transition will keep it for a
    // while. Register function will check if location of the file is correct
    // and will move file if md5 in uri does not match file. It expects that
    // the MD5 of the file is correct.
    $transition_uri = self::register_transition_file($transition_uri);
// @todo: remove from release.
    mediamosa_debug::log_export('debug in ' . __FUNCTION__ . '() on line ' . __LINE__ . ' in ' . __FILE__);
    mediamosa_debug::log_export($transition_uri);
// @todo: remove from release.

    // Need streamwrapper.
    $mediamosa_io_streamwrapper_transition = mediamosa_io::require_stream_wrapper_instance_by_uri($transition_uri);

    // Get md5.
    $transition_md5 = $mediamosa_io_streamwrapper_transition->get_md5_from_uri();

    // Ok, now set the MD5 for this mediafile or else we will never find this
    // downloaded file later.
    mediamosa_asset_mediafile_metadata::create_mediafile_metadata_char($mediafile_id, $transition_md5, mediamosa_asset_mediafile_metadata::MD5);

    // Return the transition uri.
    return $transition_uri;
  }
}
