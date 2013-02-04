<?php
/**
 * @file
 * Job Still functions.
 */

class mediamosa_job_still {

  /**
   * Get the job still data.
   * @param integer $job_id
   *  The job ID.
   */
  public static function get($job_id, $fields = array()) {
    return mediamosa_db::db_select(mediamosa_job_still_db::TABLE_NAME, 'js')
      ->fields('js', $fields)
      ->condition(mediamosa_job_still_db::JOB_ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Create new still job.
   *
   * @param integer $job_id
   * @param array $a_still_parameters
   */
  public static function create($job_id, array $a_still_parameters) {

    $a_still_parameters += array(
      mediamosa_asset_mediafile_metadata::STILL_TYPE => NULL,
      'still_per_mediafile' => NULL,
      'still_every_second' => NULL,
      'start_frame' => NULL,
      'end_frame' => NULL,
      mediamosa_job_still_db::H_PADDING => NULL,
      mediamosa_job_still_db::V_PADDING => NULL,
      mediamosa_job_still_db::FRAMETIME => mediamosa_settings::STILL_DEFAULT_FRAME_TIME,
    );

    switch ($a_still_parameters[mediamosa_asset_mediafile_metadata::STILL_TYPE]) {
      case mediamosa_asset_mediafile_metadata::STILL_TYPE_NORMAL:
      case mediamosa_asset_mediafile_metadata::STILL_TYPE_SECOND:
      case mediamosa_asset_mediafile_metadata::STILL_TYPE_SCENE:
        $blackstill_check = mediamosa_job_still_db::BLACKSTILL_CHECK_FALSE;
        break;

      default:
        $blackstill_check = mediamosa_job_still_db::BLACKSTILL_CHECK_TRUE;
        break;
    }

    // We need to unset these. (these were not copied?).
    unset($a_still_parameters[mediamosa_asset_mediafile_metadata::WIDTH]);
    unset($a_still_parameters[mediamosa_asset_mediafile_metadata::HEIGHT]);

    // Create the insert object.
    $query = mediamosa_db::db_insert(mediamosa_job_still_db::TABLE_NAME);

    // Insert values.
    $fields = array(
      mediamosa_job_still_db::JOB_ID => $job_id,
      mediamosa_job_still_db::FRAMETIME => $a_still_parameters[mediamosa_job_still_db::FRAMETIME],
      mediamosa_job_still_db::SIZE => $a_still_parameters[mediamosa_job_still_db::SIZE],
      mediamosa_job_still_db::H_PADDING => $a_still_parameters[mediamosa_job_still_db::H_PADDING],
      mediamosa_job_still_db::V_PADDING => $a_still_parameters[mediamosa_job_still_db::V_PADDING],
      mediamosa_job_still_db::BLACKSTILL_CHECK => $blackstill_check,
      mediamosa_job_still_db::STILL_PARAMETERS => serialize($a_still_parameters),
    );

    // Enrich with created/changed.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    return $query->fields($fields)->execute();
  }
}
