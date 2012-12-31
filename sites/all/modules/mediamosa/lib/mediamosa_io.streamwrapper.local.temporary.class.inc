<?php
/**
 * @file
 * The stream wrapper for temporary files.
 *
 * mediamosa-storage-temporary://
 * mediamosa-storage://data/transcode/
 */

// Current installations will break as wrapper hook will load before parent
// class is indexed for autoload.
require_once 'mediamosa_io.streamwrapper.class.inc';

class mediamosa_io_streamwrapper_local_temporary extends mediamosa_io_streamwrapper_local {
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

  // --------------------------------------------------------- Abstract methods.
  /**
   * Validate the uri format.
   *
   * Expects;
   * mediafile_id
   * job_id
   * No sub maps.
   *
   * @param string $mediafile_path
   *   The mediafile path to validate.
   *
   * @return boolean
   *   Returns TRUE for match, FALSE otherwise.
   */
  public function validate_uri_mediafile_path($mediafile_path) {
    $reg = '#^[^/]+$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
  }

  // ---------------------------------------------- MediaMosa related functions.
  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public static function get_storage_path_linux() {
    if (mediamosa::in_simpletest_sandbox()) {
      return mediamosa_storage::trim_uri(mediamosa_storage_simpletest_stream_wrapper::get_storage_path_linux()) . 'data/transcode';
    }

    return mediamosa::variable_get('mediamosa_current_mount_point_temporary', '/srv/mediamosa/data/transcode');
  }

  /**
   * Get the root of the storage setting for windows.
   *
   * @return string
   *   The path of the storage root.
   */
  public static function get_storage_path_windows() {
    return mediamosa::variable_get('mediamosa_current_mount_point_temporary_windows', '\\');
  }
}
