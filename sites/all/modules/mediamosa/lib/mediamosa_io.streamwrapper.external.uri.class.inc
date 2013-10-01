<?php
/**
 * @file
 * External streamwrapper for uri based mediafiles.
 *
 * Uri:
 * {scheme}://{storage_profile_id}/[path][/-/mediafile_path].
 *
 * Difference with local wrapper:
 *   storage_profile_id in uri is required.
 *   The mediafile path contains 3 (optional) parts.
 *   If mediafile path contains filename it must have a extension.
 *     [[[mediafile_id{0}]/[mediafile_id]]/[filename.ext]]
 *
 * Make sure you implement unlink with the following code;
 *
 *   // Make sure md5file is recalculated now this file is removed.
 *   $md5s = &drupal_static(self::STATIC_MD5S, array());
 *   unset($md5s[$uri]);
 */

class mediamosa_io_streamwrapper_external_uri extends mediamosa_io_streamwrapper_external {
  // --------------------------------------------------------- Abstract methods.
  /**
   * Build the storage object that belongs to this streamwrapper.
   *
   * @param array $storage_profile
   *   Storage profile.
   *
   * @return mediamosa_storage_uri
   *   The mediamosa storage URI object.
   */
  public function create_storage_object(array $storage_profile) {
    return new mediamosa_storage_uri($storage_profile);
  }

  /**
   * Get the file on an local location and return the location.
   *
   * @throws mediamosa_exception_error_unexpected_error
   */
  public function get_uri_local_file() {
    throw new mediamosa_exception_error_unexpected_error('An external URI based mediafile does not have an local file.');
  }

  /**
   * Copy file to any location.
   *
   * Function must make sure that on return $this->uri is updated with new
   * location and $this->md5 is set on the destination (self).
   *
   * @param mediamosa_io_streamwrapper $mediamosa_io_streamwrapper_source
   *   The local stream wrapper.
   *
   * @throws mediamosa_exception_error_unexpected_error
   */
  public function mediafile_copy($mediamosa_io_streamwrapper_source) {
    throw new mediamosa_exception_error_unexpected_error('Can not copy external URI based mediafile.');
  }

  /**
   * Get MD5 from s3 file.
   *
   * @return string
   *   The MD5 of the file or FALSE otherwise.
   */
  public function md5_file() {
    // We don't know the md5 of an external file.
    return self::MD5_DEFAULT;
  }
  // ------------------------------------------------------ Abstract IO Methods.
  /**
   * These IO methods reflect file / IO functions that is not supported by the
   * PHP IO layer. However, mediamosa_io will support these.
   */

  /**
   * Check if the the file exists.
   *
   * @return boolean
   *   Returns TRUE when file exists.
   */
  public function file_exists() {
    // Implement into your class file_exists().
    return TRUE;
  }

  /**
   * Return the file size.
   *
   * @return integer
   *   Return the filesize or FALSE when file does not exists.
   */
  public function filesize() {
    // Implement into your class file_exists().
    return $this->file_exists() ? TRUE : FALSE;
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Process media view request.
   *
   * @param integer $app_id
   *   Application ID.
   * @param string $user_id
   *   The owner for possible ticket.
   * @param array $mediafile
   *   The mediafile to process.
   * @param string $response_type
   *   See mediamosa_media::RESPONSE_*
   * @param boolean $is_app_admin
   *   Is app admin.
   *
   * @return array
   *   An associative array; (see mediamosa_io_streamwrapper::MEDIA_VIEW_*)
   *   - 'ticket_id'
   *     The ticket ID used.
   *   - 'link'
   *     The symbolic link complete path.
   *   - 'filename'
   *     The filename of the media.
   *   - 'server_uri_build'
   *     The finished server uri.
   */
   public function media_view($app_id, $user_id, array $mediafile, $response_type, $is_app_admin) {
    // Get the stuff we need.
    $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::ID];
    $uri = $mediafile[mediamosa_asset_mediafile_db::URI];

    // Get the streaming server URI.
    $server_uri = mediamosa_media::get_server_uri($mediafile_id, $uri, $response_type);

    // Get the permanent status, public mediafiles can be public.
    $is_public = mediamosa_media::is_public($mediafile, $is_app_admin);

    // Return the information needed.
    return array(
      mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID => '',
      mediamosa_io_streamwrapper::MEDIA_VIEW_FILENAME => '',
      mediamosa_io_streamwrapper::MEDIA_VIEW_LINK => '',
      mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD => $server_uri,
      mediamosa_io_streamwrapper::MEDIA_VIEW_IS_PUBLIC => $is_public,
    );
  }

  /**
   * External URI's have no unlink by default.
   *
   * @param string $uri
   *   A string containing the URI to the resource to delete.
   *
   * @return bool
   *   TRUE if resource was successfully deleted.
   *
   * @see http://php.net/manual/en/streamwrapper.unlink.php
   */
  public function unlink($uri) {
    return TRUE;
  }
}
