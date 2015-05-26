<?php
/**
 * @file
 * PDF tool functions.
 */

class mediamosa_tool_pdf {

  /**
   * Mime type: application/pdf.
   */
  const MEDIAMOSA_TOOL_PDF_MIME_TYPE_PDF = 'application/pdf';

  /**
   * Short id of the module.
   */
  const NAME = 'pdf';

  /**
   * Key value separator.
   */
  const MEDIAMOSA_TOOL_PDF_KEY_VALUE_SEPARATOR = ':';

  /**
   * Warning level for pdfinfo.
   */
  const MEDIAMOSA_MAINTENANCE_STATUS_CONFIGURATION_PDFINFO_WARNING = '0';

  /**
   * Is this my supported mime type.
   *
   * @param string $mime_type
   *   mime_type
   */
  public static function is_supported($mime_type) {
    return in_array($mime_type, array(self::MEDIAMOSA_TOOL_PDF_MIME_TYPE_PDF));
  }

  /**
   * Generate still.
   *
   * @param string $jobserver_job_id
   *   id of jobserver.
   * @param string $mediafile_id_source
   *   mediafile_id of source.
   */
  public static function get_generate_still_exec($jobserver_job_id, $mediafile_id_source) {

    $job_server_still = mediamosa_job_server_still::get($jobserver_job_id);
    $job_server = mediamosa_job_server::get($jobserver_job_id);
    $job_id = $job_server[mediamosa_job_server_db::JOB_ID];

    $parameter_string = array();
    if ($job_server_still) {

      // WxH.
      $size = $job_server_still[mediamosa_job_server_still_db::SIZE];

      // Resize. The resize is done separately as combining them in 1
      // convert call produces black images.
      if ($size != '') {
        $resize_option = 'nice convert -resize ' . $size . ' @mediafile_dest @mediafile_dest;';
      }

      // Source.
      $mediafile_source = mediamosa_storage::get_realpath_mediafile($mediafile_id_source);

      // Dest.
      $mediafile_dest = mediamosa_storage::get_realpath_temporary_file($job_id . '-001.jpeg');

      $execute_string_options = array(
        '@mediafile_dest' => $mediafile_dest,
        // Add [0] for first page.
        '@mediafile_source' => $mediafile_source . '[0]',
        '@parameter_string' => implode(' ', $parameter_string),
        '@status_file' => mediamosa_storage::get_realpath_status_file($job_id),
        '@working_dir' => dirname($mediafile_dest),
      );

      // Execution string:
      return strtr(
        '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 0.000"; echo "Warnings: none"; } > @status_file;' .
        'export MAGICK_TMPDIR=@working_dir;nice convert @mediafile_source @parameter_string @mediafile_dest; ' .
        $resize_option .
        '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
        $execute_string_options);
    }
  }

  /**
   * Get the exec string for transcoding the file.
   *
   * @deprecated: Although new, its deprecated. We will replace this function
   *              with transcode() very soon and no longer rely on exec
   *              strings return. Rewrite of jobs is planned for v3.5.
   *
   * @param array $options
   *   array of conversion options.
   */
  public static function get_transcode_exec($options) {

    if ($options['file_extension'] == 'swf') {
      // Build the values.
      $execution_string_options = array(
        '@file_location' => dirname($options['location_dest_file']) . DIRECTORY_SEPARATOR . $options['jobserver_job_id'] . DIRECTORY_SEPARATOR,
        '@mediafile_location' => $options['location_source_file'],
        '@mediafile_id' => $options['mediafile_id'],
        '@params' => $options['parameter_string'],
        '@status_file' => $options['status_file'],
      );

      // Build the execute_string.
      return strtr('mkdir @file_location; cd @file_location; pdf2swf @mediafile_location -o @mediafile_id @params; cp @mediafile_id ../@mediafile_id.swf; cd ..; rm -r @file_location; { echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;', $execution_string_options);
    }
    else {

      $params = explode(' ', $options['parameter_string']);
      $mf_ids = unserialize(urldecode($params[1]));
      $files_to_pdf = array();
      foreach (array_keys($mf_ids) as $mf_id) {
        $files_to_pdf[] = mediamosa_storage::get_realpath_mediafile($mf_id);
      }

      $mfs = implode(' ', $files_to_pdf);

      // Build the values.
      $execution_string_options = array(
        '@source_files' => $mfs,
        '@mediafile_dest' => $options['location_dest_file'] . '.' . $options['file_extension'],
        '@status_file' => $options['status_file'],
      );

      // Execution string.
      // MAGICK_TEMPORY_PATH: use current dir for temp files instead
      // of /tmp.
      // -resize: only resize if image is > 8000pts.
      $cmd = strtr(
        '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 0.000"; echo "Warnings: none"; } > @status_file;' .
        'export MAGICK_TEMPORARY_PATH=.; nice convert -quality 95 @source_files -resize 8000x8000\> pdfa:@mediafile_dest; '
        . '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
        $execution_string_options);

      return $cmd;
    }

    return FALSE;
  }

  /**
   * Implements hook_post_convert().
   */
  public static function post_convert($job_id, $mediafile_id_dest) {

    // Get job info.
    $job_info = mediamosa_job::get_job_ext($job_id);

    $command = mediamosa_transcode_profile::commandToArray($job_info['command']);
    $pdffilename = $command['pdffilename'];
    $replace = $command['replace'];

    // Make zipfilename safe.
    $pdffilename = preg_replace("([^\w\s\d\-_~,;:\[\]\(\).])", '', $pdffilename);
    $pdffilename = preg_replace("([\.]{2,})", '', $pdffilename);

    // Get profile_id.
    $transcode_profile = mediamosa_transcode_profile::get_by_machine_name('PDF');

    mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields(array(
        mediamosa_asset_mediafile_db::IS_DOWNLOADABLE => mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_TRUE,
        mediamosa_asset_mediafile_db::FILENAME => isset($pdffilename) ? $pdffilename : 'pdf.pdf',
        mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID => $transcode_profile['transcode_profile_id'],
        mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
      ))
      ->condition(mediamosa_asset_mediafile_db::ID, $mediafile_id_dest)
      ->execute();

    // If replace == true, remove all other mediafiles with tool=pdf
    // in this asset.
    if ($replace) {
      $mfs = mediamosa_asset_mediafile::get_by_asset_id($job_info['asset_id'], array(mediamosa_asset_mediafile_db::ID), array('tool' => self::NAME));

      // Delete all mediafiles with tool pdf except mediafile_id_dest.
      unset($mfs[$mediafile_id_dest]);
      mediamosa_asset_mediafile::delete($mfs);
    }
  }
}
