<?php
/**
 * @file
 * mediamosa_configuration_storage has been replaced with mediamosa_storage.
 * This module will be removed in 4.x.
 *
 * @deprecated
 * @see class mediamosa_storage.
 */

class mediamosa_configuration_storage {

  // ------------------------------------------------------------------ Static Function.
  /**
   * @deprecated
   * @see mediamosa_storage::is_storage_setup()
   */
  public static function is_mount_setup() {
    return mediamosa_storage::is_storage_setup();
  }

  /**
   * @deprecated
   * @see mediamosa_storage_stream_wrapper::get_storage_path_linux()
   */
  public static function linux_get() {
    return mediamosa_storage_stream_wrapper::get_storage_path_linux();
  }

  /**
   * @deprecated
   * @see mediamosa_storage_stream_wrapper::get_storage_path_windows()
   */
  public static function windows_get() {
    return mediamosa_storage_stream_wrapper::get_storage_path_windows();
  }

  /**
   * @deprecated
   * Function has been removed.
   */
  public static function linux_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point', $value);
  }

  /**
   * @deprecated
   * Function has been removed.
   */
  public static function windows_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point_windows', $value);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_data_location()
   */
  public static function data_location_get() {
    return mediamosa_storage::get_path_data();
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_path_data()
   */
  public static function data_still_location_get() {
    return mediamosa_storage::get_path_data();
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_data()
   */
  public static function data_still_get_path() {
    return mediamosa_storage::get_uri_data();
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_data_file()
   */
  public static function data_still_get_file($mediafile_id) {
    return mediamosa_storage::get_uri_data_file($mediafile_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_data_transcode()
   */
  public static function transcode_tmp_location_get() {
    return mediamosa_storage::get_uri_data_transcode();
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_data_transcode_file()
   */
  public static function transcode_file_location_get($filename) {
    return mediamosa_storage::get_uri_data_transcode_file($filename);
  }

  /**
   * @deprecated
   * @see
   */
  public static function ticket_play_location_get($app_id = -1) {
    return mediamosa_storage::get_uri_media_play($app_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_media_download_ticket()
   */
  public static function ticket_download_location_get($app_id = -1, $mediafile_id = '') {
    return mediamosa_storage::get_uri_media_download_ticket($app_id, $mediafile_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_media()
   */
  public static function media_get_path($app_id = -1, $mediafile_id = '') {
    return mediamosa_storage::get_uri_media($app_id, $mediafile_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_media_file()
   */
  public static function media_get_file($app_id, $mediafile_id) {
    return mediamosa_storage::get_uri_media_file($app_id, $mediafile_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_media_ticket()
   */
  public static function media_get_ticket_path($app_id = -1, $mediafile_id = '') {
    return mediamosa_storage::get_uri_media_ticket($app_id, $mediafile_id);
  }

  /**
   * @deprecated
   * @see self::get_uri_media_ticket_file()
   */
  public static function media_get_ticket_file($app_id, $ticket_id) {
    // Return link.
    return mediamosa_storage::get_uri_media_ticket_file($app_id, $ticket_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_status_file()
   */
  public static function status_file_location_get($filename) {
    return mediamosa_storage::get_uri_status_file($filename);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_scene_file()
   */
  public static function still_scene_file_location_get($job_id) {
    return mediamosa_storage::get_uri_scene_file($job_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_asx_file()
   */
  public static function asx_file_location($app_id, $ticket) {
    return mediamosa_storage::get_uri_asx_file($app_id, $ticket);
  }

  /**
   * @deprecated
   *
   * Note: $in_simpletest_sandbox will no longer work.
   *
   * @see mediamosa_storage::get_realpath()
   */
  public static function mount_point_get($in_simpletest_sandbox = FALSE) {
    return mediamosa_storage::get_realpath();
  }

  /**
   * @deprecated
   * @see mediamosa_storage::map_uuid_to_path()
   */
  public static function objectid_to_location($id, $ds = DIRECTORY_SEPARATOR) {
    return mediamosa_storage::map_uuid_to_path($id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_mediafile()
   */
  public static function mediafile_filename_get(array $mediafile) {
    return mediamosa_storage::get_uri_mediafile($mediafile);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_mediafile_id();
   */
  public static function mediafile_id_filename_get($mediafile_id) {
    return mediamosa_storage::get_uri_mediafile_id($mediafile_id);
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_data()
   */
  public static function get_data_location() {
    return mediamosa_storage::get_uri_data();
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_data_file()
   */
  public static function mediafile_still_filename_get($mediafile_id, $mount_point = '') {
    return mediamosa_storage::get_uri_data_file($mediafile_id, $mount_point);
  }

  /**
   * @deprecated
   */
  public static function ftp_location_get() {
    return 'ftp';
  }

  /**
   * @deprecated
   */
  public static function history_location_get() {
    return 'history';
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_ftp_history();
   */
  public static function get_full_ftp_path() {
    return mediamosa_storage::get_uri_ftp();
  }

  /**
   * @deprecated
   * @see mediamosa_storage::get_uri_ftp_history()
   */
  public static function get_full_history_path() {
    return mediamosa_storage::get_uri_ftp_history();
  }

  /**
   * @deprecated
   * @see
   */
  public static function current_job_path() {
    return mediamosa_storage::get_uri_ftp_history_job();
  }

  /**
   * @deprecated
   * @see
   */
  public static function file_get_htaccess_contents() {
    return mediamosa_storage::file_get_htaccess_contents();
  }

  /**
   * @deprecated
   * @see
   */
  public static function file_create_htaccess($directory, $contents) {
    return mediamosa_storage::file_create_htaccess($directory, $contents);
  }
}
