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
  * The job handler server.
  */
class mediamosa_job_scheduler {

  /**
   * Log message.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'job scheduler');
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
    mediamosa_watchdog::log_mediafile($mediafile_id, $message, $variables, $asset_id, $severity, 'job scheduler');
  }

  /**
   * Log message (low prio, is only logged when mediamosa_debug is lvl 5 or higher).
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log_debug($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'job_scheduler');
  }

  /**
   * Log message (High prio, is only logged when mediamosa_debug is lvl 10 or higher).
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  static public function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_high($message, $variables, 'job_scheduler');
  }

  /**
   * Trigger the job scheduler.
   */
  static public function trigger() {
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
   *
   */
  static public function run_parse_queue() {
    // Triggert from sandbox, run once, and done.
    if (mediamosa::in_simpletest_sandbox()) {
      self::log('Job Scheduler; run_parse_queue() in sandbox, hit and return.');
      self::parse_queue();
      return;
    }

    $run_last = variable_get('mediamosa_jobscheduler_cron_last', NULL);

    // may run? (0 - 60 seconds)
    if (round(REQUEST_TIME - $run_last) < 60) {
      return; // Still running.
    }

    // A period of 60 to 120 seconds we wait till its killed (or done running).
    $run_last = variable_get('mediamosa_jobstatus_cron_running', NULL);
    if (round(REQUEST_TIME - $run_last) < 120) {
      self::log('run_parse_queue() in safety timeout zone.');
      return; // In timeout zone.
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
        // ignore, its logged
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
   * This function is the heart of the handler system. It must be
   * called at regular intervals, like every 1 minute.
   * It will cleanup, run and update jobs.
   */
  static public function parse_queue() {

    // Log start point.
    self::log_debug('Start parse queue job scheduler');

    /**
     * Get all servers for their status and process the result.
     */
    self::check_status_all_servers();

    /**
     * Start new jobs when slots are available.
     */
    self::start_new_jobs();

    /**
     * Remove any timedout upload jobs.
     */
    self::check_upload_timeout();


    /**
     * Remove any timedout transcode jobs.
     */
    self::check_transcode_timeout();


    /**
     * Put server that are set on close, on off when they have done all jobs.
     */
    self::check_server_on_status_close();


    // Log end point.
    self::log_debug('Ending parse queue job scheduler');
  }

  /**
   * Get a list of servers and process the job status.
   */
  static public function check_status_all_servers() {
    $job_processor_servers = mediamosa_server::get_enabled_job_processor();

    foreach ($job_processor_servers as $job_processor_server) {
      self::update_server_job_status($job_processor_server[mediamosa_server_db::SERVER_URI]);
    }
  }

  /**
   * With A REST call we'll retrieve the status of the jobs.
   * And we'll process the jobs.
   *
   * @param $uri
   */
  static public function update_server_job_status($uri) {

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
      self::log('Parse XML: @msg (@resp)', array('@msg' => $e->getMessage(), '@resp' => print_r($response->data, TRUE)), WATCHDOG_ERROR);
      return;
    }

    for ($i = 0; $i < $response_data->header->item_count; $i++) {
      // Get current status.
      $a_job = mediamosa_job::get($response_data->items->item[$i]->job_id);

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
        ->condition(mediamosa_job_db::ID, $response_data->items->item[$i]->job_id)
        ->execute();

      // Trigger url.
      mediamosa_job::notify_transcoding($a_job[mediamosa_job_db::JOB_STATUS], $response_data->items->item[$i]->status, $response_data->items->item[$i]->job_id);

      if ($response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FINISHED ||
          $response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FAILED ||
          $response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_CANCELLED) {

        // Remove link between server and job.
        mediamosa_server_job::delete_by_jobid($response_data->items->item[$i]->job_id);

        // Remove job on jobserver.
        self::remove_job_on_server($uri, $response_data->items->item[$i]->job_id);
      }

      if ($response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FINISHED) {
        switch ($response_data->items->item[$i]->job_type) {
          case mediamosa_job_db::JOB_TYPE_TRANSCODE:
            // Statistics.
            mediamosa_statistics::insert_job_transcode($response_data->items->item[$i]->mediafile_dest, $a_job[mediamosa_job_db::ID]);
            // If the transcode job is completed, add it to the mediafile.
            self::parse_finished_transcode($response_data->items->item[$i]->job_id, $response_data->items->item[$i]->mediafile_src, $response_data->items->item[$i]->mediafile_dest);
            break;

          case mediamosa_job_db::JOB_TYPE_ANALYSE:
            // If the analyse job is completed, add it to the technical metadata.
            mediamosa_asset_mediafile_metadata::store_analyse($response_data->items->item[$i]->job_id, explode("\n" , $response_data->items->item[$i]->analyse_result));
            break;

          case mediamosa_job_db::JOB_TYPE_STILL:
            // If the still job is completed, add the still to db.
            // We serialize it, because of the multiple stills
            self::add_still_to_db($response_data->items->item[$i]->job_id, unserialize($response_data->items->item[$i]->mediafile_dest));
            break;
        }
      }
      elseif ($response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_FAILED ||
              $response_data->items->item[$i]->status == mediamosa_job_db::JOB_STATUS_CANCELLED) {

        switch ($response_data->items->item[$i]->job_type) {
          case mediamosa_job_db::JOB_TYPE_TRANSCODE:
            // if transcode has failed.
            self::parse_failed_transcode($response_data->items->item[$i]->job_id, $response_data->items->item[$i]->mediafile_dest);
            break;
        }
      }
    }
  }

  /**
   * Send a delete on server to remove job.
   *
   * @param string $uri
   * @param integer $job_id
   */
  static public function remove_job_on_server($uri, $job_id) {

    $rest_url = strtr('server/@job_id/delete', array('@job_id' => $job_id));

    // Call URL.
    mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);
  }

  /**
   * Get the informatiob of transcode job plus
   * related info.
   *
   * @param $job_id
   */
  function get_transcodejob_info($job_id) {
    $result = array();

    $query = mediamosa_db::db_select(mediamosa_job_db::TABLE_NAME, 'j');
    $query->join(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf', 'mf.asset_id = j.asset_id');
    $query->join(mediamosa_job_transcode_db::TABLE_NAME, 'jt', 'jt.job_id = j.job_id');
    $query->fields('j', array(mediamosa_job_db::ASSET_ID, mediamosa_job_db::JOB_TYPE, mediamosa_job_db::OWNER_ID, mediamosa_job_db::APP_ID));
    $query->fields('mf', array(mediamosa_asset_mediafile_db::FILENAME, mediamosa_asset_mediafile_db::ID));
    $query->fields('jt', array(mediamosa_job_transcode_db::TOOL, mediamosa_job_transcode_db::COMMAND, mediamosa_job_transcode_db::FILE_EXTENSION, mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID));
    $query->condition('j.' . mediamosa_job_db::ID, $job_id);
    $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE);
    $result_row = $query->execute()->fetchAssoc();

    if ($result_row) {
      $new_filename = mediamosa_io::get_base_filename($result_row[mediamosa_asset_mediafile_db::FILENAME]) . '.' . $result_row[mediamosa_asset_mediafile_db::FILE_EXTENSION];
      $result = array(
        'asset_id' => $result_row[mediamosa_job_db::ASSET_ID],
        'owner' => $result_row[mediamosa_job_db::OWNER_ID],
        'app_id' => $result_row[mediamosa_job_db::APP_ID],
        'filename' => $new_filename,
        'transcode_profile_id' => $result_row[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID],
        'tool' => $result_row[mediamosa_job_transcode_db::TOOL],
        'file_extension' => $result_row[mediamosa_asset_mediafile_db::FILE_EXTENSION],
        'command' => $result_row[mediamosa_job_transcode_db::COMMAND],
        'mediafile_id' => $result_row[mediamosa_asset_mediafile_db::ID]
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
   * @param $asset_id
   * @param $mediafile_id
   */
  static public function create_wmv_job($job_id, $asset_id, $mediafile_id) {

    // FIXME: @mediafile_id is empty.
    self::log_mediafile($mediafile_id, 'Start job wmv job: job_id = @job_id, mediafile_id = @mediafile_id', array('@job_id' => $job_id, '@mediafile_id' => $mediafile_id), $asset_id);

    // Get all info.
    $a_job_ext = mediamosa_job::get_job_ext($job_id);
    if (!$a_job_ext) {
      // FIXME: better throw exception?
      $command = '';
      $owner = '';
      $app_id = 0;
      self::log_mediafile($mediafile_id, 'A wmv job (@job_id) without transcode_job!', array('@job_id' => $job_id), $asset_id, WATCHDOG_EMERGENCY);
    }
    else {
      $command = $a_job_ext[mediamosa_job_transcode_db::COMMAND];
      $owner = $a_job_ext[mediamosa_job_db::OWNER_ID];
      $app_id = $a_job_ext[mediamosa_job_db::APP_ID];
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

    // Now create the transcode job.
    $fields = array(
      mediamosa_job_transcode_db::JOB_ID => $job_id_new,
      mediamosa_job_transcode_db::TOOL => mediamosa_job::JOBWINDOWS_TOOL,
      mediamosa_job_transcode_db::COMMAND => $command . ';internal_previous_job:' . $job_id,
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
   * Remove the raw avi file for wmv transcode
   *
   * @param integer $job_id
   */
  static public function wmv_remove_raw_file($job_id) {

    // Get the job.
    $a_job = mediamosa_job::get($job_id);

    // Get the mediafile ID.
    $mediafile_id = $a_job[mediamosa_job_db::MEDIAFILE_ID];

    if ($mediafile_id) {
      // Delete the mediafile.
      mediamosa_asset_mediafile::delete($mediafile_id);

      self::log_mediafile($mediafile_id, "Temp. mediafile '@mediafile_id' of job '@job_id' is removed.", array('@mediafile_id' => $mediafile_id, '@job_id' => $job_id), NULL, WATCHDOG_NOTICE);
    }
    else{
      self::log("Job @job_id not found, can't delete temp mediafile.", array('@job_id' => $job_id), NULL, WATCHDOG_ERROR);
    }
  }


  /**
   * Called when transcode was successfully finished.
   *
   * @param integer $job_id
   * @param string $mediafile_id_src
   * @param string $mediafile_id_dest
   */
  static public function parse_finished_transcode($job_id, $mediafile_id_src, $mediafile_id_dest) {

    // Get the original transcode job.
    $job_info = self::get_transcodejob_info($job_id);

    // Is the original job id found?
    $command_parameters = mediamosa_lib::create_named_array($job_info['command'], ';', ':');
    $original_job_id = isset($command_parameters['internal_previous_job']) ? $command_parameters['internal_previous_job'] : 0;
    if ($original_job_id > 0) {
      // Check if extension requires extra job, e.g. windows transcoding.
      if ($job_info['file_extension'] == mediamosa_job::JOBRAW_FILE_EXTENSION) {
        // 1st pass: the raw intermediate has been created, now convert it to wmv

        // NOTE should *not* be converted to api call: the API sets
        // ownership, and we do not want this intermediate result to
        // influence quota &c.
        $fields = array(
          mediamosa_asset_mediafile_db::ID => $mediafile_id_dest,
          mediamosa_asset_mediafile_db::ASSET_ID => $job_info['asset_id'],
          mediamosa_asset_mediafile_db::FILENAME => $job_info["filename"],
          mediamosa_asset_mediafile_db::APP_ID => $job_info["app_id"],
          mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_configuration_storage::mount_point_get(),
          mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
        );

        // Add created.
        $fields = mediamosa_db::db_insert_enrich($fields);

        // Create.
        mediamosa_db::db_insert(mediamosa_asset_mediafile_db::TABLE_NAME)
          ->fields($fields)
          ->execute();

        // Add job for schedular.
        self::create_wmv_job($original_job_id, $job_info['asset_id'], $mediafile_id_dest);

        // Set progress.
        mediamosa_job::set_progress($original_job_id, '0.666', FALSE);

        // bail out: the mediafile record for the intermediate result
        // has been created; we don't want any further action here.
        return;
      }
      else {
        // 2nd pass: the raw intermediate has been converted to wmv, now delete the intermediate
        self::wmv_remove_raw_file($job_id);

        // Current status.
        $status = $job_info[mediamosa_job_db::JOB_STATUS];

        // Set progress.
        mediamosa_job::set_progress($original_job_id, '1.000', FALSE, mediamosa_job_db::JOB_STATUS_FINISHED);
        mediamosa_job::notify_transcoding($status, JOBSTATUS_FINISHED, $original_job_id);
      }
    }

    // Get Asset.
    $a_asset = mediamosa_asset::get($job_info['asset_id']);
    assert(!empty($a_asset));

    $fields = array(
      mediamosa_asset_mediafile_db::ID => $mediafile_id_dest,
      mediamosa_asset_mediafile_db::ASSET_ID => $job_info['asset_id'],
      mediamosa_asset_mediafile_db::FILENAME => $job_info['filename'],
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => mediamosa_configuration_storage::mount_point_get(),
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

    // Get mediafile.
    $a_mediafile = mediamosa_asset_mediafile::get($job_info['mediafile_id']);

    if ($a_mediafile[mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL] == mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE) {
      mediamosa_acl::replace_mediafile_to_mediafile($job_info['mediafile_id'], $mediafile_id_dest);
    }

    // Analyse the mediafile.
    $analyse_result = mediamosa_asset_mediafile::analyse($job_info["app_id"], (string)$mediafile_id_dest);
    assert(is_array($analyse_result));

    // Store it.
    mediamosa_asset_mediafile_metadata::store_analyse_without_job($job_id, $analyse_result, (string)$mediafile_id_dest);
  }

  /**
   * Process the data from a still job.
   *
   * @param $job_id
   * @param $filenames
   */
  static public function add_still_to_db($job_id, $filenames) {
    // Scene changes
    $destination_path = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::data_location_get() . DIRECTORY_SEPARATOR . 'transcode' . DIRECTORY_SEPARATOR;
    $my_file = $destination_path . $job_id . '_scene.txt';

    $scenes = array();

    if (file_exists($my_file)) {
      $fh = @fopen($my_file, 'r');
      if ($fh) {
        while (!feof($fh)) {
          $scenes[] = (int) fgets($fh);
        }
        fclose($fh);
      }
    }

    // Get job.
    $a_job = mediamosa_job::get($job_id);
    $asset_id = $a_job['asset_id'];
    $still_parameters = unserialize($a_job['still_parameters']);

    // Get Asset.
    $a_asset = mediamosa_asset::get($asset_id);

    self::log_mediafile($a_job['mediafile_id'], 'Start creation DB multiple still, e.g. @filename, job: @job_id, still_parameters: @still_parameters', array('@filename' => $filenames[0], '@job_id' => (string)$job_id, '@still_parameters' => print_r($still_parameters, TRUE)), $asset_id);

    // Remove old stills
    // We have multiple stills now, so we don't delete the old ones
    // And deleting with asset_id is definetly not a good idea, while we have multiple stills per mediafile
    // _media_management_delete_still($asset_id);

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
        mediamosa_asset_mediafile_still::create($asset_id, $filename, $a_job['app_id'], $a_job['owner'], '', $order, !$order, $still_parameters, ($scenes == array() ? $sec : $scenes[$i]), $a_job['mediafile_id'], $tag);
        $order++;
        if (isset($second) && is_numeric($second)) {
          $sec += $second;
        }

        $i++;
      }
    }

    // Remove file.
    mediamosa_io::unlink($my_file);
  }

  /**
   * Called when transcode failed or was canceled.
   *
   * @param integer $job_id
   * @param string $mediafile_id_dest
   */
  static public function parse_failed_transcode($job_id, $mediafile_id_dest) {
    // haal de asset id en original filename op basis van een job op
    $a_job_info = self::get_transcodejob_info($job_id);

    // controleer of de vorige job_id naar boven is te halen.
    $command_parameters = mediamosa_lib::create_named_array($a_job_info["command"], ";", ":");
    $original_job_id = empty($command_parameters['internal_previous_job']) ? 0 : $command_parameters['internal_previous_job'];
    if ($original_job_id > 0) {
      if ($a_job_info[mediamosa_job_transcode_db::FILE_EXTENSION] != mediamosa_job::JOBRAW_FILE_EXTENSION) {

        // Remove the raw file
        self::wmv_remove_raw_file($job_id);

        // Get original.
        $a_job_original = mediamosa_job::get($job_id);
        $status = $a_job_original[mediamosa_job_db::JOB_STATUS];

        // Set failed.
        mediamosa_job::set_progress($original_job_id, '1.000', FALSE, mediamosa_job_db::JOB_STATUS_FAILED);

        // Trigger url.
        mediamosa_job::notify_transcoding($status, mediamosa_job_db::JOBSTATUS_FAILED, $original_job_id);
      }
    }
  }

  /**
   * Get the list and find empty slots to start jobs.
   */
  static public function start_new_jobs() {
    // Get for every active server (status ON)
    // the number of job that is not FINISHED, FAILED of CANCELLED
    // and job type is not UPLOAD (does not take a slot).
    $server_jobs = mediamosa_db::db_query(
      "SELECT #nid, #uri, #slots,
             (SELECT COUNT(msj.#job_id)
              FROM {#mediamosa_server_job} AS msj
              JOIN {#mediamosa_job} AS mj ON mj.#job_id = msj.#job_id
              WHERE
                msj.#server_id = ms.#nid
                AND mj.#job_type != :JOBTYPE_UPLOAD
                AND mj.#job_status NOT IN (:JOBSTATUS_FINISHED, :JOBSTATUS_FAILED, :JOBSTATUS_CANCELLED)) AS jobcount
      FROM {#mediamosa_server} AS ms
      WHERE #server_status = :server_status_on AND #server_type = :server_type",
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
          break; // no more for this server.
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
   * Based on job type extra information is gathered.
   * Then with REST the job is send.
   * At the end the job is attached to the mediamosa_server_job.
   *
   * @param $job_id
   * @param $job_type
   * @param $server_id
   * @param $uri
   * @param $mediafile_id
   * @param $mediafile_dest
   */
  static public function start_server_job($job_id, $job_type, $server_id, $uri, $mediafile_id) {
    self::log_mediafile($mediafile_id, 'Start job: job_id = @job_id, job_type = @job_type, server_id = @server_id, mediafile_id = @mediamosa_id', array('@job_id' => $job_id, '@job_type' => $job_type, '@server_id' => $server_id, '@mediamosa_id' => $mediafile_id));

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
    }

    if ($job_type == mediamosa_job_db::JOB_TYPE_DELETE_MEDIAFILE) {
        self::start_server_delete_job($job_id, $mediafile_id);
        self::log_mediafile($mediafile_id, 'Process @job_id job: mediafile @mediafile_id removed.', array('@job_id' => $job_id, '@mediafile_id' => $mediafile_id));
    }
    else {
      // if okay update the database with the job info
      if ($errorcode == mediamosa_error::ERRORCODE_OKAY) {

        // Check if job still exists
        // to be 100%, we might have to lock table ?
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

          self::log_mediafile($mediafile_id, 'Sending @job_type (ID: @job_id) to server @uri.', array('@job_type' => $job_type, '@job_id' => $job_id, '@uri' => $uri));
        }
        else {
          self::log_mediafile($mediafile_id, 'Making @job_type server job has stoped, job @job_id was deleted during the process.', array('@job_type' => $job_type, '@job_id' => $job_id));
        }
      }
      else {
        self::log_mediafile($mediafile_id, 'Error (@errorcode) was reponsed during sending @job_type (ID: @job_id) to server @uri.', array('@errorcode' => $errorcode, '@job_type' => $job_type, '@job_id' => $job_id, '@uri' => $uri));
      }
    }
  }

  /**
   * Start transcode job by REST call.
   *
   * 1.x: _vpx_jobhandler_start_server_transcode_job
   *
   * @param integer $job_id
   * @param string $uri
   * @param string $mediafile_id
   */
  static public function start_server_transcode_job($job_id, $uri, $mediafile_id) {
    // Get job parameters;
    $job_parameters = self::get_job_parameters($job_id, mediamosa_job_db::JOB_TYPE_TRANSCODE, $mediafile_id);

    $rest_url = sprintf('server/jobstart?job_id=%s&job_type=%s&mediafile_src=%s&tool=%s&file_extension=%s&command=%s',
      $job_id,
      mediamosa_job_db::JOB_TYPE_TRANSCODE,
      $mediafile_id,
      $job_parameters['tool'],
      $job_parameters['file_extension'],
      $job_parameters['command']
    );

    // Log call.
    self::log_mediafile($mediafile_id, 'Start job:' . $rest_url);

    // Do request.
    $response = mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);

    if ($response->code == '201' || $response->code == '200') {
      try {
        $simple_xml_response = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        self::log_mediafile($mediafile_id, 'Parse XML: @msg (@resp)', array('@msg' => $e->getMessage(), '@resp' => print_r($response->data, TRUE)), NULL, WATCHDOG_ERROR);
      }

      return (int) $simple_xml_response->header->request_result_id;
    }

    self::log_mediafile($mediafile_id, 'errorcode @response with url @rest_url (@header)', array('@response' => $response->code, '@rest_url' => $rest_url, '@header' => var_export($response->header, TRUE)));
    return $response->code;
  }

  /**
   * Start analyse job by REST call.
   *
   * 1.x: _vpx_jobhandler_start_server_analyse_job
   *
   * @param integer $job_id
   * @param string $uri
   * @param string $mediafile_id
   */
  static public function start_server_analyse_job($job_id, $uri, $mediafile_id) {

    // Delete metadata.
    mediamosa_asset_mediafile_metadata::delete_by_mediafileid($mediafile_id);

    $rest_url = sprintf('server/jobstart?job_id=%s&job_type=%s&mediafile_src=%s',
      $job_id,
      mediamosa_job_db::JOB_TYPE_ANALYSE,
      $mediafile_id
    );

    // Log call.
    self::log_mediafile($mediafile_id, 'Start job:' . $rest_url);

    // Do request.
    $response = mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);

    if ($response->code == "201" || $response->code == "200") {
      try {
        $simple_xml_response = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        self::log_mediafile($mediafile_id, 'Parse XML: @msg (@resp)', array('@msg' => $e->getMessage(), '@resp' => print_r($response->data, TRUE)), NULL, WATCHDOG_ERROR);
      }

      return (int) $simple_xml_response->header->request_result_id;
    }

    self::log_mediafile($mediafile_id, 'errorcode @response with url @host/@rest_url', array('@response' => $response->code, '@rest_url' => $rest_url, '@host' => $uri));
    return $response->code;
  }

  /**
   * Function for starting a still job
   */
  static public function start_server_still_job($job_id, $uri, $mediafile_id) {
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
      if ($video_duration < $job_parameters["frametime"]) {
        // If the video duration is less than still picture time, we decrease the still picture time to half of video duration
        $job_parameters["frametime"] = ($video_duration >> 1);
      }
    }
    // stuur de job naar de job server
    $rest_url = sprintf('server/jobstart?job_id=%s&job_type=%s&mediafile_src=%s&frametime=%s&size=%s&h_padding=%s&v_padding=%s&blackstill_check=%s&still_type=%s&still_per_mediafile=%s&still_every_second=%s&start_frame=%s&end_frame=%s&video_duration=%s&fps=%s&tag=%s&watermark_id=%s&watermark_dst_x=%s&watermark_dst_y=%s&watermark_pct=%s&watermark_v_align=%s&watermark_h_align=%s',
      $job_id, mediamosa_job_db::JOB_TYPE_STILL, $mediafile_id,
      $job_parameters['frametime'],
      $job_parameters['size'],
      $job_parameters['h_padding'],
      $job_parameters['v_padding'],
      $job_parameters['blackstill_check'],
      $job_parameters['still_parameters']['still_type'],
      $job_parameters['still_parameters']['still_per_mediafile'],
      $job_parameters['still_parameters']['still_every_second'],
      $job_parameters['still_parameters']['start_frame'],
      $job_parameters['still_parameters']['end_frame'],
      $video_duration,
      $fps,
      $job_parameters['still_parameters']['tag'],
      $job_parameters['still_parameters']['watermark_id'],
      $job_parameters['still_parameters']['watermark_dst_x'],
      $job_parameters['still_parameters']['watermark_dst_y'],
      $job_parameters['still_parameters']['watermark_pct'],
      $job_parameters['still_parameters']['watermark_v_align'],
      $job_parameters['still_parameters']['watermark_h_align']
    );

    // Log call.
    self::log_mediafile($mediafile_id, 'Start job:' . $rest_url);

    // Do request.
    $response = mediamosa_http::do_internal_call($uri, $rest_url, '', mediamosa_http::METHOD_POST);

    if ($response->code == '201' || $response->code == '200') {

      try {
        $simple_xml_response = new mediamosa_connector_response($response->data);
      }
      catch (Exception $e) {
        self::log_mediafile($mediafile_id, 'Parse XML: @msg (@resp)', array('@msg' => $e->getMessage(), '@resp' => print_r($response->data, TRUE)), NULL, WATCHDOG_ERROR);
      }

      return (int) $simple_xml_response->header->request_result_id;
    }

    self::log_mediafile($mediafile_id, 'errorcode @response with url @rest_url', array('@response' => $response->code, '@rest_url' => $rest_url));
    return $response->code;
  }

  /**
   * Choose a job that applies to the following rules;
   *  - There are no other jobs working for the same asset (on any server)
   *  - job is not WAITING
   *  - There is no active upload for the asset.
   *  - Low priority goes first. ?
   *  - Job may not be started on other server.
   *
   *  1.x: _vpx_jobhandler_fetch_single_available_job
   *
   * @param integer $server_id
   */
  static public function fetch_single_available_job($server_id) {
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
            mj.job_type != :JOBTYPE_UPLOAD
          AND
            (
              (mj.job_type = :JOBTYPE_TRANSCODE AND
                (SELECT COUNT(*) FROM {#mediamosa_job_transcode} AS mjt
                  JOIN {#mediamosa_server_tool} USING(tool)
                  WHERE #nid = :nid AND mjt.job_id = mj.job_id LIMIT 1) = 1)
            OR
              (mj.job_type = :JOBTYPE_ANALYSE AND
                (SELECT COUNT(*) FROM {#mediamosa_server_tool} AS mstt
                  WHERE mstt.tool = :JOBTYPE_ANALYSE AND mstt.#nid = :nid LIMIT 1) = 1)
            OR
              (mj.job_type = :JOBTYPE_STILL AND
                (SELECT COUNT(*) FROM {#mediamosa_server_tool} AS mstt2
                  WHERE mstt2.tool = :JOBTYPE_STILL AND mstt2.#nid = :nid LIMIT 1) = 1)
            OR
              (mj.job_type = :JOBTYPE_DELETE_MEDIAFILE)
            )
          AND
            mj.job_id NOT IN
              (SELECT job_id FROM {#mediamosa_server_job})
          ORDER BY priority, job_id
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
   * @param $job_id
   * @param $job_type
   * @param $mediafile_id
   */
  static public function get_job_parameters($job_id, $job_type, $mediafile_id) {
    $result = array();

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        // Get job transcode.
        $job_transcode = mediamosa_job_transcode::get($job_id);

        $result["profile_id"] = $job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID];
        $result["tool"] = $job_transcode[mediamosa_job_transcode_db::TOOL];
        $result["file_extension"] = $job_transcode[mediamosa_job_transcode_db::FILE_EXTENSION];
        $result["command"] = self::map_parameters($job_transcode[mediamosa_job_transcode_db::TOOL], $job_transcode[mediamosa_job_transcode_db::COMMAND], $mediafile_id);
        break;

      case mediamosa_job_db::JOB_TYPE_STILL:
        $query_job = mediamosa_job_still::get($job_id);

        if ($query_job) {
          $result["blackstill_check"] = $query_job['blackstill_check'];
          $result["still_parameters"] = unserialize($query_job['still_parameters']);
          $result["frametime"] = $result["still_parameters"]['frametime'];
          $result["h_padding"] = $result["still_parameters"]['h_padding'];
          $result["v_padding"] = $result["still_parameters"]['v_padding'];
          $result["tag"] = $result["still_parameters"]['tag'];

          $mediafile = mediamosa_asset_mediafile::get($mediafile_id, NULL, array(mediamosa_asset_mediafile_db::APP_ID));
          $app_id = $mediafile[mediamosa_asset_mediafile_db::APP_ID];

          // Calculating aspect ratio
          // Base value
          $target_size = $result["still_parameters"]['size'];
          switch ($query_job['size']) {
            case 'sqcif':
              $target_size = '128x96';
              break;
            case 'qcif':
              $target_size = '176x144';
              break;
            case 'cif':
              $target_size = '352x288';
              break;
            case '4cif':
              $target_size = '704x576';
              break;
            case 'qqvga':
              $target_size = '160x120';
              break;
            case 'qvga':
              $target_size = '320x240';
              break;
            case 'vga':
              $target_size = '640x480';
              break;
            case 'svga':
              $target_size = '800x600';
              break;
            case 'xga':
              $target_size = '1024x768';
              break;
            case 'uxga':
              $target_size = '1600x1200';
              break;
            case 'qxga':
              $target_size = '2048x1536';
              break;
            case 'sxga':
              $target_size = '1280x1024';
              break;
            case 'qsxga':
              $target_size = '2560x2048';
              break;
            case 'hsxga':
              $target_size = '5120x4096';
              break;
            case 'wvga':
              $target_size = '852x480';
              break;
            case 'wxga':
              $target_size = '1366x768';
              break;
            case 'wsxga':
              $target_size = '1600x1024';
              break;
            case 'wuxga':
              $target_size = '1920x1200';
              break;
            case 'woxga':
              $target_size = '2560x1600';
              break;
            case 'wqsxga':
              $target_size = '3200x2048';
              break;
            case 'wquxga':
              $target_size = '3840x2400';
              break;
            case 'whsxga':
              $target_size = '6400x4096';
              break;
            case 'whuxga':
              $target_size = '7680x4800';
              break;
            case 'cga':
              $target_size = '320x200';
              break;
            case 'ega':
              $target_size = '640x350';
              break;
            case 'hd360':
              $target_size = '640x360';
              break;
            case 'hd480':
              $target_size = '852x480';
              break;
            case 'hd720':
              $target_size = '1280x720';
              break;
            case 'hd1080':
              $target_size = '1920x1080';
              break;
            default:
              // FIXME Unspecified sizes ends up here.
              // Check the size.
              preg_match('/(\d+)x(\d+)/',  $target_size, $matches);
              if (!is_array($matches) || count($matches) != 3) {
                // Get the video size.
                $target_size = mediamosa_asset_mediafile::get_size($mediafile_id);

                // If there is a still default size for the client app, then use that.
                $still_default_size = mediamosa_app::get_still_default_value($app_id);
                if ($still_default_size) {
                  $target_size =  $still_default_size;
                }
              }
              break;
          }

          // First get source width and height.
          $a_metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id, array(
            array(
              'prop_name' => mediamosa_asset_mediafile_metadata::WIDTH,
              'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
            ),
            array(
              'prop_name' => mediamosa_asset_mediafile_metadata::HEIGHT,
              'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
            ),
          ));
          $width = $a_metadata[mediamosa_asset_mediafile_metadata::WIDTH];
          $height = $a_metadata[mediamosa_asset_mediafile_metadata::HEIGHT];
          $still_padding = mediamosa_app::get_still_padding_value($app_id);

          // Get the parameter string.
          $cmmd = self::calc_aspect_ratio($width, $height, $target_size, FALSE, $result['h_padding'], $result['v_padding'], $still_padding == mediamosa_app_db::STILL_PADDING_YES);

          // Set result
          if ($cmmd) {
            $result['size'] = $cmmd['width'] . 'x' . $cmmd['height'];
            $result['h_padding'] = $cmmd['h_padding'];
            $result['v_padding'] = $cmmd['v_padding'];
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
          // Something went wrong in the analyse script
          // Fall back to the default values
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
   * @param string $command
   * @param string $mediafile_id
   */
  static public function map_parameters($tool, $command, $mediafile_id) {
    $command_line = array();

    // First determine if we have to alter the aspect ratio.
    $alter_size_param = FALSE;
    if (strstr($command, 'maintain_aspect_ratio:yes') !== FALSE && strstr($command, 'size:') !== FALSE) {
      $alter_size_param = TRUE;
    }

    // Get all parameters based on the toolname that can be queried.
    // Check the given paramters.
    $result = mediamosa_tool_params::get_by_tool($tool);
    foreach ($result as $result_row) {
      if ($result_row[mediamosa_tool_params_db::NICE_PARAMETER] == 'size' && $alter_size_param) {
        $a_parameters = mediamosa_lib::create_named_array($command, ";", ":");
        $target_size = $a_parameters['size'];// We skip the size here.
      }
      else {
        $value = self::mapping_value(
          $result_row[mediamosa_tool_params_db::NICE_PARAMETER],
          $result_row[mediamosa_tool_params_db::TOOL_PARAMETER],
          $result_row[mediamosa_tool_params_db::MIN_VALUE],
          $result_row[mediamosa_tool_params_db::MAX_VALUE],
          $result_row[mediamosa_tool_params_db::ALLOWED_VALUE],
          $result_row[mediamosa_tool_params_db::DEFAULT_VALUE],
          $result_row[mediamosa_tool_params_db::REQUIRED],
          $result_row[mediamosa_tool_params_db::TYPE_PARAMETER],
          $command);

        if ($value != '') {
          $command_line[] = $value;
        }
      }
    }

    // Adjust for aspect ratio? makes only sense if size is set, else we
    // convert to size same as source (default ffmpeg behaviour).
    if ($alter_size_param) {

      // First get source width and height.
      $a_metadata = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id, array(
        array(
          'prop_name' => mediamosa_asset_mediafile_metadata::WIDTH,
          'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
        ),
        array(
          'prop_name' => mediamosa_asset_mediafile_metadata::HEIGHT,
          'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
        ),
      ));

      // Determine if padding necessary or not.
      $is_padding = TRUE;
      if (strstr($command, 'padding:no') !== FALSE) {
        $is_padding = FALSE;
      }

      // Get the parameter string.
      $ratio = self::calc_aspect_ratio($a_metadata[mediamosa_asset_mediafile_metadata::WIDTH], $a_metadata[mediamosa_asset_mediafile_metadata::HEIGHT], $target_size, TRUE, NULL, NULL, $is_padding);
      if (!empty($ratio)) {
        $command_line[] = $ratio;
      }
    }

    return implode(';', $command_line);
  }

  /**
   * Check the mapping and value.
   *
   * @param $nice_parameter
   * @param $tool_parameter
   * @param $min_value
   * @param $max_value
   * @param $allowed_value
   * @param $default_value
   * @param $required
   * @param $type_parameter
   * @param $command
   */
  static public function mapping_value($nice_parameter, $tool_parameter, $min_value, $max_value, $allowed_value, $default_value, $required, $type_parameter, $command) {
    $a_parameters = mediamosa_lib::create_named_array($command, ";", ":");
    $result = "";

    // If the 2 pass encoding parameter doesn't have the value '2', then we skip it. Then we ensure, we have "-pass 2" or nothing.
    if ($nice_parameter == '2_pass_h264_encoding' && isset($a_parameters['2_pass_h264_encoding']) && $a_parameters['2_pass_h264_encoding'] != '2') {
      return $result;
    }

    foreach ($a_parameters as $name => $value) {
      if ($name == $nice_parameter && $tool_parameter != '') {

        $result = sprintf('%s:%s', $tool_parameter, $value);

        // Switch parameter.
        if ($type_parameter == mediamosa_tool_params_db::TYPE_PARAMETER_CHECKBOX) {
          if ($value == mediamosa_tool_params_db::ALLOWED_VALUE_FOR_SWITCH) {
            return $result;
          }
          return '';
        }

        // Validate values.
        if ($min_value != NULL) {
          if ($min_value > $value) {
            return sprintf('%s:%s', $tool_parameter, $default_value);
          }
        }

        if ($max_value != NULL) {
          if ($max_value < $value) {
            return sprintf('%s:%s', $tool_parameter, $default_value);
          }
        }

        if ($allowed_value != NULL) {
          $allowed_values = unserialize($allowed_value);
          if (!(in_array($value, $allowed_values))) {
            return sprintf('%s:%s', $tool_parameter, $default_value);
          }
        }
        // Done all checks.
        return $result;
      }
    }

    // Check if found and not required.
    if ($result == '') {
      if ($required == 'TRUE') {
        return sprintf(';%s:%s', $tool_parameter, $default_value);
      }
    }

    return $result;
  }

  /**
   * Calculate ffmpeg parameters if we have to maintain aspect ratio.
   *
   * @param $w_source
   *   Width of video source.
   * @param $h_source
   *   Height of video source.
   * @param $w_target
   *   Width of video to create.
   * @param $h_target
   *   Height of video to create.
   * @return
   *   Parameter string to use with ffmpeg.
   */
  static public function calc_aspect_ratio($w_source, $h_source, $target_size, $response_string = TRUE, $h_padding = NULL, $v_padding = NULL, $is_padding = TRUE) {

    // Get target width and height, format is 'wxh'.
    preg_match('/(\d+)x(\d+)/',  $target_size, $matches);
    if ((is_array($matches)) && (count($matches) == 3)) {
      $w_target = $matches[1];
      $h_target = $matches[2];
    }
    else {
      return '';
    }

    // if source is unknown, but target is, just return the target size.
    if ((w_target > 0 && $h_target > 0) && (!($w_source > 0 && $h_source > 0))) {
      return(array(
        'width' => $w_target,
        'height' => $h_target,
        'h_padding' => 0,
        'v_padding' => 0,
      ));
    }
    // if source is unknown, and target is unknown, jump out and decide there.
    if (!($w_target > 0 && $h_target > 0 && $w_source > 0 && $h_source > 0)) {
      return '';
    }

    if (isset($h_padding) && is_numeric($h_padding) && $h_padding >= 0 && isset($v_padding) && is_numeric($v_padding) && $v_padding >= 0) {
      if (mediamosa_settings::get_ffmpeg_pad()) {
        $param_string = sprintf("-s:%dx%d;-padtop:%d;-padbottom:%d;-padleft:%d;-padright:%d", (int)$w_target, (int)$h_target, (int)$h_padding, (int)$h_padding, (int)$v_padding, (int)$v_padding);
      }
      else {
        $param_string = sprintf("-s:%dx%d;-vf:'pad=%d:%d:%d:%d:black'", (int)$w_target, (int)$h_target, (int)$w_target + (2 * (int)$v_padding), (int)$h_target + (2 * (int)$h_padding), (int)$v_padding, (int)$h_padding);
      }
      $param_array = array('width' => (int)$w_target, 'height' => (int)$h_target, 'h_padding' => (int)$h_padding, 'v_padding' => $v_padding);
      return ( $response_string ? $param_string : $param_array );
    }

    $w_ratio = $w_source / $w_target;
    $h_ratio = $h_source / $h_target;

    if ($w_ratio > $h_ratio) {
      // Adjust height padding.
      $padding = $h_target - ($h_source / $w_ratio); // Total size of padding.
      $padding = (($padding - ($padding % 4)) /2);   // Single size of padding (must be even).
      $new_height = $h_target - (2 * $padding);
      $new_height = ($new_height - ($new_height % 2)); // must be even.
      if (mediamosa_settings::get_ffmpeg_pad()) {
        $param_string = sprintf("-s:%dx%d;-padtop:%d;-padbottom:%d",
          (int)$w_target,
          (int)$new_height,
          ($is_padding ? (int)$padding : 0),
          ($is_padding ? (int)$padding : 0)
        );
      }
      else {
        $param_string = sprintf("-s:%dx%d;-vf:'pad=%d:%d:%d:%d:black'",
          (int)$w_target,
          (int)$new_height,
          (int)$w_target,
          (int)$new_height + (2 * ($is_padding ? (int)$padding : 0)),
          0,
          ($is_padding ? (int)$padding : 0)
        );
      }
      $param_array = array(
        'width' => (int)$w_target,
        'height' => (int)$new_height,
        'h_padding' => ($is_padding ? (int)$padding : 0),
        'v_padding' => 0
      );
    }
    else {
      // Adjust width padding.
      $padding = $w_target - ($w_source / $h_ratio); // Total size of padding.
      $padding = (($padding - ($padding % 4)) /2);   // Single size of padding (must be even).
      $new_width = $w_target - (2 * $padding);
      $new_width = ($new_width - ($new_width % 2)); // must be even.
      if (mediamosa_settings::get_ffmpeg_pad()) {
        $param_string = sprintf("-s:%dx%d;-padleft:%d;-padright:%d",
          (int)$new_width,
          (int)$h_target,
          ($is_padding ? (int)$padding : 0),
          ($is_padding ? (int)$padding : 0)
        );
      }
      else {
        $param_string = sprintf("-s:%dx%d;-vf:'pad=%d:%d:%d:%d:black'",
          (int)$new_width,
          (int)$h_target,
          (int)$new_width + (2 * ($is_padding ? (int)$padding : 0)),
          (int)$h_target,
          ($is_padding ? (int)$padding : 0),
          0
        );
      }
      $param_array = array(
        'width' => (int)$new_width,
        'height' => (int)$h_target,
        'h_padding' => 0,
        'v_padding' => ($is_padding ? (int)$padding : 0)
      );
    }
    return ( $response_string ? $param_string : $param_array );
  }


  /**
   * Function for starting a delete job
   *
   * 1.x: _vpx_jobhandler_start_server_delete_job
   *
   * @param $job_id
   * @param $mediafile_id
   * @param $testtag
   */
  static public function start_server_delete_job($job_id, $mediafile_id) {

    // Get complete job.
    $a_job_ext = mediamosa_job::get_job_ext($job_id);

    // Delete mediafile.
    try {
      mediamosa_asset_mediafile::delete($mediafile_id);

      // Update status.
      mediamosa_job::update_status($a_job_ext, mediamosa_job_db::JOB_STATUS_FINISHED, '1.000');
    }
    catch (Exception $e) {
      // update with error.
      mediamosa_job::update_status($a_job_ext, mediamosa_job_db::JOB_STATUS_FAILED, '1.000', $e->getMessage());
    }
  }

  /**
   * Remove timed-out transcoding jobs.
   *
   * 1.x: _vpx_jobhandler_check_transcode_timeout.
   */
  static public function check_transcode_timeout() {

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
      $a_job = mediamosa_job::get($result_row[mediamosa_job_db::ID], array(mediamosa_job_db::JOB_STATUS));
      $status = $a_job[mediamosa_job_db::JOB_STATUS];
      mediamosa_job::set_job_status_failed($result_row[mediamosa_job_db::ID], $error_message, $status);

      self::log_mediafile($result_row[mediamosa_job_db::MEDIAFILE_ID], 'Transcode job was deleted because of timeout, job_id: @job_id', array('@job_id' => $result_row[mediamosa_job_db::ID]), $result_row[mediamosa_job_db::ASSET_ID]);
    }
  }

  /**
   * Check Jobs which have no progress. Change status to FAILED.
   *
   * 1.x: _vpx_jobhandler_check_upload_timeout
   */
  static public function check_upload_timeout() {

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
          ':error_description' => strtr('JOB changed timeout expired (#JOB_UPLOAD_TIMEOUTs)', array('#JOB_UPLOAD_TIMEOUT' => mediamosa_settings::JOB_JOB_TIMEOUT))
        )
    );
  }

  /**
   * Check if there are servers on CLOSE and have no jobs, then
   * set them on OFF.
   *
   * 1.x: _vpx_jobhandler_check_server_on_status_close
   */
  static public function check_server_on_status_close() {

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
