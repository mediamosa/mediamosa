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
  * The job server module.
  */


class mediamosa_job_server {

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
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
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
  static public function log_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL, $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'job server');
  }

  /**
   * Log debug for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  static public function log_debug($message, array $variables = array()) {
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
  static public function log_debug_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL) {
    mediamosa_debug::log_mediafile($mediafile_id, $message, $variables, $asset_id, 'job_server');
  }

  /**
   * Log debug for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  static public function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_high($message, $variables, 'job_server');
  }

  /**
   * Log debug mediafile_id for job_server.
   *
   * @param string $message
   * @param array $variables
   */
  static public function log_debug_high_mediafile($mediafile_id, $message, array $variables = array(), $asset_id = NULL) {
    mediamosa_debug::log_high_mediafile($mediafile_id, $message, $variables, $asset_id, 'job_server');
  }

  /**
   * Trigger every minute.
   *
   * Will timeout on itself after 2 minutes.
   */
  static public function run_parse_queue() {

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
      return; // In timeout zone.
    }

    variable_set('mediamosa_jobserver_cron_last_' . $server_id, REQUEST_TIME);

    // Log it.
    self::log_debug('Start parse queue job server');

    // We set this to NULL when we are done.
    variable_set('mediamosa_jobserver_cron_running_' . $server_id, REQUEST_TIME);

    // Set max run time on 120 sec.
    set_time_limit(120);

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
   * This function is the engine of the job server (job processor. Should be
   * called at regular intervals.
   *
   * 1. Get a list of all jobs that are not finished, failed or canceled.
   * 2. For all jobs with status WAITING, the job is started.
   * 3. For all running jobs de status is updated.
   */
  static public function parse_queue() {
    // Retrieve all jobs of this server.
    $job_server_jobs = mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::get_server_id())
      ->condition(mediamosa_job_server_db::JOB_STATUS, array(mediamosa_job_server_db::JOB_STATUS_FINISHED, mediamosa_job_server_db::JOB_STATUS_FAILED, mediamosa_job_server_db::JOB_STATUS_CANCELLED), 'NOT IN')
      ->orderBy(mediamosa_job_server_db::ID, 'ASC')
      ->execute();

    foreach ($job_server_jobs as $job_server_job) {
      if ($job_server_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING) {
        // New job, start it.
        self::waiting_job_start($job_server_job);
      }
      else {
        // Running job, update it.
        self::running_job_update($job_server_job);
      }
    }
  }


  /**
   * Create analyse server job.
   *
   * @param $job_id
   * @param $mediafile_id_src
   */
  static public function create_job_analyse($job_id, $mediafile_id_src) {
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
  static public function create_job_transcode($job_id, $mediafile_id_src, $tool, $file_extension, $command) {
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
  static public function create_job_still($job_id, $mediafile_id_src, $frametime, $size, $h_padding, $v_padding, $blackstill_check, array $a_still_parameters) {
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
   * @param $job_id
   */
  static public function create_job($job_id, $job_type, $mediafile_id_src) {

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
   */
  static public function waiting_job_start($job_server_job) {

    // FIXME:
    // Check if jobs module is enable has been removed.
    // Old code checked it without using the app_id. Which is unknown.
    // Think its better not to check here but to prevent new jobs from being inserted
    // when job module is turned off. Lets process jobs, even when its been disabled.

    // Job ID.
    $job_id = $job_server_job[mediamosa_job_server_db::JOB_ID];

    // Source Mediafile ID.
    $mediafile_id_src = $job_server_job[mediamosa_job_server_db::MEDIAFILE_ID_SRC];

    // Job type.
    $job_type = $job_server_job[mediamosa_job_server_db::JOB_TYPE];

    // Job server id (ID of the row in the mediamosa_job_server table).
    $jobserver_job_id = $job_server_job[mediamosa_job_server_db::ID];

    // Set status in progress.
    self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_INPROGRESS, '0.000');

    // Check if source mediafile exists.
    // but first do a clear cache.
    mediamosa_io::clearstatcache();

    // Get the filename of the source.
    $filename = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id_src);

    // Check of the source file exists.
    if (!file_exists($filename)) {
      $link_asset = self::get_asset_link($job_id);
      self::log_mediafile($mediafile_id_src, "Job ID: @job_id File '@file' not found,<br /><br/>@link", array('@job_id' => $job_id, '@file' => $filename, '@link' => $link_asset), NULL, WATCHDOG_CRITICAL);
      self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'File not found (@file)', array('@file' => $filename));
      self::log_mediafile($mediafile_id_src, "Job ID: @job_id, memory_get_peak_usage = @memory_get_peak_usage, memory_usage: @memory_get_usage", array('@job_id' => $job_id, '@memory_get_peak_usage' => memory_get_peak_usage(), '@memory_get_usage' => memory_get_usage()));
      return;
    }

    // Based on jobtype we execute the job.
    $execution_string = "";
    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        // Clean up possible earlier status files.
        mediamosa_io::unlink(mediamosa_configuration_storage::transcode_file_location_get($mediafile_id_src . '.status'));

        // Make sure the tmp transcode directory exists.
        mediamosa_io::mkdir(mediamosa_configuration_storage::transcode_file_location_get(''));
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

    // Now execute.
    if ($job_type == mediamosa_job_server_db::JOB_TYPE_ANALYSE) {
      $a_output = array();
      $s_output = mediamosa_io::exec($execution_string . ' 2>&1', $a_output);

      $link_asset = self::get_asset_link($job_id);
      self::log_mediafile($mediafile_id_src, 'Job @job_type (Job ID: @job_id) returned output: @s_output - @a_output<br /><br />@link',
        array(
          '@job_type' => $job_type,
          '@job_id' => $job_id,
          '@s_output' => $s_output,
          '@a_output' => implode("\n", $a_output),
          '@link' => $link_asset,
        )
      );

      if (!empty($s_output)) {
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FINISHED, '1.000');
      }
      else {
        self::set_job_status($job_id, mediamosa_job_server_db::JOB_STATUS_FAILED, '1.000', 'Empty result, analyse failed.');
      }

      // Log it.
      self::log_mediafile($mediafile_id_src, 'Starting new followup analyse job for job ID: @job_id.', array('@job_id' => $job_id));

      // Update job_server_analyse row.
      $fields = array(
        mediamosa_job_server_analyse_db::ANALYSE_RESULT => implode("\n", $a_output),
      );

      // Enrich with changed.
      $fields = mediamosa_db::db_update_enrich($fields);

      // And update row.
      mediamosa_db::db_update(mediamosa_job_server_analyse_db::TABLE_NAME)
        ->fields($fields)
        ->condition(mediamosa_job_server_analyse_db::ID, $jobserver_job_id)
        ->execute();
    }
    else {
      $a_output = array();
      $s_output = mediamosa_io::exec($execution_string, $a_output);
      $link_asset = self::get_asset_link($job_id);
      self::log_mediafile($mediafile_id_src, 'Job @job_type (Job ID: @job_id) returned output: @s_output - @a_output<br /><br />@link',
        array(
          '@job_type' => $job_type,
          '@job_id' => $job_id,
          '@s_output' => $s_output,
          '@a_output' => implode("\n", $a_output),
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
  static public function running_job_update($job_server_job) {

    // Job ID.
    $job_id = $job_server_job[mediamosa_job_server_db::JOB_ID];

    // Source Mediafile ID.
    $mediafile_id_src = $job_server_job[mediamosa_job_server_db::MEDIAFILE_ID_SRC];

    // Job type.
    $job_type = $job_server_job[mediamosa_job_server_db::JOB_TYPE];

    // Read the contents of the status file in an array.
    $job_status = self::get_status_contents($mediafile_id_src);

    // Default status.
    $status = mediamosa_job_server_db::JOB_STATUS_INPROGRESS;

    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        // No status file found, we just have to wait.
        if (empty($job_status['Status'])) {
          self::log_debug_mediafile($mediafile_id_src, 'No status file found with name @name for job @job_id, maybe next run.', array('@name' => mediamosa_configuration_storage::status_file_location_get($mediafile_id_src), '@job_id' => $job_id));
          return;
        }

        if ($job_status['Status'] == 'done' && $job_status['Errors'] == 'none') {
          // Status to finished.
          $status = mediamosa_job_server_db::JOB_STATUS_FINISHED;

          // Store the transcode.
          self::store_new_mediafile($job_id, $mediafile_id_src);

          // Log it.
          self::log_mediafile($mediafile_id_src, 'End job @job_type, Job ID: @job_id, status: @status', array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status));

          // Set job status.
          self::set_job_status($job_id, $status, $job_status['Progress']);
        }
        elseif ($job_status['Status'] == 'error' && (empty($job_status['Errors']) || $job_status['Errors'] != 'none')) {
          $status = mediamosa_job_server_db::JOB_STATUS_FAILED;
          $link_asset = self::get_asset_link($job_id);

          self::log_mediafile($mediafile_id_src, "End @job_type job, Job ID @job_id, with status: @status<br /><br />@link", array('@job_type' => $job_type, '@job_id' => $job_id, '@status' => $status, '@link' => $link_asset));
          self::log_mediafile($mediafile_id_src, "Info @job_type job, Job ID @job_id, status file '@statusfile'", array('@job_type' => $job_type, '@job_id' => $job_id, '@statusfile' => self::get_status_contents($mediafile_id_src, TRUE)));

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
        $file_scene = mediamosa_configuration_storage::still_scene_file_location_get($job_id);

        if (!file_exists($file_scene) && empty($job_status)) {
          // No status file found, we just have to wait.
          self::log_debug_mediafile($mediafile_id_src, 'No status file found with name @name for job @job_id, maybe next run.', array('@name' => mediamosa_configuration_storage::status_file_location_get($mediafile_id_src), '@job_id' => $job_id));
          return;
        }

        // Set defaults, to fix some possible notices.
        $job_status += array(
          'Status' => '',
          'Errors' => 'none',
          'Progress' => '0.000',
        );

        if (file_exists($file_scene) || ($job_status['Status'] == 'done' && $job_status['Errors'] == 'none')) {
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
          self::log_debug_high_mediafile($mediafile_id_src, "Info @job_type job, Job ID @job_id, status file '@statusfile'", array('@job_type' => $job_type, '@job_id' => $job_id, '@statusfile' => self::get_status_contents($mediafile_id_src, TRUE)));
        }

        // Update the status.
        if (!file_exists($file_scene) && $job_status['Errors'] != 'none') {
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
  static public function search() {
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
  static public function get($jobserver_job_id) {
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
  static public function get_with_jobid($job_id) {
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
  static public function set_job_status($job_id, $job_status, $progress, $error_description = '', $a_error_description_args = array()) {

    // Set args in description.
    if (!empty($a_error_description_args)) {
      $error_description = strtr($error_description, $a_error_description_args);
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
    $a_jobserver_job = self::get_with_jobid($job_id);
    if (!$a_jobserver_job) {
      self::log('Fatal: trying to update job with ID; @job_id', array('@job_id' => $job_id));
      assert(0);
      return;
    }

    // Set status.
    if ($a_jobserver_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING && $job_status == mediamosa_job_server_db::JOB_STATUS_INPROGRESS) {
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
  static public function get_asset_link($job_id) {

    // Get the job.
    $a_jobserver_job = mediamosa_job::get($job_id);

    // Get asset ID from job.
    $asset_id = $a_jobserver_job[mediamosa_job_db::ASSET_ID];

    // Get the asset.
    $a_asset = mediamosa_asset::get($a_jobserver_job[mediamosa_job_db::ASSET_ID]);

    // Return link.
    return l(mediamosa::t('Go to asset @asset_id', array('@asset_id' => $asset_id)), mediamosa_settings::get_url_asset($asset_id));
  }

  /**
   * Create the string send with the vpx-analyse script.
   *
   * @param string $mediafile_src
   * @param $job_id
   */
  static public function get_analyse_string($mediafile_id, $job_id) {

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
        $options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
      }

      if ($app[mediamosa_app_db::ALWAYS_INSERT_MD] == mediamosa_app_db::ALWAYS_INSERT_MD_TRUE) {
        $options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
      }
    }
    elseif ($hint == 'TRUE') {
      // We want to hint.
      $options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
      $options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
    }
    else {
      // Do nothing. We don't want hinting.
    }

    assert(file_exists(mediamosa_settings::analyse_file()));

    $execution_string = sprintf('%s %s %s',
      mediamosa_settings::analyse_file(),
      mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(),
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
  static public function get_transcode_exec($jobserver_job_id, $mediafile_id) {

    // Get it.
    $job_server_transcode = mediamosa_job_server_transcode::get($jobserver_job_id);

    if (empty($job_server_transcode)) {
      self::log_mediafile($mediafile_id, 'Transcode job not found, jobserver_id: @jobserver_id', array('@jobserver_id' => $jobserver_job_id));
      return '';
    }

    $tool = $job_server_transcode[mediamosa_job_server_transcode_db::TOOL];
    $file_extension = $job_server_transcode[mediamosa_job_server_transcode_db::FILE_EXTENSION];
    $parameter_list = $job_server_transcode[mediamosa_job_server_transcode_db::COMMAND];

    // Mime type.
    $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::MIME_TYPE);

    // Get the mediafile.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Create parameter string from the command.
    $parameter_string = '';
    $pairs = explode(';', $parameter_list);
    foreach ($pairs as $pair) {
      $parameter_string .= preg_replace('/:/', ' ', $pair, 1) . ' ';
    }

    // Replace the one parameter placeholder.
    $parameter_string = str_replace(mediamosa_tool_params_db::ALLOWED_VALUE_FOR_SWITCH, '', $parameter_string);

    // Empty.
    $execution_string = '';

    // Combine based on selection.
    switch ($tool) {
      // TODO: move to tool.
      case 'ffmpeg':
        $parameter_string = trim($parameter_string);
        if (!empty($parameter_string)) {
          $parameter_string = "'" . $parameter_string . "'";
        }

        $execution_string = sprintf('%s %s %s %s %s > /dev/null &', mediamosa_settings::ffmpeg_transcode_file(), mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(), $mediafile_id, $file_extension, $parameter_string);
        break;

      default:
        // Now check if for this tool the hook exists.
        $class_name = 'mediamosa_tool_' . $tool;

        // FIXME:
        // This code here is first attempt to rewrite the jobs module in more
        // flexible one. In future ::generate_transcode() is called directly and jobs
        // will no longer worry about exec strings.
        // Will start moving all ffmpeg code out of core into ffmpeg tool very
        // soon.

        // Now see if transcode function is found.
        if (class_exists($class_name) && method_exists($class_name, 'get_transcode_exec')) {
          $args = array(
            // Jobserver job ID.
            'jobserver_job_id' => $jobserver_job_id,
            // ID of mediafile to transcode.
            'mediafile_id' => $mediafile_id,
            // File extension of dest.
            'file_extension' => $file_extension,
            // Parameter string for cmd.
            'parameter_string' => $parameter_string,
            // the data dir in sannas (extra).
            'path_mount_point_data' => mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get() . DIRECTORY_SEPARATOR,
            // Path to the transcode file.
            'location_dest_file' => mediamosa_configuration_storage::transcode_file_location_get($mediafile_id),
            // Location of source mediafile.
            'location_source_file' => mediamosa_configuration_storage::mediafile_filename_get($mediafile),
          );

          $execution_string = call_user_func_array($class_name . '::get_transcode_exec', array($args));
        }
    }

    // Unknown.
    if (empty($execution_string)) {
      return strtr('{ echo "Status: error"; echo "Errors: Error"; } > @status', array(
        '@status' => mediamosa_configuration_storage::status_file_location_get($mediafile_id),
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
  static public function get_generate_still_exec($jobserver_job_id, $mediafile_id_source) {

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
   * @param $filename
   * @param $orig
   */
  static public function get_status_contents($filename, $orig = FALSE) {
    $statusfile = mediamosa_configuration_storage::status_file_location_get($filename);

    if (!mediamosa_io::file_exists($statusfile)) {
      self::log('Unable to load status contents; file @file does not exists.', array('@file' => $statusfile), WATCHDOG_CRITICAL);
      return array();
    }

    $result = $lines = array();

    // Set default.
    $result += array(
      'Errors' => 'none',
    );

    // FIXME: move to mediamosa_io
    $handle = fopen($statusfile, "r");
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
   *  Current job id.
   * @param string $mediafile_id_src
   *  Contains a file path to the mediafile
   * @return string
   *  Contains the error message
   */
  static public function store_new_still($job_id, $mediafile_id_src) {

    $base_filename = mediamosa_io::get_base_filename($mediafile_id_src);

    // Check if there really is an image ($file_size > 0)
    $filename = mediamosa_configuration_storage::transcode_file_location_get($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, 1) . ".jpeg");
    if (!file_exists($filename) || !filesize($filename)) {
      // Something failed, very likely the frametime was too high. Remove the files and fail the job.
      $still_error = mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION);

      // Update status.
      self::set_job_status($job_id, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', $still_error);

      // Remove all of the still images.
      $i = 1;
      while (file_exists(mediamosa_configuration_storage::transcode_file_location_get($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg')) && $i <= mediamosa_settings::STILL_MAXIMUM) {
        mediamosa_io::unlink(mediamosa_configuration_storage::transcode_file_location_get($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg'));
        $i++;
      }
      mediamosa_io::unlink(mediamosa_configuration_storage::status_file_location_get($base_filename));

      self::log_mediafile($mediafile_id_src, $still_error);
      return mediamosa_job_server_db::JOB_STATUS_FAILED;
    }

    // Check if the frame has any usefull content. We do this by checking the amount of dominant colors.
    mediamosa_job_server_still::still_validate($job_id, $base_filename);

    $i = 1;
    $mediafile_dest = array();
    while (file_exists(mediamosa_configuration_storage::transcode_file_location_get($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg'))) {
      if ($i <= mediamosa_settings::STILL_MAXIMUM) {
        // Generate new hash
        $filename = mediamosa_db::uuid(rand(1, 9999));

        $source = mediamosa_configuration_storage::transcode_file_location_get($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg');
        $dest = mediamosa_configuration_storage::mediafile_still_filename_get($filename);

        // Make sure dest. exists.
        mediamosa_io::mkdir(mediamosa_io::dirname($dest), TRUE);

        // Everything went ok, move the still and remove other files
        mediamosa_io::rename($source, $dest);
        $mediafile_dest[] = $filename;
      }
      else {
        // Reached the maximum, just delete the remain stills
        mediamosa_io::unlink(mediamosa_configuration_storage::transcode_file_location_get($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg'));
      }

      $i++;
    }
    mediamosa_io::unlink(mediamosa_configuration_storage::status_file_location_get($base_filename));

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
   * @param $filename
   * @param $orig
   */
  static public function store_new_mediafile($job_id, $old_filename) {

    // Generate the filename.
    $new_filename = mediamosa_db::uuid(rand(1, 9999));

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

    // If extension is RAW, attach it to the filename.
    if ($file_extension == mediamosa_job::JOBRAW_FILE_EXTENSION) {
      // First character check and extension.
      $new_filename = (mt_rand(0, 9)) . mediamosa_unicode::substr($new_filename, 1) . '.' . mediamosa_job::JOBRAW_FILE_EXTENSION;
    }

    // Get the filenames.
    $file_status = mediamosa_configuration_storage::status_file_location_get($old_filename);
    $file_transcode = mediamosa_configuration_storage::transcode_file_location_get($old_filename . '.' . $file_extension);
    $file_dest = mediamosa_configuration_storage::mediafile_id_filename_get($new_filename);

    // Rename transcoded file to new dest.
    mediamosa_io::rename($file_transcode, $file_dest);

    // Now remove the status file.
    mediamosa_io::unlink($file_status);

    $fields = array(
      mediamosa_job_server_db::MEDIAFILE_DEST => $new_filename,
    );

    // Enrich with update date.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Update the filename in mediafile_dest.
    mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_server_db::JOB_ID, $job_id)
      ->execute();

    // Log it.
    self::log_mediafile($job_server[mediamosa_job_server_db::MEDIAFILE_ID_SRC], "Job with ID @job_id ready, new mediafile stored as '@file_dest'.", array('@job_id' => $job_id, '@file_dest' => $file_dest));
  }

  /**
   * Delete a server job.
   * @param integer $job_id
   * @param bool $killjob
   */
  static public function delete_job($job_id, $killjob = FALSE) {

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
}
