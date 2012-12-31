<?php
/**
 * @file
 * Our MediaMosa storage stream wrapper class.
 *
 * Storage for mediafiles;
 *
 * mediamosa-storage-data://[mediafile_id]
 * mediamosa-storage://data/[mediafile_id]
 */

// Current installations will break as wrapper hook will load before parent
// class is indexed for autoload.
require_once 'mediamosa_io.streamwrapper.class.inc';

class mediamosa_io_streamwrapper_local_data extends mediamosa_io_streamwrapper_local {
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

    $target = parent::getTarget();
    return 'data' . (empty($target) ? '' : '/' . $target);
  }
}
