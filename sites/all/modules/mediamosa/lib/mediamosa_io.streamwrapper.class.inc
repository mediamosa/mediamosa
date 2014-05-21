<?php
/**
 * @file
 * The mediamosa_io_streamwrapper base class.
 *
 * Is base class for all mediamosa_io_streamwrapper_* classes.
 *
 * Notes:
 * The uri of typical local mediamosa stream;
 *
 * Uri:
 * [scheme]://[path][/-/mediafile_path].
 *
 * Scheme identifies the wrapper.
 * Path comes from mediamosa_storage_app or is fixed value by wrapper.
 * Mediafile_path contains the mediafile_id. The format of mediafile_path is
 * determined by the wrapper. Default is mediafile_id[0]/mediafile_id.
 *
 * mediafile_path:
 * mediafile_id[0]/mediafile_id[/extra]
 * or
 * mediafile_id[/extra]
 *
 * Extra part is optional and can be anything but must end with filename.
 */

abstract class mediamosa_io_streamwrapper extends DrupalLocalStreamWrapper {
  // ------------------------------------------------------------------- Consts.
  const STATIC_MD5S = 'mediamosa_io_streamwrapper::md5';
  const MD5_DEFAULT = '00000000000000000000000000000000';

  /**
   * Key values result for media_view().
   */
  const MEDIA_VIEW_TICKET_ID = 'ticket_id';
  const MEDIA_VIEW_FILENAME = 'filename';
  const MEDIA_VIEW_LINK = 'link';
  const MEDIA_VIEW_SERVER_URI_BUILD = 'server_uri_build';
  const MEDIA_VIEW_IS_PUBLIC = 'is_pubic';

  // --------------------------------------------------------------------- Vars.
  /**
   * The MD5 of the file.
   *
   * @var string
   */
  protected $md5 = NULL;

  // --------------------------------------------------------- Abstract Methods.
  /**
   * Build the storage object that belongs to this streamwrapper.
   *
   * @param array $storage_profile
   *   Storage profile.
   *
   * @return mediamosa_storage
   *   The mediamosa storage object or one extended on mediamosa_storage.
   */
  abstract public function create_storage_object(array $storage_profile);

  /**
   * Create the complete URI for the mediafile.
   *
   * @param string|array $mediafile
   *   The mediafile array or string ID.
   *
   * @return string
   *   The complete local/external uri.
   */
  abstract public function create_storage_uri($mediafile);

  /**
   * Create the uri of the current mediafile.
   *
   * @param array $mediafile
   *   The mediafile array
   *
   * @retrun string
   *   The location of the mediafile.
   *
   */
  abstract public function get_uri_mediafile(array $mediafile);

  /**
   * Test if the current path/file is local or external (like Amazon S3).
   *
   * @return bool
   *   Returns TRUE when local or FALSE otherwise.
   */
  abstract public function is_local();

  /**
   * Get the file on an local location and return the location.
   *
   * The function will make the file available for copy on local file system. If
   * file is external, the file needs to be downloaded.
   *
   * The location can be anywhere on the file system. It can be the mediafile
   * in its storage location, or stored in transition. However, the file might
   * not be available for a long time and should be processed immediately.
   *
   * @return string
   *   Returns the uri to the local file.
   */
  abstract public function get_uri_local_file();

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
  abstract public function mediafile_copy($mediamosa_io_streamwrapper_source);

  /**
   * Validate the uri format.
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
  abstract public function validate_uri($uri);

  /**
   * Validate the uri format.
   *
   * Expects;
   * mediafile_id[0]/mediafile_id
   *
   * @param string $mediafile_path
   *   The mediafile path to validate.
   *
   * @return bool
   *   Returns TRUE for match, FALSE otherwise.
   */
  abstract public function validate_uri_mediafile_path($mediafile_path);

  /**
   * Generate MD5 from source file.
   *
   * @return string
   *   The MD5 of the file or FALSE otherwise.
   */
  abstract public function md5_file();

  /**
   * Process media view request.
   *
   * @param int $app_id
   *   Application ID.
   * @param string $user_id
   *   The owner for possible ticket.
   * @param array $mediafile
   *   The mediafile to process.
   * @param string $response_type
   *   See mediamosa_media::RESPONSE_*
   * @param bool $is_app_admin
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
  abstract public function media_view($app_id, $user_id, array $mediafile, $response_type, $is_app_admin);

  // -------------------------------------------------------- Hotfix IO methods.
  /**
   * Tells whether the filename is a regular file.
   *
   * As current minimum version of PHP 5.2.4 streams do not work with is_file(),
   * will support it through mediamosa_io::is_file(). It will only work when
   * called by mediamosa_io::is_file().
   *
   * @return bool
   *   Returns TRUE if the filename exists and is a regular file, FALSE
   *   otherwise.
   */
  public function is_file() {
    if (!mediamosa_io::file_exists($this->uri)) {
      return FALSE;
    }

    // Stat the file.
    $stat = $this->url_stat($this->uri, STREAM_URL_STAT_QUIET);

    // Check the file flag.
    return ($stat['mode'] & mediamosa_io::STAT_IFREG) == mediamosa_io::STAT_IFREG;
  }

  /**
   * Tells whether the filename is a directory.
   *
   * As current minimum version of PHP 5.2.4 streams do not work with is_dir(),
   * will support it through mediamosa_io::is_dir(). It will only work when
   * called by mediamosa_io::is_dir().
   *
   * @return bool
   *   Returns TRUE if the filename exists and is a directory, FALSE otherwise.
   */
  public function is_dir() {
    if (!mediamosa_io::file_exists($this->uri)) {
      return FALSE;
    }

    // Stat the file.
    $stat = $this->url_stat($this->uri, STREAM_URL_STAT_QUIET);

    // Check the file flag.
    return ($stat['mode'] & mediamosa_io::STAT_IFDIR) == mediamosa_io::STAT_IFDIR;
  }

  /**
   * Get the filesize of the file.
   *
   * @return integer
   *   Returns the size of the file in bytes, or FALSE.
   */
  public function filesize() {
    // Must exists.
    if (!mediamosa_io::file_exists($this->uri)) {
      return FALSE;
    }

    // Stat the file.
    $stat = $this->url_stat($this->uri, STREAM_URL_STAT_QUIET);

    // Check the file flag.
    return $stat['size'];
  }

  /**
   * Test if the file exists.
   *
   * @return bool
   *   Returns TRUE when file exists.
   */
  public function file_exists() {
    // Use normal PHP version.
    return file_exists($this->uri);
  }

  //
  /**
   * Support for unlink().
   *
   * @param $uri
   *   A string containing the URI to the resource to delete.
   *
   * @return
   *   TRUE if resource was successfully deleted.
   *
   * @see http://php.net/manual/en/streamwrapper.unlink.php
   */
  public function unlink($uri) {
    // Make sure md5file is recalculated now this file is removed.
    $md5s = &drupal_static(self::STATIC_MD5S, array());
    unset($md5s[$uri]);

    return parent::unlink($uri);
  }

  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Returns the working path. Any info used by storage profile object is
   * stripped from the path.
   *
   * Expects the path to be [storage_profile_id]/[path][/-/mediafile_path].
   * @param string $uri
   *   (optional) The uri.
   *
   * @return string
   *   The storage path.
   */
  public function getTarget($uri = NULL) {
    // When $uri is provided, then we must assume that we are not in 'realpath'
    // mode.
    if (!empty($uri)) {
      list(, $target) = explode('://', $uri, 2);
      return $target;
    }

    $targets = array();
    $path = $this->get_path();
    $mediafile_path = $this->get_mediafile_path();

    if ($path != '') {
      $targets[] = $path;
    }
    if ($mediafile_path != '') {
      $targets[] = $mediafile_path;
    }

    return implode('/', $targets);
  }

  /**
   * Overruled implementation of realpath().
   *
   * The base version breaks because its calls getTarget() with $uri, preventing
   * my code not to translate information in uri into path data. Also breaks
   * if directory does not exists.
   */
  public function realpath() {
    // Extra debug.
//    if (mediamosa::in_simpletest_sandbox()) {
      $this->validate_uri($this->uri);
//   }

    return $this->getDirectoryPath() . '/' . $this->getTarget();
  }

  /**
   * Returns the canonical absolute path of the URI, if possible.
   *
   * Overriding the version in DrupalLocalStreamWrapper, as this version breaks
   * my wrappers. My stream uri contain 3 part (scheme, path, mediafile_path)
   * and Drupal expects 2 parts (scheme, target). It also does not react on
   * problems when path or file does not exist.
   *
   * @param string $uri
   *   (optional) The stream wrapper URI to be converted to a canonical
   *   absolute path. This may point to a directory or another type of file.
   *
   * @return string|false
   *   If $uri is not set, returns the canonical absolute path of the URI
   *   previously set by the DrupalStreamWrapperInterface::setUri() function.
   *   If $uri is set and valid for this class, returns its canonical absolute
   *   path, as determined by the realpath() function. If $uri is set but not
   *   valid, returns FALSE.
   */
  protected function getLocalPath($uri = NULL) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    return $this->realpath();
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Split up the uri into its parts.
   *
   * @param string $uri
   *   The uri to split up.
   * @param string $part
   *   (optional) Get specific part. Either scheme, storage_profile_id, path or
   *   mediafile_path.
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
  public function get_parts($uri, $part = '') {
    if (empty($uri)) {
      $uri = $this->uri;
    }

    // Some caching.
    $uris = &drupal_static(__CLASS__ . '::' . __FILE__, array());

    // Lookup in cache.
    if (empty($uris[$uri])) {
      // Set result.
      $uris[$uri] = $this->validate_uri($uri);
    }

    return empty($part) ? $uris[$uri] : $uris[$uri][$part];
  }

  /**
   * Return the scheme stored in the uri.
   *
   * Scheme is returned with out '://'.
   *
   * @return string
   *   The scheme.
   */
  public function get_scheme() {
    return $this->get_parts($this->uri, 'scheme');
  }

  /**
   * Return the storage profile ID.
   *
   * @return integer
   *   The storage profile ID.
   */
  public function get_storage_profile_id() {
    return (int) $this->get_parts($this->uri, 'storage_profile_id');
  }

  /**
   * Return the path stored in the uri (without mediafile path).
   *
   * @return string
   *   The path.
   */
  public function get_path() {
    return $this->get_parts($this->uri, 'path');
  }

  /**
   * Return possible mediafile information.
   *
   * @return string
   *   The mediafile path.
   */
  public function get_mediafile_path() {
    return $this->get_parts($this->uri, 'mediafile_path');
  }

  /**
   * Return the mount pount as real path.
   *
   * @return string
   *   The real path of the mount pount.
   */
  public function get_realpath_mount_point() {
    return mediamosa_io::realpath($this->get_uri_mount_point());
  }

  /**
   * Convert the uri to its storage class.
   *
   * @return array
   *   The storage profile.
   *
   * @throws mediamosa_exception_error()
   */
  public function get_storage_profile() {
    $storage_profiles = &drupal_static(__CLASS__ . '::' . __FUNCTION__, array());

    // Get the storage profile ID.
    $storage_profile_id = $this->get_storage_profile_id();

    if (empty($storage_profiles[$storage_profile_id])) {
      // Get the storage profile.
      $storage_profile[$storage_profile_id] = mediamosa_storage_profile::get($storage_profile_id);
      if (!$storage_profile[$storage_profile_id]) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_PROFILE_NOT_FOUND, array('@profile_id' => $storage_profile_id));
      }
    }

    return $storage_profile[$storage_profile_id];
  }

  /**
   * Try to retrieve the mediafile ID from the uri.
   *
   * @return string|boolean
   *   Either the mediafile ID or ''.
   */
  public function get_mediafile_id() {
    // Get possible mediafile path.
    $mediafile_path = $this->get_mediafile_path();

    if (empty($mediafile_path)) {
      return '';
    }

    // Get parts.
    $parts = explode('/', $mediafile_path);

    // No parts, then suspect the result as mediafile_id.
    if (count($parts) == 1) {
      return mediamosa_io::get_base_filename(reset($parts));
    }

    // At least 2 parts.
    //
    // One letter?
    if (strlen($parts[0]) == 1) {
      // Expect 2nd part to be mediafile.
      return mediamosa_io::get_base_filename($parts[1]);
    }

    // 2nd part must be mediafile_id.
    return mediamosa_io::get_base_filename($parts[2]);
  }

  /**
   * Get the mount point.
   *
   * @return string
   *   The mount point, usable to store in sannas_mount_point field.
   */
  public function get_uri_mount_point() {
    $path = $this->get_path();
    return $this->get_scheme() . '://' . $this->get_storage_profile_id() . (empty($path) ? '' : '/' . $path);
  }

  /**
   * Return the md5 of the file.
   *
   * @return string
   *   The md5 of the current file or FALSE otherwise.
   */
  final public function getMD5() {
    return $this->md5_file();
  }

  /**
   * Set the md5 value of the file.
   *
   * @param string $md5
   *   The MD5 to set.
   */
  public function setMD5($md5) {
    $md5s = &drupal_static(self::STATIC_MD5S, array());
    $md5s[$this->uri] = $md5;
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Build the mediafile part of the uri.
   *
   * The '/-/' is a seperator, to allow the streamwrapper to split on scheme,
   * path and mediafile_id.
   *
   * Should only be used in URI's.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   *
   * @return string
   *   The mediafile ID uri part.
   */
  public static function create_mediafile_path($mediafile_id) {
    return empty($mediafile_id) ? '' : ('/-/' . $mediafile_id[0] . '/' . $mediafile_id);
  }

  /**
   * Find the wrapper and return its info.
   *
   * @param string $uri
   *   The uri of the wrapper to find.
   *
   * @return array|FALSE
   *   Either array with info or FALSE.
   */
  public static function get_wrapper_info($uri) {
    // Get the wrappers.
    $wrappers = file_get_stream_wrappers();

    // Get scheme.
    $scheme = file_uri_scheme($uri);

    return isset($wrappers[$scheme]) ? $wrappers[$scheme] : FALSE;
  }
}
