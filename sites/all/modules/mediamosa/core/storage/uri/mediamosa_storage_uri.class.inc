<?php
/**
 * @file
 * Local storage class module.
 */

class mediamosa_storage_uri extends mediamosa_storage {
  // ------------------------------------------------------------------- Consts.
  /**
   * Name / ID of the storage module.
   */
  const NAME = 'uri';

  const SCHEME = 'mediamosa-storage-external-uri';

  // --------------------------------------------------------- Abstract Methods.
  // ------------------------------------------------------------------ Methods.
  /**
   * Return with uri of the profile mount point.
   *
   * The local mount point is based on the default settings. We do not store
   * extra information in $storage_profile_app array, and therefor we don't
   * store $storage_profile_app::ID in the uri nor store we the
   * $storage_profile_app::PATH or $storage_profile_app::PATH_STILLS.
   *
   * Other storage types must include the $storage_profile_app::ID and the
   * $storage_profile_app::PATH / $storage_profile_app::PATH_STILLS in the URI.
   *
   * @param integer $storage_profile_id
   *   (ignored) The storage profile ID.
   * @param string $path
   *   (ignored) The storage path.
   * @param boolean $is_still
   *   Is mount point for still?
   *
   * @return string
   *   The uri of the storage profile.
   */
  public function build_mount_point_uri($storage_profile_id, $path, $is_still = FALSE) {
    return $this->get_scheme($is_still) . '://';
  }

  /**
   * Return the storage options available for the storage type
   */
  public function get_storage_options() {
    return array();
  }

  /**
   * Download mediafile to transition.
   */
  public function download() {
  }

  /**
   * Get the default scheme.
   *
   * @param boolean $is_still
   *   Is mount point for still?
   *
   * @return string
   *   The uri of the moint point to store.
   */
  public function get_scheme($is_still = FALSE) {
    return self::SCHEME;
  }
}
