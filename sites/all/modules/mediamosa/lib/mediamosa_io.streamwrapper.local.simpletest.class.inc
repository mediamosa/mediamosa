<?php
/**
 * Our MediaMosa storage simpletest stream wrapper class. Use this wrapper in
 * case you need to use the simpletest path during normal operations.
 *
 * mediamosa-storage-simpletest://
 *
 * @see mediamosa_storage::get_uri_simpletest()
 */

class mediamosa_storage_simpletest_stream_wrapper extends mediamosa_io_streamwrapper_local {
  // ------------------------------------------------------------------ Methods.
  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public function get_storage_path_linux() {
    return mediamosa_storage::trim_uri(mediamosa::variable_get('mediamosa_current_mount_point', '/srv/mediamosa')) . 'media/simpletest';
  }

  /**
   * Get the root of the storage setting for linux.
   *
   * @return string
   *   The path of the storage root.
   */
  public function get_storage_path_windows() {
    return mediamosa_storage::trim_uri(mediamosa::variable_get('mediamosa_current_mount_point_windows', '\\\\')) . 'media/simpletest';
  }
}
