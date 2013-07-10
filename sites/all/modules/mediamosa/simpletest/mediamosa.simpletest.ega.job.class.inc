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
  * Simpletest sandbox base for testing job server / processor.
  */

class MediaMosaTestCaseEgaJob extends MediaMosaTestCaseEga {
  // --------------------------------------------------------------------- Vars.

  // -------------------------------------------------------------- Constructor.
  function __construct($test_id = NULL) {
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
   * Create still.
   * Based on REST call /mediafile/$mediafile_id/still/create (POST)
   */
  protected function createMediafileStill($mediafile_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/still/create', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Started still job for mediafile ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
        )
      )
    );

    if ((string)$response['xml']->header->request_result_id != mediamosa_error::ERRORCODE_OKAY) {
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
   * Create transcode.
   * Based on REST call /mediafile/$mediafile_id/transcode (POST)
   */
  protected function createMediafileTranscode($mediafile_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/transcode', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string) $response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        'Started transcode job for mediafile ID: @mediafile_id, got result @result (@result_description)',
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string) $response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
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
   * Based on REST call /mediafile/$mediafile_id/retranscode (POST)
   */
  protected function createMediafileRetranscode($mediafile_id, array $a_parameters = array(), array $a_expected_result_ids = array(mediamosa_error::ERRORCODE_OKAY), $do_internal = FALSE) {
    // Parameters asset.
    $a_parameters += array(
      mediamosa_rest_call_job_create_still_for_mediafile::USER_ID => self::SIMPLETEST_USER_ID,
    );

    // Set the URI.
    $uri = strtr('mediafile/@mediafile_id/retranscode', array('@mediafile_id' => $mediafile_id));

    // Do Post call.
    $response = $this->restCallPost($uri, $a_parameters, array(), $a_expected_result_ids, $do_internal);

    // Check response.
    $this->assertTrue(
      in_array((string)$response['xml']->header->request_result_id, $a_expected_result_ids),
      strtr(
        "Started retranscode job for mediafile ID: @mediafile_id, got result @result (@result_description)",
        array(
          '@mediafile_id' => $mediafile_id,
          '@result' => (string)$response['xml']->header->request_result_id,
          '@result_description' => (string) $response['xml']->header->request_result_description
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

    // Keep in mind that with less slots, max_runs needs to be at least 12.
    $max_runs = 15;
    $running_jobs = 1;
    while ($max_runs-- && $running_jobs) {
      // Run one job cycle.
      self::doQueueCycle();

      // Set flag to know if all jobs are finished or failed.
      $running_jobs = 0;

      // Get all jobs.
      $a_jobs = mediamosa_job::get_job_list(0, '', TRUE);

      // Check if the jobs are all done.
      foreach ($a_jobs as $a_job) {
        if (!in_array($a_job[mediamosa_job_db::JOB_STATUS], array(
            mediamosa_job_db::JOB_STATUS_FINISHED, mediamosa_job_db::JOB_STATUS_FAILED, mediamosa_job_db::JOB_STATUS_CANCELLED))) {
          $running_jobs++;
        }
      }

      if ($running_jobs) {
        $this->pass(strtr('Found @running running jobs, waiting for next cycle.', array('@running' => $running_jobs)));
        // Wait 5 seconds.
        sleep(5);
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
