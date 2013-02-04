<?php
/**
 * @file
 * Contains mediamosa_job_server.
 */

/**
 * Represents a job server.
 */
class mediamosa_job_server {
  // ------------------------------------------------------------------- Consts.
  /**
   * Quick fix for tool type for job server selection for the job types
   * JOB_TYPE_TRANSFER_MEDIA_*.
   *
   * @see mediamosa_mediamosa_tool_info().
   */
  const MEDIAMOSA_JOB_SERVER_TOOL_TRANSFER = 'TRANSFER';

  /**
   * For analyse runs (is part of any tool).
   */
  const MEDIAMOSA_JOB_SERVER_TOOL_ANALYSE = 'ANALYSE';

  /**
   * For still generation (is part of any tool).
   */
  const MEDIAMOSA_JOB_SERVER_TOOL_STILL = 'STILL';

  // ---------------------------------------------------------------- Functions.
  /**
   * Directory extension for tools
   */
  const DIRECTORY_EXTENSION = '_tool';
  /**
   * Log for job_server.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'job server');
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
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'job server');
  }

  /**
   * Log debug for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  public static function log_debug($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'job_server');
  }

  /**
   * Log debug mediafile id for job_server.
   *
   * @param string $mediafile_id
   * @param string $asset_id
   * @param string $message
   * @param array $variables
   */
  public static function log_debug_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL) {
    mediamosa_debug::log_mediafile($mediafile_id, $message, $variables, $asset_id, 'job_server');
  }

  /**
   * Log debug for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  public static function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_high($message, $variables, 'job_server');
  }

  /**
   * Log debug mediafile_id for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  public static function log_debug_high_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL) {
    mediamosa_debug::log_high_mediafile($mediafile_id, $message, $variables, $asset_id, 'job_server');
  }

  /**
   * Trigger every minute.
   *
   * Will timeout on itself after 2 minutes.
   */
  public static function run_parse_queue() {

    // Trigger from sandbox, run once, and done.
    if (mediamosa::in_simpletest_sandbox()) {
      self::log('Job Server; run_parse_queue() in sandbox, hitting.');
      self::parse_queue();
      self::log('Job Server; run_parse_queue() in sandbox, returning.');
      return;
    }

    // Get the current server ID.
    $server_id = mediamosa::get_server_id();

    $run_last = variable_get('mediamosa_jobserver_cron_last_' . $server_id, NULL);

    // may run? (0 - 60 seconds).
    if (round(REQUEST_TIME - $run_last) < 60) {
      return; // Still running.
    }

    // A period of 60 to 120 seconds we wait till its killed (or done running).
    $run_last = variable_get('mediamosa_jobstatus_cron_running_' . $server_id, NULL);
    if (round(REQUEST_TIME - $run_last) < 120) {
      self::log('run_parse_queue() in safety timeout zone.');
      return;
    }

    variable_set('mediamosa_jobserver_cron_last_' . $server_id, REQUEST_TIME);

    // Log it.
    self::log_debug('Start parse queue job server');

    // We set this to NULL when we are done.
    variable_set('mediamosa_jobserver_cron_running_' . $server_id, REQUEST_TIME);

    // Set max run time on 120 sec.
    drupal_set_time_limit(120);
    for ($x = 0; $x < 6; $x++) {
      $start_at = microtime(TRUE);

      try {
        // Parse queue.
        self::parse_queue();
      }
      catch (mediamosa_exception $e) {
        // ignore, its logged
      }
      // Now take the time it took, and subtrac that from 10.
      $time_to_sleep = 10 - round(microtime(TRUE) - $start_at);
      self::log_debug_high('Server run_parse_queue: Time to sleep @time', array('@time' => $time_to_sleep));
      if ($time_to_sleep > 0) {
        sleep($time_to_sleep);
      }

      // And repeat for 6 times = 1 minute = 1 cron run.
    }

    // Done.
    variable_set('mediamosa_server_cron_running_' . $server_id, NULL);

    // Log it.
    self::log_debug('Ending parse queue job server');
  }

  /**
   * Parse the queue jobs.
   *
   * This function is the engine of the job server (job processor). Should be
   * called at regular intervals.
   *
   * 1. Get a list of all jobs that are not finished, failed or canceled.
   * 2. For all jobs with status WAITING, the job is started.
   * 3. For all running jobs de status is updated.
   */
  public static function parse_queue() {
    // Retrieve all jobs of this server.
    $job_server_jobs = mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::get_server_id())
      ->condition(mediamosa_job_server_db::JOB_STATUS, array(mediamosa_job_server_db::JOB_STATUS_FINISHED, mediamosa_job_server_db::JOB_STATUS_FAILED, mediamosa_job_server_db::JOB_STATUS_CANCELLED), 'NOT IN')
      ->orderBy(mediamosa_job_server_db::ID, 'ASC')
      ->execute();

    foreach ($job_server_jobs as $job_server_job) {
        if ($job_server_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING) {
          try {
            // New job, start it.
            self::waiting_job_start($job_server_job);
          }
          catch (Exception $e) {
            // Failed, make sure it gets canceled.
            self::set_job_status($job_server_job[mediamosa_job_server_db::JOB_ID], mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'Exception caught during job start, message; ' . $e->getMessage());
          }
        }
        else {
          try {
            // Running job, update it.
            self::running_job_update($job_server_job);
          }
          catch (Exception $e) {
            // Failed, make sure it gets canceled.
            self::set_job_status($job_server_job[mediamosa_job_server_db::JOB_ID], mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'Exception caught during running job update, message; ' . $e->getMessage());
          }
        }
    }
  }

  /**
   * Create analyse server job.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $mediafile_id_src
   *   The ID of the mediafile to analyse.
   */
  public static function create_job_analyse($job_id, $mediafile_id_src) {
    // Set job type.
    $job_type = mediamosa_job_server_db::JOB_TYPE_ANALYSE;

    // Create basic server job.
    $jobserver_job_id = self::create_job($job_id, $job_type, $mediafile_id_src);

    $fields = array(
      mediamosa_job_server_analyse_db::ID => $jobserver_job_id,
      mediamosa_job_server_analyse_db::ANALYSE_RESULT => '',
    );

    // Enrich with created time.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_server_analyse_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Create transcode server job.
   *
   * @param $job_id
   */
  public static function create_job_transcode($job_id, $mediafile_id_src, $tool, $file_extension, $command) {
    // Set job type.
    $job_type = mediamosa_job_server_db::JOB_TYPE_TRANSCODE;

    // Create basic server job.
    $jobserver_job_id = self::create_job($job_id, $job_type, $mediafile_id_src);

    $fields = array(
      mediamosa_job_server_transcode_db::ID => $jobserver_job_id,
      mediamosa_job_server_transcode_db::TOOL => $tool,
      mediamosa_job_server_transcode_db::FILE_EXTENSION => $file_extension,
      mediamosa_job_server_transcode_db::COMMAND => $command,
    );

    // Enrich with created time.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_server_transcode_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Create still server job.
   *
   * @param $job_id
   */
  public static function create_job_still($job_id, $mediafile_id_src, $frametime, $size, $h_padding, $v_padding, $blackstill_check, array $a_still_parameters) {
    // Set job type.
    $job_type = mediamosa_job_server_db::JOB_TYPE_STILL;

    // Create basic server job.
    $jobserver_job_id = self::create_job($job_id, $job_type, $mediafile_id_src);

    $fields = array(
      mediamosa_job_server_still_db::ID => $jobserver_job_id,
      mediamosa_job_server_still_db::FRAMETIME => $frametime,
      mediamosa_job_server_still_db::SIZE => $size,
      mediamosa_job_server_still_db::H_PADDING => $h_padding,
      mediamosa_job_server_still_db::V_PADDING => $v_padding,
      mediamosa_job_server_still_db::BLACKSTILL_CHECK => ($blackstill_check ? mediamosa_job_server_still_db::BLACKSTILL_CHECK_TRUE : mediamosa_job_server_still_db::BLACKSTILL_CHECK_FALSE),
      mediamosa_job_server_still_db::STILL_PARAMETERS => serialize($a_still_parameters)
    );

    // Enrich with created time.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_server_still_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Create a job server job.
   *
   * @param integer $job_id
   *   The job ID.
   */
  public static function create_job($job_id, $job_type, $mediafile_id_src) {

    // Get the job_server_job, make sure its not already created.
    $job_server_job = mediamosa_job_server::get_with_jobid($job_id);

    if (!empty($job_server_job)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STARTING_JOB_FAILED);
    }

    // Data we want to insert.
    $fields = array(
      mediamosa_job_server_db::JOB_ID => $job_id,
      mediamosa_job_server_db::JOB_TYPE => $job_type,
      mediamosa_job_server_db::JOB_STATUS => mediamosa_job_server_db::JOB_STATUS_WAITING,
      mediamosa_job_server_db::MEDIAFILE_ID_SRC => $mediafile_id_src,
      mediamosa_job_server_db::INSTALL_ID => mediamosa::get_server_id(),
    );

    // Enrich with created time.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_job_server_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Start the job.
   *
   * @param array $job_server_job
   *   The server job array contains all the information stored in one single
   *   job server row in mediamosa_job_server table.
   */
  public static function waiting_job_start(array $job_server_job) {

    // Job ID.
    $job_id = $job_server_job[mediamosa_job_server_db::JOB_ID];

    // Job type.
    $job_type = $job_server_job[mediamosa_job_server_db::JOB_TYPE];

    // Some jobs are started by job/$job_id/execute, but not here.
    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD:
      case mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_MOVE:
      case mediamosa_job_server_db::JOB_TYPE_TRANSFER_MEDIA_UPLOAD:
        return;

      default:
        break;
    }

    // Source Mediafile ID.
    $mediafile_id_src = $job_server_job[mediamosa_job_server_db::MEDIAFILE_ID_SRC];

    // Job server id (ID of the row in the mediamosa_job_server table).
    $jobserver_job_id = $job_server_job[mediamosa_job_server_db::ID];

    // Need mediafile.
    $mediafile_src = mediamosa_asset_mediafile::must_exists($mediafile_id_src);

    // Get the filename of the source.
    $mediamosa_uri = mediamosa_storage::get_uri_mediafile($mediafile_src);

    // Check of the source file exists.
    if (!mediamosa_io::file_exists($mediamosa_uri)) {
      $link_asset = self::get_asset_link($job_id);
      self::log_mediafile($mediafile_id_src, "Job ID: @job_id File '@file' not found,<br /><br/>@link", array('@job_id' => $job_id, '@file' => mediamosa_io::realpath_safe($mediamosa_uri), '@link' => $link_asset), NULL, WATCHDOG_CRITICAL);
      self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'File not found (@file)', array('@file' => mediamosa_io::realpath_safe($mediamosa_uri)));
      self::log_mediafile($mediafile_id_src, "Job ID: @job_id, memory_get_peak_usage = @memory_get_peak_usage, memory_usage: @memory_get_usage", array('@job_id' => $job_id, '@memory_get_peak_usage' => memory_get_peak_usage(), '@memory_get_usage' => memory_get_usage()));
      return;
    }

    // Based on jobtype we execute the job.
    $execution_string = '';
    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        // Make sure the tmp transcode directory exists.
        mediamosa_io::mkdir(mediamosa_storage::get_uri_temporary());

        // Clean up possible earlier status files.
        mediamosa_io::unlink(mediamosa_storage::get_realpath_status_file($job_id));

        $execution_string = self::get_transcode_exec($jobserver_job_id, $mediafile_id_src);
        break;

      case mediamosa_job_server_db::JOB_TYPE_STILL:
        $execution_string = self::get_generate_still_exec($jobserver_job_id, $mediafile_id_src);
        break;

      case mediamosa_job_server_db::JOB_TYPE_ANALYSE:
        $execution_string = self::get_analyse_string($mediafile_id_src, $job_id);
        break;

      default:
        self::log_mediafile($mediafile_id_src, 'Unknown job type: @job_type in job id: @job_id.', array('@job_type' => $job_type, '@job_id' => $job_id), NULL, WATCHDOG_ALERT);
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'Unknown job type: @job_type.', array('@job_type' => $job_type));
        return FALSE;
    }

    // Log our action.
    self::log_mediafile($mediafile_id_src, 'About to start @job_type job: @job_id calling exec: @execution_string', array('@job_type' => $job_type, '@job_id' => $job_id, '@execution_string' => $execution_string));

    // Set job in progress.
    self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_INPROGRESS, '0.000');

    // Now execute analyse directly, as its really fast.
    if ($job_type == mediamosa_job_server_db::JOB_TYPE_ANALYSE) {
      $analyse_result = mediamosa_asset_mediafile::analyse(0, $mediafile_id_src, $execution_string);

      $link_asset = self::get_asset_link($job_id);
      self::log_mediafile($mediafile_id_src, 'Job @job_type (Job ID: @job_id) returned output: @output<br /><br />@link',
        array(
          '@job_type' => $job_type,
          '@job_id' => $job_id,
          '@output' => print_r($analyse_result, TRUE),
          '@link' => $link_asset,
        )
      );

      if (!empty($analyse_result)) {
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FINISHED, '1.000');
      }
      else {
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'Empty result, analyse failed.');
      }

      // Log it.
      self::log_mediafile($mediafile_id_src, 'Starting new followup analyse job for job ID: @job_id.', array('@job_id' => $job_id));

      // Update job_server_analyse row.
      $fields = array(
        mediamosa_job_server_analyse_db::ANALYSE_RESULT => serialize($analyse_result),
      );

      // Enrich with changed.
      $fields = mediamosa_db::db_update_enrich($fields);

      // Generate event analyse finished.
      if (module_exists('rules')) {
        rules_invoke_event('mediamosa_event_analyse_finished', $mediafile_id_src);
      }

      // And update row.
      mediamosa_db::db_update(mediamosa_job_server_analyse_db::TABLE_NAME)
        ->fields($fields)
        ->condition(mediamosa_job_server_analyse_db::ID, $jobserver_job_id)
        ->execute();
    }
    else {
      $exec_output = mediamosa_io::exec($execution_string);
      $link_asset = self::get_asset_link($job_id);
      self::log_mediafile($mediafile_id_src, 'Job @job_type (Job ID: @job_id) returned output: @output<br /><br />@link',
        array(
          '@job_type' => $job_type,
          '@job_id' => $job_id,
          '@output' => implode("\n", $exec_output),
          '@link' => $link_asset,
        )
      );
    }
  }

  /**
   * Update the job.
   *
   * Updates running jobs.
   *
   * 1. Get possible status file and parse it.
   * 2. When transcode or still job is done, files will be moved to the
   *    correct location.
   */
  public static function running_job_update($job_server_job) {

    // Job type.
    $job_type = $job_server_job[mediamosa_job_server_db::JOB_TYPE];

    // Only update these types.
    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
      case mediamosa_job_server_db::JOB_TYPE_STILL:
        break;

      default:
        return;
    }

    // Job ID.
    $job_id = $job_server_job[mediamosa_job_server_db::JOB_ID];

    // Source Mediafile ID.
    $mediafile_id_src = $job_server_job[mediamosa_job_server_db::MEDIAFILE_ID_SRC];

    // Read the contents of the status file in an array.
    $job_status = self::get_status_contents($job_id);

    // Default status.
    $status = mediamosa_job_server_db::JOB_STATUS_INPROGRESS;

    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        // No status file found, we just have to wait.
        if (empty($job_status['Status'])) {
          self::log_debug_mediafile($mediafile_id_src, "No status file found with @statusfile for job @job_id, maybe next run.", array('@statusfile' => mediamosa_storage::get_realpath_status_file($job_id), '@job_id' => $job_id));
          return;
        }

        if ($job_status['Status'] == 'done' && $job_status['Errors'] == 'none') {
          // Status to finished.
          $status = mediamosa_job_server_db::JOB_STATUS_FINISHED;

          // Store the transcode.
          self::store_new_mediafile($job_id);

          // Log it.
          self::log_mediafile($mediafile_id_src, 'End job @job_type, Job ID: @job_id, status: @status', array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status));

          // Set job status.
          self::set_job_status($job_id, $status, $job_status['Progress']);
        }
        elseif ($job_status['Status'] == 'error' && (empty($job_status['Errors']) || $job_status['Errors'] != 'none')) {
          $status = mediamosa_job_server_db::JOB_STATUS_FAILED;
          $link_asset = self::get_asset_link($job_id);

          self::log_mediafile($mediafile_id_src, "End @job_type job, Job ID @job_id, with status: @status<br /><br />@link", array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status, '@link' => $link_asset));
          self::log_mediafile($mediafile_id_src, "Info @job_type job, Job ID @job_id, status file '@statusfile'", array('@job_type' => $job_type, '@job_id' => $job_id, '@statusfile' => self::get_status_contents($job_id, TRUE)));

          // Set status to failed.
          self::set_job_status($job_id, $status, $job_status['Progress'],  isset($job_status["ffmpeg-output"]) ? ($job_status["Errors"] != "" ? $job_status["Errors"] . "-\n" : '') . $job_status["ffmpeg-output"] : $job_status["Errors"]);
        }
        else {
          // Set job status.
          self::set_job_status($job_id, $status, $job_status['Progress']);
        }
        break;

      case mediamosa_job_server_db::JOB_TYPE_STILL:
        // Scene still filename.
        $file_scene = mediamosa_storage::get_uri_scene_file($job_id);

        if (!mediamosa_io::file_exists($file_scene) && empty($job_status)) {
          // No status file found, we just have to wait.
          self::log_debug_mediafile($mediafile_id_src, 'No status file found with name @name for job @job_id, maybe next run.', array('@name' => mediamosa_storage::get_realpath_status_file($job_id), '@job_id' => $job_id));
          return;
        }

        // Set defaults, to fix some possible notices.
        $job_status += array(
          'Status' => '',
          'Errors' => 'none',
          'Progress' => '0.000',
        );

        if (mediamosa_io::file_exists($file_scene) || ($job_status['Status'] == 'done' && $job_status['Errors'] == 'none')) {
          $status = self::store_new_still($job_id, $mediafile_id_src);

          if ($status == mediamosa_job_server_db::JOB_STATUS_INPROGRESS) {
            self::log_debug_mediafile($mediafile_id_src, 'Running @job_type job (storing file busy), Job ID @job_id, with status: @status', array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status));
          }
          else {
            self::log_debug_mediafile($mediafile_id_src, 'End @job_type job, Job ID @job_id, with status: @status', array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status));
          }
        }
        elseif ($job_status['Status'] == 'error' && $job_status['Errors'] != 'none') {
          $status = mediamosa_job_server_db::JOB_STATUS_FAILED;
          $link_asset = self::get_asset_link($job_id);
          self::log_debug_mediafile($mediafile_id_src, 'End @job_type job, Job ID @job_id, with status: @status<br /><br />@link', array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status, '@link' => $link_asset));
          self::log_debug_high_mediafile($mediafile_id_src, "Info @job_type job, Job ID @job_id, status file '@statusfile'", array('@job_type' => $job_type, '@job_id' => $job_id, '@statusfile' => self::get_status_contents($job_id, TRUE)));
        }

        // Update the status.
        if (!mediamosa_io::file_exists($file_scene) && $job_status['Errors'] != 'none') {
          // Might be because there is no status file, dont bother to update.
          if (isset($job_status['Errors'])) {
            self::set_job_status($job_id, $status, $job_status['Progress'], $job_status['Errors']);
          }
        }
        else {
          self::set_job_status($job_id, $status, $job_status['Progress']);
        }

        break;
    }
  }

  /**
   * Retrieve a listing of the jobs.
   *
   * This function may only be called from jobserver (via internal).
   */
  public static function search() {
    $query = mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js');
    $query->leftJoin(mediamosa_job_server_analyse_db::TABLE_NAME, 'jsa', 'js.jobserver_job_id = jsa.jobserver_job_id');
    $query->fields('js');
    $query->fields('jsa');
    $query->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::get_server_id());
    return $query->execute();
  }

  /**
   * Get the job.
   * (only from this installation(!))
   *
   * @param integer $job_id
   */
  public static function get($jobserver_job_id) {
    return mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::ID, $jobserver_job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the job.
   * (only from this installation(!))
   *
   * @param integer $job_id
   */
  public static function get_with_jobid($job_id) {
    return mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::get_server_id())
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Set the job status on a jobserver table.
   *
   * @param $job_id
   * @param $job_status
   * @param $progress
   * @param $error_description
   */
  public static function set_job_status($job_id, $job_status, $progress, $error_description = '', $error_description_args = array()) {

    // Set args in description.
    if (!empty($error_description_args)) {
      $error_description = strtr($error_description, $error_description_args);
    }

    $fields = array(
      mediamosa_job_server_db::JOB_STATUS => $job_status,
      mediamosa_job_server_db::PROGRESS => is_null($progress) ? '0.000' : $progress,
    );

    switch ($job_status) {
      case mediamosa_job_server_db::JOB_STATUS_FINISHED:
      case mediamosa_job_server_db::JOB_STATUS_FAILED:
      case mediamosa_job_server_db::JOB_STATUS_CANCELLED:
        $fields[mediamosa_job_server_db::FINISHED] = mediamosa_datetime::utc_current_timestamp_now(TRUE);
        break;
    }

    // Check if its started.
    $jobserver_job = self::get_with_jobid($job_id);
    if (!$jobserver_job) {
      self::log('Fatal: trying to update job with ID; @job_id', array('@job_id' => $job_id));
      assert(0);
      return;
    }

    // Invalidate technical metadata if analyse fails.
    if ($job_status == mediamosa_job_server_db::JOB_STATUS_FAILED && $jobserver_job[mediamosa_job_server_db::JOB_TYPE] == mediamosa_job_server_db::JOB_TYPE_ANALYSE) {
      $mediafile_id = $jobserver_job[mediamosa_job_server_db::MEDIAFILE_ID_SRC];

      // Delete metadata, its no longer valid when analyse fails.
      mediamosa_asset_mediafile_metadata::delete_by_mediafileid($mediafile_id);
    }

    // Set status.
    if ($jobserver_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING && $job_status == mediamosa_job_server_db::JOB_STATUS_INPROGRESS) {
      $fields[mediamosa_job_server_db::STARTED] = mediamosa_datetime::utc_current_timestamp_now(TRUE);
    }

    if (!empty($error_description)) {
      $fields[mediamosa_job_server_db::ERROR_DESCRIPTION] = $error_description;
    }

    // Update.
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();
  }

  /**
   * Create a link to the parent asset belonging to a given job id.
   *
   * @param int $job_id
   *
   * @return string
   *  Link to an asset.
   */
  public static function get_asset_link($job_id) {

    // Get the job.
    $jobserver_job = mediamosa_job::get($job_id);

    // Get asset ID from job.
    $asset_id = $jobserver_job[mediamosa_job_db::ASSET_ID];

    // Return link.
    return l(mediamosa::t('Go to asset @asset_id', array('@asset_id' => $asset_id)), mediamosa_settings::get_url_asset($asset_id));
  }

  /**
   * Create the string send with the vpx-analyse script.
   *
   * @param string $mediafile_src
   * @param $job_id
   */
  public static function get_analyse_string($mediafile_id, $job_id) {

    // Get the job.
    $job = mediamosa_job::get($job_id, array(mediamosa_job_db::APP_ID, mediamosa_job_db::HINT));
    $app_id = $job[mediamosa_job_db::APP_ID];
    $hint = $job[mediamosa_job_db::HINT];

    // Get the app.
    $app = mediamosa_app::get_by_appid($app_id);

    $options = array();

    if (is_null($hint)) {
      // If hint is null, we want to use the site default parameters.
      if ($app[mediamosa_app_db::ALWAYS_HINT_MP4] == mediamosa_app_db::ALWAYS_HINT_MP4_TRUE) {
        $options[] = mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
      }

      if ($app[mediamosa_app_db::ALWAYS_INSERT_MD] == mediamosa_app_db::ALWAYS_INSERT_MD_TRUE) {
        $options[] = mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
      }
    }
    elseif ($hint == 'TRUE') {
      // We want to hint.
      $options[] = mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
      $options[] = mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
    }
    else {
      // Do nothing. We don't want hinting.
    }

    assert(mediamosa_io::file_exists(mediamosa_settings::lua_analyse_script()));

    // Script will add the mediamosa_id[0]/mediamosa_id to the path.
    $execution_string = sprintf('%s %s %s',
      mediamosa_settings::lua_analyse_script(),
      mediamosa_storage::get_local_mediafile_path($mediafile_id),
      $mediafile_id
    );

    $execution_string .= (count($options) ? ' ' . implode(' ', $options) : '');

    return $execution_string;
  }

  /**
   * Generate the string that is used for the vpx_transcode script.
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id
   */
  public static function get_transcode_exec($jobserver_job_id, $mediafile_id) {

    // Get it.
    $job_server_transcode = mediamosa_job_server_transcode::get($jobserver_job_id);

    if (empty($job_server_transcode)) {
      self::log_mediafile($mediafile_id, 'Transcode job not found, jobserver_id: @jobserver_id', array('@jobserver_id' => $jobserver_job_id));
      return '';
    }

    $tool = $job_server_transcode[mediamosa_job_server_transcode_db::TOOL];
    $file_extension = $job_server_transcode[mediamosa_job_server_transcode_db::FILE_EXTENSION];
    $command = $job_server_transcode[mediamosa_job_server_transcode_db::COMMAND];

    // Get the mediafile.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Create parameter string from the command.
    $commands = mediamosa_transcode_profile::commandToArray($command);

    // Build the parameter string.
    $parameters = array();
    foreach ($commands as $name => $value) {
      if ($value == mediamosa_tool_params_db::ALLOWED_VALUE_FOR_SWITCH) {
        $value = '';
      }
      $parameters[] = $name . ' ' . $value;
    }

    // Rebuild.
    $parameter_string = implode(' ', $parameters);

    // The name of the status file is '<job_id>.status'.
    $job_server = mediamosa_job_server::get($jobserver_job_id);
    $status_file = mediamosa_storage::get_realpath_status_file($job_server[mediamosa_job_server_db::JOB_ID]);

    // Empty.
    $execution_string = '';

    // Combine based on selection.
    switch ($tool) {
      // TODO: move to tool.
      case 'ffmpeg':
        $parameter_string = trim($parameter_string);
        if (!empty($parameter_string)) {
          $parameter_string = escapeshellarg($parameter_string);
        }

        $execution_string = sprintf('%s %s %s %s %s %s %s > /dev/null &', mediamosa_settings::lua_transcode_script(), mediamosa_storage::get_local_mediafile_path($mediafile), mediamosa_storage::get_realpath_temporary(), $mediafile_id, $job_server[mediamosa_job_server_db::JOB_ID], $file_extension, $parameter_string);
        break;

      default:
        // Now check if for this tool the hook exists.
        $class_name = 'mediamosa_tool_' . $tool;

        // FIXME:
        // This code here is first attempt to rewrite the jobs module in more
        // flexible one. In future ::generate_transcode() is called directly and
        // jobs will no longer worry about exec strings.
        // Will start moving all ffmpeg code out of core into ffmpeg tool very
        // soon.

        // Now see if transcode function is found.
        if (class_exists($class_name) && method_exists($class_name, 'get_transcode_exec')) {
          $args = array(
            // Job ID.
            'job_id' => $job_server[mediamosa_job_server_db::JOB_ID],
            // ID of mediafile to transcode.
            'mediafile_id' => $mediafile_id,
            // File extension of dest.
            'file_extension' => $file_extension,
            // Parameter string for cmd.
            'parameter_string' => $parameter_string,
            // the data dir in sannas (extra).
            'path_mount_point_data' => mediamosa_storage::get_local_mediafile_path($mediafile),
            // Path to the transcode file.
            'location_dest_file' => mediamosa_storage::get_realpath_temporary_file($mediafile_id),
            // Location of source mediafile.
            'location_source_file' => mediamosa_storage::get_realpath_mediafile($mediafile),
            // Location of the status_file.
            'status_file' => $status_file,
          );

          // PHP 5.2.3 or higher.
          $execution_string = call_user_func($class_name . '::get_transcode_exec', $args);
        }
    }

    // Unknown.
    if (empty($execution_string)) {
      return strtr('{ echo "Status: error"; echo "Errors: Error"; } > @status', array(
        '@status' => $status_file,
      ));

      // One tool has been found. Process started. Return now.
    }

    return $execution_string;
  }

  /**
   * Generate the still execute string.
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id
   * @return string
   *  The execution string
   */
  public static function get_generate_still_exec($jobserver_job_id, $mediafile_id_source) {

    // Get the mime-type.
    $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id_source, mediamosa_asset_mediafile_metadata::MIME_TYPE);

    // Call the mediamosa_tool_can_generate_still hook.
    foreach (module_implements('mediamosa_tool_can_generate_still') as $module) {
      if (module_invoke($module, 'mediamosa_tool_can_generate_still', $mime_type)) {
        // Get generate still exec.
        return module_invoke($module, 'mediamosa_tool_get_generate_still_exec', $jobserver_job_id, $mediafile_id_source);
      }
    }

    // FIXME: Fall back on ffmpeg for now.
    return mediamosa_tool_ffmpeg::get_generate_still_exec($jobserver_job_id, $mediafile_id_source);
  }

  /**
   * Create an array of the status file.
   *
   * @param $job_id
   *   The job ID is used as status filename.
   * @param $orig
   */
  public static function get_status_contents($job_id, $orig = FALSE) {
    // Get the statusfile filename.
    $statusfile = mediamosa_storage::get_realpath_status_file($job_id);
    if (!mediamosa_io::file_exists($statusfile)) {
      self::log('Unable to load status contents; file @file does not exists.', array('@file' => $statusfile), WATCHDOG_CRITICAL);
      return array();
    }

    $result = array();
    $lines = array();

    // Set default.
    $result += array(
      'Errors' => 'none',
    );

    // FIXME: move to mediamosa_io
    $handle = fopen($statusfile, 'r');
    while (!feof($handle)) {
      $lines[] = fgets($handle);
    }
    fclose($handle);

    // Return the original?
    if ($orig) {
      return implode('', $lines);
    }

    // Strip the garbage from the file.
    foreach ($lines as $line) {
      if (mediamosa_unicode::strpos($line, ':') === FALSE) {
        continue;
      }

      list($name, $value) = explode(':', $line, 2);
      if ($name == 'Progress' && empty($value)) {
        $value = '0.000';
      }
      elseif ($name == 'Progress' || $name == 'Status' || $name == 'Errors') {
        $result[$name] = trim($value);
      }
      elseif ($name == 'ffmpeg-output') {
        $result[$name] = implode("\n", explode('}-{', trim($value)));
      }
    }

    // If there is no result we return empty array.
    if (!empty($result)) {
      // Set defaults, to fix some possible notices.
      $result += array(
        'Status' => '',
        'Errors' => 'none',
        'Progress' => '0.000',
      );
    }

    return $result;
  }

  /**
   * Check the created still and save it if everything is ok.
   *
   * @param string $job_id
   *   Current job id.
   * @param string $mediafile_id_src
   *   Contains a file path to the mediafile
   * @return string
   *   Contains the error message
   */
  public static function store_new_still($job_id, $mediafile_id_src) {
    // Need job for app_id.
    $job = mediamosa_job::get($job_id, array(mediamosa_job_db::APP_ID, mediamosa_job_db::HINT));
    $app_id = $job[mediamosa_job_db::APP_ID];

    $base_filename = mediamosa_io::get_base_filename($job_id);

    // Check if there really is an image ($file_size > 0)
    $filename = mediamosa_storage::get_uri_temporary_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, 1) . '.jpeg');

    if (!mediamosa_io::file_exists($filename) || !mediamosa_io::filesize($filename)) {
      // Something failed, very likely the frametime was too high. Remove the
      // files and fail the job.
      $still_error = mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION);

      // Update status.
      self::set_job_status($job_id, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', $still_error);

      // Remove all of the still images.
      $i = 1;
      while (mediamosa_io::file_exists(mediamosa_storage::get_realpath_temporary_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg')) && $i <= mediamosa_settings::STILL_MAXIMUM) {
        mediamosa_io::unlink(mediamosa_storage::get_realpath_temporary_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg'));
        $i++;
      }
      mediamosa_io::unlink(mediamosa_storage::get_realpath_status_file($job_id));

      self::log_mediafile($mediafile_id_src, $still_error);
      return mediamosa_job_server_db::JOB_STATUS_FAILED;
    }

    // Check if the frame has any usefull content. We do this by checking the amount of dominant colors.
    mediamosa_job_server_still::still_validate($job_id, $base_filename);

    $i = 1;
    $mediafile_dest = array();
    while (mediamosa_io::file_exists(mediamosa_storage::get_uri_temporary_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg'))) {
      if ($i <= mediamosa_settings::STILL_MAXIMUM) {
        // Generate new hash.
        $mediafile_id = mediamosa_db::uuid($app_id);

        $source_uri = mediamosa_storage::get_uri_temporary_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg');
        $destination_uri = mediamosa_storage::create_local_mediafile_uri($app_id, $mediafile_id);

        // Make sure destination dir exists.
        mediamosa_io::mkdir(mediamosa_io::dirname($destination_uri));

        // Everything went ok, move the still and remove other files
        mediamosa_io::rename($source_uri, $destination_uri);
        $mediafile_dest[] = $mediafile_id;
      }
      else {
        // Reached the maximum, just delete the remain stills.
        mediamosa_io::unlink(mediamosa_storage::get_realpath_temporary_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg'));
      }

      $i++;
    }
    mediamosa_io::unlink(mediamosa_storage::get_realpath_status_file($job_id));

    // Data to update.
    $fields = array(
      mediamosa_job_server_db::MEDIAFILE_DEST => serialize($mediafile_dest),
    );

    // Add changed.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Update mediafile_dest of the job
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();

    // Log it.
    self::log_mediafile($mediafile_id_src, 'Job (job_id: @job_id) finished: Multiple stills saved as e.g.: @filenames.', array('@job_id' => $job_id, '@filenames' => implode(',', $mediafile_dest)));

    return mediamosa_job_server_db::JOB_STATUS_FINISHED;
  }

  /**
   * Save a new mediafile.
   *
   * @param integer $job_id
   *   The job ID.
   */
  public static function store_new_mediafile($job_id) {

    // Get the job.
    $job = mediamosa_job::get($job_id, array(mediamosa_job_db::APP_ID, mediamosa_job_db::HINT));
    $app_id = $job[mediamosa_job_db::APP_ID];

    // Generate new mediafile ID.
    $mediafile_id = mediamosa_db::uuid($job_id);

    $job_server = mediamosa_db::db_query(
      'SELECT mjst.#command, mjst.#file_extension, mjs.#mediafile_id
       FROM {#mediamosa_job_server_transcode} AS mjst
       JOIN {#mediamosa_job_server} AS mjs ON mjs.#jobserver_job_id = mjst.#jobserver_job_id
       WHERE mjs.#job_id = :job_id',
      array(
        '#command' => mediamosa_job_server_transcode_db::COMMAND,
        '#file_extension' => mediamosa_job_server_transcode_db::FILE_EXTENSION,
        '#mediafile_id' => mediamosa_job_server_db::MEDIAFILE_ID_SRC,
        '#mediamosa_job_server_transcode' => mediamosa_job_server_transcode_db::TABLE_NAME,
        '#mediamosa_job_server' => mediamosa_job_server_db::TABLE_NAME,
        '#jobserver_job_id' => mediamosa_job_server_db::ID,
        '#job_id' => mediamosa_job_server_db::JOB_ID,
        ':job_id' => $job_id,
      )
    )->fetchAssoc();

    if ($job_server == FALSE) {
      self::log('Transcode job not fould for job with ID @job_id', array('@job_id' => $job_id));
      return;
    }

    // Get file extension.
    $file_extension = $job_server[mediamosa_job_server_transcode_db::FILE_EXTENSION];

    // Get the filenames.
    $file_status_uri = mediamosa_storage::get_uri_status_file($job_id);
    $file_transcode_uri = mediamosa_storage::get_uri_temporary_file($job_id . '.' . $file_extension);
    $file_destination_uri = mediamosa_storage::create_local_mediafile_uri($app_id, $mediafile_id);

    // Rename transcoded file to new dest.
    mediamosa_io::rename($file_transcode_uri, $file_destination_uri);

    // Now remove the status file.
    mediamosa_io::unlink($file_status_uri);

    $fields = array(
      mediamosa_job_server_db::MEDIAFILE_DEST => $mediafile_id,
    );

    // Enrich with update date.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Update the filename in mediafile_dest.
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();

    // Log it.
    self::log_mediafile($job_server[mediamosa_job_server_db::MEDIAFILE_ID_SRC], "Job with ID @job_id ready, new mediafile stored as '@uri' (@path).", array('@job_id' => $job_id, '@uri' => $file_destination_uri, '@path' => mediamosa_io::realpath($file_destination_uri)));
  }

  /**
   * Delete a server job.
   *
   * @param integer $job_id
   * @param bool $killjob
   */
  public static function delete_job($job_id, $killjob = FALSE) {

    // Get the job.
    $jobserver_job = self::get_with_jobid($job_id);

    if (empty($jobserver_job)) {
      return;
    }

    if ($killjob) {
      $fields = array(
        mediamosa_job_server_db::JOB_STATUS => mediamosa_job_server_db::JOB_STATUS_CANCELLED,
      );

      // Enrich with changed value.
      $fields = mediamosa_db::db_update_enrich($fields);

      // Update.
      mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
        ->fields($fields)
        ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::get_server_id())
        ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
        ->execute();

      // FIXME:
      // Send kill command to specific job.
      // Remove files.
    }

    $jobserver_job_id = $jobserver_job[mediamosa_job_server_db::ID];
    $job_type = $jobserver_job[mediamosa_job_server_db::JOB_TYPE];

    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_ANALYSE:
        // Remove.
        mediamosa_db::db_delete(mediamosa_job_server_analyse_db::TABLE_NAME)
          ->condition(mediamosa_job_server_analyse_db::ID, $jobserver_job_id)
          ->execute();
        break;
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        // Remove.
        mediamosa_db::db_delete(mediamosa_job_server_transcode_db::TABLE_NAME)
          ->condition(mediamosa_job_server_transcode_db::ID, $jobserver_job_id)
          ->execute();
        break;
      case mediamosa_job_server_db::JOB_TYPE_STILL:
        // Remove.
        mediamosa_db::db_delete(mediamosa_job_server_still_db::TABLE_NAME)
          ->condition(mediamosa_job_server_still_db::ID, $jobserver_job_id)
          ->execute();
        break;
    }

    // Remove.
    mediamosa_db::db_delete(mediamosa_job_server_db::TABLE_NAME)
      ->condition(mediamosa_job_server_db::ID, $jobserver_job_id)
      ->execute();
  }

  /**
   * Execute the job.
   *
   * Warning; This job can take a lot of time to complete and should only be
   * called with /job/$job_id/execute REST call with method HEAD.
   *
   * We could use lock() code to prevent running the job twice. But re-write is
   * planned for job server and using lock now will only make it unnecessary
   * more complex, as job scheduler will make sure only one instance of this
   * job runs.
   *
   * @param integer $job_id
   *   The job ID.
   * @param string $jop_type
   *   The job type, see mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_*.
   * @param string $mediafile_id
   *   The mediafile ID to process.
   * @param array $job_data
   *   The data for job.
   */
  public static function job_execute($job_id, $job_type, $mediafile_id, array $job_data) {
    try {
      // Create the job on the job server.
      self::create_job($job_id, $job_type, $mediafile_id);

      // Set status in progress.
      self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_INPROGRESS, '0.000');

      // Now do the extreme, and set the time limit of this script to max run
      // time of a job.
      $max_execution_time = ini_get('max_execution_time');
      $time_limit = mediamosa_settings::JOB_JOB_TIMEOUT - $max_execution_time;
      if ($time_limit) {
        drupal_set_time_limit($time_limit);
      }

      // Get the mediafile.
      $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

      // Now based on type, call the correct function.
      switch ($job_type) {
        case mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_DOWNLOAD:
          self::job_media_download($mediafile, $job_data);
          break;

        case mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_UPLOAD:
          self::job_media_upload($mediafile, $job_data);
          break;

        case mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_MOVE:
          self::job_media_move($mediafile, $job_data);
          break;

        // Unknown type.
        default:
          self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000');
          throw new mediamosa_exception_program_error("Job execute not allowed on type '@type'.", array('@type' => $job_type));
      }
    }
    catch (Exception $e) {
       self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', $e->getMessage());
       throw $e;
    }

    // We are done, put our job on finished.
    self::set_job_status($job_id, mediamosa_job_db::JOB_STATUS_FINISHED, '1.000');
  }

  /**
   * Execute the media download job.
   *
   * Download the file and store the file into the transition area.
   *
   * @param array $mediafile
   *   The mediafile to download.
   * @param array $job_data
   *   The job data (empty for now).
   */
  protected static function job_media_download(array $mediafile, array $job_data) {
    // Need some data.
    $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::ID];

    // Get the source uri.
    $source_uri = mediamosa_storage::get_uri_mediafile($mediafile);

    // Need space to download, store in transition area.
    // Transition will ask source for the MD5 and will create uri accordingly.
    // However, if for some reason the MD5 could not be retrieved, the MD5
    // default is used (see mediamosa_io_streamwrapper::MD5_DEFAULT).
    // When downloaded, the verification will accept the diffence in MD5 uri and
    // file, as mediamosa_storage_transition::register_transition_file() will
    // move the file to its correct place when MD5 does not match the file.
    $transition_uri = mediamosa_storage_transition::get_transition_uri($source_uri);

    // If file exists, then its already downloaded.
    //
    // However, the md5 in the transition_uri must match the md5 of the file.
    // We might have files that where created or downloaded, but never verified.
    if (mediamosa_io::file_exists($transition_uri) && !mediamosa_storage_transition::verify_transition_file($transition_uri)) {
      // Invalid file, remove it, can not use it, redownload.
      mediamosa_storage_transition::unregister_transition_file($transition_uri);
    }

    // If the file exists, its md5 matches the transition_uri.
    //
    // If the file is not on expected location, then download it now.
    if (!mediamosa_io::file_exists($transition_uri)) {
      // Download mediafile.
      mediamosa_storage::mediafile_copy($source_uri, $transition_uri);

      // Verify downloaded file md5 vs transition_uri's md5. They must match.
      $verification = mediamosa_storage_transition::verify_transition_file($transition_uri, TRUE);

      // Verfication failed, remove download.
      if (!$verification) {
        // Invalid file, remove it, can not use it.
        mediamosa_storage_transition::unregister_transition_file($transition_uri);

        // Download failed.
        throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t("download failed, download file with mediafile ID (@id) did not match expected checksum.", array('@id' => $mediafile_id))));
      }
    }

    // Download done, register the download so transition will keep it for a
    // while. Register function will check if location of the file is correct
    // and will move file if md5 in uri does not match file. It expects that
    // the MD5 of the file is correct.
    $transition_uri = mediamosa_storage_transition::register_transition_file($transition_uri);

    // Need streamwrapper.
    $mediamosa_io_streamwrapper_transition = mediamosa_io::require_stream_wrapper_instance_by_uri($transition_uri);

    // Get md5.
    $transition_md5 = $mediamosa_io_streamwrapper_transition->get_md5_from_uri();

    // Ok, now set the MD5 for this mediafile or else we will never find this
    // downloaded file later.
    mediamosa_asset_mediafile_metadata::create_mediafile_metadata_char($mediafile_id, $transition_md5, mediamosa_asset_mediafile_metadata::MD5);
  }

  /**
   * Execute the media upload job.
   *
   * Upload the mediafile from transition to its own location (update).
   *
   * @param array $mediafile
   *   The mediafile to upload.
   * @param array $job_data
   *   The job data (empty for now).
   */
  protected static function job_media_upload(array $mediafile, array $job_data = array()) {
    // Need some data.
    $app_id = $mediafile[mediamosa_asset_mediafile_db::APP_ID];
    $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::ID];
    $is_still = mediamosa_asset_mediafile::is_still($mediafile);

    // Get current transition location.
    $source_uri = mediamosa_storage::get_uri_transition_links() . mediamosa_io_streamwrapper::create_mediafile_path($mediafile_id);

    // Need destination storage location.
    $destination_uri = mediamosa_storage::create_storage_uri($app_id, $mediafile_id, $is_still);

    // Create destination wrapper now, in case storage uri app changes during
    // upload.
    $mediamosa_io_streamwrapper_destination = mediamosa_io::require_stream_wrapper_instance_by_uri($destination_uri);

    // Now upload file.
    mediamosa_storage::mediafile_copy($source_uri, $destination_uri);

    // Done, update the new location.
    mediamosa_storage::mediafile_update_mointpoint($mediafile, $mediamosa_io_streamwrapper_destination->get_uri_mount_point());
  }

  /**
   * Execute the media move job.
   *
   * @param array $mediafile
   *   The mediafile to move.
   * @param array $job_data
   *   The job data;
   *   - storage_profile_id
   *     The destination storage profile ID.
   *   - path
   *     The path on the storage.
   */
  protected static function job_media_move(array $mediafile, array $job_data) {

    if (empty($job_data['path'])) {
      throw new mediamosa_exception_program_error('Missing path info in @func', array('@func' => __FUNCTION__));
    }

    // Move mediafile.
    mediamosa_storage::mediafile_move($mediafile, $job_data['storage_profile_id'], $job_data['path']);
  }
}
