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
    return 'mediamosa-transition';
  }

  /**
   * Register the file in our database so we know what is stored and how long
   * to keep it.
   *
   * Will return the transition_uri. Make sure to use this uri instead of
   * provided. It will make sure the MD5 matches the uri.
   *
   * @param string $transition_uri
   *   The URI of the transition file.
   */
  public static function register_transition_file($transition_uri) {
    // Need streamwrapper.
    $mediamosa_io_streamwrapper_transition = mediamosa_io::require_stream_wrapper_instance_by_uri($transition_uri);

    // Must be transition location.
    if (!($mediamosa_io_streamwrapper_transition instanceof mediamosa_io_streamwrapper_local_transition)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("transition uri (@uri) is not valid.", array('@uri' => $transition_uri))));
    }

    // Need md5.
    $md5 = mediamosa_io::md5_file($transition_uri);

    // Get md5 from uri.
    $md5_uri = $mediamosa_io_streamwrapper_transition->get_md5_from_uri();

    // Get mediafile_id.
    $mediafile_id = $mediamosa_io_streamwrapper_transition->get_mediafile_id();

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

      $md5 = $md5_uri;
      $transition_uri = $transition_new_uri;
    }

    // Now register or update the accessed date.
    $filesize = mediamosa_io::filesize($transition_uri);

    // Either update accessed or insert a new row.
    mediamosa_db::db_merge(mediamosa_storage_transition_db::TABLE_NAME)
      ->key(array(
        mediamosa_storage_transition_db::MEDIAFILE_ID => $mediafile_id,
        mediamosa_storage_transition_db::MD5 => $md5,
      ))
      ->fields(array(
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

//  public static function find_transition_file($mediafile_id, $md5) {
//
//    // Get transition uri.
//    $transition_uri = build_transition_uri($mediafile_id, $md5);
//
//    if (mediamosa_io::file_exists($transition_uri)) {
//      return $transition_uri;
//    }
//
//  }

  /**
   * Get and build the transition uri.
   *
   * This function will use the md5 of the mediafile (either physical md5 or
   * stored md5) to build the transition uri. However, it does not garantee that
   * the file exists.
   *
   * @param string $source_uri
   *   The transition uri expected for mediafile.
   */
  public static function get_transition_uri($source_uri) {
    // Build wrapper.
    $mediamosa_io_streamwrapper_source = mediamosa_io::require_stream_wrapper_instance_by_uri($source_uri);

    // Get the mediafile ID.
    $mediafile_id = $mediamosa_io_streamwrapper_source->get_mediafile_id();

    // Get the md5 file value.
    $source_md5 = $mediamosa_io_streamwrapper_source->getMD5();

    // Build it.
    return $this->build_transition_uri($mediafile_id, $source_md5);
  }


}
