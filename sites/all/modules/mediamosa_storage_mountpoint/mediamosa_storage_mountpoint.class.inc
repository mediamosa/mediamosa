<?php
/**
 * @file
 * mountpoint storage class module.
 */

class mediamosa_storage_mountpoint extends mediamosa_storage_local {
  // ------------------------------------------------------------------- Consts.
  /**
   * Name / ID of the module.
   */
  const NAME = 'mountpoint';

  // -------------------------------------------------------------- Constructor.
  /**
   * Main constructor.
   *
   * @param object|integer $storage_profile
   *   Storage profile node or nid
   *
   * @param $uri
   *   Optional uri to the storage object
   */
  public function __construct($storage_profile) {
    // Get the specific data for mountpoint.
    $storage_mountpoint = mediamosa_db::db_select(mediamosa_storage_mountpoint_db::TABLE_NAME, 'mp')
      ->fields('mp')
      ->condition(mediamosa_storage_mountpoint_db::ID, $storage_profile[mediamosa_storage_mountpoint_db::ID])
      ->execute()
      ->fetchAssoc();

    // Merge.
    $storage_profile += $storage_mountpoint;

    // Call parent.
    parent::__construct($storage_profile);
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Return with uri of the profile mount point.
   *
   * Unless local mount point, the uri must include $storage_profile::ID and
   * $storage_profile_app::PATH or $storage_profile_app::PATH_STILLS.
   *
   * @param integer $storage_profile_id
   *   The storage profile ID.
   * @param string $path
   *   The storage path.
   * @param boolean $is_still
   *   Ignored, the still path is handled in the path and not by stream scheme.
   *
   * @return string
   *   The uri of the storage profile.
   */
  public function build_mount_point_uri($storage_profile_id, $path, $is_still = FALSE) {
    return self::trim_uri($this->get_scheme($is_still) . '://' . $storage_profile_id) . $path;
  }

  /**
   * Return the mountpoint's subdirs
   */
  public function get_storage_options() {
    $options = array();

    $mountpoint = preg_replace('/\/$/', '', $this->storage_profile[mediamosa_storage_mountpoint_db::MOUNTPOINT]) . '/';
    $options[$this->storage_profile[mediamosa_storage_profile_db::ID] . '/'] = $mountpoint;

    foreach (glob($this->storage_profile[mediamosa_storage_mountpoint_db::MOUNTPOINT]. '/*', GLOB_ONLYDIR) as $path) {
      if (is_writable($path)) {
        $uri = $this->storage_profile[mediamosa_storage_profile_db::ID] . preg_replace('/^' . str_replace('/', '\/', $mountpoint) . '/', '/', $path);
        $options[$uri] = $path;
      }
    }

    return $options;
  }

  public function get_upload_parameters($app_id, $user_id, $group_id) {
    return array();
  }

  // ---------------------------------------------------------------- Functions.
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
    return 'mediamosa-storage-mountpoint';
  }

  /**
   * Get storage profile.
   *
   * @param $storage_profile_id
   *   Profile to get.
   *
   * @return array
   *   An associative array with all profile attributes.
   */
  public static function get($storage_profile_id) {
    $query = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp');
    $query->join(mediamosa_storage_mountpoint_db::TABLE_NAME, 'smp', '%alias.nid=sp.nid');
    return $query
      ->fields('sp')
      ->fields('smp', array(mediamosa_storage_mountpoint_db::MOUNTPOINT, mediamosa_storage_mountpoint_db::MOUNTPOINT_WINDOWS))
      ->condition('sp.' . mediamosa_storage_profile_db::ID, $storage_profile_id)
      ->execute()
      ->fetchAssoc();
  }
}
