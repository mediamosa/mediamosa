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
   * Black still interval.
   */
  const BLACK_STILL_INTERVAL = 30;
  /**
   * Video duration max value in second.
   */
  const VIDEO_DURATION_MAX = 99999;
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
   *
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
   *
   * 1.x: vpx_jobserver_parse_queue.
   */
  static public function parse_queue() {
    // Retrieve all jobs of this server.
    $result = mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->condition(mediamosa_job_server_db::INSTALL_ID, mediamosa::get_server_id())
      ->condition(mediamosa_job_server_db::JOB_STATUS, array(mediamosa_job_server_db::JOB_STATUS_FINISHED, mediamosa_job_server_db::JOB_STATUS_FAILED, mediamosa_job_server_db::JOB_STATUS_CANCELLED), 'NOT IN')
      ->orderBy(mediamosa_job_server_db::ID, 'ASC')
      ->execute();

    foreach ($result as $a_job_server_job) {
      if ($a_job_server_job[mediamosa_job_server_db::JOB_STATUS] == mediamosa_job_server_db::JOB_STATUS_WAITING) {
        // New job, start it.
        self::waiting_job_start($a_job_server_job);
      }
      else {
        // Running job, update it.
        self::running_job_update($a_job_server_job);
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
   *
   * 1.x: _vpx_jobserver_startjob.
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

    $filename = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id_src);

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
        // Clean up earlier status files.
        mediamosa_io::unlink(mediamosa_configuration_storage::transcode_file_location_get($mediafile_id_src . '.status'));

        // Make sure the tmp transcode directory exists.
        mediamosa_io::mkdir(mediamosa_configuration_storage::transcode_file_location_get(''));
        $execution_string = self::get_transcode_string($jobserver_job_id, $mediafile_id_src);
        break;

      case mediamosa_job_server_db::JOB_TYPE_STILL:
        $execution_string = self::get_still_string($jobserver_job_id, $mediafile_id_src);
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

      if ($s_output != "") {
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
   *
   * 1.x: _vpx_jobserver_updatestatus.
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
   *
   * 1.x: vpx_jobserver_get_jobs_status (not exact port(!)).
   *
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
   * _vpx_jobserver_set_job_status
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
   * 1.x: _vpx_jobserver_get_asset_link
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
   * 1.x: _vpx_jobserver_get_analyse_string
   *
   * @param string $mediafile_src
   * @param $job_id
   */
  static public function get_analyse_string($mediafile_id, $job_id) {

    // Get the job.
    $a_job = mediamosa_job::get($job_id, array(mediamosa_job_db::APP_ID, mediamosa_job_db::HINT));
    $app_id = $a_job[mediamosa_job_db::APP_ID];
    $hint = $a_job[mediamosa_job_db::HINT];

    // Get the app.
    $app = mediamosa_app::get_by_appid($app_id);

    $a_options = array();

    if (is_null($hint)) {
      // If hint is null, we want to use the site default parameters.
      if ($app[mediamosa_app_db::ALWAYS_HINT_MP4] == mediamosa_app_db::ALWAYS_HINT_MP4_TRUE) {
        $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
      }

      if ($app[mediamosa_app_db::ALWAYS_INSERT_MD] == mediamosa_app_db::ALWAYS_INSERT_MD_TRUE) {
        $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
      }
    }
    elseif ($hint == 'TRUE') {
      // We want to hint.
      $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION;
      $a_options[] =  mediamosa_settings::ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION;
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

    $execution_string .= (count($a_options) ? ' ' . implode(' ', $a_options) : '');

    return $execution_string;
  }

  /**
   * Generate the string that is used for the vpx_transcode script.
   *
   * 1.x: _vpx_jobserver_get_transcode_string
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id
   */
  static public function get_transcode_string($jobserver_job_id, $mediafile_id) {

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
      $parameter_string .= str_replace(':' , ' ', $pair) . ' ';
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

        // Now see if transcode function is found.
        if (class_exists($class_name) && method_exists($class_name, 'transcode')) {
          $options = array(
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

          // PHP 5.2.3 or higher.
          $execution_string = call_user_func_array($class_name . '::transcode', $options);
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
   * 1.x: _vpx_jobserver_get_still_string
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id
   * @return string|string|string
   */
  static public function get_still_string($jobserver_job_id, $mediafile_id) {

    $job_server_still = mediamosa_job_server_still::get($jobserver_job_id);
    $job_server = mediamosa_job_server::get($jobserver_job_id);
    $job_id = $job_server[mediamosa_job_server_db::JOB_ID];

    $execution_string = '';

    if ($job_server_still) {
      $frametime = (int) $job_server_still[mediamosa_job_server_still_db::FRAMETIME];
      $size = $job_server_still[mediamosa_job_server_still_db::SIZE];
      $h_padding = $job_server_still[mediamosa_job_server_still_db::H_PADDING];
      $v_padding = $job_server_still[mediamosa_job_server_still_db::V_PADDING];

      $still_parameters = unserialize($job_server_still[mediamosa_job_server_still_db::STILL_PARAMETERS]);
      $fps = (int) $still_parameters['fps'];
      $video_duration = (int) $still_parameters['video_duration'];

      // Video duration check, if still_type = scene.
      if (!$video_duration && $still_parameters['still_type'] == mediamosa_asset_mediafile_metadata::STILL_TYPE_SCENE) {
        // Some videos don't have video durations according to FFMpeg.
        // If we want to create multiple stills, we usually need this information calculating other data.
        // But in case of scene detection we have a work around, we can put the video duration on a maximum value.
        self::log_debug_high_mediafile($mediafile_id, t('No video duration, when still_type = scene. Try to get the complete video. video_duration = @video_duration.', array(
          '@video_duration' => print_r($video_duration, TRUE),
        )));
        $video_duration = self::VIDEO_DURATION_MAX;
      }

      $start_frame = min(max(0, (int) $still_parameters['start_frame']), max(0, $video_duration - 1));
      if (isset($still_parameters['end_frame']) && (int) $still_parameters['end_frame']) {
        $end_frame = min(max(0, (int) $still_parameters['end_frame']), max(0, $video_duration - 1));
      }
      else {
        $end_frame = max(0, $video_duration - 1);
      }

      // Make sure start and end are in the correct order from low to high.
      $start_frame_cpy = $start_frame;
      $start_frame = min($start_frame, $end_frame);
      $end_frame = max($start_frame_cpy, $end_frame);

      // When video_duration is 0, then fall back to the original behaviour.
      if (!$video_duration || $start_frame == $end_frame) {
        $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
        self::log_debug_high_mediafile($mediafile_id, t('No video duration or start frame = end frame. Fall back to still_type = NONE. video_duration = @video_duration, start_frame = @start_frame, end_frame = @end_frame.', array(
          '@video_duration' => print_r($video_duration, TRUE),
          '@start_frame' => print_r($start_frame, TRUE),
          '@end_frame' => print_r($end_frame, TRUE),
        )));
      }

      // Make sure still_type is uppercase.
      $still_parameters['still_type'] = mediamosa_unicode::strtoupper($still_parameters['still_type']);

      // Data check
      switch ($still_parameters['still_type']) {
        case 'NORMAL':
          if (!is_numeric($still_parameters['still_per_mediafile']) || $still_parameters['still_per_mediafile'] < 1) {
            $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
            $error_description = 'Still_per_mediafile parameter is missing. Still creation fall back to still_type = NONE';
            self::log_debug_high_mediafile($mediafile_id, t('Still_per_mediafile parameter is missing. Still creation fall back to still_type = NONE.'));
          }
          break;

        case 'SECOND':
          if (!is_numeric($still_parameters['still_every_second']) || $still_parameters['still_every_second'] < 0) {
            $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
            $error_description = 'Still_every_second parameter is missing. Still creation fall back to still_type = NONE';
            self::log_debug_high_mediafile($mediafile_id, t('Still_every_second parameter is missing. Still creation fall back to still_type = NONE.'));
          }
          break;

        case 'SCENE':
          if (!$fps) {
            $still_parameters['still_type'] = mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE;
            $error_description = 'FPS parameter is 0. Still creation fall back to still_type = NONE';
            self::log_debug_high_mediafile($mediafile_id, t('FPS parameter is 0. Still creation fall back to still_type = NONE.'));
          }
          break;

        default:
          break;
      }

      // Store error_description.
      if (!empty($error_description)) {
        self::log_mediafile($mediafile_id, $error_description);
        mediamosa_db::db_update(mediamosa_job_server_db::TABLE_NAME)
          ->fields(array(
            mediamosa_job_server_db::ERROR_DESCRIPTION => $error_description,
          ))
          ->condition(mediamosa_job_server_db::ID, $jobserver_job_id)
          ->execute();
      }

      // Frametime but be between start and end frame. Put it roughly 1 / 3 in
      // length.
      if ($frametime < $start_frame || $frametime > $end_frame) {
        $frametime = (($end_frame - $start_frame) / 3) + $start_frame;
      }

      $duration = $framerate = 1;

      switch (mediamosa_unicode::strtoupper($still_parameters['still_type'])) {

        case 'NONE':
          if ($job_server_still[mediamosa_job_server_still_db::BLACKSTILL_CHECK] == mediamosa_job_server_still_db::BLACKSTILL_CHECK_FALSE || $video_duration <= 1) {
            break;
          }

          $still_parameters['still_per_mediafile'] = 5;
          $start_frame = max($frametime - self::BLACK_STILL_INTERVAL, 0);
          $end_frame = min($frametime + self::BLACK_STILL_INTERVAL, $video_duration - 1);
          // Go to the NORMAL now.

        case 'NORMAL':
          $still_per_mediafile = max($still_parameters['still_per_mediafile'], 1);
          $frametime = $start_frame;
          $duration = ($end_frame - $start_frame);
          // |--------- video length -----------|
          // Stills per mediafile = 4
          // |------S------S------S------S------|
          // $framesecond = duration / (4 + 1)
          // .......|-- video length -----------|
          // frametime += $framesecond
          // duration -= $framesecond
          $framesecond = $duration / ($still_per_mediafile + 1);
          $frametime += $framesecond;
          $duration -= $framesecond;
          // Frames per second
          $framerate = 1 / max($framesecond, 1);
          // Safety check
          if ($duration / max($framesecond, 1) > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $framesecond;
          }
          break;

        case 'SECOND':
          $still_every_second = $still_parameters['still_every_second'];
          $frametime = $start_frame;
          $duration = ($end_frame - $start_frame);
          $framerate = 1 / $still_every_second;
          // Safety check
          if ($duration / $still_every_second > mediamosa_settings::STILL_MAXIMUM) {
            $duration = mediamosa_settings::STILL_MAXIMUM * $still_every_second;
          }
          break;

        case 'SCENE':
          self::log_mediafile($mediafile_id, 'Scene detection starts');
          $destination_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get() . DIRECTORY_SEPARATOR . 'transcode' . DIRECTORY_SEPARATOR;
          $input = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id);

          // Clean up I.
          mediamosa_io::unlink($destination_path . $mediafile_id . '_scene.avi');
          mediamosa_io::unlink($destination_path . $mediafile_id . '_list.el');


  /*
  // Old code
          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $input, $destination_path . $mediafile_id .'_scene.avi');
          //self::log_mediafile($mediafile_id, $order);
          exec($order);
  */

  // Hot fix
          $order = sprintf('ffmpeg -i %s %s', $input, $destination_path . $mediafile_id . '_scene.wmv');
          //self::log_mediafile($mediafile_id, $order);
          mediamosa_io::exec($order);

          $order = sprintf('ffmpeg -i %s -r 25 -an -vcodec mjpeg %s', $destination_path . $mediafile_id . '_scene.wmv', $destination_path . $mediafile_id . '_scene.avi');
          //self::log_mediafile($mediafile_id, $order);
          mediamosa_io::exec($order);

          mediamosa_io::unlink($destination_path . $mediafile_id . '_scene.wmv');

  // Hot fix - end

          $order = sprintf('lav2yuv -T 1 -S %s %s', $destination_path . $mediafile_id . '_list.el', $destination_path . $mediafile_id . '_scene.avi');
          self::log_mediafile($mediafile_id, $order);
          mediamosa_io::exec($order);

          $output = NULL;
          $order = sprintf('cat %s', $destination_path . $mediafile_id . '_list.el');
          //self::log_mediafile($mediafile_id, $order);
          mediamosa_io::exec($order, $output);
          //self::log_mediafile($mediafile_id, sprintf('output = %s', print_r($output, TRUE)));

          // Clean up II.
          mediamosa_io::unlink($destination_path . $mediafile_id . '_list.el');

          // Analyze
          if ($output && is_array($output) && $output != array()) {
            //self::log_mediafile($mediafile_id, sprintf('Output is array with data'));

            // tbr & map from the best stream
            $map = NULL;
            $tbr = NULL;
            $kbs = NULL;
            $order = sprintf('ffmpeg -i %s 2>&1', $input);
            //self::log_mediafile($mediafile_id, $order);
            $details = array();
            mediamosa_io::exec($order, $details);
            //self::log_mediafile($mediafile_id, 'Details: '. print_r($details, TRUE));
            foreach ($details as $line) {
              if (stripos($line, ' Stream ') !== FALSE && stripos($line, ' Video: ') !== FALSE) {
                if (preg_match('/Stream #[0-9]*\.([0-9]*)/', $line, $reg_map) && preg_match('/ ([0-9]*) kb\/s/', $line, $reg_kbs)) {
                  if (!$map || !$kbs || $kbs < $reg_kbs[1]) {
                    $map = $reg_map[1];
                    $kbs = $reg_kbs[1];
                  }
                  if (preg_match('/ ([0-9]*(\.[0-9]*)+) tb/', $line, $regs)) {
                    $tbr = $regs[1];
                  }
                }
                elseif (!$map && preg_match('/ ([0-9]*(\.[0-9]*)+) tb/', $line, $regs)) {
                  $tbr = $regs[1];
                  if (preg_match('/Stream #[0-9]*\.([0-9]*)/', $line, $reg_map)) {
                    $map = $reg_map[1];
                  }
                }
              }
            }
            //self::log_mediafile($mediafile_id, 'tbr: '. print_r($tbr, TRUE) .', map: '. print_r($map, TRUE) .', kbs: '. print_r($kbs, TRUE));
            if (!$tbr) {
              // This is the base value
              $tbr = 25;
            }
            // Go further, if we have tbr
            if ($tbr) {

              $scene_frame = NULL;
              $i = 1;
              $scenes = array();
              foreach ($output as $line) {
                //self::log_mediafile($mediafile_id, sprintf('Line: %s', print_r($line, TRUE)));
                $line_args = explode(' ', $line);
                //self::log_mediafile($mediafile_id, sprintf('Line args: %s', print_r($line_args, TRUE)));
                if ($line_args && is_array($line_args) && $line_args != array()) {
                  //self::log_mediafile($mediafile_id, sprintf('Line args is array with data'));
                  if (isset($line_args[0]) && isset($line_args[1]) && isset($line_args[2]) && !isset($line_args[3]) && is_numeric($line_args[0]) && is_numeric($line_args[1]) && is_numeric($line_args[2])) {
                    //self::log_mediafile($mediafile_id, sprintf('Line args has 3 numbers'));
                    // $line_args[1] + num = a little bit after the change
                    $change_frame = $line_args[1] + mediamosa_settings::STILL_SCENE_AFTER;
                    //self::log_mediafile($mediafile_id, 'change_frame: '. print_r($change_frame, TRUE) .', start_frame: '. print_r($start_frame, TRUE) .', fps = '. $fps .', end_frame: '. print_r($end_frame, TRUE));
                    if ($change_frame >= ($start_frame*$fps) && $change_frame <= ($end_frame*$fps)) {
                      // $scene_frame + num = minimal distance with two changes
                      //self::log_mediafile($mediafile_id, 'scene_frame: '. print_r($scene_frame, TRUE));
                      if (!$scene_frame || $scene_frame + mediamosa_settings::STILL_SCENE_DISTANCE < $change_frame) {
                        $scene_frame = $change_frame;
                        //self::log_mediafile($mediafile_id, sprintf('New scene frame: %s', $scene_frame));
                        // The avi has 25 frame/sec
                        $scene_sec = (int)($scene_frame/25);
                        $scenes[] = $scene_sec;
                        $i++;
                        if ($i > mediamosa_settings::STILL_MAXIMUM) {
                          // Emergency break
                          break;
                        }
                      }
                    }
                  }
                }
              }
            }
          }

          // Save the sec data (see: vpx_jobhandler.module)
          if (is_array($scenes) && $scenes != array()) {
            $my_file = $destination_path . $job_id . '_ffmpeg_scene.txt';
            $fh = fopen($my_file, 'w') or $fh = NULL;
            if ($fh) {
              foreach ($scenes as $scene) {
                fwrite($fh, $scene . "\n");

              }
              fclose($fh);

              // This version is creating stills for the original video
              if (mediamosa_settings::get_ffmpeg_pad()) {
                $order = sprintf(mediamosa_settings::STILL_SCENE_STRING, $input, $size, $h_padding, $h_padding, $v_padding, $v_padding, $my_file, ($map ? '-map 0.' . $map : ''), $destination_path . $mediafile_id . mediamosa_settings::STILL_EXTENSION . '.jpeg');
              }
              else {
                $size_explode = explode('x', $size);
                $size_width = $size_explode[0] + (2 * $v_padding);
                $size_height = $size_explode[1] + (2 * $h_padding);
                $order = sprintf(mediamosa_settings::STILL_SCENE_STRING_VFPAD, $input, $size, $size_width, $size_height, $v_padding, $h_padding, $my_file, ($map ? '-map 0.' . $map : ''), $destination_path . $mediafile_id . mediamosa_settings::STILL_EXTENSION . '.jpeg');
              }

              // This version is creating stills for the analysed file
              //$order = sprintf(mediamosa_settings::STILL_SCENE_STRING, $destination_path . $mediafile_id .'_scene.avi', $size, $h_padding, $h_padding, $v_padding, $v_padding, $my_file, '', $destination_path . $mediafile_id . mediamosa_settings::STILL_EXTENSION .".jpeg");
              self::log_mediafile($mediafile_id, $order);
              mediamosa_io::exec($order, $details);
              //self::log_mediafile($mediafile_id, 'Details: '. print_r($details, TRUE));

              mediamosa_io::unlink($my_file);
            }

            // Clean up III.
            mediamosa_io::unlink($destination_path . $mediafile_id . '_scene.avi');

            // It has to be the last, because it triggers the further processes
            $my_file = $destination_path . $job_id . '_scene.txt';
            $fh = fopen($my_file, 'w') or $fh = NULL;
            if ($fh) {
              foreach ($scenes as $scene) {
                fwrite($fh, $scene . "\n");
              }
              fclose($fh);
            }

            // Do nothing after this
            return 'echo "Scene stills are rock"';
          }

          // Clean up IV.
          mediamosa_io::unlink($destination_path . $mediafile_id . '_scene.avi');

          // We reached this point, so something went wrong in the creation of scene stills.
          // So we are creating still type NONE (original behaviour).
          break;

        default:
          break;
      }

    }
    else {
      self::log_mediafile($mediafile_id, 'Still job not found, jobserver_job_id: @jobserver_job_id', array('@jobserver_job_id' => $jobserver_job_id));
      return $execution_string;
    }

    $still_parameters['frametime'] = $frametime;
    $still_parameters['duration'] = $duration;
    $still_parameters['framerate'] = $framerate;

    $fields = array(
      mediamosa_job_db::STILL_PARAMETERS => serialize($still_parameters),
    );

    // Enrich with changed/created.
    $fields = mediamosa_db::db_update_enrich($fields);
    mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_job_db::ID, $job_id)
      ->execute();

    // Combine the string (only for linux, windows does not generate stills).
    if (mediamosa_settings::get_ffmpeg_pad()) {
      $execution_string = sprintf('%s %s %s jpeg ' . mediamosa_settings::STILL_STRING, mediamosa_settings::still_file(), mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(), $mediafile_id, $size, $h_padding, $h_padding, $v_padding, $v_padding,
      round($duration * $framerate),
      $frametime, $duration, $framerate);
    }
    else {
      $size_explode = explode('x', $size);
      $size_width = $size_explode[0] + (2 * $v_padding);
      $size_height = $size_explode[1] + (2 * $h_padding);
      $execution_string = sprintf('%s %s %s jpeg ' . mediamosa_settings::STILL_STRING_VFPAD, mediamosa_settings::still_file(), mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get(), $mediafile_id, $size, $size_width, $size_height, $v_padding, $h_padding,
      round($duration * $framerate),
      $frametime, $duration, $framerate);
    }
    $execution_string .= ' > /dev/null &';
    return $execution_string;
  }

  /**
   * Create an array of the status file.
   *
   * 1.x: _vpx_jobserver_get_status
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
   * 1.x: _vpx_jobserver_store_new_still
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
   * 1.x: _vpx_jobserver_store_new_mediafile
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
