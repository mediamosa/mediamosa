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
      // @todo: remove from release.
      mediamosa_debug::log_export('debug in ' . __FUNCTION__ . '() on line ' . __LINE__ . ' in ' . __FILE__);
      mediamosa_debug::log_export(mediamosa_io::dirname($this->uri));
// @todo: remove from release.


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
   * {scheme}://{storage_profile_id}[/{path}][/-/{mediafile_path}]
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
   * Validate the uri format.
   *
   * Expects;
   * mediafile_id[0]/mediafile_id
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
    $reg = '#^.*[a-zA-Z0-9](/[a-zA-Z0-9]+)?$#';
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

  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Overrides getExternalUrl().
   */
  public function getExternalUrl() {
    return '';

//    $path = str_replace('\\', '/', $this->getTarget());
//    return url('system/temporary/' . $path, array('absolute' => TRUE));
  }

  /**
   * Implements getDirectoryPath().
   */
  public function getDirectoryPath() {
    return rtrim(mediamosa::is_windows_platform() ? static::get_storage_path_windows() : static::get_storage_path_linux(), '/');
  }

  /**
   * Implements getTarget()
   */
  public function getTarget($uri = NULL) {
    // When $uri is provided, then we must assume that we are not in 'realpath'
    // mode.
    if (!empty($uri)) {
      return parent::getTarget($uri);
    }

    // Target.
    $target = parent::getTarget();

    // Target might contain mediafile path.
    if (strpos($target, '/-/') !== FALSE) {
      list($target, $mediafile_path) = explode('/-/', $target, 2);
      return trim($target . '/' . $mediafile_path, '/');
    }

    return $target;
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public static function get_storage_path_linux() {
    if (mediamosa::in_simpletest_sandbox()) {
      return mediamosa_storage_simpletest_stream_wrapper::get_storage_path_linux();
    }

    return mediamosa::variable_get('mediamosa_current_mount_point', '/srv/mediamosa');
  }

  /**
   * Get the root of the storage setting for windows.
   *
   * @return string
   *   The path of the storage root.
   */
  public static function get_storage_path_windows() {
    if (mediamosa::in_simpletest_sandbox()) {
      return mediamosa_storage_simpletest_stream_wrapper::get_storage_path_windows();
    }

    return mediamosa::variable_get('mediamosa_current_mount_point_windows', '\\');
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
