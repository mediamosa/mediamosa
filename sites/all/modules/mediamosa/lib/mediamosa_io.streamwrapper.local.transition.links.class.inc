<?php
/**
 * @file
 * The stream wrapper for transition links.
 *
 * The location of the transition path is used for mediafiles with temporary
 * usage and kept as long as the transition diskspace allows it.
 *
 * The streamwrapper for links handles the symlinks to translation files.
 */

// Current installations will break as wrapper hook will load before parent
// class is indexed for autoload.
require_once 'mediamosa_io.streamwrapper.class.inc';

class mediamosa_io_streamwrapper_local_transition_links extends mediamosa_io_streamwrapper_local_transition {
  // --------------------------------------------------- Stream wrapper methods.
  /**
   * Implements getTarget()
   */
  public function getTarget($uri = NULL) {
    // When $uri is provided, then we must assume that we are not in 'realpath'
    // mode.
    if ($uri != '') {
      return parent::getTarget($uri);
    }

    $target = parent::getTarget();
    return 'links' . ($target == '' ? '' : ('/' . $target));
  }

  // ------------------------------------------------------------------ Methods.
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
    $reg = '#^[a-zA-Z0-9](/[a-zA-Z0-9]+)?$#';
    $matches = array();
    return preg_match($reg, $mediafile_path, $matches);
  }
}
