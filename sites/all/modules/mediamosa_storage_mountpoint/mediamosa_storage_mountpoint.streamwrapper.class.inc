<?php
/**
 * @file
 * The mediamose storage s3 wrapper handles files that are either in transition
 * or on the S3 storage.
 */

class mediamosa_storage_mountpoint_streamwrapper extends mediamosa_io_streamwrapper_local {
  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Implements getDirectoryPath().
   */
  public function getDirectoryPath() {
    // Get the storage profile.
    $storage_profile = $this->get_storage_profile();

    // Return the correct mountpoint.
    return (mediamosa::is_windows_platform() ?
      $storage_profile[mediamosa_storage_mountpoint_db::MOUNTPOINT_WINDOWS] : $storage_profile[mediamosa_storage_mountpoint_db::MOUNTPOINT]);
  }

  /**
   * Returns the working path. Any info used by storage profile object is
   * stripped from the path.
   *
   * Expects the path to be [storage_profile_id]/[path][/-/mediafile_path].
   *
   * @return string
   *   The storage path.
   */
  public function getTarget($uri = NULL) {
    // When $uri is provided, then we must assume that we are not in 'realpath'
    // mode.
    if (!empty($uri)) {
      return parent::getTarget($uri);
    }

    // Need path.
    $target = parent::getTarget();

    if (empty($target) || strpos($target, '/') === FALSE) {
      return '';
    }

    // Expecting path is be [storage_profile_id]/[path]/mediafile_id[0]/[mediafile_id].
    list(, $target) = explode('/', $target, 2);

    return $target;
  }

  // --------------------------------------------------------- Abstract methods.
  /**
   * Build the storage object that belongs to this streamwrapper.
   *
   * @param array $storage_profile
   *   Storage profile.
   *
   * @return mediamosa_storage_mountpoint
   *   The mediamosa storage mountpoint object.
   */
  public function create_storage_object(array $storage_profile) {
    return new mediamosa_storage_mountpoint($storage_profile);
  }

  /**
   * Get the mount point.
   *
   * Because we extend on local, we need to override the implementation of
   * get_uri_mount_point(), as local does not use storage profile ID or path,
   * as these are fixed in the local stream wrappers.
   *
   * @return string
   *   The mount point, usable to store in sannas_mount_point field.
   */
  public function get_uri_mount_point() {
    $path = $this->get_path();
    return $this->get_scheme() . '://' . $this->get_storage_profile_id() . (empty($path) ? '' : '/' . $path);
  }

  // ------------------------------------------------------------------ Methods.
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
    $reg = '#^(?P<scheme>.+)://(?P<strorage_profile_id>[0-9]+)/?(?:(?P<path>.*)/-/(?P<mediafile_path>.*)|(?P<altpath>.+))?$#';
    if (!preg_match($reg, $uri, $matches)) {
      // External and mount point module uri's must have valid URI's. Local
      // stream wrappers work fine with only the scheme.
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_IO_ERROR, array('@error' => "invalid storage uri: '" . $uri . "'"));
    }

    // Rename altpath.
    if (isset($matches['altpath'])) {
      $matches['path'] = $matches['altpath'];
      unset($matches['altpath']);
    }

    $mediafile_path = empty($matches['mediafile_path']) ? '' : trim($matches['mediafile_path'], '\/');

//    // Validate the mediafile_path part.
//    if (!empty($mediafile_path) && !$this->validate_uri_mediafile_path($mediafile_path)) {
//      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_IO_ERROR, array('@error' => "invalid storage uri: '" . $uri . "', ' . $mediafile_path . '"));
//    }

    // Return parts.
    return array(
      'scheme' => $matches['scheme'],
      'storage_profile_id' => $matches['strorage_profile_id'],
      'path' => empty($matches['path']) ? '' : trim($matches['path'], '\/'),
      'mediafile_path' => $mediafile_path,
    );
  }

  /**
   * Validate the uri format.
   *
   * Expects;
   * mediafile_id[0]/mediafile_id
   *
   * @param string $mediafile_path
   *   The mediafile path to validate.
   *
   * @return boolean
   *   Returns TRUE for match, FALSE otherwise.
   */
  public function validate_uri_mediafile_path($mediafile_path) {
    $reg = '#^[a-zA-Z0-9](/[a-zA-Z0-9]+)?$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
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

    // Get storage ID.
    $storage_profile_id = $this->get_storage_profile_id();

    if (empty($storage_profiles[$storage_profile_id])) {
      // Get the storage profile.
      $storage_profile = mediamosa_storage_mountpoint::get($storage_profile_id);
      if (!$storage_profile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_PROFILE_NOT_FOUND, array('@profile_id' => $storage_profile_id));
      }
    }

    return $storage_profile;
  }
}
