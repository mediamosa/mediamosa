<?php
/**
 * @file
 * Image tool functions.
 */

class mediamosa_tool_image {
  /**
   * Mime type: image/tiff
   */
  const MEDIAMOSA_TOOL_IMAGE_MIME_TYPE_TIFF = 'image/tiff';

  /**
   * Name / ID of the module.
   */
  const NAME = 'image';

  /**
   * Key value separator.
   */
  const MEDIAMOSA_TOOL_IMAGE_KEY_VALUE_SEPARATOR = ':';

  /**
   * Warning level for ImageMagic.
   */
  const MEDIAMOSA_MAINTENANCE_STATUS_CONFIGURATION_IMAGEMAGIC_WARNING = '6';

  /**
   * Log for image tool.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'tool image');
  }

  /**
   * Returns TRUE when we support this mime-type.
   *
   * @param string $mime_type
   */
  public static function is_supported($mime_type) {
    return preg_match('#^image\/.+#', $mime_type);
  }

  /**
   * Generate still.
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id_source
   */
  public static function get_generate_still_exec($jobserver_job_id, $mediafile_id_source) {

    $job_server_still = mediamosa_job_server_still::get($jobserver_job_id);
    $job_server = mediamosa_job_server::get($jobserver_job_id);
    $job_id = $job_server[mediamosa_job_server_db::JOB_ID];

    $execution_string = '';

    if ($job_server_still) {
      $size = $job_server_still[mediamosa_job_server_still_db::SIZE]; // wxh
      $h_padding = $job_server_still[mediamosa_job_server_still_db::H_PADDING];
      $v_padding = $job_server_still[mediamosa_job_server_still_db::V_PADDING];

      // Resize.
      $parameter_string[] = '-resize ' . $size;

      // Source.
      $mediafile_source = mediamosa_storage::get_realpath_mediafile($mediafile_id_source);

      // Dest.
      $mediafile_dest = mediamosa_storage::get_realpath_temporary_file($mediafile_id_source . sprintf(mediamosa_settings::STILL_EXTENSION, 1) . '.jpeg');

      $execute_string_options = array(
        '@mediafile_dest' => $mediafile_dest,
        '@mediafile_source' => $mediafile_source,
        '@parameter_string' => implode(' ', $parameter_string),
        '@status_file' => mediamosa_storage::get_realpath_status_file($job_id),
      );

    // Execution string:
    return strtr(
      '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 0.000"; echo "Warnings: none"; } > @status_file;' .
      'export MAGICK_TMPDIR=.;nice convert @mediafile_source @parameter_string @mediafile_dest; ' .
      '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
      $execution_string_options);
    }
  }

  /**
   * Get the exec string for transcoding the file.
   *
   * @param array $options
   */
  public static function get_transcode_exec($options) {

    // Build the values.
    $execution_string_options = array(
      '@mediafile_source' => $options['location_source_file'],
      '@mediafile_dest' => $options['location_dest_file'] . '.' . $options['file_extension'],
      '@parameter_string' => $options['parameter_string'],
      '@status_file' => $options['status_file'],
    );

    // Execution string.
    return strtr(
      '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 0.000"; echo "Warnings: none"; } > @status_file;' .
      'export MAGICK_TMPDIR=.;nice convert @mediafile_source @parameter_string @mediafile_dest; ' .
      '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
      $execution_string_options);
  }

  /**
   * Hook perform post transcode action.
   *
   */
  public static function post_transcode($asset_id, $mediafile_id, $mediafile_id_dest, $file_ext) {

  }
}
