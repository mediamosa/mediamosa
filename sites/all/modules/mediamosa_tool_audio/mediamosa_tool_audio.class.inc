<?php
/**
 * @file
 * Audio tool functions.
 */

class mediamosa_tool_audio extends mediamosa_tool_ffmpeg {
  // ------------------------------------------------------------------- Consts.
  /**
   * Name / ID of the module.
   */
  const NAME = 'audio';

  /**
   * Returns TRUE when we support this mime-type.
   *
   * @param string $mime_type
   *   mimetype request.
   */
  public static function is_supported($mime_type) {
    return preg_match('#^audio\/.+#', $mime_type);
  }

  /**
   * Get the exec string for generating a still.
   *
   * @param string $job_info
   *   all info about job.
   * @param string $mediafile_id_source
   *   id of mediafile.
   */
  public static function get_generate_still_exec($job_info, $mediafile_id_source) {

    // Refinement:
    // http://stackoverflow.com/questions/2381243/how-does-soundcloud-com-generate-the-waveform-for-their-mp3-player.
    // http://stackoverflow.com/questions/9956815/generate-visual-waveform-from-mp3-wav-file-in-windows-2008-server.
    // http://stackoverflow.com/questions/1215326/open-source-c-sharp-code-to-present-wave-form.
    // For now we just use the image in /img: audio.png

    $parameter_string = array();
    if ($job_info['job_id']) {

      // WxH.
      $size = $job_info[mediamosa_job_server_still_db::SIZE];

      // Resize.
      if ($size != '') {
        $convert_command = 'nice convert -resize ' . $size . ' @mediafile_dest @mediafile_dest;';
      }
      else {
        $convert_command = 'nice convert @mediafile_dest @mediafile_dest;';
      }

      // Source.
      $mediafile_source = drupal_get_path('module', 'mediamosa_tool_audio') . '/img/audio.png';

      // Dest.
      $mediafile_dest = mediamosa_storage::get_realpath_temporary_file($job_info['job_id'] . '-001.jpeg');

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
        'export MAGICK_TMPDIR=@working_dir;cp @mediafile_source @mediafile_dest; ' .
        $convert_command .
        '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
        $execute_string_options);
    }
  }
}
