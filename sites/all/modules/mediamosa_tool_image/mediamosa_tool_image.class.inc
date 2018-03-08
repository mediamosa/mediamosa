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
   *   message to log.
   * @param array $variables
   *   variables to be used in the log message.
   * @param string $severity
   *   see WATCHDOG_* severity.
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'tool image');
  }

  /**
   * Returns TRUE when we support this mime-type.
   *
   * @param string $mime_type
   *   mimetype request.
   */
  public static function is_supported($mime_type) {
    return preg_match('#^image\/.+#', $mime_type);
  }

  /**
   * Generate still.
   *
   * @param string $job_info
   *   id of jobserver.
   * @param string $mediafile_id_source
   *   mediafile_id of source.
   */
  public static function get_generate_still_exec($job_info, $mediafile_id_source) {

    $parameter_string = array();
    if ($job_info['job_id']) {

      // WxH.
      $size = $job_info[mediamosa_job_server_still_db::SIZE];

      // Resize.
      if ($size != '') {
        $parameter_string[] = '-resize ' . $size;
      }

      // Source.
      $mediafile_source = mediamosa_storage::get_realpath_mediafile($mediafile_id_source);

      // Select the first image frame in a multi-image file format such as tiff.
      // This is a no-op for single-image file formats.
      $mediafile_source .= escapeshellarg('[0]');

      // Dest.
      $mediafile_dest = mediamosa_storage::get_realpath_temporary_file($job_info['job_id'] . sprintf(mediamosa_settings::STILL_EXTENSION, 1) . '.jpeg');

      $execute_string_options = array(
        '@mediafile_dest' => $mediafile_dest,
        '@mediafile_source' => $mediafile_source,
        '@parameter_string' => implode(' ', $parameter_string),
        '@status_file' => mediamosa_storage::get_realpath_status_file($job_info['job_id']),
        '@working_dir' => dirname($mediafile_dest),
      );

      // Execution string:
      return strtr(
        '{ echo "Errors: none"; echo "Status: converting"; echo "Progress: 0.000"; echo "Warnings: none"; } > @status_file;' .
        'export MAGICK_TMPDIR=@working_dir;nice convert -limit memory 512MiB -limit map 512MiB @mediafile_source @parameter_string @mediafile_dest; ' .
        '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
        $execute_string_options);
    }
  }

  /**
   * Get the exec string for transcoding the file.
   *
   * @param array $options
   *   Options array for transcode.
   */
  public static function get_transcode_exec($options) {

    $parameter_string = $options['parameter_string'];

    // Convert a fictive "-source-frame-range" parameter to an actual frame range selection.
    $optional_source_frame_range = '';

    $parameters = explode(' ',$options['parameter_string']);
    $range_index = array_search('-source-frame-range',$parameters);

    if ($range_index !== False) {
      list(,$range_value) = array_splice($parameters,$range_index,2);

      $optional_source_frame_range = escapeshellarg('[' . $range_value . ']');

      $parameter_string = implode(' ',$parameters);
    }

    // Build the values.
    $execution_string_options = array(
      '@mediafile_source' => $options['location_source_file'] . $optional_source_frame_range,
      '@mediafile_dest' => $options['location_dest_file'] . '.' . $options['file_extension'],
      '@parameter_string' => $parameter_string,
      '@status_file' => $options['status_file'],
      '@working_dir' => dirname($options['location_dest_file']),
    );

    // Execution string.
    return strtr(
      '{ echo "Errors: none"; echo "Status: converting"; echo "Progress: 0.000"; echo "Warnings: none"; } > @status_file;' .
      'export MAGICK_TMPDIR=@working_dir;nice convert -limit memory 512MiB -limit map 512MiB @mediafile_source @parameter_string @mediafile_dest; ' .
      '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
      $execution_string_options);
  }

  /**
   * Hook perform post transcode action.
   */
  public static function post_transcode($asset_id, $mediafile_id, $mediafile_id_dest, $file_ext) {
  }
}
