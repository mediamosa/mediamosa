<?php
// $Id$


/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Common function for sannas.
 */
/* Data locations
 */
class mediamosa_configuration_storage {

  // ------------------------------------------------------------------ Static Function.
  /**
   * Get the sannas setting for linux.
   *
   * @return string
   */
  public static function linux_get() {
    return mediamosa::variable_get('mediamosa_current_mount_point', '');
  }

  /**
   * Get the sannas setting for windows.
   *
   * @return string
   */
  public static function windows_get() {
    return mediamosa::variable_get('mediamosa_current_mount_point_windows', '');
  }

  /**
   * Return TRUE when mount point has been specified.
   */
  public static function is_mount_setup() {
    return mediamosa_configuration_storage::mount_point_get() != '';
  }

  /**
   * Set the sannas setting for linux.
   *
   * @param string $value
   */
  public static function linux_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point', $value);
  }

  /**
   * Set the sannas setting for windows.
   *
   * @param string $value
   */
  public static function windows_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point_windows', $value);
  }

  /**
   * Return the [mountpoint]/data folder.
   *
   * @return string
   */
  public static function data_location_get() {
    return mediamosa_settings::SAN_NAS_DATA_FOLDER . DIRECTORY_SEPARATOR;
  }

  /**
   * Return the [mountpoint]/data/still folder.
   *
   * @return string
   */
  public static function data_still_location_get() {
    return self::data_location_get() . mediamosa_settings::SAN_NAS_STILLS_FOLDER . DIRECTORY_SEPARATOR;
  }

  /**
   * Return the complete still data location folder.
   *
   * @return string
   */
  public static function data_still_get_path() {
    return self::mount_point_get() . self::data_still_location_get();
  }

  /**
   * Return the still file name location.
   *
   * @return string
   */
  public static function data_still_get_file($mediafile_id) {
    return self::data_still_get_path() . self::objectid_to_location($mediafile_id);
  }

  /**
   * Return the tmp transcode folder.
   */
  public static function transcode_tmp_location_get() {
    return self::data_location_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER . DIRECTORY_SEPARATOR;
  }

  /**
   * Return the default path for the play ticket.
   *
   * @return string
   */
  public static function ticket_play_location_get($app_id = -1) {
    return self::mount_point_get() . mediamosa_settings::PLAY_TICKET_LOCATION . DIRECTORY_SEPARATOR . ($app_id >= 0 ? $app_id . DIRECTORY_SEPARATOR : '');
  }

  /**
   * Return the default path for the download ticket.
   * Download dir same as play dir.
   *
   * @return string
   */
  public static function ticket_download_location_get($app_id = -1) {
    return self::mount_point_get() . mediamosa_settings::DOWNLOAD_TICKET_LOCATION . DIRECTORY_SEPARATOR;
  }

  /**
   * Return the path of the media directory.
   *
   * @param $app_id
   *   The application ID (optional).
   * @param $ticket_id
   *   The ticket ID (optional, required when app_id is provided).
   *
   * @return
   *   The path.
   */
  public static function media_get_path($app_id = -1, $mediafile_id = '') {
    $path = self::mount_point_get() . mediamosa_settings::SAN_NAS_MEDIA_FOLDER . DIRECTORY_SEPARATOR;

    // Full path?
    if ($app_id > 0) {
      $path .= $app_id . DIRECTORY_SEPARATOR . (empty($mediafile_id) ? '' : $mediafile_id[0] . DIRECTORY_SEPARATOR);
    }

    return $path;
  }

  /**
   * Return the full path and filename of the media.
   *
   * @param $app_id
   *   The application ID.
   * @param $mediafile_id
   *   The mediafile ID.
   *
   * @return
   *   The path.
   */
  public static function media_get_file($app_id, $mediafile_id) {
    // Return link.
    return self::media_get_path($app_id, $mediafile_id) . $mediafile_id;
  }

  /**
   * Return the path of the media directory.
   *
   * @param $app_id
   *   The application ID (optional).
   * @param $ticket_id
   *   The ticket ID (optional, required when app_id is provided).
   *
   * @return
   *   The path.
   */
  public static function media_get_ticket_path($app_id = -1, $mediafile_id = '') {
    $path = self::mount_point_get() . mediamosa_settings::SAN_NAS_MEDIA_FOLDER . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TICKET_FOLDER . DIRECTORY_SEPARATOR;

    // Full path?
    if ($app_id > 0) {
      assert(!empty($mediafile_id));
      $path .= $app_id . DIRECTORY_SEPARATOR . (empty($mediafile_id) ? '' : $mediafile_id[0] . DIRECTORY_SEPARATOR);
    }

    return $path;
  }

  /**
   * Return the full path and filename of the media.
   *
   * @param $app_id
   *   The application ID.
   * @param $ticket_id
   *   The ticket ID.
   *
   * @return
   *   The path.
   */
  public static function media_get_ticket_file($app_id, $ticket_id) {
    // Return link.
    return self::media_get_ticket_path($app_id, $ticket_id) . $ticket_id;
  }

  /**
   * Returns the full path and filename of the status file of
   * given filename.
   *
   * @param string $filename
   */
  public static function status_file_location_get($filename) {
    return self::transcode_file_location_get($filename) . '.status';
  }

  /**
   * Returns the full path and filename of the transcode file of
   * given filename.
   *
   * FIXME: also used for stills, should be called jobprocessor tmp?
   *
   * @param string $filename
   */
  public static function transcode_file_location_get($filename) {
    return mediamosa_configuration_storage::mount_point_get() . mediamosa_configuration_storage::transcode_tmp_location_get() . DIRECTORY_SEPARATOR . $filename;
  }

  /**
   * Returns the full path and filename of the scene file of
   * given job.
   *
   * @param integer $job_id
   */
  public static function still_scene_file_location_get($job_id) {
    return mediamosa_configuration_storage::mount_point_get() . mediamosa_configuration_storage::transcode_tmp_location_get() . DIRECTORY_SEPARATOR . $job_id . '_scene.txt';
  }

  /**
   * Return file location of asx file.
   * The ASX file is stored in still directory, true, must be downloadable.
   *
   * @return string
   *  path to ticket location
   */
  public static function asx_file_location($app_id, $ticket) {
    return self::media_get_file($app_id, $ticket) . '.asx';
  }

  /**
   * Return the moint point path based on OS.
   *
   * Will return path to sandbox if in simpletest.
   *
   * @return string
   */
  public static function mount_point_get($in_simpletest_sandbox = FALSE) {
    $path = rtrim(mediamosa::is_windows_platform() ? self::windows_get() : self::linux_get(), '/');
    return (mediamosa::in_simpletest_sandbox() || $in_simpletest_sandbox ? $path . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_MEDIA_FOLDER . DIRECTORY_SEPARATOR . 'simpletest' . DIRECTORY_SEPARATOR : $path . DIRECTORY_SEPARATOR);
  }

  /**
   * Map the object ID to store file into location and
   * filename. Format: ID[0]/ID
   *
   * @param string $id
   */
  public static function objectid_to_location($id, $ds = DIRECTORY_SEPARATOR) {
    return $id[0] . $ds . $id;
  }

  /**
   * Get the Mediafile mountpath path.
   *
   * @param $mediafile
   *   The mediafile array or simular, must contain at least 'mediafile_id'.
   */
  public static function mediafile_filename_get(array $mediafile) {
    // Get the mount point either from mediafile or our default setting.
    $mount_point = empty($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]) ? self::mount_point_get() : rtrim($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT], DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;

    return $mount_point . self::data_location_get() . DIRECTORY_SEPARATOR . self::objectid_to_location($mediafile[mediamosa_asset_mediafile_db::ID]);
  }

  /**
   * Get the Mediafile mountpath path.
   *
   * @param $mediafile_id
   *   The mediafile_id.
   */
  public static function mediafile_id_filename_get($mediafile_id) {
    $file = self::mount_point_get() . DIRECTORY_SEPARATOR . self::data_location_get() . DIRECTORY_SEPARATOR . self::objectid_to_location($mediafile_id);

    // FIXME: Is this really required?
    mediamosa_io::mkdir(mediamosa_io::dirname($file));
    return $file;
  }

  /**
   * Get the data location path.
   */
  public static function get_data_location() {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . self::data_location_get() . DIRECTORY_SEPARATOR;
  }

  /**
   * Get the Mediafile still path.
   *
   * @param $mediafile_id
   *   The mediafile ID.
   */
  public static function mediafile_still_filename_get($mediafile_id, $mount_point = '') {
    $mount_point = empty($mount_point) ? self::mount_point_get() : rtrim($mount_point, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
    return $mount_point . self::data_still_location_get() . DIRECTORY_SEPARATOR . self::objectid_to_location($mediafile_id);
  }

  /**
   * Get the FTP location.
   */
  public static function ftp_location_get() {
    return mediamosa_settings::FTP_ROOT_DIR;
  }

  /**
   * Get the (FTP) history location.
   */
  public static function history_location_get() {
    return mediamosa_settings::FTP_HISTORY_DIR;
  }

  /**
   * Get the SAN/NAS FTP path.
   */
  public static function get_full_ftp_path() {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . self::ftp_location_get();
  }

  /**
   * Get the SAN/NAS history path.
   */
  public static function get_full_history_path() {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . self::ftp_location_get() . DIRECTORY_SEPARATOR . self::history_location_get();
  }

  /**
   * Get the current job path.
   */
  public static function current_job_path() {
    return self::get_full_history_path() . DIRECTORY_SEPARATOR . mediamosa_settings::FTP_CURRENT_JOB_DIR;
  }

  /**
   * Returns the contents for the .htaccess file under [mountpoint]/media.
   *
   * @return
   *   The contents for the .htaccess file for [mount_pount]/media/.htaccess.
   */
  public static function media_get_htaccess_contents() {
    $subdir = trim(url(), '/');
    $ht_access = file_get_contents(drupal_get_path('module', 'mediamosa') . '/lib/misc/media_htaccess_contents.txt');
    return strtr($ht_access, array('{SUBDIR}' => $subdir == '' ? '' : '/' . $subdir));
  }

  /**
   * Creates an .htaccess file in the given directory using the given contents.
   *
   * @param $directory
   *   The directory.
   * @param $contents
   *   The contents to write.
   */
  public static function file_create_htaccess($directory, $contents) {
    if (file_uri_scheme($directory)) {
      $directory = file_stream_wrapper_uri_normalize($directory);
    }
    else {
      $directory = rtrim($directory, '/\\');
    }
    $htaccess_path =  $directory . '/.htaccess';

    // If file already exists, then bail out.
    if (file_exists($htaccess_path)) {
      return TRUE;
    }

    // Write the .htaccess file.
    if (file_put_contents($htaccess_path, $contents)) {
      drupal_chmod($htaccess_path, 0444);
      return TRUE;
    }

    // Make log entry about error.
    $variables = array('%directory' => $directory, '@path' => $htaccess_path, '!htaccess' => '<br />' . nl2br(check_plain($contents)));
    mediamosa_watchdog::log("Warning: Couldn't write .htaccess file in location @path. Please create a .htaccess file in your %directory directory which contains the following lines: <code>!htaccess</code>", $variables, WATCHDOG_ERROR);
    return FALSE;
  }
}
