<?php
/**
 * @file
 * standard storage class module.
 */

class mediamosa_storage_standard extends mediamosa_storage {
  
  /**
   * Name / ID of the storage module.
   */
  const NAME = 'standard';
  
  /**
   * Return the current mountpoint
   */
  public static function get_current_mountpoint() {
    $mountpoint = preg_replace('/\/$/', '', variable_get('mediamosa_current_mount_point'));
    return $mountpoint;
  }
  
  /**
   * Return the current windows mountpoint
   */
  public static function get_current_mountpoint_windows() {
    return variable_get('mediamosa_current_mount_point_windows');
  }

  /**
   * Return the standard uri for the given file
   * 
   * @param $file
   *   Path to the file
   *   
   * @return
   *   URI for the file
   */
  public static function get_uri($file) {
    $path = preg_replace('/^' . str_replace('/', '\/', mediamosa_storage_standard::get_current_mountpoint()) . '/', '', $file);
    return mediamosa_storage_standard::NAME . '://1/' . $path;
  }
  
  /**
   * Return the storage options available for the storage type
   */
  public function get_storage_options() {
    return array(
      self::NAME . '://' . $this->profile->{mediamosa_storage_profile_db::ID} . '/global' => t('global mountpoint'),
    );
  }
  
  public function get_upload_parameters($app_id, $user_id, $group_id) {
    //TODO: what if $this->mediafile[mediamosa_asset_mediafile_db::APP_ID] != $app_id ??
    
    // Create ticket.
    $ticket_id = mediamosa_ticket::create_for_upload($app_id, $this->mediafile[mediamosa_asset_mediafile_db::ID], $user_id, $group_id);
    
    // Get serverupload.
    $server_upload = mediamosa_server::get_random_upload();
    
    if (!$server_upload) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_UPLOAD_NOT_FOUND);
    }
    
    // Generate unique ID.
    $uuid = rand(1000000, 9999999);
    
    // Return parameters.
    // TODO: adjust parameters to new parameter structure
    return array(
      'properties' => array(
        'action' => strtr(mediamosa_server::get_server_url($server_upload), array('{TICKET}' => $ticket_id, '{PROGRESS_ID}' => $uuid)),
        'method' => 'post',
      ),
      'fields' => array(
        'uploadprogress_url' => strtr($server_upload[mediamosa_server_db::URI_UPLOAD_PROGRESS], array('{PROGRESS_ID}' => $uuid, '{SERVER_ID}' => $server_upload[mediamosa_server_db::NID])),
        'ticket_id' => $ticket_id,
        'progress_id' => $uuid,
        'server_id' => $server_upload[mediamosa_server_db::NID],
      ),
      'file' => array(
        'name' => 'file[]',
      )
    );
  }
  
}