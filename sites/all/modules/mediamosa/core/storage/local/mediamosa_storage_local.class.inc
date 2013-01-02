<?php
/**
 * @file
 * Local storage class module.
 */

class mediamosa_storage_local extends mediamosa_storage {
  // ------------------------------------------------------------------- Consts.
  /**
   * Name / ID of the storage module.
   */
  const NAME = 'local';

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
    return self::get_scheme($is_still) . '://';
  }

  /**
   * Return the storage options available for the storage type
   */
  public function get_storage_options() {
    return array(
      $this->storage_profile[mediamosa_storage_profile_db::ID] . '/global' => t('global mountpoint'),
    );
  }

  /**
   * Download mediafile to transition.
   */
  public function download() {

  }

//  public function get_upload_parameters($app_id, $user_id, $group_id) {
//    //TODO: what if $this->mediafile[mediamosa_asset_mediafile_db::APP_ID] != $app_id ??
//
//    // Create ticket.
//    $ticket_id = mediamosa_ticket::create_for_upload($app_id, $this->mediafile[mediamosa_asset_mediafile_db::ID], $user_id, $group_id);
//
//    // Get serverupload.
//    $server_upload = mediamosa_server::get_random_upload();
//
//    if (!$server_upload) {
//      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_UPLOAD_NOT_FOUND);
//    }
//
//    // Generate unique ID.
//    $uuid = rand(1000000, 9999999);
//
//    // Return parameters.
//    // TODO: adjust parameters to new parameter structure
//    return array(
//      'properties' => array(
//        'action' => strtr(mediamosa_server::get_server_url($server_upload), array('{TICKET}' => $ticket_id, '{PROGRESS_ID}' => $uuid)),
//        'method' => 'post',
//      ),
//      'fields' => array(
//        'uploadprogress_url' => strtr($server_upload[mediamosa_server_db::URI_UPLOAD_PROGRESS], array('{PROGRESS_ID}' => $uuid, '{SERVER_ID}' => $server_upload[mediamosa_server_db::NID])),
//        'ticket_id' => $ticket_id,
//        'progress_id' => $uuid,
//        'server_id' => $server_upload[mediamosa_server_db::NID],
//      ),
//      'file' => array(
//        'name' => 'file[]',
//      )
//    );
//  }

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
  public static function get_scheme($is_still = FALSE) {
    return $is_still ? 'mediamosa-storage-stills' : 'mediamosa-storage-data';
  }
}
