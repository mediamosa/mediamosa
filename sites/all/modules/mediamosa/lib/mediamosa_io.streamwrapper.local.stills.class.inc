<?php
/**
 * @file
 * Our MediaMosa storage stream wrapper class.
 *
 * Storage for mediafiles;
 *
 * mediamosa-storage-stills://
 * mediamosa-storage://data/
 */

// Current installations will break as wrapper hook will load before parent
// class is indexed for autoload.
require_once 'mediamosa_io.streamwrapper.class.inc';

class mediamosa_io_streamwrapper_local_stills extends mediamosa_io_streamwrapper_local_data {
  // ------------------------------------------------------------------ Methods.
  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public function get_storage_path_linux() {
    return mediamosa::variable_get('mediamosa_current_mount_point_stills', parent::get_storage_path_linux());
  }

  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public function get_storage_path_windows() {
    return mediamosa::variable_get('mediamosa_current_mount_point_stills_windows', parent::get_storage_path_windows());
  }
}
