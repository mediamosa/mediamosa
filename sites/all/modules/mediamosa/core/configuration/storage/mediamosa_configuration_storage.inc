<?php
// $Id$


/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  static public function linux_get() {
    return mediamosa::variable_get('mediamosa_current_mount_point', '');
  }

  /**
   * Get the sannas setting for windows.
   *
   * @return string
   */
  static public function windows_get() {
    return mediamosa::variable_get('mediamosa_current_mount_point_windows', '');
  }

  /**
   * Return TRUE when mount point has been specified.
   */
  static public function is_mount_setup() {
    return mediamosa_configuration_storage::mount_point_get() != '';
  }

  /**
   * Set the sannas setting for linux.
   *
   * @param string $value
   */
  static public function linux_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point', $value);
  }

  /**
   * Set the sannas setting for windows.
   *
   * @param string $value
   */
  static public function windows_set($value) {
    mediamosa::variable_set('mediamosa_current_mount_point_windows', $value);
  }

  /**
   * Return the data location folder.
   * Based on 1.x define DATA_LOCATION.
   *
   * @return string
   */
  static public function data_location_get() {
    return mediamosa_settings::SAN_NAS_DATA_FOLDER;
  }

  /**
   * Return the still file name location.
   *
   * @return string
   */
  static public function still_filename_get($still_id) {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . self::still_location_get() . DIRECTORY_SEPARATOR . self::objectid_to_location($still_id);
  }

  /**
   * Return the still data location folder.
   * Based on the 1.x define STILL_LOCATION.
   *
   * @return string
   */
  static public function still_location_get() {
    return self::data_location_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_STILLS_FOLDER;
  }

  /**
   * Return the tmp transcode folder.
   */
  static public function transcode_tmp_location_get() {
    return self::data_location_get() . DIRECTORY_SEPARATOR . mediamosa_settings::SAN_NAS_TRANSCODE_FOLDER;
  }

  /**
   * Return the default path for the play ticket.
   * Based on the 1.x function _play_proxy_play_ticket_location.
   *
   * @return string
   */
  static public function ticket_play_location_get($app_id = -1) {
    if ($app_id >= 0) {
      return self::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::PLAY_TICKET_LOCATION . DIRECTORY_SEPARATOR . $app_id;
    }

    return self::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::PLAY_TICKET_LOCATION;
  }

  /**
   * Return the default path for the download ticket.
   * Download dir same as play dir.
   *
   * @return string
   */
  static public function ticket_download_location_get($app_id = -1) {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::DOWNLOAD_TICKET_LOCATION . DIRECTORY_SEPARATOR;
  }

  /**
   * Return the default path for the ticket stills.
   *
   * @return string
   */
  static public function ticket_still_location_get() {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::STILL_TICKET_LOCATION . DIRECTORY_SEPARATOR;
  }

  /**
   * Returns the full path and filename of the status file of
   * given filename.
   *
   * @param string $filename
   */
  static public function status_file_location_get($filename) {
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
  static public function transcode_file_location_get($filename) {
    return mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::transcode_tmp_location_get() . DIRECTORY_SEPARATOR . $filename;
  }

  /**
   * Returns the full path and filename of the scene file of
   * given job.
   *
   * @param integer $job_id
   */
  static public function still_scene_file_location_get($job_id) {
    return mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::transcode_tmp_location_get() . DIRECTORY_SEPARATOR . $job_id . '_scene.txt';
  }

  /**
   * Return file location of asx file.
   * The ASX file is stored in still directory, true, must be downloadable.
   *
   * @return string
   *  path to ticket location
   */
  function asx_file_location($ticket) {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_settings::STILL_TICKET_LOCATION . DIRECTORY_SEPARATOR . $ticket . '.asx';
  }

  /**
   * Return the moint point path based on OS.
   *
   * Will return path to sandbox if in simpletest.
   *
   * @return string
   */
  static public function mount_point_get($in_simpletest_sandbox = FALSE) { // vpx_get_san_nas_base_path
    $path = mediamosa::is_windows_platform() ? self::windows_get() : self::linux_get();
    return (mediamosa::in_simpletest_sandbox() || $in_simpletest_sandbox ? $path . DIRECTORY_SEPARATOR . self::data_location_get() . DIRECTORY_SEPARATOR . 'simpletest' : $path);
  }

  /**
   * Map the object ID to store file into location and
   * filename. Format: ID[0]/ID
   *
   * @param string $id
   */
  static public function objectid_to_location($id) {
    return $id[0] . DIRECTORY_SEPARATOR . $id;
  }

  /**
   * Get the Mediafile mountpath path.
   *
   * @param array $mediafile
   *  The mediafile array or simular, must contain at least 'mediafile_id'.
   */
  static public function mediafile_filename_get($mediafile) {
    if (!isset($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]) || empty($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT])) {
      $mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT] = self::mount_point_get();
    }

    return $mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT] . DIRECTORY_SEPARATOR . self::data_location_get() . DIRECTORY_SEPARATOR . self::objectid_to_location($mediafile[mediamosa_asset_mediafile_db::ID]);
  }

  /**
   * Get the Mediafile mountpath path.
   *
   * @param string $mediafile_id
   *  The mediafile_id.
   */
  static public function mediafile_id_filename_get($mediafile_id) {
    $directory = self::mount_point_get() . DIRECTORY_SEPARATOR . self::data_location_get() . DIRECTORY_SEPARATOR;
    mediamosa_io::mkdir($directory . $mediafile_id[0]);
    return $directory . self::objectid_to_location($mediafile_id);
  }

  /**
   * Get the data location path.
   */
  static public function get_data_location() {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . self::data_location_get();
  }

  /**
   * Get the still location path.
   */
  static public function get_still_location() {
  	return self::mount_point_get() . DIRECTORY_SEPARATOR . self::still_location_get();
  }

  /**
   * Get the Mediafile still path.
   * @param string $mediafile_id
   *  The mediafile ID.
   */
  static public function mediafile_still_filename_get($mediafile_id, $mount_point = '') {
    return (empty($mount_point) ? self::mount_point_get() : $mount_point) . DIRECTORY_SEPARATOR . self::still_location_get() . DIRECTORY_SEPARATOR . self::objectid_to_location($mediafile_id);
  }

  /**
   * Get the FTP location.
   */
  static public function ftp_location_get() {
    return mediamosa_settings::FTP_ROOT_DIR;
  }

  /**
   * Get the (FTP) history location.
   */
  static public function history_location_get() {
    return mediamosa_settings::FTP_HISTORY_DIR;
  }

  /**
   * Get the SAN/NAS FTP path.
   */
  static public function get_full_ftp_path() {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . self::ftp_location_get();
  }

  /**
   * Get the SAN/NAS history path.
   */
  static public function get_full_history_path() {
    return self::mount_point_get() . DIRECTORY_SEPARATOR . self::ftp_location_get() . DIRECTORY_SEPARATOR . self::history_location_get();
  }

  /**
   * Get the current job path.
   */
  static public function current_job_path() {
    return self::get_full_history_path() . DIRECTORY_SEPARATOR . mediamosa_settings::FTP_CURRENT_JOB_DIR;
  }
}
