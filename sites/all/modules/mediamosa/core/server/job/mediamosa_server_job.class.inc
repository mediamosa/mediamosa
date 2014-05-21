<?php
/**
 * @file
 * The server job class.
 */

class mediamosa_server_job {
  // ---------------------------------------------------------------- Functions.
  /**
   * Delete the job-server-relation of the job.
   *
   * Remove server<->job link.
   *
   * @param int $job_id
   *   The Job ID.
   */
  public static function delete_by_jobid($job_id) {
    // Remove link.
    mediamosa_db::db_delete(mediamosa_server_job_db::TABLE_NAME)
      ->condition(mediamosa_server_job_db::JOB_ID, $job_id)
      ->execute();
  }
}
