<?php
/**
 * @file
 * Our MediaMosa storage stream wrapper class.
 *
 * Local in the streamwrapper name means that the files are on the local machine
 * and not external like S3.
 *
 * Provides support for all files and symlinks used by MediaMosa using the
 * PHP and Drupal file stream wrappers.
 *
 * Notes:
 * The uri of typical local mediamosa stream;
 *
 * Uri:
 * {scheme}://[path][/-/mediafile_path].
 */

// Current installations will break as wrapper hook will load before parent
// class is indexed for autoload.
require_once 'mediamosa_io.streamwrapper.class.inc';

class mediamosa_io_streamwrapper_local extends mediamosa_io_streamwrapper {
  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Overrides getExternalUrl().
   */
  public function getExternalUrl() {
    return '';
  }

  /**
   * Implements getDirectoryPath().
   */
  public function getDirectoryPath() {
    return rtrim(mediamosa::is_windows_platform() ? $this->get_storage_path_windows() : $this->get_storage_path_linux(), '/');
  }

  // --------------------------------------------------------- Abstract methods.
  /**
   * Build the storage object that belongs to this streamwrapper.
   *
   * @param array $storage_profile
   *   Storage profile.
   *
   * @return mediamosa_storage_local
   *   The mediamosa storage local object.
   */
  public function create_storage_object(array $storage_profile) {
    return new mediamosa_storage_local($storage_profile);
  }

  /**
   * Create the complete URI for the mediafile.
   *
   * @param string|array $mediafile
   *   The mediafile array or string ID.
   *
   * @return string
   *   The complete local uri.
   */
  public function create_storage_uri($mediafile) {
    // Get mediafile ID.
    $mediafile_id = is_string($mediafile) ? $mediafile : $mediafile[mediamosa_asset_mediafile_db::ID];

    // Return letter/mediafile for local data paths.
    return $this->get_uri_mount_point() . mediamosa_io_streamwrapper::create_mediafile_path($mediafile_id);
  }

  /**
   * Test if the current path/file is local or external (like Amazon S3).
   *
   * @return boolean
   *   Returns TRUE when local or FALSE otherwise.
   */
  public function is_local() {
    return TRUE;
  }

  /**
   * Get the mount point.
   *
   * @return string
   *   The mount point, usable to store in sannas_mount_point field.
   */
  public function get_uri_mount_point() {
    return $this->get_scheme() . '://';
  }

  /**
   * Get the file on an local location and return the location.
   *
   * Our file is local, so we return our uri.
   *
   * @return string
   *   Returns the uri to the local file.
   */
  public function get_uri_local_file() {
    return $this->uri;
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
   * @throws mediamosa_exception_error
   */
  public function mediafile_copy($mediamosa_io_streamwrapper_source) {
    // First get source file as local.
    $source_uri = $mediamosa_io_streamwrapper_source->get_uri_local_file();

    // Make sure source exists.
    if (!mediamosa_io::file_exists($source_uri)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("source file '@uri' (@path) was not found.", array('@uri' => $source_uri, '@path' => mediamosa_io::realpath_safe($source_uri)))));    }
    elseif (!mediamosa_io::is_file($source_uri)) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("source file '@uri' (@path)  is not a file.", array('@uri' => $source_uri, '@path' => mediamosa_io::realpath_safe($source_uri)))));
    }

    // We can calculate md5 of local source.
    $source_md5 = mediamosa_io::md5_file($source_uri);

    // See if destination already exists.
    if (mediamosa_io::file_exists($this->uri)) {
      if (!mediamosa_io::is_file($this->uri)) {
        throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("destination file '@uri' (@path)  already exists and is not a file.", array('@uri' => $source_uri, '@path' => mediamosa_io::realpath_safe($source_uri)))));
      }

      // Get md5 of found file.
      $destination_md5 = mediamosa_io::md5_file($this->uri);

      // Destination is already this file, so we are done.
      if ($source_md5 == $destination_md5) {
        $this->setMD5($destination_md5);
        return;
      }

      // No need to delete destination, as copy will replace it.
    }
    else {
      // Make sure directory exists.
      mediamosa_io::mkdir(mediamosa_io::dirname($this->uri));
    }

    // Check for writable on destination..
    if (!mediamosa_io::is_writable(mediamosa_io::dirname($this->uri))) {
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ACCESS_ERROR, array('@error' => 'no write access on ' . $this->uri));
    }

    // Now copy.
    if (!mediamosa_io::copy($source_uri, $this->uri)) {
      throw new mediamosa_exception_error(
        mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR,
        array('@error' => strtr(
          'copy from @source_uri (@source_path) to @destination_uri (@destination_path) failed, please check logs.',
          array(
            '@source_uri' => $source_uri,
            '@source_path' => mediamosa_io::realpath_safe($source_uri),
            '@destination_uri' => $this->uri,
            '@destination_path' => mediamosa_io::realpath_safe($this->uri),
          )
        ))
      );
    }

    // Check if the copy went ok.
    if (!mediamosa_io::file_exists($this->uri)) {
      throw new mediamosa_exception_error(
        mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => 'copied source file, but copied file on destination location was not found, please check logs.'));
    }

    // Compare the md5 again.
    $destination_md5 = mediamosa_io::md5_file($this->uri);

    // Copy went ok?
    if ($destination_md5 != $source_md5) {
      mediamosa_io::unlink($this->uri);
      throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => 'copied source file, but copied file did not match original MD5, please check logs.'));
    }

    // Done.
    $this->setMD5($destination_md5);
  }

  /**
   * Validate the uri format.
   *
   * Expects;
   * {scheme}://[{path}][/-/{mediafile_path}]
   *
   * @param string $uri
   *   The uri to validate.
   *
   * @return array
   *   An associtieve array (optional items are either set or empty);
   *   - 'scheme'
   *     The wrapper scheme.
   *   - 'storage_profile_id'
   *     The storage profile ID.
   *   - 'path'
   *     (optional) The path.
   *   - 'mediafile_path'
   *     (optional) The mediafile path.
   *
   * @throws mediamosa_exception_error
   */
  public function validate_uri($uri) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    $matches = array();
    $reg = '#^(?P<scheme>.+)://(?:(?P<path>.*)/-/(?P<mediafile_path>.*)|(?P<altpath>.+))?$#';
    if (!preg_match($reg, $uri, $matches)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_IO_ERROR, array('@error' => "invalid storage uri: '" . $uri . "'"));
    }

    // Rename altpath.
    if (isset($matches['altpath'])) {
      $matches['path'] = $matches['altpath'];
      unset($matches['altpath']);
    }

    // Return parts.
    return array(
      'scheme' => $matches['scheme'],
      'storage_profile_id' => mediamosa_storage_profile::DEFAULT_STORAGE_PROFILE_ID,
      'path' => empty($matches['path']) ? '' : trim($matches['path'], '\/'),
      'mediafile_path' => empty($matches['mediafile_path']) ? '' : trim($matches['mediafile_path'], '\/'),
    );
  }

  /**
   * Create the uri of the current mediafile.
   *
   * @param array $mediafile
   *   The mediafile array
   *
   * @return string
   *   The location of the mediafile.
   *
   */
  public function get_uri_mediafile(array $mediafile) {
    return mediamosa_storage::trim_uri($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT], '') . mediamosa_io_streamwrapper::create_mediafile_path($mediafile[mediamosa_asset_mediafile_db::ID]);
  }

  /**
   * Validate the uri format.
   *
   * Expects;
   * mediafile_id[0]/mediafile_id[.ext]
   *
   * Remember that mediafile_id is optional.
   *
   * @param string $mediafile_path
   *   The mediafile path to validate.
   *
   * @return boolean
   *   Returns TRUE for match, FALSE otherwise.
   */
  public function validate_uri_mediafile_path($mediafile_path) {
    $reg = '#^([a-zA-Z0-9](/[a-zA-Z0-9]+(/\.[a-zA-Z0-9]+)?)?)?$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
  }

  /**
   * Generate MD5 from source file.
   *
   * @return string
   *   The MD5 of the file or FALSE otherwise.
   */
  public function md5_file() {
    $md5s = &drupal_static(self::STATIC_MD5S, array());
    assert(!empty($this->uri));

    if (!empty($md5s[$this->uri]) && $md5s[$this->uri] != self::MD5_DEFAULT) {
      return $md5s[$this->uri];
    }

    // Get md5 from file or other source.
    $md5s[$this->uri] = mediamosa_io::md5_file($this->uri);

    return $md5s[$this->uri];
  }

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
   *   - 'is_public'
   *     Public urls have no ACL rules.
   */
   public function media_view($app_id, $user_id, array $mediafile, $response_type, $is_app_admin) {

    // Get the stuff we need.
    $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::ID];
    $uri = $mediafile[mediamosa_asset_mediafile_db::URI];

    // Make sure the physical file exists (local check only).
    mediamosa_storage::file_must_exist($mediafile);

    // Get the permanent status, public mediafiles can be public.
    $is_public = mediamosa_media::is_public($mediafile, $is_app_admin);

    // Get the filename.
    $filename = mediamosa_media::get_media_filename($mediafile);

    // Is a public link?
    if ($is_public) {
      $ticket_id = $mediafile_id;

      // Get the paths needed for the symlink.
      $link = mediamosa_storage::get_realpath_media_permanent_file($app_id, $ticket_id, $filename);
      $path = mediamosa_storage::get_path_media_permanent_file($app_id, $ticket_id, $filename);
    }
    else {
      // Convert to ticket type.
      $ticket_type = mediamosa_media::responsetype2tickettype($response_type);

      // Create ticket.
      $ticket_id = mediamosa_media_ticket::create($app_id, $ticket_type, $user_id, $mediafile_id);

      // Get the paths needed for the symlink.
      $link = mediamosa_storage::get_realpath_media_ticket_file($app_id, $ticket_id, $filename, $ticket_type);
      $path = mediamosa_storage::get_path_media_ticket_file($app_id, $ticket_id, $filename, $ticket_type);
    }

    // Create the symlink.
    mediamosa_media::handle_media_symlink($this->uri, $link);

    // Get the streaming server URI.
    $server_uri = mediamosa_media::get_server_uri($mediafile_id, $uri, $response_type);

    // Rebuild uri with path.
    if($is_public) {
      $path = mediamosa_storage::get_path_media_permanent_file($app_id, $ticket_id, $filename, true);
    }
    $server_uri_build = strtr($server_uri, array(mediamosa_media::PARAM_URI_TICKET => $path));

    // Return the information needed.
    return array(
      mediamosa_io_streamwrapper::MEDIA_VIEW_TICKET_ID => $ticket_id,
      mediamosa_io_streamwrapper::MEDIA_VIEW_FILENAME => $filename,
      mediamosa_io_streamwrapper::MEDIA_VIEW_LINK => $link,
      mediamosa_io_streamwrapper::MEDIA_VIEW_SERVER_URI_BUILD => $server_uri_build,
      mediamosa_io_streamwrapper::MEDIA_VIEW_IS_PUBLIC => $is_public,
    );
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public function get_storage_path_linux() {
    if (mediamosa::in_simpletest_sandbox()) {
      $wrapper = new mediamosa_storage_simpletest_stream_wrapper();
      return $wrapper->get_storage_path_linux();
    }

    return mediamosa::variable_get('mediamosa_current_mount_point', '/srv/mediamosa');
  }

  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public function get_storage_path_windows() {
    if (mediamosa::in_simpletest_sandbox()) {
      $wrapper = new mediamosa_storage_simpletest_stream_wrapper();
      return $wrapper->get_storage_path_windows();
    }

    return mediamosa::variable_get('mediamosa_current_mount_point_windows', '\\\\');
  }

  // ------------------------------------------------------- Basic IO overrides.
  /**
   * Support for mkdir().
   *
   * @param string $uri
   *   A string containing the URI to the directory to create.
   * @param integer $mode
   *   Permission flags - see mkdir().
   * @param integer $options
   *   A bit mask of STREAM_REPORT_ERRORS (unsupported) and
   *   STREAM_MKDIR_RECURSIVE.
   *
   * @return boolean
   *   TRUE if directory was successfully created.
   *
   * @see http://php.net/manual/en/streamwrapper.mkdir.php
   */
  public function mkdir($uri, $mode, $options) {
    $this->uri = $uri;

    // Call our mkdir.
    return mediamosa_io::mkdir($this->realpath(), ($options & STREAM_MKDIR_RECURSIVE) == STREAM_MKDIR_RECURSIVE, $mode);
  }
}
