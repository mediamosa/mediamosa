<?php
/**
 * @file
 * REST calls for Job execute.
 */

/**
 * URI: /job/$job_id/execute
 * Execute the job.
 *
 * Internal only.
 */
class mediamosa_rest_call_server_job_execute extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const JOB_ID = 'job_id';
  const JOB_TYPE = 'job_type';
  const MEDIAFILE_ID = 'mediafile_id';
  const JOB_DATA = 'job_data';

  // ------------------------------------------------------------------ Methods.
  /**
   * Implements get_var_setup().
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The job ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_JOB,
          self::VAR_DESCRIPTION => 'The job type.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The mediafile ID used during the job.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_DATA => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "The job's data it needs to execute the job.",
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  /**
   * Implements do_call().
   */
  public function do_call() {
    // Get the mediamosa object.
    $mediamosa = mediamosa::get();

    // Get params.
    $job_id = $this->get_param_value(self::JOB_ID);
    $job_type = $this->get_param_value(self::JOB_TYPE);
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $job_data = unserialize($this->get_param_value(self::JOB_DATA));

    // Execute the job.
    mediamosa_job_server::job_execute($job_id, $job_type, $mediafile_id, $job_data);

    // Done.
    $mediamosa->set_result_okay();
  }
}
