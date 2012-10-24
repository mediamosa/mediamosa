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
  * The job module.
  */
class mediamosa_job {

  // ------------------------------------------------------------------ Consts.
  // Indicate windows tool.
  const JOBWINDOWS_TOOL = 'windows';

  // Raw job
  const JOBRAW_TOOL = 'ffmpeg';
  const JOBRAW_FILE_EXTENSION = 'avi';
  const JOBRAW_COMMAND = 'videocodec:rawvideo;audiocodec:pcm_s16le;internal_previous_job:%d';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Log for job.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'job');
  }

  /**
   * Log mediafile ids.
   *
   * @param string $mediafile_id
   * @param string $asset_id
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'job');
  }

  /**
   * Make sure the job exists.
   *
   * @param integer $job_id
   *
   * @return array
   *  The found job.
   */
  public static function must_exists($job_id) {
    return mediamosa_db::db_must_exists(mediamosa_job_db::TABLE_NAME, array(mediamosa_job_db::ID => $job_id));
  }

  /**
   * Get the job from the main job table.
   * @param integer $job_id
   *  The job ID.
   */
  public static function get($job_id, array $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the job from the main job table.
   * @param integer $mediafile_id
   *  The mediafile ID.
   */
  public static function get_by_mediafileid($mediafile_id, array $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $fields)
      ->condition(mediamosa_job_db::MEDIAFILE_ID, $mediafile_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the job from the main job table.
   * @param integer $mediafile_id
   *  The mediafile ID.
   */
  public static function get_by_mediafileid_all($mediafile_id, array $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $fields)
      ->condition(mediamosa_job_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();
  }

  /**
   * Get the failed transcode jobs.
   *
   * @param $visibility
   * @param $order_by
   * @param $order_direction
   *
   * @return $result
   */
  public static function get_failed($visibility, $limit, $offset, $order_by = mediamosa_job_db::ID, $order_direction = 'asc') {

    // Build the query.
    $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'job');

    // Joins.
    $query->join(mediamosa_asset_mediafile_db::TABLE_NAME, 'mediafile', 'mediafile.' . mediamosa_asset_mediafile_db::ID . ' = job.' . mediamosa_job_db::MEDIAFILE_ID);
    $query->leftjoin(mediamosa_job_display_db::TABLE_NAME, 'display', 'display.' . mediamosa_job_display_db::ID . ' = job.' . mediamosa_job_db::ID);

    $query->fields('job', array(
      mediamosa_job_db::ID,
      mediamosa_job_db::APP_ID,
      mediamosa_job_db::JOB_TYPE,
      mediamosa_job_db::ASSET_ID,
      mediamosa_job_db::MEDIAFILE_ID,
      mediamosa_job_db::STARTED,
      mediamosa_job_db::CHANGED,
      mediamosa_job_db::ERROR_DESCRIPTION,
    ));
    $query->fields('mediafile', array(mediamosa_asset_mediafile_db::FILENAME));

    //$query->where('job.job_id = (SELECT max(j.job_id) FROM mediamosa_job j WHERE j.mediafile_id = job.mediafile_id AND j.job_type = job.job_type)');
    $query->where('job.' . mediamosa_job_db::ID . ' = (SELECT MAX(j.' . mediamosa_job_db::ID . ') FROM ' . mediamosa_job_db::TABLE_NAME . ' j WHERE j.' . mediamosa_job_db::MEDIAFILE_ID . ' = job.' . mediamosa_job_db::MEDIAFILE_ID . ' AND j.' . mediamosa_job_db::JOB_TYPE . ' = job.' . mediamosa_job_db::JOB_TYPE . ')');

    if ($visibility == mediamosa_job_display_db::VISIBILITY_HIDDEN) {
      $query->condition('display.' . mediamosa_job_display_db::VISIBILITY, mediamosa_job_display_db::VISIBILITY_HIDDEN);
    }
    else {
      $query->isNull('display.' . mediamosa_job_display_db::VISIBILITY);
    }

    $job_type_or = db_or();
    $job_type_or->condition('job.' . mediamosa_job_db::JOB_TYPE, mediamosa_job_db::JOB_TYPE_ANALYSE);
    $job_type_or->condition('job.' . mediamosa_job_db::JOB_TYPE, mediamosa_job_db::JOB_TYPE_TRANSCODE);
    $query->condition($job_type_or);

    $job_status_or = db_or();
    $job_status_or->condition('job.' . mediamosa_job_db::JOB_STATUS, mediamosa_job_db::JOB_STATUS_FAILED);
    $job_status_or->condition('job.' . mediamosa_job_db::JOB_STATUS, mediamosa_job_db::JOB_STATUS_CANCELLED);
    $query->condition($job_status_or);

    $query->condition('mediafile.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

    // Order_by and order_direction.
    if ($order_by) {
      $query->orderBy($order_by, $order_direction);
    }

    // Number of total result.
    $num_rows = $query->countQuery()->execute()->fetchField();

    // Range.
    $query->range($offset, $limit);

    $result = $query->execute();

    return array(
      'item_count_total' => $num_rows,
      'result' => $result,
    );
  }

  /**
   * Store the error description.
   *
   * @param integer $job_id
   *  The job ID.
   * @param string $error_description
   *  Text to store.
   */
  public static function store_error_description($job_id, $error_description) {
    $fields = array(
      mediamosa_job_db::ERROR_DESCRIPTION => $error_description,
    );

    // Add changed.
    $fields = mediamosa_db::db_update_enrich($fields);

    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_db::ID, $job_id);
  }

  /**
   * Get all information of this job in the database.
   *
   * @param integer $job_id
   */
  public static function get_job_ext($job_id) {

    // Get the basic job.
    $job_ext = self::get($job_id);

    // Any extra information stored here.
    $a_job_child = NULL;

    // Based on type, we will get the sub-table info.
    switch ($job_ext[mediamosa_job_db::JOB_TYPE]) {
      case mediamosa_job_db::JOB_TYPE_STILL:
        $a_job_child = mediamosa_job_still::get($job_id);
        break;

      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $a_job_child = mediamosa_job_transcode::get($job_id);
        break;

      case mediamosa_job_db::JOB_TYPE_UPLOAD:
        $a_job_child = mediamosa_job_upload::get($job_id);
        break;
    }

    // If set, then merge.
    if ($a_job_child) {
      // Lapping info will prefer what is in job_ext.
      return array_merge($a_job_child, $job_ext);
    }

    return $job_ext;
  }

  /**
   * Update the progress in the job.
   *
   * @param integer $job_id
   *  The job ID.
   * @param string $progress
   *  In format x.xxx (0.000 to 1.000)
   * @param bool $started
   */
  public static function set_progress($job_id, $progress, $started = FALSE, $job_status = NULL) {
    assert(mediamosa_unicode::strlen($progress) <= 5);

    $fields = array(
      mediamosa_job_db::PROGRESS => $progress,
    );

    if ($started) {
      $fields[mediamosa_job_db::STARTED] = mediamosa_datetime::utc_current_timestamp_now();
      $fields[mediamosa_job_db::JOB_STATUS] = mediamosa_job_db::JOB_STATUS_INPROGRESS;
    }

    if ($job_status) {
      $fields[mediamosa_job_db::JOB_STATUS] = $job_status;
    }

    return mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute();
  }

  /**
   * Create an extra raw job for windows transcoding.
   *
   * @param integer $job_id
   *  The job ID.
   */
  public static function create_raw($job_id) {
    // Update job for progress.
    self::set_progress($job_id, '0.333', TRUE);

    // Get the job.
    $a_job = self::get($job_id);
    if (!$a_job) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_NOT_FOUND, array('@job_id' => $job_id));
    }

    // Fields to insert.
    $fields = array(
      mediamosa_job_db::ASSET_ID => $a_job[mediamosa_job_db::ASSET_ID],
      mediamosa_job_db::MEDIAFILE_ID => $a_job[mediamosa_job_db::MEDIAFILE_ID],
      mediamosa_job_db::OWNER_ID => $a_job[mediamosa_job_db::OWNER_ID],
      mediamosa_job_db::APP_ID => $a_job[mediamosa_job_db::APP_ID],
      mediamosa_job_db::PRIORITY => $a_job[mediamosa_job_db::PRIORITY],
      mediamosa_job_db::JOB_TYPE => $a_job[mediamosa_job_db::JOB_TYPE],
    );

    // Add created/changed.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    $job_id_raw = mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Create a new transcode job.
    mediamosa_job_transcode::create(
      $a_job[mediamosa_job_db::APP_ID],
      $job_id_raw,
      mediamosa_job::JOBRAW_FILE_EXTENSION,
      sprintf(mediamosa_job::JOBRAW_COMMAND, $job_id),
      mediamosa_job::JOBRAW_TOOL
    );
  }

  /**
   * Add retranscode jobs.
   *
   * @param string $asset_id
   *  The asset ID.
   * @param string $mediafile_id
   *  The mediafile ID.
   * @param string $user_id
   *  The owner.
   * @param integer $app_id
   *  The application ID.
   */
  public static function add_retranscode_jobs($asset_id, $mediafile_id, $user_id, $app_id) {
    // Get all mediafiles that are transcoded, so we can retranscode them.
    $mediafiles = mediamosa_asset_mediafile::get_all_transcoded($asset_id);

    $counter = 0;
    foreach ($mediafiles as $mediafile) {
      $counter++;

      // Create new job.
      try {
        mediamosa_job::create_job_transcode($app_id, $user_id, '', FALSE, $mediafile_id, FALSE, array(), $mediafile[mediamosa_asset_mediafile_db::FILE_EXTENSION], $mediafile[mediamosa_asset_mediafile_db::COMMAND], $mediafile[mediamosa_asset_mediafile_db::TOOL]);
      }
      catch (mediamosa_exception $e) {
        // ignore..
      }
    }

    return $counter;
  }

  /**
   * Create a transcode job
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param string $file_extension
   * @param string $command
   * @param string $tool
   * @param integer $profile_id
   * @param string $completed_transcoding_url
   * @param string $priority
   */
  public static function create_job_transcode($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $file_extension, $command, $tool = '', $profile_id = '', $completed_transcoding_url = '', $priority = NULL, $hint = FALSE, $mediafile_dest = NULL) {

    // Must have user_quota.
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Must be transcodable.
    mediamosa_asset_mediafile_metadata::is_transcodable($mediafile_id);

    // Create job.
    $job_id = self::create(mediamosa_job_db::JOB_TYPE_TRANSCODE, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $priority, $hint, $mediafile_dest);

    // Create sub transcode job.
    mediamosa_job_transcode::create($app_id, $job_id, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $mediafile_id);

    return $job_id;
  }

  /**
   * Create a retranscode job
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $user_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param string $file_extension
   * @param string $command
   * @param string $tool
   * @param integer $profile_id
   * @param string $completed_transcoding_url
   * @param string $priority
   */
  public static function create_job_retranscode($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $file_extension, $command, $tool = '', $profile_id = '', $completed_transcoding_url = '', $priority = NULL) {

    // Must have user_quota.
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Move the transcoded mediafile_id to mediafile_dest.
    $mediafile_dest = $mediafile_id;

    // Get the source of the transcoded file.
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id, $app_id, array(mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE), mediamosa_asset_mediafile_db::IS_STILL_FALSE);
    if (!$mediafile) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_TRANSCODE_MEDIAFILE, array('@mediafile_id' => $mediafile_dest, '@reason' => 'this mediafile is a still'));
    }
    $mediafile_id_source = $mediafile[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];
    if (!$mediafile_id_source) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CANT_TRANSCODE_MEDIAFILE, array('@mediafile_id' => $mediafile_dest, '@reason' => 'this mediafile is an original mediafile'));
    }

    // Make sure the mediafile exists, returns the mediafile.
    $mediafile_source = mediamosa_asset_mediafile::must_exists($mediafile_id_source, $app_id);

    // Must be transcodable.
    mediamosa_asset_mediafile_metadata::is_transcodable($mediafile_id_source);

    // Get the transcode_profile_id.
    if (!$profile_id) {
      $profile = mediamosa_asset_mediafile::get($mediafile_dest, $app_id, array(mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID));
      if ($profile) {
        $profile_id = $profile[mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID];
      }
    }

    // Create job.
    $job_id = self::create(mediamosa_job_db::JOB_TYPE_TRANSCODE, $app_id, $user_id, $is_app_admin, $mediafile_id_source, $create_still, $still_parameters, $priority, FALSE, $mediafile_dest);

    // Create sub transcode job.
    mediamosa_job_transcode::create($app_id, $job_id, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $mediafile_id_source);

    return $job_id;
  }

  /**
   * Decide wheter the still is creatable from the mediafile or not.
   *
   * @param string $mediafile_id
   *
   * @return boolean
   *   TRUE in case of still is creatable.
   */
  public static function is_still_creatable($mediafile_id) {
    $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::MIME_TYPE);

    $pos = strpos($mime_type, '/');
    if ($pos !== FALSE) {
      $mime_type = drupal_substr($mime_type, 0, $pos);
    }

    return ($mime_type != mediamosa_lib::MIME_TYPE_AUDIO);
  }

  /**
   * Create still job.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param array $still_parameters
   * @param integer $priority
   */
  public static function create_job_still($app_id, $user_id, $is_app_admin, $mediafile_id, array $still_parameters = array(), $priority = NULL) {
    if (self::is_still_creatable($mediafile_id)) {
      // Create job.
      $job_id = self::create(mediamosa_job_db::JOB_TYPE_STILL, $app_id, $user_id, $is_app_admin, $mediafile_id, FALSE, $still_parameters, $priority);

      // Create sub still job.
      mediamosa_job_still::create($job_id, $still_parameters);

      return $job_id;
    }

    // Throw exception.
    throw new mediamosa_exception_error_still_is_not_creatable(array('@mediafile_id' => $mediafile_id));
  }

  /**
   * Create upload job.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $group_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param integer $file_size
   * @param bool $retranscode
   * @param bool $create_still
   * @param array $still_parameters
   * @param integer $priority
   */
  public static function create_job_upload($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $file_size, $retranscode = FALSE, $create_still = FALSE, $still_parameters = array(), $priority = NULL) {

    // Enough quota?
    mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);

    // Create job.
    $job_id = self::create(mediamosa_job_db::JOB_TYPE_UPLOAD, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $priority);

    // Create sub upload job.
    mediamosa_job_upload::create($app_id, $job_id, $user_id, $group_id, $mediafile_id, $file_size, $retranscode, $create_still);

    // Return the new job_id.
    return $job_id;
  }

  /**
   * Create a job.
   *
   * @param string $job_type
   * @param integer $app_id
   * @param string $user_id
   * @param bool $is_app_admin
   * @param string $mediafile_id
   * @param bool $create_still
   * @param array $still_parameters
   */
  public static function create($job_type, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still = FALSE, $still_parameters = array(), $priority = NULL, $hint = FALSE, $mediafile_dest = NULL) {

    // Get the mediafile.
    $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Owner check.
    mediamosa_acl::owner_check($app_id, $user_id, $a_mediafile[mediamosa_asset_mediafile_db::APP_ID], $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID], $is_app_admin);

    // Get the asset ID.
    $asset_id = $a_mediafile[mediamosa_asset_mediafile_db::ASSET_ID];

    // Re-analysing still.
    if ($job_type == mediamosa_job_db::JOB_TYPE_ANALYSE && $a_mediafile[mediamosa_asset_mediafile_db::IS_STILL] == mediamosa_asset_mediafile_db::IS_STILL_TRUE) {
      // Get the metadata.
      $mediafile_metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id);

      if (!$mediafile_metadata) {
        $still_id = $a_mediafile['mediafile_id'];

        // Destination.
        $destination = mediamosa_configuration_storage::data_still_get_file($still_id);

        // Still size.
        $size = mediamosa_gd::getimagesize($destination);
        if (!$size) {
          self::post_create_job_failed($app_id, $user_id, $mediafile_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, 'File is not an image.', $still_parameters);
        }

        $width = $size[0];
        $height = $size[1];

        // File type.
        $file_type = '';
        $pos = strrpos($size['mime'], '/');
        if ($pos !== FALSE) {
          $file_type = mediamosa_unicode::substr($size['mime'], $pos + 1);
        }
        $filesize = filesize($destination);

        mediamosa_asset_mediafile_metadata::create_metadata_still_default($still_id, $width, $height, $filesize, $size['mime'], $file_type);

        self::post_create_job_finished($app_id, $user_id, $mediafile_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, '', $still_parameters);
      }

      return 0;
    }

    // Create the job.
    $fields = array(
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $user_id,
      mediamosa_job_db::APP_ID => $app_id,
      mediamosa_job_db::JOB_TYPE => $job_type,
      mediamosa_job_db::CREATE_STILL => $create_still ? mediamosa_job_db::CREATE_STILL_TRUE : mediamosa_job_db::CREATE_STILL_FALSE,
      mediamosa_job_db::STILL_PARAMETERS => serialize($still_parameters),
      mediamosa_job_db::PROGRESS => '0.000',
    );

    if (!is_null($hint)) {
      $fields[mediamosa_job_db::HINT] = ($hint ? mediamosa_job_db::HINT_TRUE : mediamosa_job_db::HINT_FALSE);
    }

    if (isset($priority)) {
      $fields[mediamosa_job_db::PRIORITY] = $priority;
    }

    if (isset($mediafile_dest)) {
      $fields[mediamosa_job_db::MEDIAFILE_DEST] = $mediafile_dest;
    }

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Called after the job has been created.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $mediafile_id
   * @param string $job_type
   * @param string $error_description
   * @param array $still_parameters
   * @param string $status
   */
  private static function post_create_job($app_id, $user_id, $mediafile_id, $asset_id, $job_type = mediamosa_job_db::JOB_TYPE_ANALYSE, $error_description = '', $still_parameters = array(), $status = mediamosa_job_db::JOB_STATUS_FINISHED) {

    if ($status == mediamosa_job_db::JOB_STATUS_FINISHED) {
      // Get the mediafile.
      $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

      // Owner check.
      $is_app_admin = FALSE;
      mediamosa_acl::owner_check($app_id, $user_id, $a_mediafile[mediamosa_asset_mediafile_db::APP_ID], $a_mediafile[mediamosa_asset_mediafile_db::OWNER_ID], $is_app_admin);

      // Get the asset ID.
      $asset_id = $a_mediafile[mediamosa_asset_mediafile_db::ASSET_ID];
    }

    // Create the job.
    $fields = array(
      mediamosa_job_db::APP_ID => $app_id,
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $user_id,
      mediamosa_job_db::JOB_STATUS => $status,
      mediamosa_job_db::PROGRESS => '1.000',
      mediamosa_job_db::PRIORITY => 0,
      mediamosa_job_db::JOB_TYPE => $job_type,
      mediamosa_job_db::STARTED => mediamosa_datetime::utc_current_timestamp_now(),
      mediamosa_job_db::FINISHED => mediamosa_datetime::utc_current_timestamp_now(),
      mediamosa_job_db::CREATE_STILL => mediamosa_job_db::CREATE_STILL_FALSE,
      mediamosa_job_db::STILL_PARAMETERS => serialize($still_parameters),
      mediamosa_job_db::ERROR_DESCRIPTION => $error_description,
    );

    // Add created and changed value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Create a finished job.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $mediafile_id
   * @param string $job_type
   * @param string $error_description
   * @param array $still_parameters
   */
  public static function post_create_job_finished($app_id, $user_id, $mediafile_id, $asset_id, $job_type = mediamosa_job_db::JOB_TYPE_ANALYSE, $error_description = '', $still_parameters = array()) {
    self::post_create_job($app_id, $user_id, $mediafile_id, $asset_id, $job_type, $error_description, $still_parameters, mediamosa_job_db::JOB_STATUS_FINISHED);
  }

  /**
   * Create a failed job.
   *
   * @param integer $app_id
   * @param string $user_id
   * @param string $mediafile_id
   * @param string $job_type
   * @param string $error_description
   * @param array $still_parameters
   */
  public static function post_create_job_failed($app_id, $user_id, $mediafile_id, $asset_id, $job_type = mediamosa_job_db::JOB_TYPE_ANALYSE, $error_description = '', $still_parameters = array()) {
    self::post_create_job($app_id, $user_id, $mediafile_id, $asset_id, $job_type, $error_description, $still_parameters, mediamosa_job_db::JOB_STATUS_FAILED);
  }

  /**
   * Create a analyse job, after upload has been completed.
   *
   * @param $job_ext_upload
   */
  public static function create_analyse_job_with_jobext($job_ext_upload) {

    assert(isset($job_ext_upload[mediamosa_job_upload_db::FILE_SIZE]));

    // Get the mediafile ID.
    $mediafile_id = $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID];

    // Get the mediafile.
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    // Get the newly uploaded filename.
    $uploaded_filename = mediamosa_configuration_storage::mediafile_filename_get($mediafile);

    // Get the filesize of the uploaded file.
    $uploaded_filesize = mediamosa_io::filesize($uploaded_filename);

    if ($uploaded_filesize != $job_ext_upload[mediamosa_job_upload_db::FILE_SIZE]) {
      // Job failed.
      // Change status to failed.
      return array('status' => mediamosa_job_db::JOB_STATUS_FAILED, 'progress' => '1.000');
    }

    // Do analyse.

    // Check if we need to retranscode.
    $retranscode = $job_ext_upload[mediamosa_job_upload_db::RETRANSCODE];
    $create_still = $job_ext_upload[mediamosa_job_upload_db::CREATE_STILL];

    if ($retranscode == mediamosa_job_upload_db::RETRANSCODE_TRUE) {
      self::add_retranscode_jobs(
        $job_ext_upload[mediamosa_job_db::ASSET_ID],
        $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID],
        $job_ext_upload[mediamosa_job_db::OWNER_ID],
        $job_ext_upload[mediamosa_job_db::APP_ID]
      );
    }
    else {
      // Delete the unoriginal mediafiles of this asset.
      mediamosa_asset_mediafile::delete_transcoded_unoriginal_mediafiles($job_ext_upload[mediamosa_job_db::ASSET_ID], $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID]);
    }

    // Lets try to get it so we know we either UPDATE or INSERT.
    $mediafile_metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($job_ext_upload[mediamosa_job_db::MEDIAFILE_ID]);

    // Insert or update.
    $mediafile_id = $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID];
    mediamosa_asset_mediafile_metadata::create_mediafile_metadata_int($mediafile_id, (int) $uploaded_filesize, mediamosa_asset_mediafile_metadata::FILESIZE);

    // Analyse the mediafile.
    $analyse_result = mediamosa_asset_mediafile::analyse($job_ext_upload[mediamosa_job_db::APP_ID], $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID]);
    assert(is_array($analyse_result));

    // Store it.
    mediamosa_asset_mediafile_metadata::store_analyse($job_ext_upload[mediamosa_job_db::ID], $analyse_result);
  }

  /**
   * Create basic analyse job.
   *
   * @param string $owner
   * @param string $asset_id
   * @param string $mediafile_id
   * @param integer $app_id
   */
  public static function create_analyse_job($owner, $asset_id, $mediafile_id, $app_id) {
    self::log_mediafile($mediafile_id, 'Starting analyse job for @mediafile_id', array('@mediafile_id' => (string)$mediafile_id), $asset_id);

    $fields = array(
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $owner,
      mediamosa_job_db::PRIORITY => -1,
      mediamosa_job_db::JOB_TYPE => mediamosa_job_db::JOB_TYPE_ANALYSE,
      mediamosa_job_db::APP_ID => $app_id,
    );

    // Add created.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Create.
    mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Trigger completed_transcoding_url.
   *
   * @param string $old_status
   *  Old status.
   * @param string $new_status
   *  New status.
   * @param integer $job_id
   *  The job ID.
   */
  public static function notify_transcoding($old_status, $new_status, $job_id) {
    if ($old_status == $new_status) {
      return;
    }

    $a_job_transcode = mediamosa_job_transcode::get($job_id);

    if (!empty($a_job_transcode[mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL])) {
      $completed_transcoding_url = $a_job_transcode[mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL];
      self::log($completed_transcoding_url . $new_status, array(), WATCHDOG_NOTICE);

      // Do the call (through proxy).
      mediamosa_http::do_head_call($completed_transcoding_url . $new_status, array('use_curlproxy' => TRUE));
    }
  }

  /**
   * Change the job status.
   *
   * @param integer job_id
   * @param string $job_status
   * @param string $error_description
   */
  public static function set_job_status_failed($job_id, $error_description, $old_status = 'NONE') {
    $fields = array(
      mediamosa_job_db::JOB_STATUS => mediamosa_job_db::JOB_STATUS_FAILED,
      mediamosa_job_db::FINISHED => mediamosa_datetime::utc_current_timestamp_now(TRUE),
      mediamosa_job_db::ERROR_DESCRIPTION => $error_description,
    );

    // Add changed.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Update
    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute();

    // Trigger completed url.
    self::notify_transcoding($old_status, mediamosa_job_db::JOB_STATUS_FAILED, $job_id);
  }

  /**
   * Make sure if the job information to start new job is correct.
   *
   * @param $job_id
   *  Job ID.
   * @param $job_type
   *  Job type.
   * @param $mediafile_id
   *  The Mediafile ID.
   */
  public static function is_valid_job($job_id, $job_type, $mediafile_id) {

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $mediafile = mediamosa_asset_mediafile::get($mediafile_id);
        if (!$mediafile || !mediamosa_asset_mediafile::is_original($mediafile)) {
          self::set_job_status_failed($job_id, mediamosa::t('The original mediafile (ID: @mediafile_id is not present or not the original.', array('@mediafile_id' => $mediafile_id)));
          return FALSE;
        }
        break;
    }

    return TRUE;
  }

  /**
   * Start harvesting metadata by tool.
   *
   * @param $job_ext
   */
  public static function start_harvesting_metadata_by_tool($job_ext) {
    // Get the mediafile ID.
    $mediafile_id = $job_ext[mediamosa_job_db::MEDIAFILE_ID];
    $asset_id = $job_ext[mediamosa_job_db::ASSET_ID];

    // Harvest the metadata, if it is the first original mediafile.
    $fields = array(mediamosa_asset_mediafile_db::ID);
    $options = array(
      'get_originals_only' => TRUE,
      'exclude_stills' => TRUE,
    );
    $mediafiles = mediamosa_asset_mediafile::get_by_asset_id($asset_id, $fields, $options);
    if (count($mediafiles) > 1) {
      return;
    }

    // Get all the metadata.
    $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::MIME_TYPE);

    // Call the mediamosa_tool_can_analyse hook.
    foreach (module_implements('mediamosa_tool_can_analyse') as $module) {
      if (module_invoke($module, 'mediamosa_tool_can_analyse', $mime_type)) {
        // Get the tool id.
        $tool_info = module_invoke($module, 'mediamosa_tool_info');

        // Get the app.
        $app = mediamosa_app::get_by_appid($job_ext[mediamosa_job_db::APP_ID]);
        $tool_analyse = unserialize($app[mediamosa_app_db::TOOL_ANALYSE]);

        // Check if the tool is enabled for this app.
        if (isset($tool_analyse[key($tool_info)])) {
          // Start the analyse process.
          module_invoke($module, 'mediamosa_tool_analyse', $job_ext);
          break;
        }
      }
    }
  }

  /**
   * Update the status of the job.
   *
   * @param array $job_ext
   * @param string $progress
   * @param string $status
   * @param string $error_description
   */
  public static function update_status(array $job_ext, $status, $progress = '', $error_description = '') {

    /**
     * If JOB is upload and we are setting the status on FINISHED, then start analyse and transcode jobs.
     */
    if ($job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD &&
        $progress == '1.000' &&
        $job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FINISHED) {

      // Create an analyse job.
      $resp = self::create_analyse_job_with_jobext($job_ext);
      if (is_array($resp) && isset($resp['status']) && isset($resp['progress'])) {
        // Job failed.
        $status = $resp['status'];
        $progress = $resp['progress'];

        // generate event upload failed.
        if (module_exists('rules')) {
          rules_invoke_event('mediamosa_event_upload_failed', $job_ext[mediamosa_job_db::MEDIAFILE_ID]);
        }
      }
      else {
        // generate event upload finished.
        if (module_exists('rules')) {
          rules_invoke_event('mediamosa_event_upload_finished', $job_ext[mediamosa_job_db::MEDIAFILE_ID]);
        }
        // Start harvesting metadata by tools.
        self::start_harvesting_metadata_by_tool($job_ext);
      }
    }

    $fields = array();

    // Set progress.
    if ($progress != '') {
      if ($job_ext[mediamosa_job_db::PROGRESS] != $progress) {
        $fields[mediamosa_job_db::PROGRESS] = $progress;
      }

      if (($progress == '1.000') &&
          ( ($status != mediamosa_job_db::JOB_STATUS_FINISHED) &&
            ($status != mediamosa_job_db::JOB_STATUS_FAILED) &&
            ($job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FINISHED) &&
            ($job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FAILED))) {
        $status = mediamosa_job_db::JOB_STATUS_FINISHED;
      }
    }

    // Start value.
    if ($job_ext[mediamosa_job_db::STARTED] == '' && $status != mediamosa_job_db::JOB_STATUS_WAITING) {
      $fields[mediamosa_job_db::STARTED] = mediamosa_datetime::utc_current_timestamp_now();
    }

    // Finished value.
    if ($job_ext[mediamosa_job_db::FINISHED] == '' &&
        (($status == mediamosa_job_db::JOB_STATUS_FAILED) ||
        ($status == mediamosa_job_db::JOB_STATUS_FINISHED) )) {
      $fields[mediamosa_job_db::FINISHED] = mediamosa_datetime::utc_current_timestamp_now(TRUE);
    }

    // Set status
    if ($job_ext[mediamosa_job_db::JOB_STATUS] != $status) {
      $fields[mediamosa_job_db::JOB_STATUS] = $status;
    }

    // Set de error description.
    if ($job_ext[mediamosa_job_db::ERROR_DESCRIPTION] != $error_description && $error_description) {
      $fields[mediamosa_job_db::ERROR_DESCRIPTION] = $error_description;
    }

    if (!empty($fields)) {
      $current_status = $job_ext[mediamosa_job_db::JOB_STATUS];

      mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
        ->fields($fields)
        ->condition(mediamosa_job_db::ID, $job_ext[mediamosa_job_db::ID])
        ->execute();

      // Trigger url.
      self::notify_transcoding($current_status, $status, $job_ext[mediamosa_job_db::ID]);
    }
  }

  /**
   * Give a list of jobs and as much as information.
   *
   * 1.x: vpx_jobs_get_job_list
   *
   * @param $app_id
   *   can be single appication ID or an array of.
   * @param $a_parameters
   */
  public static function get_job_list($app_id, $user_id, $is_app_admin = FALSE, $job_id = 0, $asset_id = '', $mediafile_id = '', $cql = NULL, $limit = mediamosa_settings::LIMIT_DEFAULT_JOBLIST, $offset = 0) {

    $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j');
    $query->fields('j');

    if (!$is_app_admin) {
      // If there are couple of IDs, we use SQL 'IN', otherwise reqular SQL '=' sign.
      $query->condition('j.' . mediamosa_job_db::APP_ID, $app_id, is_array($app_id) ? 'IN' : '=' );
      $query->condition('j.' . mediamosa_job_db::OWNER_ID, $user_id);
    }

    if (!empty($asset_id)) {
      $query->join(mediamosa_asset_db::TABLE_NAME, 'a', 'a.asset_id = j.asset_id');
      $query->condition('a.' . mediamosa_asset_db::ID, $asset_id);
    }

    if ($cql) {
      // CQL.
      if (!is_array($app_id)) {
        $app_id = array($app_id);
      }
      $a_result_cql2sql = mediamosa_core_cql::parse_job($cql, $app_id);

      if (isset($a_result_cql2sql['str_where']) && $a_result_cql2sql['str_where'] != "") {
        $query->where($a_result_cql2sql['str_where']);
      }

      if (count($a_result_cql2sql['a_order_by']) > 1) {
        throw new mediamosa_exception_error_cql_error(array('@error' => 'you can not use \'sortBy\' on multiple columns, only specify one column'));
      }
      elseif (count($a_result_cql2sql['a_order_by']) == 1) {
        $a_order_by = reset($a_result_cql2sql['a_order_by']);
        if (isset($a_order_by['column']) && $a_order_by['column']) {
          $order_by = $a_order_by['column'];
          $order_direction = 'ASC';
          if (isset($a_order_by['direction']) && $a_order_by['direction']) {
            $order_direction = $a_order_by['direction'];
          }
          $query->orderBy('j.' . $order_by, $order_direction);
        }
      }
    }
    else {
      // Normal search (not CQL).
      if (!empty($job_id)) {
        $query->condition('j.' . mediamosa_job_db::ID, $job_id);
      }

      if (!empty($mediafile_id)) {
        $query->condition('j.' . mediamosa_job_db::MEDIAFILE_ID, $mediafile_id);
      }
      $query->orderBy('j.' . mediamosa_job_db::ASSET_ID, 'ASC');
      $query->orderBy('j.' . mediamosa_job_db::PRIORITY, 'ASC');
    }

    // Count all.
    $item_count_total = $query->countQuery()->execute()->fetchField();

    // Limit and offset.
    $query->range($offset, $limit);

    $result = $query->execute();
    $result->found_rows = $item_count_total;

    return $result;
  }

  /**
   * Deletes the job. Moved into function for re-use.
   *
   * 1.x: _vpx_jobs_delete_job
   *
   * @param integer $job_id
   * @return bool TRUE; job has been deleted
   */
  public static function delete_job($job_id) {
    mediamosa_db::db_query(
      'DELETE tj FROM {#mediamosa_job_transcode} AS tj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.job_id = :job_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_transcode' => mediamosa_job_transcode_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE sj FROM {#mediamosa_job_still} AS sj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.job_id = :job_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_still' => mediamosa_job_still_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE uj FROM {#mediamosa_job_upload} AS uj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.job_id = :job_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query('DELETE FROM {#mediamosa_job} WHERE job_id = :job_id AND status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':job_id' => $job_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    // See if delete has worked.
    $num_rows = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'mj')
      ->fields('mj', array(mediamosa_job_db::ID))
      ->condition(mediamosa_job_db::ID, $job_id)
      ->countQuery()->execute()->fetchField();

    return ($num_rows ? FALSE : TRUE);
  }

  /**
   * Deletes the job. Moved into function for re-use.
   *
   * @param $asset_id
   *   The ID of the asset to remove jobs from.
   *
   * @return
   *   Returns TRUE when all jobs where deleted.
   */
  public static function delete_job_by_asset($asset_id) {
    mediamosa_db::db_query(
      'DELETE tj FROM {#mediamosa_job_transcode} AS tj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.asset_id = :asset_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_transcode' => mediamosa_job_transcode_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE sj FROM {#mediamosa_job_still} AS sj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.asset_id = :asset_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_still' => mediamosa_job_still_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query(
      'DELETE uj FROM {#mediamosa_job_upload} AS uj JOIN {#mediamosa_job} AS j USING(job_id) WHERE j.asset_id = :asset_id AND j.status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    mediamosa_db::db_query('DELETE FROM {#mediamosa_job} WHERE asset_id = :asset_id AND status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED)',
      array(
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        ':asset_id' => $asset_id,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
      )
    );

    // See if delete has worked.
    $num_rows = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'mj')
      ->fields('mj', array(mediamosa_job_db::ASSET_ID))
      ->condition(mediamosa_job_db::ASSET_ID, $asset_id)
      ->countQuery()
      ->execute()
      ->fetchField();

    return ($num_rows ? FALSE : TRUE);
  }

  /**
   * Cancel job.
   * Can only be removed if status is WAITING, FINISHED or FAILED and
   * not assigned to server (WAITING).
   */
  public static function cancel_job($app_id, $job_id, $user_id) {

    // Must exist to cancel.
    self::must_exists($job_id);

    // Check if can be removed.
    $job = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_job} WHERE
       #job_id = :job_id AND
       #status IN (:JOBSTATUS_WAITING, :JOBSTATUS_FINISHED, :JOBSTATUS_FAILED) AND
       #job_id NOT IN (SELECT #job_id FROM {#mediamosa_server_job})',
      array(
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        '#mediamosa_server_job' => mediamosa_server_job_db::TABLE_NAME,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
        '#job_id' => mediamosa_job_db::ID,
        '#status' => mediamosa_job_db::JOB_STATUS,
        ':job_id' => $job_id,
      )
    )->fetchAssoc();

    if ($job == FALSE) {
      throw new mediamosa_exception_error_job_could_not_be_removed(array('@job_id' => $job_id));
    }

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $job['app_id'], $job['owner']);

    $job_status = $job['status'];
    $job_type = $job['job_type'];

    // If we try to delete a analyse job, then make sure its has a metadata row.
    if ($job_type == mediamosa_job_db::JOB_TYPE_ANALYSE && $job_status == mediamosa_job_db::JOB_STATUS_WAITING) {

      $num_rows = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mamm')
        ->fields('mamm', array(mediamosa_asset_mediafile_metadata_db::ID))
        ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $job['mediafile_id'])
        ->countQuery()->execute()->fetchField();

      if (!$num_rows) {
        throw new mediamosa_exception_error_job_could_not_be_removed(array('@job_id' => $job_id));
      }
    }

    // Delete it.
    if (!self::delete_job($job_id)) {
      throw new mediamosa_exception_error_job_could_not_be_removed(array('@job_id' => $job_id));
    }
  }

  public static function update_progress_upload($job_id, $uploaded_file_size) {

      // Get complete job.
    $job_ext = mediamosa_job::get_job_ext($job_id);

    if (!$uploaded_file_size && $job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD) {
      // If the filesize is 0, then we are done...
      if (!$job_ext[mediamosa_job_upload_db::FILE_SIZE]) {

        // Update the job.
        mediamosa_job::update_status($job_ext, mediamosa_job_db::JOB_STATUS_INPROGRESS, '1.000');
        return; // done
      }
    }

    // Set uploaded size.
    $fields = array(
      mediamosa_job_upload_db::UPLOADED_FILE_SIZE => $uploaded_file_size,
    );

    // Set fields.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Update.
    mediamosa_db::db_update(mediamosa_job_upload_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_upload_db::JOB_ID, $job_id)
      ->execute();

    // Get progress
    $progress = mediamosa_db::db_query(
      'SELECT #uploaded_file_size / #file_size FROM {#mediamosa_job_upload} WHERE #job_id = :job_id',
      array(
        '#uploaded_file_size' => mediamosa_job_upload_db::UPLOADED_FILE_SIZE,
        '#file_size' => mediamosa_job_upload_db::FILE_SIZE,
        '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
        '#job_id' => mediamosa_job_upload_db::JOB_ID,
        ':job_id' => $job_id,
      )
    )->fetchField();

    // 1.xxx.
    $progress = number_format(floatval($progress), 3, '.', '');

    // Update the job.
    mediamosa_job::update_status($job_ext, mediamosa_job_db::JOB_STATUS_INPROGRESS, $progress);
  }
}
