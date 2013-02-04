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
   * @deprecated: Although new, its deprecated. We will replace this function
   *              with generate_still() very soon and no longer rely on exec
   *              strings return. Rewrite of jobs is planned for v3.5.
   *
   * @param string $jobserver_job_id
   * @param string $mediafile_id_source
   */
  public static function get_generate_still_exec($jobserver_job_id, $mediafile_id_source) {

    $job_server_still = mediamosa_job_server_still::get($jobserver_job_id);
    $job_server = mediamosa_job_server::get($jobserver_job_id);
    $job_id = $job_server[mediamosa_job_server_db::JOB_ID];

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

      $options = array(
        '@mediafile_dest' => $mediafile_dest,
        '@mediafile_source' => $mediafile_source,
        '@parameter_string' => implode(' ', $parameter_string),
        '@status_file' => mediamosa_storage::get_realpath_status_file($job_id),
      );

      return strtr('convert @mediafile_source @parameter_string @mediafile_dest; { echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file', $options);
    }

    return '';
  }

  /**
   * Get the exec string for transcoding the file.
   *
   * @deprecated: Although new, its deprecated. We will replace this function
   *              with transcode() very soon and no longer rely on exec
   *              strings return. Rewrite of jobs is planned for v3.5.
   *
   * @param array $options
   */
  public static function get_transcode_exec($options) {

    // Build the values.
    $execution_string_options = array(
      '@file_location' => dirname($options['location_dest_file']) . DIRECTORY_SEPARATOR . $options['job_id'] . DIRECTORY_SEPARATOR,
      '@mediafile_location' => $options['location_source_file'],
      '@mediafile_id' => $options['mediafile_id'],
      '@mediafile_dest' => $options['job_id'],
      '@parameter_string' => $options['parameter_string'],
      '@status_file' => $options['status_file'],
    );

    // Execution string.
    // Go to the subdirectory.
    // Create jpg from image (eg. from tiff).
    // Copy the result to transcode directory.
    // Remove the temporary directory (and files).
    // Create status file.
    return strtr('mkdir @file_location; cd @file_location; convert @mediafile_location @parameter_string @mediafile_id.jpg; cp @mediafile_id.jpg ../@mediafile_dest.jpg; cd ..; rm @file_location -r; { echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;', $execution_string_options);
  }
}
