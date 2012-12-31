<?php
/**
 * @file
 * The stream wrapper for transition files.
 *
 * The location of the transition path is used for mediafiles with temporary
 * usage and kept as long as the transition diskspace allows it.
 */

// Current installations will break as wrapper hook will load before parent
// class is indexed for autoload.
require_once 'mediamosa_io.streamwrapper.class.inc';

class mediamosa_io_streamwrapper_local_transition extends mediamosa_io_streamwrapper_local {
  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Overrides getExternalUrl().
   *
   * Return the HTML URI of a public file.
   */
  public function getExternalUrl() {
    // Unsupported for now.
    return '';

    //$path = str_replace('\\', '/', $this->getTarget());
    //return self::getDirectoryPath() . '/' . drupal_encode_path($path);
  }

  /**
   * Try to retrieve the mediafile ID from the uri.
   *
   * @return string|boolean
   *   Either the mediafile ID or NULL.
   */
  public function get_mediafile_id() {
    // Need path.
    $mediafile_path = $this->get_mediafile_path();

    // letter/mediafile_id/md5.
    list(,$mediafile_id,) = explode('/', $mediafile_path, 3);

    return $mediafile_id;
  }

  /**
   * Try to retrieve the MD5 from the uri.
   *
   * @return string
   *   Either the MD5 or NULL.
   */
  public function get_md5_from_uri() {
    // Need path.
    $mediafile_path = $this->get_mediafile_path();

    // letter/mediafile_id/md5.
    list(,,$md5) = explode('/', $mediafile_path);

    return $md5;
  }

  // --------------------------------------------------------- Abstract methods.
  /**
   * Validate the uri format.
   *
   * Expects;
   * mediafile_id[0]/mediafile_id/md5
   *
   * @param string $mediafile_path
   *   The mediafile path to validate.
   *
   * @return boolean
   *   Returns TRUE for match, FALSE otherwise.
   */
  public function validate_uri_mediafile_path($mediafile_path) {
    $reg = '#^.*[a-zA-Z0-9]/[a-zA-Z0-9]+/[a-zA-Z0-9]+$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
  }

  // ---------------------------------------------- MediaMosa related functions.
  /**
   * Get the root of the transition setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public static function get_storage_path_linux() {
    if (mediamosa::in_simpletest_sandbox()) {
      return mediamosa_storage::trim_uri(mediamosa_storage_simpletest_stream_wrapper::get_storage_path_linux()) . 'data/transition';
    }

    return mediamosa::variable_get('mediamosa_current_mount_point_transition', '/srv/mediamosa/data/transition');
  }

  /**
   * Get the root of the transition setting for windows.
   *
   * @return string
   *   The path of the storage root.
   */
  public static function get_storage_path_windows() {
    return mediamosa::variable_get('mediamosa_current_mount_point_transition_windows', '\\');
  }
}
