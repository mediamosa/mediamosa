<?php
/**
 * @file
 * Jobserver still functions.
 */

class mediamosa_job_server_still {

  static function get($jobserver_id, $fields = array()) {

    return mediamosa_db::db_select(mediamosa_job_server_still_db::TABLE_NAME, 'jss')
      ->fields('jss', $fields)
      ->condition(mediamosa_job_server_still_db::ID, $jobserver_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Black still check.
   *
   * @param integer $job_id
   * @param string $base_filename
   */
  public static function still_validate($job_id, $base_filename) {
    // Retrieve the job settings
    $result_row = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_job_server_still} AS jsj
       LEFT JOIN {#mediamosa_job_server} AS jj ON jj.jobserver_job_id = jsj.jobserver_job_id
       WHERE jj.job_id = :job_id',
      array(
        '#mediamosa_job_server_still' => mediamosa_job_server_still_db::TABLE_NAME,
        '#mediamosa_job_server' => mediamosa_job_server_db::TABLE_NAME,
        ':job_id' => $job_id
      )
    )->fetchAssoc();

    if (!$result_row) {
      return;
    }

    $blackstill_check = $result_row[mediamosa_job_server_still_db::BLACKSTILL_CHECK];
    if ($blackstill_check == mediamosa_job_server_still_db::BLACKSTILL_CHECK_FALSE) {
      return;
    }

    // Check the generated stills
    $i = 1;
    $pixel_rgb = NULL;
    $destination = mediamosa_storage::get_realpath_data_transcode_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg');
    while (mediamosa_io::file_exists(mediamosa_storage::get_realpath_data_transcode_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg'))) {
      $source = mediamosa_storage::get_realpath_data_transcode_file($base_filename . sprintf(mediamosa_settings::STILL_EXTENSION, $i) . '.jpeg');
      $im = mediamosa_gd::imagecreatefromjpeg($source);

      if ($im) {
        // Calculate the color variation.
        $size = mediamosa_gd::get_image_size($source);
        $rgb = mediamosa_gd::imagecolorat($im, ($size[0] >> 1), ($size[1] >> 1));
        $r = ($rgb >> 16) & 0xFF;
        $g = ($rgb >> 8) & 0xFF;
        $b = $rgb & 0xFF;
        $rgb = mediamosa_gd::imagecolorat($im, ($size[0] >> 2), ($size[1] >> 2));
        $r2 = ($rgb >> 16) & 0xFF;
        $g2 = ($rgb >> 8) & 0xFF;
        $b2 = $rgb & 0xFF;
        $pixel_rgb_new = abs($r - $r2) + abs($g - $g2) + abs($b - $b2);

        if (!$pixel_rgb || $pixel_rgb < $pixel_rgb_new) {
          $pixel_rgb = $pixel_rgb_new;
          // Is it not the first image?
          if ($i != 1) {
            mediamosa_io::copy($source, $destination);
          }
        }

        // Is it not the first image?
        if ($i != 1) {
          mediamosa_io::unlink($source);
        }
      }

      $i++;
    }
  }
}
