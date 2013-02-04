<?php
/**
 * @file
 * Job Display functions.
 */

class mediamosa_job_display {

  /**
   * Get the job display data.
   * @param integer $job_id
   *  The job ID.
   */
  public static function get($job_id, $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_display_db::TABLE_NAME, 'js')
      ->fields('js', $fields)
      ->condition(mediamosa_job_display_db::ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Set hide attribute.
   *
   * @param $job_id
   */
  public static function hide($job_id) {
    // Hide all the jobs with the same mediafile id.
    $fields = array(
      mediamosa_job_db::MEDIAFILE_ID,
      mediamosa_job_db::JOB_TYPE,
    );
    $job = mediamosa_job::get($job_id, $fields);

    $fields = array(
      mediamosa_job_db::ID,
      mediamosa_job_db::JOB_TYPE,
    );
    $result = mediamosa_job::get_by_mediafileid_all($job[mediamosa_job_db::MEDIAFILE_ID], $fields);

    foreach ($result as $record) {
      if ($job[mediamosa_job_db::JOB_TYPE] == $record[mediamosa_job_db::JOB_TYPE] && !self::get($record[mediamosa_job_db::ID])) {
        $query = mediamosa_db::db_insert(mediamosa_job_display_db::TABLE_NAME);
        $query->fields(array(
          mediamosa_job_display_db::ID => $record[mediamosa_job_db::ID],
          mediamosa_job_display_db::VISIBILITY => mediamosa_job_display_db::VISIBILITY_HIDDEN,
        ));
        $query->execute();
      }
    }
  }

  /**
   * Set unhide attribute.
   *
   * @param $job_id
   */
  public static function unhide($job_id) {
    // Unhide all the jobs with the same mediafile id.
    $fields = array(
      mediamosa_job_db::MEDIAFILE_ID,
      mediamosa_job_db::JOB_TYPE,
    );
    $job = mediamosa_job::get($job_id, $fields);

    $fields = array(
      mediamosa_job_db::ID,
      mediamosa_job_db::JOB_TYPE,
    );
    $result = mediamosa_job::get_by_mediafileid_all($job[mediamosa_job_db::MEDIAFILE_ID], $fields);
    foreach ($result as $record) {
      if ($job[mediamosa_job_db::JOB_TYPE] == $record[mediamosa_job_db::JOB_TYPE]) {
        mediamosa_db::db_delete(mediamosa_job_display_db::TABLE_NAME)
          ->condition(mediamosa_job_display_db::ID, $record[mediamosa_job_db::ID])
          ->execute();
      }
    }
  }
}
