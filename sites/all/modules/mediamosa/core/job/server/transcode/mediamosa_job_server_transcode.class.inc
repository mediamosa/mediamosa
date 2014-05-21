<?php
/**
 * @file
 * Function for job_server_transcode.
 */

class mediamosa_job_server_transcode {
  // ---------------------------------------------------------------- Functions.
  /**
   * Get the extra jobserver data for transcode jobs.
   *
   * @param int $jobserver_id
   *   The job server ID.
   * @param array $fields
   *   Possible selection of fields.
   *
   * @return array
   *   The extra job info.
   */
  static function get($jobserver_id, $fields = array()) {

    return mediamosa_db::db_select(mediamosa_job_server_transcode_db::TABLE_NAME, 'jst')
      ->fields('jst', $fields)
      ->condition(mediamosa_job_server_transcode_db::ID, $jobserver_id)
      ->execute()
      ->fetchAssoc();
  }
}
