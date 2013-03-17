<?php
/**
 * @file
 * The job handler server.
 */

class mediamosa_job_scheduler {
  /**
   * Log message.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable by not
   *   concatenating dynamic values into it! Variables in the message should be
   *   added by using placeholder strings alongside the variables argument to
   *   declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or NULL if
   *   message is already translated or not possible to translate.
   * @param integer $severity
   *   The severity of the message; one of the following values as defined in
   *   RFC 3164.
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
    mediamosa_watchdog::log($message, $variables, $severity, 'job scheduler');
  }

  /**
   * Logs for a specific MediaFile.
   *
   * @param string $mediafile_id
   *   The MediaFile ID.
   * @param string $message
   *   The message to store in the log. Keep $message translatable by not
   *   concatenating dynamic values into it! Variables in the message should be
   *   added by using placeholder strings alongside the variables argument to
   *   declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or NULL if
   *   message is already translated or not possible to translate.
   * @param integer $severity
   *   The severity of the message; one of the following values as defined in
   *   RFC 3164:
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   */
  public static function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = '', $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'job scheduler');
  }

  /**
   * Log debug message.
   *
   * Will only when normal debug level is on.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable by not
   *   concatenating dynamic values into it! Variables in the message should be
   *   added by using placeholder strings alongside the variables argument to
   *   declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   */
  public static function log_debug($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'job_scheduler');
  }

  /**
   * Log debug message.
   *
   * Will only log message when high debug level is on.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable by not
   *   concatenating dynamic values into it! Variables in the message should be
   *   added by using placeholder strings alongside the variables argument to
   *   declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   */
  public static function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_high($message, $variables, 'job_scheduler');
  }

  /**
   * Trigger the job scheduler.
   */
  public static function trigger() {
    // Now trigger the jobscheduler.
    $mediamosa_jobscheduler_uri = variable_get('mediamosa_jobscheduler_uri', NULL);

    if (isset($mediamosa_jobscheduler_uri)) {
      $url = mediamosa_http::uri2url($mediamosa_jobscheduler_uri) . '/internal/cron/jobscheduler';

      // Log it.
      self::log_debug('Triggering job scheduler @ @uri', array('@uri' => $url));

      // Dont trigger in sandbox.
      if (mediamosa::in_simpletest_sandbox()) {
        // So we wait till finished.
        MediaMosaTestCase::staticInternalRestCallGet($url);
      }
      else {
        // Trigger.
        mediamosa_http::do_head_internal_call($url);
      }
    }
    else {
      self::log('Jobschedular URL not set, please setup jobschedular server in the @link.', array('@link' => l(t('MediaMosa configuration'), 'admin/mediamosa/config/global')), WATCHDOG_ALERT, 'job_cron');
    }
  }

  /**
   * Trigger every minute.
   *
   * Will timeout on itself after 2 minutes.
   */
  public static function run_parse_queue() {
    // Triggert from sandbox, run once, and done.
    if (mediamosa::in_simpletest_sandbox()) {
      self::log('Job Scheduler; run_parse_queue() in sandbox, hit and return.');
      self::parse_queue();
      return;
    }

    $run_last = variable_get('mediamosa_jobscheduler_cron_last', NULL);

    // May run? (0 - 60 seconds).
    if (round(REQUEST_TIME - $run_last) < 60) {
      // Still running.
      return;
    }

    // A period of 60 to 120 seconds we wait till its killed (or done running).
    $run_last = variable_get('mediamosa_jobstatus_cron_running', NULL);
    if (round(REQUEST_TIME - $run_last) < 120) {
      self::log('run_parse_queue() in safety timeout zone.');
      // In timeout zone.
      return;
    }

    // If we get here then we can go.
    variable_set('mediamosa_jobscheduler_cron_last', REQUEST_TIME);

    // We set this to NULL when we are done.
    variable_set('mediamosa_jobscheduler_cron_running', REQUEST_TIME);

    // Set max run time on 120 sec.
    set_time_limit(120);

    // Wait 5 seconds so we match possible job parser.
    sleep(5);

    for ($x = 0; $x < 6; $x++) {
      $start_at = microtime(TRUE);

      try {
        // Parse queue.
        self::parse_queue();
      }
      catch (mediamosa_exception $e) {
        // ignore, its logged.
      }
      // Now take the time it took, and subtrac that from 10.
      $time_to_sleep = 10 - round(microtime(TRUE) - $start_at);
      self::log_debug_high('run_parse_queue: Time to sleep @time', array('@time' => $time_to_sleep));
      if ($time_to_sleep > 0) {
        sleep($time_to_sleep);
      }

      // And repeat for 6 times = 1 minute = 1 cron run.
    }

    // Done.
    variable_set('mediamosa_jobscheduler_cron_running', NULL);
  }

  /**
   * This function is the heart of the handler system.
   *
   * It must be called at regular intervals, best every 1 minute. It will
   * cleanup, run and update jobs.
   */
  public static function parse_queue() {

    // Log starting.
    self::log_debug('Start parse queue job scheduler');

    // Get all servers for their status and process the result.
    self::check_status_all_servers();

    // Start new jobs when slots are available.
    self::start_new_jobs();

    // Remove any timedout upload jobs.
    self::check_upload_timeout();

    // Remove any timedout transcode jobs.
    self::check_transcode_timeout();

    // Put server that are set on close, on off when they have done all jobs.
    self::check_server_on_status_close();

    // Log ending.
    self::log_debug('Ending parse queue job scheduler');
  }

  /**
   * Get a list of servers and process the job status.
   */
  public static function check_status_all_servers() {
    $job_processor_servers = mediamosa_server::get_enabled_job_processor();

    foreach ($job_processor_servers as $job_processor_server) {
      self::update_server_job_status($job_processor_server[mediamosa_server_db::SERVER_URI]);
    }
  }

  /**
   * With A REST call we'll retrieve the status of the jobs.
   *
   * @param string $uri
   *   The URI of the job server.
   */
  public static function update_server_job_status($uri) {

    // Get response.
    $response = mediamosa_http::do_internal_call($uri, 'server/joblist');
    if ($response->code != '200') {
      self::log('Call to server/joblist did not return a 200 result (header: @header)', array('@header' => var_export($response, TRUE)));
      return;
    }

    try {
      $response_data = new mediamosa_connector_response($response->data);
    }
    catch (Exception $e) {
      mediamosa_debug::log_export($response->data);
      self::log(
        'Parse XML: @msg (@resp)',
        array(
          '@msg' => $e->getMessage(),
          '@resp' => print_r($response->data, TRUE),
        ),
        WATCHDOG_ERROR
      );
      return;
    }

    for ($i = 0; $i < $response_data->header->item_count; $i++) {
      // Get current status.
      $job = mediamosa_job::get((string) $response_data->items->item[$i]->job_id);

      $fields = array(
        mediamosa_job_db::JOB_STATUS => $response_data->items->item[$i]->status,
        mediamosa_job_db::PROGRESS => $response_data->items->item[$i]->progress,
        mediamosa_job_db::ERROR_DESCRIPTION => $response_data->items->item[$i]->error_description,
      );

      // Add started time.
      if ($response_data->items->item[$i]->started != '') {
        $fields[mediamosa_job_db::STARTED] = $response_data->items->item[$i]->started;
      }

      // Add finished time.
      if ($response_data->items->item[$i]->finished != '') {
        $fields[mediamosa_job_db::FINISHED] = $response_data->items->item[$i]->finished;
      }

      // Add changed.
      $fields = mediamosa_db::db_update_enrich($fields);

      // Update.
      mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
        ->fields($fields)
        ->condition(mediamosa_job_db::ID, (string) $response_data->items->item[$i]->job_id)
        ->execute();

      // Trigger url.
      mediamosa_job::notify_transcoding($job[mediamosa_job_db::JOB_STATUS], (string) $response_data->items->item[$i]->status, (string) $response_data->items->item[$i]->job_id);

      if ($response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FINISHED ||
          $response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FAILED ||
          $response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_CANCELLED) {

        // Remove link between server and job.
        mediamosa_server_job::delete_by_jobid((string) $response_data->items->item[$i]->job_id);

        // Remove job on jobserver.
        self::remove_job_on_server($uri, (string) $response_data->items->item[$i]->job_id);
      }

      if ($response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FINISHED) {
        switch ($response_data->items->item[$i]->job_type) {
          case mediamosa_job_db::JOB_TYPE_TRANSCODE:
            // Statistics.
            mediamosa_statistics::insert_job_transcode($response_data->items->item[$i]->mediafile_dest, $job[mediamosa_job_db::ID]);
            // If the transcode job is completed, add it to the mediafile.
            self::parse_finished_transcode((string) $response_data->items->item[$i]->job_id, (string) $response_data->items->item[$i]->mediafile_src, (string) $response_data->items->item[$i]->mediafile_dest);
            break;

          case mediamosa_job_db::JOB_TYPE_ANALYSE:
            // If the analyse job is completed, add it to the technical
            // metadata.
            mediamosa_asset_mediafile_metadata::store_analyse(
              (string) $response_data->items->item[$i]->job_id,
              unserialize((string) $response_data->items->item[$i]->analyse_result)
            );
            break;

          case mediamosa_job_db::JOB_TYPE_STILL:
            // If the still job is completed, add the still to db.
            // We serialize it, because of the multiple stills.
            self::add_still_to_db((string) $response_data->items->item[$i]->job_id, unserialize((string) $response_data->items->item[$i]->mediafile_dest));
            break;
        }
      }
      elseif ($response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FAILED ||
              $response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_CANCELLED) {

        switch ($response_data->items->item[$i]->job_type) {
          case mediamosa_job_db::JOB_TYPE_TRANSCODE:
            // When transcode has failed.
            self::parse_failed_transcode((string) $response_data->items->item[$i]->job_id);
            break;
        }
      }
    }
  }

  /**
   * Send a delete on server to remove job.
   *
   * @param string $uri
   *   The job server URI.
   * @param integer $job_id
   *   The job ID.
   */
  public static function remove_job_on_server($uri, $job_id) {
    // Build the rest URL.
    $rest_url = strtr('server/@job_id/delete', array('@job_id' => $job_id));

    // Call URL.
    mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);
  }

  /**
   * Get the information of transcode job and related info.
   *
   * @param integer $job_id
   *   The job ID.
   */
  public static function get_transcodejob_info($job_id) {
    $result = array();

    $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j');
    $query->join(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf', 'mf.mediafile_id = j.mediafile_id');
    $query->join(mediamosa_job_transcode_db::TABLE_NAME, 'jt', 'jt.job_id = j.job_id');
    $job_transcode = $query
      ->fields('j', array(
        mediamosa_job_db::ASSET_ID,
        mediamosa_job_db::JOB_TYPE,
        mediamosa_job_db::OWNER_ID,
        mediamosa_job_db::APP_ID,
        mediamosa_job_db::FILENAME,
      ))
      ->fields('mf', array(
        mediamosa_asset_mediafile_db::FILENAME,
        mediamosa_asset_mediafile_db::ID,
      ))
      ->fields('jt', array(
        mediamosa_job_transcode_db::TOOL,
        mediamosa_job_transcode_db::COMMAND,
        mediamosa_job_transcode_db::FILE_EXTENSION,
        mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID,
      ))
      ->condition('j.' . mediamosa_job_db::ID, $job_id)
      ->condition('mf.' . mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE)
      ->execute()
      ->fetchAssoc();

    if ($job_transcode) {
      // Determine new filename.
      if ($job_transcode[mediamosa_asset_mediafile_db::FILENAME] != '') {
        $new_filename = $job_transcode[mediamosa_asset_mediafile_db::FILENAME];
      }
      else {
        $new_filename = mediamosa_io::get_base_filename($job_transcode[mediamosa_asset_mediafile_db::FILENAME]) . '.' . $job_transcode[mediamosa_asset_mediafile_db::FILE_EXTENSION];
      }

      $result = array(
        'asset_id' => $job_transcode[mediamosa_job_db::ASSET_ID],
        'owner' => $job_transcode[mediamosa_job_db::OWNER_ID],
        'app_id' => $job_transcode[mediamosa_job_db::APP_ID],
        'filename' => $new_filename,
        'transcode_profile_id' => $job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID],
        'tool' => $job_transcode[mediamosa_job_transcode_db::TOOL],
        'file_extension' => $job_transcode[mediamosa_job_transcode_db::FILE_EXTENSION],
        'command' => $job_transcode[mediamosa_job_transcode_db::COMMAND],
        'mediafile_id' => $job_transcode[mediamosa_asset_mediafile_db::ID],
      );
    }
    else {
      $link = mediamosa_job_server::get_asset_link($job_id);
      self::log('Could not find original mediafile for job_id @job_id <br /><br />@link', array('@job_id' => $job_id, '@link' => $link));
    }

    return $result;
  }

  /**
   * Create WMV job.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $asset_id
   *   The asset ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function create_wmv_job($job_id, $asset_id, $mediafile_id) {

    // FIXME: @mediafile_id is empty.
    self::log_mediafile($mediafile_id, 'Start job wmv job: job_id = @job_id, mediafile_id = @mediafile_id', array('@job_id' => $job_id, '@mediafile_id' => $mediafile_id), $asset_id);

    // Get all info.
    $job_ext = mediamosa_job::get_job_ext($job_id);
    if (!$job_ext) {
      $command = '';
      $owner = '';
      $app_id = 0;
      self::log_mediafile($mediafile_id, 'A wmv job (@job_id) without transcode_job!', array('@job_id' => $job_id), $asset_id, WATCHDOG_EMERGENCY);
    }
    else {
      $command = $job_ext[mediamosa_job_transcode_db::COMMAND];
      $owner = $job_ext[mediamosa_job_db::OWNER_ID];
      $app_id = $job_ext[mediamosa_job_db::APP_ID];
    }

    // Must have user_quota.
    mediamosa_user::must_have_user_quota($app_id, $owner, '');

    // Must be transcodable.
    mediamosa_asset_mediafile_metadata::is_transcodable($mediafile_id);

    // Create the job.
    $fields = array(
      mediamosa_job_db::ASSET_ID => $asset_id,
      mediamosa_job_db::MEDIAFILE_ID => $mediafile_id,
      mediamosa_job_db::OWNER_ID => $owner,
      mediamosa_job_db::APP_ID => $app_id,
      mediamosa_job_db::JOB_TYPE => mediamosa_job_db::JOB_TYPE_TRANSCODE,
      mediamosa_job_db::PRIORITY => -1,
    );

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    $job_id_new = mediamosa_db::db_insert(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Add internal_previous_job value to the command string.
    $commands = mediamosa_transcode_profile::commandToArray($command);
    $commands['internal_previous_job'] = $job_id;
    $command = mediamosa_transcode_profile::arrayToCommand($commands);

    // Now create the transcode job.
    $fields = array(
      mediamosa_job_transcode_db::JOB_ID => $job_id_new,
      mediamosa_job_transcode_db::TOOL => mediamosa_job::JOBWINDOWS_TOOL,
      mediamosa_job_transcode_db::COMMAND => $command,
      mediamosa_job_transcode_db::FILE_EXTENSION => 'wmv',
    );

    // Add changed/created values.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_transcode_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Remove the raw avi file for wmv transcode.
   *
   * @param integer $job_id
   *   The job ID.
   */
  public static function wmv_remove_raw_file($job_id) {

    // Get the job.
    $job = mediamosa_job::get($job_id);

    // Get the mediafile ID.
    $mediafile_id = $job[mediamosa_job_db::MEDIAFILE_ID];

    if ($mediafile_id) {
      // Delete the mediafile.
      mediamosa_asset_mediafile::delete($mediafile_id);

      self::log_mediafile($mediafile_id, "Temp. mediafile '@mediafile_id' of job '@job_id' is removed.", array('@mediafile_id' => $mediafile_id, '@job_id' => $job_id), NULL, WATCHDOG_NOTICE);
    }
    else {
      self::log(
        "Job @job_id not found, can't delete temp mediafile.",
        array('@job_id' => $job_id), NULL, WATCHDOG_ERROR
      );
    }
  }

  /**
   * Called when transcode was successfully finished.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $mediafile_id_src
   *   The mediafile ID of the source.
   * @param string $mediafile_id_dest
   *   The mediafile ID of the destination.
   */
  public static function parse_finished_transcode($job_id, $mediafile_id_src, $mediafile_id_dest) {

    // Get the original transcode job.
    $job_info = self::get_transcodejob_info($job_id);

    // Get Asset.
    $asset = mediamosa_asset::get($job_info['asset_id']);
    assert(!empty($asset));

    $fields = array(
      mediamosa_asset_mediafile_db::ID => $mediafile_id_dest,
      mediamosa_asset_mediafile_db::ASSET_ID => $job_info['asset_id'],
      mediamosa_asset_mediafile_db::FILENAME => $job_info['filename'],
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_storage::create_local_mount_point_uri($job_info['app_id']),
      mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID => $job_info['transcode_profile_id'],
      mediamosa_asset_mediafile_db::TOOL => $job_info['tool'],
      mediamosa_asset_mediafile_db::FILE_EXTENSION => $job_info['file_extension'],
      mediamosa_asset_mediafile_db::COMMAND => $job_info['command'],
      mediamosa_asset_mediafile_db::OWNER_ID => $job_info['owner'],
      mediamosa_asset_mediafile_db::APP_ID => $job_info['app_id'],
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE,
      mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => $mediafile_id_src,
    );

    // Add created/changed.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert.
    mediamosa_db::db_insert(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    //  Relocate mediafile if final storage location is not local.
    mediamosa_storage::relocate_mediafile($job_info['app_id'], $mediafile_id_dest, TRUE);

    // Perform a post transcode, if supported.
    if (method_exists('mediamosa_tool_' . $job_info['tool'], 'post_transcode')) {
      call_user_func(array('mediamosa_tool_' . $job_info['tool'], 'post_transcode'), $job_info['asset_id'], $job_info['mediafile_id'], $mediafile_id_dest, $job_info['file_extension']);
    }

    // Get mediafile.
    $mediafile = mediamosa_asset_mediafile::get($job_info['mediafile_id']);

    if ($mediafile[mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL] == mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE) {
      mediamosa_acl::replace_mediafile_to_mediafile($job_info['mediafile_id'], $mediafile_id_dest);
    }

    // Analyse the mediafile.
    $analyse_result = mediamosa_asset_mediafile::analyse($job_info['app_id'], (string) $mediafile_id_dest);

    // Store it.
    mediamosa_asset_mediafile_metadata::store_analyse_without_job($job_id, $analyse_result, (string) $mediafile_id_dest);
  }

  /**
   * Process the data from a still job.
   *
   * @param integer $job_id
   *   The job ID.
   * @param array $filenames
   *   The filenames of the stills.
   */
  public static function add_still_to_db($job_id, $filenames) {
    // Scene changes.
    $scene_realpath = mediamosa_storage::get_realpath_temporary_file($job_id . '_scene.txt');

    $scenes = array();

    if (mediamosa_io::file_exists($scene_realpath)) {
      $fh = @fopen($scene_realpath, 'r');
      if ($fh) {
        while (!feof($fh)) {
          $scenes[] = (int) fgets($fh);
        }
        fclose($fh);
      }
    }

    // Get job.
    $job = mediamosa_job::get($job_id);
    $asset_id = $job['asset_id'];
    $still_parameters = unserialize($job['still_parameters']);

    self::log_mediafile(
      $job['mediafile_id'],
      'Start creation DB multiple still, e.g. @filename, job: @job_id, still_parameters: @still_parameters',
      array(
        '@filename' => $filenames[0],
        '@job_id' => (string) $job_id,
        '@still_parameters' => print_r($still_parameters, TRUE),
      ),
      $asset_id
    );

    // Remove old stills
    // We have multiple stills now, so we don't delete the old ones
    // And deleting with asset_id is definetly not a good idea, while we have
    // multiple stills per mediafile _media_management_delete_still($asset_id);
    //
    // Add record to the mediafile metadata table.
    if (is_array($filenames)) {
      $frametime = $still_parameters['frametime'];
      if (isset($still_parameters['framerate']) && is_numeric($still_parameters['framerate'])) {
        $second = $still_parameters['framerate'] > 0 ? 1 / $still_parameters['framerate'] : 0;
      }
      $tag = $still_parameters['tag'];

      $order = 0;
      $sec = 0;
      if (isset($frametime) && is_numeric($frametime)) {
        $sec = $frametime;
      }
      $i = 0;
      foreach ($filenames as $filename) {
        mediamosa_asset_mediafile_still::create($asset_id, $filename, $job['app_id'], $job['owner'], '', $order, !$order, $still_parameters, ($scenes == array() ? $sec : $scenes[$i]), $job['mediafile_id'], $tag);
        $order++;
        if (isset($second) && is_numeric($second)) {
          $sec += $second;
        }

        $i++;
      }
    }

    // Remove file.
    mediamosa_io::unlink($scene_realpath);
  }

  /**
   * Called when transcode failed or was canceled.
   *
   * @param integer $job_id
   *   The job ID.
   */
  public static function parse_failed_transcode($job_id) {
    // Get original.
    $job_original = mediamosa_job::get($job_id);
    $status = $job_original[mediamosa_job_db::JOB_STATUS];

    // Set failed.
    mediamosa_job::set_progress($job_id, '1.000', FALSE, mediamosa_job_db::JOB_STATUS_FAILED);

    // Trigger url.
    mediamosa_job::notify_transcoding($status, mediamosa_job_db::JOB_STATUS_FAILED, $job_id);
  }

  /**
   * Get the list and find empty slots to start jobs.
   */
  public static function start_new_jobs() {
    // Get for every active server (status ON)
    // the number of job that is not FINISHED, FAILED of CANCELLED
    // and job type is not UPLOAD (does not take a slot).
    $server_jobs = mediamosa_db::db_query(
      'SELECT #nid, #uri, #slots,
             (SELECT COUNT(msj.#job_id)
              FROM {#mediamosa_server_job} AS msj
              JOIN {#mediamosa_job} AS mj ON mj.#job_id = msj.#job_id
              WHERE
                msj.#server_id = ms.#nid
                AND mj.#job_type != :JOBTYPE_UPLOAD
                AND mj.#job_status NOT IN (:JOBSTATUS_FINISHED, :JOBSTATUS_FAILED, :JOBSTATUS_CANCELLED)) AS jobcount
      FROM {#mediamosa_server} AS ms
      WHERE #server_status = :server_status_on AND #server_type = :server_type',
      array(
        '#nid' => mediamosa_server_db::NID,
        '#server_id' => mediamosa_server_job_db::SERVER_ID,
        '#uri' => mediamosa_server_db::SERVER_URI,
        '#slots' => mediamosa_server_db::SLOTS,
        '#job_type' => mediamosa_job_db::JOB_TYPE,
        '#job_id' => mediamosa_job_db::ID,
        '#mediamosa_server_job' => mediamosa_server_job_db::TABLE_NAME,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        ':JOBTYPE_UPLOAD' => mediamosa_job_db::JOB_TYPE_UPLOAD,
        ':JOBSTATUS_FINISHED' => mediamosa_job_db::JOB_STATUS_FINISHED,
        ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
        ':JOBSTATUS_CANCELLED' => mediamosa_job_db::JOB_STATUS_CANCELLED,
        '#job_status' => mediamosa_job_db::JOB_STATUS,
        '#server_status' => mediamosa_server_db::SERVER_STATUS,
        ':server_status_on' => mediamosa_server_db::SERVER_STATUS_ON,
        '#server_type' => mediamosa_server_db::SERVER_TYPE,
        ':server_type' => mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR,
      )
    )->fetchAll();

    foreach ($server_jobs as $server_job) {
      // Try to start as many jobs as there are free slots.
      for ($i = $server_job['jobcount']; $i < $server_job['slots']; $i++) {

        // Get 1 new job.
        $job_info = self::fetch_single_available_job($server_job[mediamosa_server_db::NID]);

        if (empty($job_info)) {
          // No more for this server.
          break;
        }

        if (mediamosa_job::is_valid_job($job_info[mediamosa_job_db::ID], $job_info[mediamosa_job_db::JOB_TYPE], $job_info[mediamosa_job_db::MEDIAFILE_ID])) {
          self::start_server_job(
            $job_info[mediamosa_job_db::ID],
            $job_info[mediamosa_job_db::JOB_TYPE],
            $server_job[mediamosa_server_db::NID],
            $server_job[mediamosa_server_db::SERVER_URI],
            $job_info[mediamosa_job_db::MEDIAFILE_ID]);
        }
      }
    }
  }

  /**
   * Start 1 job on server.
   *
   * Based on job type extra information is gathered. Using the RESTful
   * interface of the server, the job is send. At the end the job is attached to
   * the mediamosa_server_job.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $job_type
   *   The job type, see mediamosa_job_db::JOB_TYPE_*.
   * @param string $server_id
   *   The ID of the server.
   * @param string $uri
   *   The URI of the server.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function start_server_job($job_id, $job_type, $server_id, $uri, $mediafile_id) {
    self::log_mediafile(
      $mediafile_id,
      'Start job: job_id = @job_id, job_type = @job_type, server_id = @server_id, mediafile_id = @mediamosa_id',
      array(
        '@job_id' => $job_id,
        '@job_type' => $job_type,
        '@server_id' => $server_id,
        '@mediamosa_id' => $mediafile_id,
      )
    );

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $errorcode = self::start_server_transcode_job($job_id, $uri, $mediafile_id);
        break;

      case mediamosa_job_db::JOB_TYPE_STILL:
        $errorcode = self::start_server_still_job($job_id, $uri, $mediafile_id);
        break;

      case mediamosa_job_db::JOB_TYPE_ANALYSE:
        $errorcode = self::start_server_analyse_job($job_id, $uri, $mediafile_id);
        break;

      case mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD;
      case mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_UPLOAD;
      case mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_MOVE;
        $errorcode = self::start_server_job_media($job_id, $job_type, $uri, $mediafile_id);
        break;
    }

    if ($job_type == mediamosa_job_db::JOB_TYPE_DELETE_MEDIAFILE) {
      self::start_server_delete_job($job_id, $mediafile_id);
      self::log_mediafile($mediafile_id, 'Process @job_id job: mediafile @mediafile_id removed.', array('@job_id' => $job_id, '@mediafile_id' => $mediafile_id));
    }
    else {
      // If okay update the database with the job info.
      if ($errorcode == mediamosa_error::ERRORCODE_OKAY) {

        if (mediamosa_db::db_exists(mediamosa_job_db::TABLE_NAME, array(mediamosa_job_db::ID => $job_id))) {
          $fields = array(
            mediamosa_server_job_db::SERVER_ID => $server_id,
            mediamosa_server_job_db::JOB_ID => $job_id,
          );

          // Add created/changed.
          $fields = mediamosa_db::db_insert_enrich($fields);

          // Insert.
          mediamosa_db::db_insert(mediamosa_server_job_db::TABLE_NAME)
            ->fields($fields)
            ->execute();

          self::log_mediafile(
            $mediafile_id,
            'Sending @job_type (ID: @job_id) to server @uri.',
            array(
              '@job_type' => $job_type,
              '@job_id' => $job_id,
              '@uri' => $uri,
            )
          );
        }
        else {
          self::log_mediafile(
            $mediafile_id,
            'Making @job_type server job has stopped, job @job_id was deleted during the process.',
            array(
              '@job_type' => $job_type,
              '@job_id' => $job_id,
            )
          );
        }
      }
      else {
        self::log_mediafile(
          $mediafile_id,
          'Error (@errorcode) was reponsed during sending @job_type (ID: @job_id) to server @uri.',
          array(
            '@errorcode' => $errorcode,
            '@job_type' => $job_type,
            '@job_id' => $job_id,
            '@uri' => $uri,
          )
        );
      }
    }
  }

  /**
   * Start transcode job by REST call.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $uri
   *   The job server uri.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function start_server_transcode_job($job_id, $uri, $mediafile_id) {
    // Get job parameters;
    $job_parameters = self::get_job_parameters($job_id, mediamosa_job_db::JOB_TYPE_TRANSCODE, $mediafile_id);

    // GET parameters.
    $query_data = array(
      'job_id' => $job_id,
      'job_type' => mediamosa_job_db::JOB_TYPE_TRANSCODE,
      'mediafile_src' => $mediafile_id,
      'tool' => $job_parameters['tool'],
      'file_extension' => $job_parameters['file_extension'],
      'command' => $job_parameters['command'],
    );
    $rest_url = 'server/jobstart?' . http_build_query($query_data);

    // Log call.
    self::log_mediafile($mediafile_id, 'Start job:' . $rest_url);

    // Do request.
    $response = mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);

    if ($response->code == '201' || $response->code == '200') {
      try {
        $simple_xml_response = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        self::log_mediafile(
          $mediafile_id,
          'Parse XML: @message (@data)',
          array(
            '@message' => $e->getMessage(),
            '@data' => print_r($response->data, TRUE),
          ),
          '',
          WATCHDOG_ERROR
        );
      }

      return (int) $simple_xml_response->header->request_result_id;
    }

    self::log_mediafile(
      $mediafile_id,
      'errorcode @response with url @rest_url (@header)',
      array(
        '@response' => $response->code,
        '@rest_url' => $rest_url,
        '@header' => var_export($response->header, TRUE),
      )
    );
    return $response->code;
  }

  /**
   * Start analyse job by REST call.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $uri
   *   The job server URI.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function start_server_analyse_job($job_id, $uri, $mediafile_id) {

    // GET parameters.
    $query_data = array(
      'job_id' => $job_id,
      'job_type' => mediamosa_job_db::JOB_TYPE_ANALYSE,
      'mediafile_src' => $mediafile_id,
    );
    $rest_url = 'server/jobstart?' . http_build_query($query_data);

    // Log call.
    self::log_mediafile($mediafile_id, 'Start job:' . $rest_url);

    // Do request.
    $response = mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);

    if ($response->code == '201' || $response->code == '200') {
      try {
        $simple_xml_response = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        self::log_mediafile(
          $mediafile_id,
          'Parse XML: @message (@data)',
          array(
            '@message' => $e->getMessage(),
            '@data' => print_r($response->data, TRUE),
          ),
          '',
          WATCHDOG_ERROR
        );
      }

      return (int) $simple_xml_response->header->request_result_id;
    }

    self::log_mediafile(
      $mediafile_id,
      'errorcode @code with url @uri/@rest_url',
      array(
        '@code' => $response->code,
        '@rest_url' => $rest_url,
        '@uri' => $uri)
    );
    return $response->code;
  }

  /**
   * Function for starting a still job.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $uri
   *   The uri of the job server.
   * @param string $mediafile_id
   *   The ID for the mediafile.
   *
   * @return integer
   *   The HTTP code returned by the job server call.
   */
  public static function start_server_still_job($job_id, $uri, $mediafile_id) {
    // Get job parameters;
    $job_parameters = self::get_job_parameters($job_id, mediamosa_job_db::JOB_TYPE_STILL, $mediafile_id);

    $a_mediafile = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id, array(
      array(
        'prop_name' => mediamosa_asset_mediafile_metadata::FILE_DURATION,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
      array(
        'prop_name' => mediamosa_asset_mediafile_metadata::FPS,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      ),
    ));
    $actual_duration = $a_mediafile[mediamosa_asset_mediafile_metadata::FILE_DURATION];
    $fps = $a_mediafile[mediamosa_asset_mediafile_metadata::FPS];
    $video_duration = 0;
    if ($actual_duration && $actual_duration != '') {
      // Remove the ms from time.
      $actual_duration = mediamosa_unicode::substr($actual_duration, 0, 8);
      @list($uren, $minuten, $seconden) = explode(':', $actual_duration, 3);
      $video_duration = (($uren * 3600) + ($minuten * 60) + $seconden);
      if ($video_duration < $job_parameters['frametime']) {
        // If the video duration is less than still picture time, we decrease
        // the still picture time to half of video duration.
        $job_parameters['frametime'] = ($video_duration >> 1);
      }
    }

    // Send the job to the job server.
    $query_data = array(
      'job_id' => $job_id,
      'job_type' => mediamosa_job_db::JOB_TYPE_STILL,
      'mediafile_src' => $mediafile_id,
      'frametime' => $job_parameters['frametime'],
      'size' => $job_parameters['size'],
      'h_padding' => $job_parameters['h_padding'],
      'v_padding' => $job_parameters['v_padding'],
      'blackstill_check' => $job_parameters['blackstill_check'],
      'still_type' => $job_parameters['still_parameters']['still_type'],
      'still_per_mediafile' => $job_parameters['still_parameters']['still_per_mediafile'],
      'still_every_second' => $job_parameters['still_parameters']['still_every_second'],
      'start_frame' => $job_parameters['still_parameters']['start_frame'],
      'end_frame' => $job_parameters['still_parameters']['end_frame'],
      'video_duration' => $video_duration,
      'fps' => $fps,
      'tag' => $job_parameters['still_parameters']['tag'],
      'watermark_id' => $job_parameters['still_parameters']['watermark_id'],
      'watermark_dst_x' => $job_parameters['still_parameters']['watermark_dst_x'],
      'watermark_dst_y' => $job_parameters['still_parameters']['watermark_dst_y'],
      'watermark_pct' => $job_parameters['still_parameters']['watermark_pct'],
      'watermark_v_align' => $job_parameters['still_parameters']['watermark_v_align'],
      'watermark_h_align' => $job_parameters['still_parameters']['watermark_h_align']
    );
    $rest_url = 'server/jobstart?' . http_build_query($query_data);

    // Log call.
    self::log_mediafile($mediafile_id, 'Start job:' . $rest_url);

    // Do request.
    $response = mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);

    if ($response->code == '201' || $response->code == '200') {
      try {
        $simple_xml_response = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        self::log_mediafile(
          $mediafile_id,
          'Parse XML: @message (@data)',
          array(
            '@message' => $e->getMessage(),
            '@data' => print_r($response->data, TRUE),
          ),
          '',
          WATCHDOG_ERROR
        );
      }

      return (int) $simple_xml_response->header->request_result_id;
    }

    self::log_mediafile($mediafile_id, 'errorcode @response with url @rest_url', array('@response' => $response->code, '@rest_url' => $rest_url));
    return $response->code;
  }

  /**
   * Start media job by REST call.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $job_type
   *   The job type.
   * @param string $uri
   *   The URI of the job server that will execute the job.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function start_server_job_media($job_id, $job_type, $uri, $mediafile_id) {
    // Get job.
    $job = mediamosa_job::get($job_id);

    // GET parameters.
    $query_data = array(
      mediamosa_rest_call_server_job_execute::JOB_TYPE => $job_type,
      mediamosa_rest_call_server_job_execute::MEDIAFILE_ID => $mediafile_id,
      mediamosa_rest_call_server_job_execute::JOB_DATA => $job[mediamosa_job_db::STILL_PARAMETERS],
    );
    $rest_url = rtrim($uri, '/') . '/internal/job/' . $job_id . '/execute?' . http_build_query($query_data);

    // Log call.
    self::log_mediafile($mediafile_id, 'Execute job:' . $rest_url);

    // Do HEAD request.
    mediamosa_http::do_head_internal_call($rest_url);

    // HEAD calls do not wait for okay.
    return mediamosa_error::ERRORCODE_OKAY;
  }

  /**
   * Fetch an single available job.
   *
   * Choose a job that applies to the following rules;
   *  - Job status is not WAITING,
   *  - There is no upload job for the same asset,
   *  - Job must not already have started on another server,
   *  - Tool servers have slot left,
   *  - Priority goes first.
   *
   * @param string $server_id
   *   The server ID.
   *
   * @index `status`,`asset_id`,`job_id`, `job_type`
   */
  public static function fetch_single_available_job($server_id) {
    return mediamosa_db::db_query(
      'SELECT job_id, job_type, asset_id, mediafile_id FROM {#mediamosa_job} AS mj WHERE
            mj.#job_status = :status
          AND
            mj.asset_id NOT IN
              (SELECT asset_id FROM {#mediamosa_job} AS mj2
               JOIN {#mediamosa_server_job} AS msj USING(job_id)
               WHERE mj2.#job_status IN (:JOBSTATUS_INPROGRESS, :JOBSTATUS_CANCELLING, :JOBSTATUS_WAITING))
          AND
            mj.asset_id NOT IN
              (SELECT asset_id FROM {#mediamosa_job} AS mj3
               WHERE mj3.job_type = :JOBTYPE_UPLOAD AND mj3.#job_status IN (:JOBSTATUS_INPROGRESS, :JOBSTATUS_CANCELLING))
          AND
            mj.job_id NOT IN
              (SELECT job_id FROM {#mediamosa_server_job})
          AND
            mj.job_type != :JOBTYPE_UPLOAD
          AND
            (
              (mj.job_type = :JOBTYPE_TRANSCODE AND
                (SELECT COUNT(*) FROM {#mediamosa_job_transcode} AS mjt
                  JOIN {#mediamosa_server_tool} USING(tool)
                  WHERE #nid = :nid AND mjt.job_id = mj.job_id LIMIT 1) = 1)
            OR
              (mj.job_type = :JOBTYPE_ANALYSE AND
                (SELECT COUNT(*) FROM {#mediamosa_server_tool} AS mstt1
                  WHERE mstt1.tool = :TOOL_ANALYSE AND mstt1.#nid = :nid LIMIT 1) = 1)
            OR
              (mj.job_type = :JOBTYPE_STILL AND
                (SELECT COUNT(*) FROM {#mediamosa_server_tool} AS mstt2
                  WHERE mstt2.tool = :TOOL_STILL AND mstt2.#nid = :nid LIMIT 1) = 1)
            OR
              (mj.job_type IN (:JOBTYPE_MEDIA_DOWNLOAD, :JOBTYPE_MEDIA_UPLOAD, :JOBTYPE_MEDIA_MOVE) AND
                (SELECT COUNT(*) FROM {#mediamosa_server_tool} AS mstt3
                  WHERE mstt3.tool = :TOOL_TRANSFER AND mstt3.#nid = :nid LIMIT 1) = 1)
            OR
              (mj.job_type IN (:JOBTYPE_DELETE_MEDIAFILE))
            )
          ORDER BY priority ASC, job_id ASC
          LIMIT 0,1',
      array(
        '#job_status' => mediamosa_job_db::JOB_STATUS,
        ':status' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBSTATUS_INPROGRESS' => mediamosa_job_db::JOB_STATUS_INPROGRESS,
        ':JOBSTATUS_CANCELLING' => mediamosa_job_db::JOB_STATUS_CANCELLING,
        ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
        ':JOBTYPE_UPLOAD' => mediamosa_job_db::JOB_TYPE_UPLOAD,
        ':JOBSTATUS_INPROGRESS' => mediamosa_job_db::JOB_STATUS_INPROGRESS,
        ':JOBSTATUS_CANCELLING' => mediamosa_job_db::JOB_STATUS_CANCELLING,
        ':JOBTYPE_TRANSCODE' => mediamosa_job_db::JOB_TYPE_TRANSCODE,
        ':nid' => $server_id,
        '#nid' => mediamosa_server_tool_db::NID,
        ':JOBTYPE_ANALYSE' => mediamosa_job_db::JOB_TYPE_ANALYSE,
        ':JOBTYPE_STILL' => mediamosa_job_db::JOB_TYPE_STILL,
        ':JOBTYPE_DELETE_MEDIAFILE' => mediamosa_job_db::JOB_TYPE_DELETE_MEDIAFILE,
        ':JOBTYPE_MEDIA_DOWNLOAD' => mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD,
        ':JOBTYPE_MEDIA_UPLOAD' => mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_UPLOAD,
        ':JOBTYPE_MEDIA_MOVE' => mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_MOVE,
        ':TOOL_ANALYSE' => mediamosa_job_server::MEDIAMOSA_JOB_SERVER_TOOL_ANALYSE,
        ':TOOL_STILL' => mediamosa_job_server::MEDIAMOSA_JOB_SERVER_TOOL_STILL,
        ':TOOL_TRANSFER' => mediamosa_job_server::MEDIAMOSA_JOB_SERVER_TOOL_TRANSFER,
        '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
        '#mediamosa_server_job' => mediamosa_server_job_db::TABLE_NAME,
        '#mediamosa_job_transcode' => mediamosa_job_transcode_db::TABLE_NAME,
        '#mediamosa_server_tool' => mediamosa_server_tool_db::TABLE_NAME,
      )
    )->fetchAssoc();
  }

  /**
   * Based on job type, get more information.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $job_type
   *   The job type, see mediamosa_job_db::JOB_TYPE_*.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function get_job_parameters($job_id, $job_type, $mediafile_id) {
    $result = array();

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        // Get job transcode.
        $job_transcode = mediamosa_job_transcode::get($job_id);

        $result['profile_id'] = $job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID];
        $result['tool'] = $job_transcode[mediamosa_job_transcode_db::TOOL];
        $result['file_extension'] = $job_transcode[mediamosa_job_transcode_db::FILE_EXTENSION];
        $result['command'] = self::map_parameters($job_transcode[mediamosa_job_transcode_db::TOOL], $job_transcode[mediamosa_job_transcode_db::COMMAND], $mediafile_id);
        break;

      case mediamosa_job_db::JOB_TYPE_STILL:
        $query_job = mediamosa_job_still::get($job_id);

        if ($query_job) {
          $result['blackstill_check'] = $query_job['blackstill_check'];
          $result['still_parameters'] = unserialize($query_job['still_parameters']);
          $result['frametime'] = $result['still_parameters']['frametime'];
          $result['h_padding'] = $result['still_parameters']['h_padding'];
          $result['v_padding'] = $result['still_parameters']['v_padding'];
          $result['tag'] = $result['still_parameters']['tag'];

          $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, array(mediamosa_asset_mediafile_db::APP_ID));
          $app_id = $mediafile[mediamosa_asset_mediafile_db::APP_ID];

          // Pre-defined ratios.
          $sizes = array(
            'sqcif' => '128x96', 'qcif' => '176x144', 'cif' => '352x288', '4cif' => '704x576',
            'qqvga' => '160x120', 'qvga' => '320x240', 'vga' => '640x480', 'svga' => '800x600',
            'xga' => '1024x768', 'uxga' => '1600x1200', 'qxga' => '2048x1536', 'sxga' => '1280x1024',
            'qsxga' => '2560x2048', 'hsxga' => '5120x4096', 'wvga' => '852x480', 'wxga' => '1366x768',
            'wsxga' => '1600x1024', 'wuxga' => '1920x1200', 'woxga' => '2560x1600',
            'wqsxga' => '3200x2048', 'wquxga' => '3840x2400', 'whsxga' => '6400x4096',
            'whuxga' => '7680x4800', 'cga' => '320x200', 'ega' => '640x350', 'hd360' => '640x360',
            'hd480' => '852x480', 'hd720' => '1280x720', 'hd1080' => '1920x1080',
          );

          // Find target size.
          if (isset($sizes[$query_job['size']])) {
            $target_size = $sizes[$query_job['size']];
          }
          else {
            // Use size of still parameters instead.
            $target_size = $result['still_parameters']['size'];

            // Check the size.
            if (preg_match('/(\d+)x(\d+)/', $target_size)) {
              // If there is a still default size for the client app available,
              // then use that instead.
              $target_size = mediamosa_app::get_still_default_size($app_id);
              if (!$target_size) {
                // Get the video size.
                $target_size = mediamosa_asset_mediafile::get_size($mediafile_id);
              }
            }
          }

          // First get source width and height.
          $metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id, array(
            array(
              'prop_name' => mediamosa_asset_mediafile_metadata::WIDTH,
              'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
            ),
            array(
              'prop_name' => mediamosa_asset_mediafile_metadata::HEIGHT,
              'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
            ),
          ));
          $width = $metadata[mediamosa_asset_mediafile_metadata::WIDTH];
          $height = $metadata[mediamosa_asset_mediafile_metadata::HEIGHT];
          $still_padding = mediamosa_app::get_still_padding_value($app_id);

          // Get the parameter settings.
          $calc_aspect_ratio = mediamosa_gd::calcAspectRatio($width, $height, $target_size, $result['h_padding'], $result['v_padding'], $still_padding == mediamosa_app_db::STILL_PADDING_YES);

          // Set result.
          if ($calc_aspect_ratio) {
            $result['size'] = $calc_aspect_ratio['width'] . 'x' . $calc_aspect_ratio['height'];
            $result['h_padding'] = $calc_aspect_ratio['h_padding'];
            $result['v_padding'] = $calc_aspect_ratio['v_padding'];
          }
          else {
            if ($width && $height) {
              $result['size'] = $width . 'x' . $height;
            }
            else {
              $result['size'] = '640x360';
            }
            $result['h_padding'] = 0;
            $result['v_padding'] = 0;
          }
        }
        else {
          // Something went wrong in the analyse script.
          // Fall back to the default values.
          $result['frametime'] = mediamosa_settings::STILL_DEFAULT_FRAME_TIME;
          $result['size'] = '640x360';
          $result['h_padding'] = 0;
          $result['v_padding'] = 0;
          $result['blackstill_check'] = 'FALSE';
          $result['tag'] = '';
        }
        break;

      case mediamosa_job_db::JOB_TYPE_ANALYSE:
        break;
    }

    return $result;
  }

  /**
   * Check and map profile parameters.
   *
   * @param string $tool
   *   Name of the tool.
   * @param string $command
   *   The command string.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function map_parameters($tool, $command, $mediafile_id) {
    // Result commands.
    $commands_mapped = array();

    // Into array.
    $commands = mediamosa_transcode_profile::commandToArray($command);

    // First determine if we have to alter the aspect ratio.
    $alter_size_param = (isset($commands['maintain_aspect_ratio']) && $commands['maintain_aspect_ratio'] == 'yes' && isset($commands['size']));

    // Get all parameters based on the toolname that can be queried.
    //
    // Check the given paramters.
    $tool_params = mediamosa_tool_params::get_by_tool($tool);
    foreach ($tool_params as $tool_param) {
      if ($tool_param[mediamosa_tool_params_db::NICE_PARAMETER] == 'size' && $alter_size_param) {
        // We assume it exists.
        $target_size = $commands['size'];
      }
      else {
        $mapped = self::mapping_value(
          $commands,
          $tool_param[mediamosa_tool_params_db::NICE_PARAMETER],
          $tool_param[mediamosa_tool_params_db::TOOL_PARAMETER],
          $tool_param[mediamosa_tool_params_db::MIN_VALUE],
          $tool_param[mediamosa_tool_params_db::MAX_VALUE],
          $tool_param[mediamosa_tool_params_db::ALLOWED_VALUE],
          $tool_param[mediamosa_tool_params_db::DEFAULT_VALUE],
          $tool_param[mediamosa_tool_params_db::REQUIRED],
          $tool_param[mediamosa_tool_params_db::TYPE_PARAMETER]
        );

        // Store it?
        if (!empty($mapped) && $mapped['value'] !== '') {
          $commands_mapped[$mapped['name']] = $mapped['value'];
        }
      }
    }

    // Adjust for aspect ratio? Makes only sense if size is set, else we
    // convert to size same as source (default ffmpeg behaviour).
    if ($alter_size_param) {

      // First get source width and height.
      $metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id, array(
        array(
          'prop_name' => mediamosa_asset_mediafile_metadata::WIDTH,
          'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
        ),
        array(
          'prop_name' => mediamosa_asset_mediafile_metadata::HEIGHT,
          'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
        ),
      ));

      // Determine if padding required or not.
      $use_padding = !(isset($commands['padding']) && $commands['padding'] == 'no');

      // Get the parameter string.
      $ratio = mediamosa_tool_ffmpeg::calcAspectRatio($metadata[mediamosa_asset_mediafile_metadata::WIDTH], $metadata[mediamosa_asset_mediafile_metadata::HEIGHT], $target_size, TRUE, NULL, NULL, $use_padding);
      if (!empty($ratio)) {
        $commands_mapped = array_merge($commands_mapped, $ratio);
      }
    }

    // Convert from array to string.
    return mediamosa_transcode_profile::arrayToCommand($commands_mapped);
  }

  /**
   * Check the mapping and value.
   *
   * @param array $commands
   *   The command line as associated array (name => value).
   * @param string $nice_parameter
   *   The easily readable parameter name.
   * @param string $tool_parameter
   *   The tool parameter name.
   * @param integer $min_value
   *   The minimum value.
   * @param integer $max_value
   *   The maximum value.
   * @param string $allowed_value
   *   The allowed value serialized string.
   * @param string $default_value
   *   The default value when value is not found.
   * @param string $required
   *   Either 'TRUE' or 'FALSE'. When 'TRUE' and no value was found, then the
   *   default value is returned.
   * @param string $type_parameter
   *   If needed, the special type of the parameter.
   */
  public static function mapping_value(array $commands, $nice_parameter, $tool_parameter, $min_value, $max_value, $allowed_value, $default_value, $required, $type_parameter) {
    $result = array();

    // If the 2 pass encoding parameter doesn't have the value '2', then we skip
    // it. Then we ensure, we have "-pass 2" or nothing.
    //
    // FIXME: ffmpeg related code here.
    if ($nice_parameter == '2_pass_h264_encoding' && isset($commands['2_pass_h264_encoding']) && $commands['2_pass_h264_encoding'] != '2') {
      return $result;
    }

    foreach ($commands as $name => $value) {
      if ($name == $nice_parameter && $tool_parameter != '') {

        // To map.
        $result = array('name' => $tool_parameter, 'value' => $value);

        // Switch parameter.
        if ($type_parameter == mediamosa_tool_params_db::TYPE_PARAMETER_CHECKBOX) {
          if ($value == mediamosa_tool_params_db::ALLOWED_VALUE_FOR_SWITCH) {
            return $result;
          }
          return array();
        }

        // Validate values.
        if (!is_null($min_value) && $min_value > $value) {
          return array('name' => $tool_parameter, 'value' => $default_value);
        }

        if (!is_null($max_value) && $max_value < $value) {
          return array('name' => $tool_parameter, 'value' => $default_value);
        }

        if (!is_null($allowed_value) && $allowed_value !== '') {
          $allowed_values = unserialize($allowed_value);
          if (!in_array($value, $allowed_values)) {
            return array('name' => $tool_parameter, 'value' => $default_value);
          }
        }

        // Done all checks.
        return $result;
      }
    }

    // Check if found and not required.
    if (empty($result)) {
      if ($required == 'TRUE') {
        return array('name' => $tool_parameter, 'value' => $default_value);
      }
    }

    return $result;
  }

  /**
   * Function for starting a delete job.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function start_server_delete_job($job_id, $mediafile_id) {

    // Get complete job.
    $job_ext = mediamosa_job::get_job_ext($job_id);

    // Delete mediafile.
    try {
      mediamosa_asset_mediafile::delete($mediafile_id);

      // Update status.
      mediamosa_job::update_status($job_ext, mediamosa_job_db::JOB_STATUS_FINISHED, '1.000');
    }
    catch (Exception $e) {
      // Update with failure.
      mediamosa_job::update_status($job_ext, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', $e->getMessage());
    }
  }

  /**
   * Remove timed-out transcoding jobs.
   */
  public static function check_transcode_timeout() {

    $result = mediamosa_db::db_query(
       'SELECT mj.#job_id, mj.#asset_id, mj.#mediafile_id, ms.#uri FROM {#mediamosa_job} AS mj
        JOIN {#mediamosa_server_job} AS msj ON msj.#job_id = mj.#job_id
        JOIN {#mediamosa_server} AS ms ON ms.#nid = msj.#server_id
        WHERE NOT mj.#started IS NULL AND TIME_TO_SEC(TIMEDIFF(utc_timestamp(), mj.#started)) > :JOB_TRANSCODE_TIMEOUT AND #server_type = :server_type_job_processor',
        array(
          '#job_id' => mediamosa_job_db::ID,
          '#asset_id' => mediamosa_job_db::ASSET_ID,
          '#mediafile_id' => mediamosa_job_db::MEDIAFILE_ID,
          '#uri' => mediamosa_server_db::SERVER_URI,
          ':JOB_TRANSCODE_TIMEOUT' => mediamosa_settings::JOB_TRANSCODE_TIMEOUT,
          '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
          '#mediamosa_server_job' => mediamosa_server_job_db::TABLE_NAME,
          '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
          '#nid' => mediamosa_server_db::NID,
          '#server_id' => mediamosa_server_job_db::SERVER_ID,
          '#server_type' => mediamosa_server_db::SERVER_TYPE,
          ':server_type_job_processor' => mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR,
          '#started' => mediamosa_job_db::STARTED,
        )
    );

    foreach ($result as $result_row) {
      // For each found job, send a remove to the transcode server.
      self::remove_job_on_server($result_row[mediamosa_server_db::SERVER_URI], $result_row[mediamosa_job_db::ID]);

      // Delete the job from the server job table.
      mediamosa_server_job::delete_by_jobid($result_row[mediamosa_job_db::ID]);

      // Update the status of the job.
      $error_message = mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_JOB_TRANSCODE_TIMEOUT, array('@timeout' => mediamosa_settings::JOB_TRANSCODE_TIMEOUT));

      // Get the status.
      $job = mediamosa_job::get($result_row[mediamosa_job_db::ID], array(mediamosa_job_db::JOB_STATUS));
      $status = $job[mediamosa_job_db::JOB_STATUS];
      mediamosa_job::set_job_status_failed($result_row[mediamosa_job_db::ID], $error_message, $status);

      self::log_mediafile($result_row[mediamosa_job_db::MEDIAFILE_ID], 'Transcode job was deleted because of timeout, job_id: @job_id', array('@job_id' => $result_row[mediamosa_job_db::ID]), $result_row[mediamosa_job_db::ASSET_ID]);
    }
  }

  /**
   * Check Jobs which have no progress. Change status to FAILED.
   */
  public static function check_upload_timeout() {

    // Upload jobs in progress and waiting which have no activity.
    mediamosa_db::db_query(
       'UPDATE {#mediamosa_job} SET #job_status = :JOBSTATUS_FAILED, FINISHED = UTC_TIMESTAMP(), error_description = :error_description
        WHERE #job_status in (:JOBSTATUS_INPROGRESS, :JOBSTATUS_WAITING) AND job_id IN
        (SELECT job_id FROM {#mediamosa_job_upload} WHERE (UTC_TIMESTAMP() - changed) > #JOB_UPLOAD_TIMEOUT)',
        array(
          '#job_status' => mediamosa_job_db::JOB_STATUS,
          '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
          '#mediamosa_job_upload' => mediamosa_job_upload_db::TABLE_NAME,
          ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
          '#JOB_UPLOAD_TIMEOUT' => mediamosa_settings::JOB_UPLOAD_TIMEOUT,
          ':JOBSTATUS_INPROGRESS' => mediamosa_job_db::JOB_STATUS_INPROGRESS,
          ':JOBSTATUS_WAITING' => mediamosa_job_db::JOB_STATUS_WAITING,
          ':error_description' => strtr('UPLOAD changed timeout expired (#JOB_UPLOAD_TIMEOUTs)', array('#JOB_UPLOAD_TIMEOUT' => mediamosa_settings::JOB_UPLOAD_TIMEOUT)),
        )
    );
    // Jobs with no activity for 3 hours.
    mediamosa_db::db_query(
       'UPDATE {#mediamosa_job}
        SET #job_status = :JOBSTATUS_FAILED, FINISHED = UTC_TIMESTAMP(), error_description = :error_description
        WHERE #job_status = :JOBSTATUS_INPROGRESS AND (UTC_TIMESTAMP() - changed) > #JOB_JOB_TIMEOUT',
        array(
          '#job_status' => mediamosa_job_db::JOB_STATUS,
          '#mediamosa_job' => mediamosa_job_db::TABLE_NAME,
          ':JOBSTATUS_FAILED' => mediamosa_job_db::JOB_STATUS_FAILED,
          '#JOB_JOB_TIMEOUT' => mediamosa_settings::JOB_JOB_TIMEOUT,
          ':JOBSTATUS_INPROGRESS' => mediamosa_job_db::JOB_STATUS_INPROGRESS,
          ':error_description' => strtr('JOB changed timeout expired (#JOB_UPLOAD_TIMEOUTs)', array('#JOB_UPLOAD_TIMEOUT' => mediamosa_settings::JOB_JOB_TIMEOUT)),
        )
    );
  }

  /**
   * Check if there are servers on CLOSE and have no jobs, then set them to OFF.
   */
  public static function check_server_on_status_close() {

    mediamosa_db::db_query(
       'UPDATE {#mediamosa_server}
        SET #server_status = :server_status_off WHERE
        #server_status = :server_status_close AND #nid NOT IN (SELECT DISTINCT #nid FROM {#mediamosa_server_job})',
      array(
        '#mediamosa_server' => mediamosa_server_db::TABLE_NAME,
        '#server_status' =>  mediamosa_server_db::SERVER_STATUS,
        ':server_status_off' =>  mediamosa_server_db::SERVER_STATUS_OFF,
        ':server_status_close' => mediamosa_server_db::SERVER_STATUS_CLOSE,
        '#mediamosa_server_job' => mediamosa_server_job_db::TABLE_NAME,
        '#nid' => mediamosa_server_db::NID,
      )
    );
  }
}
