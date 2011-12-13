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
 * Integrity check PHP file
 */

// Basic values.
$_SERVER['HTTP_HOST'] = 'localhost';
$_SERVER['REMOTE_ADDR'] = '';
$_SERVER['REQUEST_METHOD'] = '';
$_SERVER['SERVER_SOFTWARE'] = '';

// get a working Drupal environment
function find_drupal() {
  while (($path = getcwd()) !== '/') {
    if (file_exists('index.php') && is_dir('includes')) {
      break;
    }
    chdir('..');
  }
  return $path;
}

// Get Drupal.
define('DRUPAL_ROOT', find_drupal());
chdir(DRUPAL_ROOT);
include_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

// 2 hour time limit (exclusive queries).
set_time_limit(7200);

// Error reporting.
error_reporting(E_ALL);

// Check media record.
function check_media_records() {
  $result = mediamosa_db::db_query('
    SELECT #mediafile_id, #created, #changed, #app_id, #owner_id
    FROM {#mediafile}
    WHERE #is_still = :is_still_false', array(
    '#mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
    '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
    '#created' => mediamosa_asset_mediafile_db::CREATED,
    '#changed' => mediamosa_asset_mediafile_db::CHANGED,
    '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
    '#owner_id' => mediamosa_asset_mediafile_db::OWNER_ID,
    '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
    ':is_still_false' => mediamosa_asset_mediafile_db::IS_STILL_FALSE,
  ));

  foreach ($result as $mediafile) {
    // Check if file exists.
    if (!file_exists(mediamosa_configuration_storage::mediafile_id_filename_get($mediafile['mediafile_id']))) {
      $mediafile_id = $mediafile['mediafile_id'];
      $filesize = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile_id, mediamosa_asset_mediafile_metadata::FILESIZE);
      $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::MIME_TYPE);

      mediamosa_db::db_query('
        INSERT INTO {#mediamosa_integrity_check}
          (#type, #object_id, #app_id, #owner_id, #created, #changed, #details) VALUES
          (:missing_mediafile, :object_id, :app_id, :owner_id, :created, :changed, :details)', array(
          '#mediamosa_integrity_check' => mediamosa_integrity_check_db::TABLE_NAME,
          '#type' => mediamosa_integrity_check_db::TYPE,
          '#object_id' => mediamosa_integrity_check_db::OBJECT_ID,
          '#app_id' => mediamosa_integrity_check_db::APP_ID,
          '#owner_id' => mediamosa_integrity_check_db::OWNER_ID,
          '#created' => mediamosa_integrity_check_db::CREATED,
          '#changed' => mediamosa_integrity_check_db::CHANGED,
          '#details' => mediamosa_integrity_check_db::DETAILS,
          ':missing_mediafile' => mediamosa_integrity_check_db::TYPE_MISSING_MEDIAFILE,
          ':object_id' => $mediafile['mediafile_id'],
          ':app_id' => $mediafile['app_id'],
          ':owner_id' => $mediafile['owner_id'],
          ':created' => $mediafile['created'],
          ':changed' => $mediafile['changed'],
          ':details' => (!$filesize || $filesize == '') ? 'Never succesfully analysed...' : 'Mime-type: ' . $mime_type,
      ));
    }
    // Sleep 0.01 seconds *100.000 mediafiles= 17 minuten.
    usleep(10000);
  }
}

// Check media files.
function check_media_files() {
  // Base folder.
  $dir = mediamosa_configuration_storage::get_data_location();
  $dh = opendir($dir);
  $missing_db_mediafiles = array();

  while (($folder = readdir($dh)) !== FALSE) {
    // Is it '.' or '..'?
    if (!is_dir($dir . DIRECTORY_SEPARATOR . $folder) || strpos($folder, '..') === 0 || strpos($folder, '.') === 0 || drupal_strlen($folder) > 1) {
      continue;
    }

    // Open the sub directory.
    $fh = opendir($dir . DIRECTORY_SEPARATOR . $folder);
    while (($file = readdir($fh)) !== FALSE) {

      // Is it '.' or '..'?
      if (strpos($file, '.') === 0 || strpos($file, '..') === 0) {
        continue;
      }

      // Check the file in the db.
      $result = mediamosa_db::db_query('
        SELECT COUNT(*)
        FROM {#mediafile}
        where #mediafile_id = :mediafile_id', array(
        '#mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $file,
      ));

      if ($result->fetchField() == 0) {
        // Collect the data.
        $file_path = $dir . DIRECTORY_SEPARATOR . $folder . DIRECTORY_SEPARATOR . $file;
        $finfo = stat($file_path);
        $more_info = exec('ls -sla ' . $file_path);
        // Make error message.
        mediamosa_db::db_query('
          INSERT INTO {#mediamosa_integrity_check}
            (#type, #object_id, #size, #mtime, #ctime, #details, #created) VALUES
            (:missing_mediarecord, :object_id, :size, :mtime, :ctime, :details, UTC_TIMESTAMP())', array(
            '#mediamosa_integrity_check' => mediamosa_integrity_check_db::TABLE_NAME,
            '#type' => mediamosa_integrity_check_db::TYPE,
            '#object_id' => mediamosa_integrity_check_db::OBJECT_ID,
            '#size' => mediamosa_integrity_check_db::SIZE,
            '#mtime' => mediamosa_integrity_check_db::MTIME,
            '#ctime' => mediamosa_integrity_check_db::CTIME,
            '#details' => mediamosa_integrity_check_db::DETAILS,
            '#created' => mediamosa_integrity_check_db::CREATED,
            ':missing_mediarecord' => mediamosa_integrity_check_db::TYPE_MISSING_MEDIARECORD,
            ':object_id' => $file,
            ':size' => $finfo['size'],
            ':mtime' => $finfo['mtime'],
            ':ctime' => $finfo['ctime'],
            ':details' => $more_info,
        ));
      }
    }

    closedir($fh);
  }

  closedir($dh);
}

function check_still_records() {
  $result = mediamosa_db::db_query("
    SELECT #mediafile_id, #created, #changed, #app_id, #owner_id
    FROM {#mediafile}
    WHERE #is_still = :is_still_true", array(
    '#mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
    '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
    '#created' => mediamosa_asset_mediafile_db::CREATED,
    '#changed' => mediamosa_asset_mediafile_db::CHANGED,
    '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
    '#owner_id' => mediamosa_asset_mediafile_db::OWNER_ID,
    '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
    ':is_still_true' => mediamosa_asset_mediafile_db::IS_STILL_TRUE,
  ));

  foreach ($result as $still) {
    // Check if file exists.
    if (!file_exists(mediamosa_configuration_storage::data_still_get_file($still['mediafile_id']))) {
      $mediafile_id = $still['mediafile_id'];
      $filesize = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile_id, mediamosa_asset_mediafile_metadata::FILESIZE);
      $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::MIME_TYPE);

      mediamosa_db::db_query('
        INSERT INTO {#mediamosa_integrity_check}
          (#type, #object_id, #app_id, #owner_id, #created, #changed, #details) VALUES
          (:missing_mediafile, :object_id, :app_id, :owner_id, :created, :changed, :details)', array(
          '#mediamosa_integrity_check' => mediamosa_integrity_check_db::TABLE_NAME,
          '#type' => mediamosa_integrity_check_db::TYPE,
          '#object_id' => mediamosa_integrity_check_db::OBJECT_ID,
          '#app_id' => mediamosa_integrity_check_db::APP_ID,
          '#owner_id' => mediamosa_integrity_check_db::OWNER_ID,
          '#created' => mediamosa_integrity_check_db::CREATED,
          '#changed' => mediamosa_integrity_check_db::CHANGED,
          '#details' => mediamosa_integrity_check_db::DETAILS,
          ':missing_mediafile' => mediamosa_integrity_check_db::TYPE_MISSING_STILLFILE,
          ':object_id' => $still['mediafile_id'],
          ':app_id' => $still['app_id'],
          ':owner_id' => $still['owner_id'],
          ':created' => $still['created'],
          ':changed' => $still['changed'],
          ':details' => (!$filesize || $filesize == '') ? 'Never succesfully analysed...' : 'Mime-type: ' . $mime_type,
      ));
    }
    // Sleep 0.01 seconds *100.000 mediafiles= 17 minuten.
    usleep(10000);
  }
}

function check_still_files() {
  // Base folder.
  $dir = mediamosa_configuration_storage::get_still_location();
  $dh = opendir($dir);

  while (($folder = readdir($dh)) !== FALSE) {
    // Is it '.' or '..'?
    if (!is_dir($dir . DIRECTORY_SEPARATOR . $folder) || strpos($folder, '..') === 0 || strpos($folder, '.') === 0 || drupal_strlen($folder) > 1) {
      continue;
    }

    // Open the sub directory.
    $fh = opendir($dir . DIRECTORY_SEPARATOR . $folder);
    while (($file = readdir($fh)) !== FALSE) {

      // Is it '.' or '..'?
      if (strpos($file, '.') === 0 || strpos($file, '..') === 0) {
        continue;
      }

      // Check the file in the db.
      $result = mediamosa_db::db_query('
        SELECT COUNT(*)
        FROM {#mediafile}
        where #mediafile_id = :mediafile_id', array(
        '#mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $file,
      ));

      if ($result->fetchField() == 0) {
        // Collect the data.
        $file_path = $dir . DIRECTORY_SEPARATOR . $folder . DIRECTORY_SEPARATOR . $file;
        $finfo = stat($file_path);
        $more_info = exec('ls -sla ' . $file_path);
        // Make error message.
        mediamosa_db::db_query('
          INSERT INTO {#mediamosa_integrity_check}
            (#type, #object_id, #size, #mtime, #ctime, #details, #created) VALUES
            (:missing_mediarecord, :object_id, :size, :mtime, :ctime, :details, UTC_TIMESTAMP())', array(
            '#mediamosa_integrity_check' => mediamosa_integrity_check_db::TABLE_NAME,
            '#type' => mediamosa_integrity_check_db::TYPE,
            '#object_id' => mediamosa_integrity_check_db::OBJECT_ID,
            '#size' => mediamosa_integrity_check_db::SIZE,
            '#mtime' => mediamosa_integrity_check_db::MTIME,
            '#ctime' => mediamosa_integrity_check_db::CTIME,
            '#details' => mediamosa_integrity_check_db::DETAILS,
            '#created' => mediamosa_integrity_check_db::CREATED,
            ':missing_mediarecord' => mediamosa_integrity_check_db::TYPE_MISSING_STILLRECORD,
            ':object_id' => $file,
            ':size' => $finfo['size'],
            ':mtime' => $finfo['mtime'],
            ':ctime' => $finfo['ctime'],
            ':details' => $more_info,
        ));
      }
    }

    closedir($fh);
  }

  closedir($dh);
}

// Start.
watchdog('integrity_check', 'running...');
variable_set('mediamosa_integrity_run_date_start', date('c'));

// Empty log table.
db_truncate('mediamosa_integrity_check');

// Run checks.
check_media_records();
check_media_files();
check_still_records();
check_still_files();

// End.
variable_set('mediamosa_integrity_run_date_end', date('c'));
watchdog('integrity_check', 'ended...');
