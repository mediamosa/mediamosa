<?php
/**
 * @file
 * mountpoint storage class module.
 */

class mediamosa_storage_mountpoint extends mediamosa_storage {
  
  /**
   * Name / ID of the module.
   */
  const NAME = 'mountpoint';
  
  /**
   * Return the mountpoint's subdirs
   */
  public function get_storage_options() {
    $options = array();
    
    $mountpoint = preg_replace('/\/$/', '', $this->profile->{mediamosa_storage_mountpoint_db::MOUNTPOINT}) . '/';
    $options[self::NAME . '://' . $this->profile->{mediamosa_storage_profile_db::ID} . '/'] = $mountpoint;
    
    foreach (glob($this->profile->{mediamosa_storage_mountpoint_db::MOUNTPOINT}. '/*', GLOB_ONLYDIR) as $path) {
      if (is_writable($path)) {
        $uri = self::NAME . '://' . $this->profile->{mediamosa_storage_profile_db::ID} . preg_replace('/^' . str_replace('/', '\/', $mountpoint) . '/', '/', $path);
        $options[$uri] = $path;
      }
    }
    
    return $options;
  }
  
  public function get_upload_parameters($app_id, $user_id, $group_id) {
    return array();
  }
}
