<?php
/**
 * @file
 * This example streamwrapper is used by our simpletests and should give an
 * example template for external wrappers.
 *
 * For better example, look at the S3 implementation.
 *
 * Notes:
 * ------
 *
 * The uri of typical external mediamosa stream;
 *
 * Uri:
 * {scheme}://{storage_profile_id}/[path][/-/mediafile_path].
 *
 * Difference with local wrapper is that storage_profile_id in uri is required.
 *
 * Make sure you implement unlink with the following code;
 *
 *   // Make sure md5file is recalculated now this file is removed.
 *   $md5s = &drupal_static(self::STATIC_MD5S, array());
 *   unset($md5s[$uri]);
 *
 * Do not call $this->unlink($uri) when you want to unlink your file, always
 * use mediamosa_io::unlink() as it will clear some cache that is not cleared
 * when calling this->unlink() version.
 */

/**
 * Just here for example.
 */
class mediamosa_storage_example extends mediamosa_storage {
  /**
   * Get the default scheme.
   *
   * @param boolean $is_still
   *   Is mount point for still? Is ignored on external uris.
   *
   * @return string
   *   The uri of the moint point to store.
   */
  public function get_scheme($is_still = FALSE) {
    return 'mediamosa-storage-external-example';
  }

  /**
   * Return the storage options available for the storage type
   *
   * @return
   *   Array of available storage options
   */
  public function get_storage_options() {
    return array('some path' => (t('my path')));
  }


  /**
   * Return alle required upload parameters for the storage type
   * which allow a frontend application to build the correct upload form
   *
   * $param $app_id
   *   Application ID.
   *
   * @param $user_id
   *   User ID
   * @param $group_id
   *   Group ID
   *
   * @return
   *   Array with upload parameters
   */
//  public function get_upload_parameters($app_id, $user_id, $group_id) {
//    return array();
//  }
}

class mediamosa_io_streamwrapper_external_example extends mediamosa_io_streamwrapper_external {
  // --------------------------------------------------------- Abstract methods.
  /**
   * Build the storage object that belongs to this streamwrapper.
   *
   * @param array $storage_profile
   *   Storage profile.
   *
   * @return mediamosa_storage_s3
   *   The mediamosa storage S3 object.
   */
  public function create_storage_object(array $storage_profile) {
    return new mediamosa_storage_example($storage_profile);
  }

  /**
   * Get the file on an local location and return the location.
   *
   * @return string
   *   Returns the uri to the local file.
   */
  public function get_uri_local_file() {
    return 'mediamosa-storage-data:///-/s/something.txt';
  }

  /**
   * Copy file to any location.
   *
   * Function must make sure that on return $this->uri is updated with new
   * location and $this->md5 is set on the destination (self).
   *
   * @param mediamosa_io_streamwrapper $mediamosa_io_streamwrapper_source
   *   The local stream wrapper.
   *
   * @throws mediamosa_exception_error
   */
  public function mediafile_copy($mediamosa_io_streamwrapper_source) {
    // See S3 implementation.
  }

  /**
   * Get MD5 from s3 file.
   *
   * @return string
   *   The MD5 of the file or FALSE otherwise.
   */
  public function md5_file() {
    // Should return a valid MD5 value.
    return self::MD5_DEFAULT;
  }
  // ------------------------------------------------------ Abstract IO Methods.
  /**
   * These IO methods reflect file / IO functions that is not supported by the
   * PHP IO layer. However, mediamosa_io will support these.
   */

  /**
   * Check if the the file exists.
   *
   * @return boolean
   *   Returns TRUE when file exists.
   */
  public function file_exists() {
    // Implement into your class file_exists().
    return TRUE;
  }

  /**
   * Return the file size.
   *
   * @return integer
   *   Return the filesize or FALSE when file does not exists.
   */
  public function filesize() {
    // Implement into your class file_exists().
    return $this->file_exists() ? TRUE : FALSE;
  }
}
