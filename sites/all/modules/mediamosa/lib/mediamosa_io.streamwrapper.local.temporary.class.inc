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
   * letter/mediafile_id[.ext]
   * letter/job_id[.ext]
   * No sub maps.
   *
   * @param string $mediafile_path
   *   The mediafile path to validate.
   *
   * @return bool
   *   Returns TRUE for match, FALSE otherwise.
   */
  public function validate_uri_mediafile_path($mediafile_path) {
    $reg = '#^([a-zA-Z0-9])(/.*(\.[a-zA-Z0-9]+)?)$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
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
      return mediamosa_storage::trim_uri(parent::get_storage_path_linux()) . 'data/transcode';
    }

    return mediamosa::variable_get('mediamosa_current_mount_point_temporary', '/srv/mediamosa/data/transcode');
  }

  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public function get_storage_path_windows() {
    if (mediamosa::in_simpletest_sandbox()) {
      return mediamosa_storage::trim_uri(parent::get_storage_path_windows()) . 'data/transcode';
    }

    return mediamosa::variable_get('mediamosa_current_mount_point_temporary_windows', '\\\\');
  }
}
