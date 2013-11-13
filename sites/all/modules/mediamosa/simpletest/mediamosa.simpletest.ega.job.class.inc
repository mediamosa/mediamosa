<?php

class MediaMosaTestCaseEgaJob extends MediaMosaTestCaseEga {
  // -------------------------------------------------------------- Constructor.
  /**
   * Implement contructor.
   */
  public function __construct($test_id = NULL) {
    parent::__construct($test_id);
    $this->skipClasses[__CLASS__] = TRUE;
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Get from outside sandbox.
    $mediamosa_jobscheduler_uri = variable_get('mediamosa_jobscheduler_uri', NULL);

    // Run parent first so we are inside sandbox.
    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array('mediamosa_job', 'mediamosa_tool_ffmpeg'), $args));
    if (drupal_substr(phpversion(), 0, 3) < '5.3') {
      call_user_func_array(array($this, 'parent::setUp'), $args);
    }
    else {
      call_user_func_array('parent::setUp', $args);
    }

    // Set jobserver selection.
    if (!empty($mediamosa_jobscheduler_uri)) {
      variable_set('mediamosa_jobscheduler_uri', $mediamosa_jobscheduler_uri);
    }
  }

  /**
   * Implements tearDown().
   */
  protected function tearDown() {

    // Dump job table contents.
    $this->dump_jobs();

    // Call parent.
    parent::tearDown();
  }

  /**
   * Dump the log so we know what happened.
   */
  protected function dump_jobs() {
    // Dump the log.
    $result = db_query('SELECT * FROM {mediamosa_job}');

    $texts = array('Dumping contents of job table');

    foreach ($result as $log_item) {
      $texts[] = highlight_string('<?php ' . var_export($log_item, TRUE), TRUE);
    }

    $result = db_query('SELECT * FROM {mediamosa_server_job}');
    if ($result->rowCount()) {
      $texts[] = 'Dumping contents of job server table';
      foreach ($result as $log_item) {
        $texts[] = highlight_string('<?php ' . var_export($log_item, TRUE), TRUE);
      }
    }

    // In verbose mode only.
    $this->verbose(implode('<br />', $texts), 'job dump');
  }

  /**
   * Create a transcode job.
   */
  protected function createTranscodeJob($job_id, $fields = array()) {
    $fields[mediamosa_job_transcode_db::JOB_ID] = $job_id;

    $fields += array(
      mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID => $this->getDefaultProfileId($this->a_app['app_id']),
      mediamosa_job_transcode_db::TOOL => '',
      mediamosa_job_transcode_db::COMMAND => '',
      mediamosa_job_transcode_db::FILE_EXTENSION => '',
      mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL => '',
    );

    $fields = mediamosa_db::db_insert_enrich($fields);

    $fields[mediamosa_job_transcode_db::ID] = mediamosa_db::db_insert(mediamosa_job_transcode_db::TABLE_NAME)->fields($fields)->execute();
    return $fields[mediamosa_job_transcode_db::ID];
  }

  /**
   * Get the information about a job using REST.
   *
   * Uses REST /job/$job_id [GET].
   *
   * @param integer $job_id
   *   The Job ID.
   *
   * @return mediamosa_connector_response
   *   The mediamosa response class.
   */
  protected function getJob($job_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('job/@job_id/status', array('@job_id' => $job_id));

    // Do Post call.
    $response = $this->restCallGet($uri, $parameters, array(), $expected_result_ids, $do_internal);

    $this->var_export($response);

    return $response['xml'];
  }

  /**
   * Basic upload of test file.
   *
   * @param $options
   *   Options to setup the upload.
   *   - filename (optional)
   *     Filename of the file to upload. Will default to ogg test file.
   *   - asset_id (optional)
   *     The asset to use for the upload.
   *   - use_put (FALSE)
   *     Use PUT as method to test.
   *   - downloadable (FALSE)
   *     Make mediafile downloadable.
   *   - create_still (FALSE)
   *     Create still after upload.
   *   - transcode_inherits_acl (FALSE)
   *     Make transcodes for mediafile same ACL rights as original.
   *   - transcode (empty array)
   *     Array of transcode profile ids to use to transcode the uploaded file.
   *
   * @see MediaMosaTestCaseEga::uploadTestFile()
   */
  protected function uploadTestFile($options = array()) {

    // Call parent.
    $result = parent::uploadTestFile($options);

      // Any options need job processing?
    if (!empty($options['create_still']) || !empty($options['transcode'])) {
      // Parse the queue.
      $this->doQueueCycleAll();
    }

    return $result;
  }

  /**
   * Create still.
   *
   * Based on REST call /mediafile/$mediafile_id/still/create (POST)
   */
  protected function createMediafileStill($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/still/create', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Started still job for mediafile ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description,
        )
      )
    );

    if ((string) $response['xml']->header->request_result_id != mediamosa_error::ERRORCODE_OKAY) {
      return FALSE;
    }

    // Get the job_id.
    $job_id = $response['xml']->items->item[0]->job_id;

    // Must have job id.
    $this->assertTrue($job_id, strtr('Got valid job ID @job_id.', array('@job_id' => $job_id)));

    // Return the job_id.
    return array('job_id' => $job_id);
  }

  /**
   * Return the most default transcode profile.
   */
  protected function getDefaultTranscodeProfile() {

    $profile = mediamosa_transcode_profile::get_default(0);
    if (isset($profile['transcode_profile_id'])) {
      return $profile['transcode_profile_id'];
    }
    return FALSE;
  }

  /**
   * Create transcode.
   *
   * Based on REST call /mediafile/$mediafile_id/transcode (POST)
   */
  protected function createMediafileAnalyse($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/analyse', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        'Started transcode job for mediafile ID: @mediafile_id, got result @result (@result_description)',
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description,
        )
      )
    );

    // Get the job_id.
    $job_id = $response['xml']->items->item[0]->job_id;

    // Must have job id.
    $this->assertTrue($job_id, strtr('Got valid job ID @job_id.', array('@job_id' => $job_id)));

    // Return the job_id.
    return array('job_id' => $job_id);
  }

  /**
   * Create transcode.
   *
   * Based on REST call /mediafile/$mediafile_id/transcode (POST)
   */
  protected function createMediafileTranscode($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/transcode', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        'Started transcode job for mediafile ID: @mediafile_id, got result @result (@result_description)',
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description,
        )
      )
    );

    // Get the job_id.
    $job_id = $response['xml']->items->item[0]->job_id;

    // Must have job id.
    $this->assertTrue($job_id, strtr('Got valid job ID @job_id.', array('@job_id' => $job_id)));

    // Return the job_id.
    return array('job_id' => $job_id);
  }

  /**
   * Create retranscode.
   *
   * Based on REST call /mediafile/$mediafile_id/retranscode (POST)
   */
  protected function createMediafileRetranscode($mediafile_id, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/retranscode', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        "Started retranscode job for mediafile ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description,
        )
      )
    );

    // Get the job_id.
    $job_id = $response['xml']->items->item[0]->job_id;

    // Must have job id.
    $this->assertTrue($job_id, strtr('Got valid job ID @job_id.', array('@job_id' => $job_id)));

    // Return the job_id.
    return array('job_id' => $job_id);
  }

  /**
   * Move a mediafile to another location.
   *
   * Based on REST call /storage/$mediafile_id/move (POST)
   */
  protected function doMediafileMove($mediafile_id, $storage_profile_id, $path, array $parameters = array(), array $expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $parameters += array(
      mediamosa_rest_call_storage_move::USER_ID => self::SIMPLETEST_USER_ID,
    );
    $parameters[mediamosa_rest_call_storage_move::STORAGE_PROFILE_ID] = $storage_profile_id;
    $parameters[mediamosa_rest_call_storage_move::PATH] = $path;

    // Set the URI.
    $uri = strtr('storage/@mediafile_id/move', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $parameters, array(), $expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $expected_result_ids),
      strtr(
        'Started storage move job for mediafile ID: @mediafile_id, got result @result (@result_description)',
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description,
        )
      )
    );

    // Get the job_id.
    $job_id = $response['xml']->items->item[0]->job_id;

    // Must have job id.
    $this->assertTrue($job_id, strtr('Got valid job ID @job_id.', array('@job_id' => $job_id)));

    // Return the job_id.
    return $job_id;
  }


  /**
   * Run and execute the current jobs (one cycle).
   */
  protected function doQueueCycle() {

    try {
      // Trigger process servers.
      mediamosa_server::trigger();

      // Trigger job scheduler.
      mediamosa_job_scheduler::trigger();
    }
    catch (Exception $e) {
      $this->var_export_fail('Exception caught; ' . $e->getMessage() . ', trace; ' . $e->getTraceAsString(), 'Job Cycle');
      throw $e;
    }
  }

  /**
   * Run and execute the current jobs (one cycle).
   */
  protected function doQueueCycleAll() {

    // Keep in mind that with less slots, this needs to be 18.
    $max_runs = 20;
    $running_jobs = 1;
    while ($max_runs-- && $running_jobs) {
      // Run one job cycle.
      self::doQueueCycle();

      // Set flag to know if all jobs are finished or failed.
      $running_jobs = 0;

      // Get all jobs.
      $jobs = mediamosa_job::get_job_list(0, '', TRUE);

      // Check if the jobs are all done.
      foreach ($jobs as $job) {
        if (!in_array($job[mediamosa_job_db::JOB_STATUS],
          array(
            mediamosa_job_db::JOB_STATUS_FINISHED,
            mediamosa_job_db::JOB_STATUS_FAILED,
            mediamosa_job_db::JOB_STATUS_CANCELLED,
          )
        )) {
          $running_jobs++;
        }
      }

      if ($running_jobs) {
        $this->pass(strtr('Found @running running jobs, waiting for next cycle.', array('@running' => $running_jobs)));
        // Wait 5 seconds.
        Sleep(5);
      }
      else {
        $this->pass('No more running jobs found, exiting job cycle.');
      }
    }

    // If we are dead locked, we need to know.
    if ($max_runs < 0) {
      $this->fail(strtr('Job scheduler deadlock; Failed to complete all jobs. Jobs waiting for: @running', array('@running' => $running_jobs)));
    }
  }
}
