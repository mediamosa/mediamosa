<?php
/**
 * @file
 * The job module.
 */

class mediamosa_job {
  // ---------------------------------------------------------------- Functions.
  /**
   * Log for job.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param integer $severity
   *   The severity of the message; one of the following values as defined in
   *   @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'job');
  }

  /**
   * Logs for specific mediafile.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile that is associated with the message.
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param string $asset_id
   *   (optional) The ID of the asset. Provide when available.
   * @param integer $severity
   *   The severity of the message; one of the following values as defined in
   *   @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   */
  public static function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'job');
  }

  /**
   * Make sure the job exists.
   *
   * @param integer $job_id
   *   The Job ID.
   *
   * @return array
   *   The job array.
   *
   * @throws mediamosa_exception_error()
   */
  public static function must_exists($job_id) {
    return mediamosa_db::db_must_exists(mediamosa_job_db::TABLE_NAME, array(mediamosa_job_db::ID => $job_id));
  }

  /**
   * Get the job from the main job table.
   *
   * @param integer $job_id
   *   The job ID.
   *
   * @return array
   *   The job array.
   */
  public static function get($job_id, array $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the job from the main job table using the mediafile ID.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   *
   * @return array
   *   The job array or FALSE.
   */
  public static function get_by_mediafileid($mediafile_id, array $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $fields)
      ->condition(mediamosa_job_db::MEDIAFILE_ID, $mediafile_id)
      ->orderBy(mediamosa_job_db::ID, 'ASC')
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get specific job from the main job table.
   *
   * @param string $mediafile_id
   *  The mediafile ID.
   *
   * @return array
   *   Get all jobs belonging to mediafile.
   */
  public static function get_by_mediafileid_all($mediafile_id, array $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $fields)
      ->condition(mediamosa_job_db::MEDIAFILE_ID, $mediafile_id)
      ->orderBy(mediamosa_job_db::ID, 'ASC')
      ->execute();
  }

  /**
   * Get job details by a given asset_id.
   *
   * @param integer $asset_id
   *  The asset ID.
   */
  static public function get_by_asset_id($asset_id, array $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->fields('j', $fields)
      ->condition(mediamosa_job_db::ASSET_ID, $asset_id)
      ->execute();
  }

  /**
   * Test if there are any running jobs.
   *
   * @return integer
   *   Returns the number of running jobs.
   */
  public static function has_running_jobs() {
    return mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'job')
      ->condition(mediamosa_job_db::JOB_STATUS, mediamosa_job_db::JOB_STATUS_INPROGRESS)
      ->countQuery()
      ->execute()
      ->fetchField();
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
   * Returns the number jobs of certain type.
   *
   * @param integer $app_id
   * @param array $job_types
   *   (optional) The job types to count.
   * @param array $job_status
   *   (optional) The job status. default: waiting, inprogess, cancelling.
   *
   * @return integer
   *   The number of jobs found.
   *
   * @index app_id, job_status, job_type
   */
  public static function count_application_jobs_by_type($app_id, array $job_types = array(), $job_status = array(mediamosa_job_db::JOB_STATUS_WAITING, mediamosa_job_db::JOB_STATUS_CANCELLING, mediamosa_job_db::JOB_STATUS_INPROGRESS)) {
    $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j')
      ->condition(mediamosa_job_db::APP_ID, $app_id);

    if (!empty($job_status)) {
      $query->condition(mediamosa_job_db::JOB_STATUS, $job_status);
    }
    if (!empty($job_types)) {
      $query->condition(mediamosa_job_db::JOB_TYPE, $job_types);
    }

    // Do count.
    return $query
      ->groupBy(mediamosa_job_db::JOB_TYPE)
      ->countQuery()
      ->execute()
      ->fetchField();
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
   *   The job ID.
   *
   * @return array
   *   The job array with extended info.
   */
  public static function get_job_ext($job_id) {

    // Get the basic job.
    $job_ext = self::get($job_id);

    // Any extra information stored here.
    $job_child = NULL;

    // Based on type, we will get the sub-table info.
    switch ($job_ext[mediamosa_job_db::JOB_TYPE]) {
      case mediamosa_job_db::JOB_TYPE_STILL:
        $job_child = mediamosa_job_still::get($job_id);
        break;

      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $job_child = mediamosa_job_transcode::get($job_id);
        break;

      case mediamosa_job_db::JOB_TYPE_UPLOAD:
        $job_child = mediamosa_job_upload::get($job_id);
        break;
    }

    // If set, then merge.
    if ($job_child) {
      // Lapping info will prefer what is in job_ext.
      return array_merge($job_child, $job_ext);
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
   * Add retranscode jobs.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param string $user_id
   *   The owner.
   * @param integer $app_id
   *   The application ID.
   *
   * @return integer
   *   Number of created jobs.
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

    // Create mediadownload job.
    self::create_job_media_download($app_id, $user_id, $is_app_admin, $mediafile_id);

    // Create job.
    $job_id = self::create_job(mediamosa_job_db::JOB_TYPE_TRANSCODE, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $priority, $hint, $mediafile_dest);

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

    // Make sure the mediafile exists.
    mediamosa_asset_mediafile::must_exists_cached($mediafile_id_source, $app_id);

    // Must be transcodable.
    mediamosa_asset_mediafile_metadata::is_transcodable($mediafile_id_source);

    // Get the transcode_profile_id.
    if (!$profile_id) {
      $profile = mediamosa_asset_mediafile::get($mediafile_dest, $app_id, array(mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID));
      if ($profile) {
        $profile_id = $profile[mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID];
      }
    }

    // Create mediadownload job.
    self::create_job_media_download($app_id, $user_id, $is_app_admin, $mediafile_id_source);

    // Create job.
    $job_id = self::create_job(mediamosa_job_db::JOB_TYPE_TRANSCODE, $app_id, $user_id, $is_app_admin, $mediafile_id_source, $create_still, $still_parameters, $priority, FALSE, $mediafile_dest);

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
      // Create mediadownload job.
      self::create_job_media_download($app_id, $user_id, $is_app_admin, $mediafile_id);

      // Create job.
      $job_id = self::create_job(mediamosa_job_db::JOB_TYPE_STILL, $app_id, $user_id, $is_app_admin, $mediafile_id, FALSE, $still_parameters, $priority);

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
    $job_id = self::create_job(mediamosa_job_db::JOB_TYPE_UPLOAD, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $priority);

    // Create sub upload job.
    mediamosa_job_upload::create($app_id, $job_id, $user_id, $group_id, $mediafile_id, $file_size, $retranscode, $create_still);

    // Return the new job_id.
    return $job_id;
  }

  /**
   * Create a media download job.
   *
   * This job will download the storage location and place it in the transition
   * dir.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The user ID.
   * @param boolean $is_app_admin
   *   Is user superuser?
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param integer $priority
   *   The priority for job.
   *
   * @return integer
   *   The job ID or FALSE.
   */
  public static function create_job_media_download($app_id, $user_id, $is_app_admin, $mediafile_id, $priority = 0) {
    // Get uri to mediafile.
    $mediafile_uri = mediamosa_storage::get_uri_mediafile($mediafile_id);

    // Build the streamwrapper.
    $mediamosa_io_streamwrapper = mediamosa_io::require_stream_wrapper_instance_by_uri($mediafile_uri);

    // Not local.
    if (!$mediamosa_io_streamwrapper->is_local()) {
      // We only download when mediafile is not available locally. If we have
      // an transition file available, then use that.
      if (mediamosa_storage_transition::find_transition_file($mediamosa_io_streamwrapper)) {
        return;
      }
    }
    else {
      // Is local, no need to download.
      return;
    }

    // Create job.
    return self::create_job(
      mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD,
      $app_id,
      $user_id,
      $is_app_admin,
      $mediafile_id,
      FALSE,
      array(),
      $priority
    );
  }

  /**
   * Create a media upload job.
   *
   * This job will upload the mediafile from source to its storage location.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The user ID.
   * @param boolean $is_app_admin
   *   Is user superuser?
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param integer $priority
   *   The priority for job.
   *
   * @return integer
   *   The job ID or FALSE.
   */
  public static function create_job_media_upload($app_id, $user_id, $is_app_admin, $mediafile_id, $priority = 0) {
    // Create job.
    return self::create_job(
      mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_UPLOAD,
      $app_id,
      $user_id,
      $is_app_admin,
      $mediafile_id,
      FALSE,
      array(),
      $priority
    );
  }

  /**
   * Move a mediafile to another storage location.
   *
   * This job will download the mediafile (if needed) and upload it on the
   * destination. Once finished and succesful, it will be removed from the
   * source location and sannas_mount_point will point to the new location.
   *
   * The mediamove will use an temporary (local) location during the copy.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The user ID.
   * @param boolean $is_app_admin
   *   Is user superuser?
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param integer $storage_profile_id
   *   The storage profile ID.
   * @param integer $priority
   *   The priority for job.
   *
   * @return integer
   *   The job ID or FALSE.
   */
  public static function create_job_media_move($app_id, $user_id, $is_app_admin, $mediafile_id, $storage_profile_id, $path, $priority = 0) {
    // Create job.
    return self::create_job(
      mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_MOVE,
      $app_id,
      $user_id,
      $is_app_admin,
      $mediafile_id,
      FALSE,
      array('storage_profile_id' => $storage_profile_id, 'path' => $path),
      $priority
    );
  }

  /**
   * Create analyse job.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $user_id
   *   The owner of the job.
   * @param boolean $is_app_admin
   *   Is app admin.
   * @param string $mediafile_id
   *   The ID of the mediafile to analyse.
   * @param integer $priority
   *   The job priority.
   * @param boolean $hint
   *   Hinting of the mediafile.
   *
   * @return integer
   *   The job ID.
   */
  public static function create_job_analyse($app_id, $user_id, $is_app_admin, $mediafile_id, $priority = NULL, $hint = FALSE) {
    // Create mediadownload job.
    self::create_job_media_download($app_id, $user_id, $is_app_admin, $mediafile_id);

    // Create analyse job.
    return self::create_job(mediamosa_job_db::JOB_TYPE_ANALYSE, $app_id, $user_id, $is_app_admin, $mediafile_id, FALSE, array(), $priority, $hint);
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
  public static function create_job($job_type, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still = FALSE, $still_parameters = array(), $priority = NULL, $hint = FALSE, $mediafile_dest = NULL) {

    // Get the mediafile.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Owner check.
    mediamosa_acl::owner_check($app_id, $user_id, $mediafile[mediamosa_asset_mediafile_db::APP_ID], $mediafile[mediamosa_asset_mediafile_db::OWNER_ID], $is_app_admin);

    // Get the asset ID.
    $asset_id = $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];

    // Re-analysing still.
    if ($job_type == mediamosa_job_db::JOB_TYPE_ANALYSE && $mediafile[mediamosa_asset_mediafile_db::IS_STILL] == mediamosa_asset_mediafile_db::IS_STILL_TRUE) {
      // Get the metadata.
      $mediafile_metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id);

      if (!$mediafile_metadata) {
        $still_id = $mediafile['mediafile_id'];

        // Destination.
        $still_uri = mediamosa_storage::get_uri_mediafile($mediafile);

        // Still size.
        $size = mediamosa_gd::get_image_size($still_uri);
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
        $filesize = mediamosa_io::filesize($still_uri);
        $md5 = mediamosa_io::md5_file($still_uri);

        mediamosa_asset_mediafile_metadata::create_metadata_still_default($still_id, $width, $height, $filesize, $size['mime'], $file_type, $md5);

        self::post_create_job_finished($app_id, $user_id, $mediafile_id, $asset_id, mediamosa_job_db::JOB_TYPE_ANALYSE, '', $still_parameters);
      }

      return 0;
    }

    // Create the job.
    $fields = array(
      mediamosa_job_db::APP_ID => $app_id,
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $user_id,
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
      $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

      // Owner check.
      $is_app_admin = FALSE;
      mediamosa_acl::owner_check($app_id, $user_id, $mediafile[mediamosa_asset_mediafile_db::APP_ID], $mediafile[mediamosa_asset_mediafile_db::OWNER_ID], $is_app_admin);

      // Get the asset ID.
      $asset_id = $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];
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
   *
   * @return array
   *   Either associative array.
   *   - status
   *     The status of the job.
   *   - progress
   *     Progress value from 0.000 till 1.000.
   *
   */
  public static function create_analyse_job_with_jobext($job_ext_upload) {
    // Get the mediafile ID.
    $mediafile_id = $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID];

    // Get the mediafile.
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    // Get the newly uploaded filename.
    $mediafile_uri = mediamosa_storage::get_uri_mediafile($mediafile);

    // Get the filesize of the uploaded file.
    $uploaded_filesize = mediamosa_io::filesize($mediafile_uri);

    if ($uploaded_filesize != $job_ext_upload[mediamosa_job_upload_db::FILE_SIZE]) {
      // Job failed.
      // Change status to failed.
      return array('status' => mediamosa_job_db::JOB_STATUS_FAILED, 'progress' => '1.000');
    }

    // Check if we need to retranscode.
    $retranscode = $job_ext_upload[mediamosa_job_upload_db::RETRANSCODE];

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

    // Insert or update.
    $mediafile_id = $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID];

    // Analyse the mediafile.
    $analyse_result = mediamosa_asset_mediafile::analyse($job_ext_upload[mediamosa_job_db::APP_ID], $job_ext_upload[mediamosa_job_db::MEDIAFILE_ID]);

    // Store it.
    mediamosa_asset_mediafile_metadata::store_analyse($job_ext_upload[mediamosa_job_db::ID], $analyse_result);

    // Success.
    return array('status' => mediamosa_job_db::JOB_STATUS_FINISHED, 'progress' => '1.000');
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

    $job_transcode = mediamosa_job_transcode::get($job_id);

    if (!empty($job_transcode[mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL])) {
      $completed_transcoding_url = $job_transcode[mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL];
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
   *   The Job ID.
   * @param $job_type
   *   The Job type, see mediamosa_job_db::JOB_TYPE_*.
   * @param $mediafile_id
   *   The Mediafile ID.
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
   *   Job to update.
   * @param string $status
   *   The new status of the job.
   * @param string $progress
   *   The progress value from 0.000 till 1.0000.
   * @param string $error_description
   *   Possible error description.
   */
  public static function update_status(array $job_ext, $status, $progress = '', $error_description = '') {

    // If JOB is upload and we are setting the status on FINISHED, then start
    // analyse and transcode jobs.
    if (
      $job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD &&
      $job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FINISHED &&
      $progress == '1.000') {

      // Create an analyse job.
      $job_analyse = self::create_analyse_job_with_jobext($job_ext);

      // Base the new status on the analyse result.
      $status = $job_analyse['status'];

      // Generate event upload failed.
      switch ($status) {
        default:
          if (module_exists('rules')) {
            rules_invoke_event('mediamosa_event_upload_failed', $job_ext[mediamosa_job_db::MEDIAFILE_ID]);
          }
          break;

        case mediamosa_job_db::JOB_STATUS_FINISHED:
          if (module_exists('rules')) {
            rules_invoke_event('mediamosa_event_upload_finished', $job_ext[mediamosa_job_db::MEDIAFILE_ID]);
          }

          // Start harvesting metadata by tools.
          self::start_harvesting_metadata_by_tool($job_ext);
          break;
      }
    }

    $fields = array();

    // Set progress.
    if ($progress != '') {
      if ($job_ext[mediamosa_job_db::PROGRESS] != $progress) {
        $fields[mediamosa_job_db::PROGRESS] = $progress;
      }

      if ($progress == '1.000' &&
          $status != mediamosa_job_db::JOB_STATUS_FINISHED &&
          $status != mediamosa_job_db::JOB_STATUS_FAILED &&
          $job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FINISHED &&
          $job_ext[mediamosa_job_db::JOB_STATUS] != mediamosa_job_db::JOB_STATUS_FAILED) {
        $status = mediamosa_job_db::JOB_STATUS_FINISHED;
      }
    }

    // Start value.
    if ($job_ext[mediamosa_job_db::STARTED] == '' && $status != mediamosa_job_db::JOB_STATUS_WAITING) {
      $fields[mediamosa_job_db::STARTED] = mediamosa_datetime::utc_current_timestamp_now();
    }

    // Finished value.
    if ($job_ext[mediamosa_job_db::FINISHED] == '' &&
       ($status == mediamosa_job_db::JOB_STATUS_FAILED || $status == mediamosa_job_db::JOB_STATUS_FINISHED)) {
      $fields[mediamosa_job_db::FINISHED] = mediamosa_datetime::utc_current_timestamp_now(TRUE);
    }

    // Set status, when different.
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
   * @param integer $job_id
   *   The job ID.
   *
   * @return boolean
   *   Returns TRUE when job was deleted, FALSE otherwise.
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
   *
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

  /**
   * Update the progress status of the upload job.
   *
   * @param integer $job_id
   *   The job ID.
   * @param integer $uploaded_file_size
   *   The size of the current position done upload in bytes.
   */
  public static function update_progress_upload($job_id, $uploaded_file_size) {
    // Get complete job.
    $job_ext = mediamosa_job::get_job_ext($job_id);

    if (!$uploaded_file_size && $job_ext[mediamosa_job_db::JOB_TYPE] == mediamosa_job_db::JOB_TYPE_UPLOAD) {
      // If the filesize is 0, then we are done...
      if (!$job_ext[mediamosa_job_upload_db::FILE_SIZE]) {
        // Update the job.
        mediamosa_job::update_status($job_ext, mediamosa_job_db::JOB_STATUS_INPROGRESS, '1.000');
        return;
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
