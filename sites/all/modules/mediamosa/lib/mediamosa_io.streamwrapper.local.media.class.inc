<?php
/**
 * @file
 * Our MediaMosa storage stream wrapper class.
 *
 * mediamosa-storage-media://
 */

// Current installations will break as wrapper hook will load before parent
// class is indexed for autoload.
require_once 'mediamosa_io.streamwrapper.local.class.inc';

class mediamosa_io_streamwrapper_local_media extends mediamosa_io_streamwrapper_local {
  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Implements getTarget()
   */
  public function getTarget($uri = NULL) {
    // When $uri is provided, then we must assume that we are not in 'realpath'
    // mode.
    if (!empty($uri)) {
      return parent::getTarget($uri);
    }

    $target = parent::getTarget($uri);
    return 'media' . (empty($target) ? '' : ('/' . $target));
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
    $reg = '#^.*[a-zA-Z0-9]/[a-zA-Z0-9]+/[^/]+$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
  }
}
