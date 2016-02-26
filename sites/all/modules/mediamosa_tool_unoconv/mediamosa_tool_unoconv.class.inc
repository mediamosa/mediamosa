<?php

/**
 * @file
 * Unoconv tool functions.
 */
class mediamosa_tool_unoconv {

  /**
   * Name / ID of the module.
   */
  const NAME = 'unoconv';


  const MEDIAMOSA_MAINTENANCE_STATUS_CONFIGURATION_UNOCONV_WARNING = '0';

  /**
   * Returns TRUE when we support this mime-type.
   *
   * @param string $mime_type
   *   mimetype request.
   */
  public static function is_supported($mime_type) {

    $supportedApplicationTypes = array(
      'msword',
      'vnd.openxmlformats-officedocument.wordprocessingml.document',
      'vnd.oasis.opendocument.text',
      'rtf',
      'vnd.ms-powerpoint',
      'vnd.openxmlformats-officedocument.presentationml.presentation',
      'vnd.openxmlformats-officedocument.presentationml.slideshow',
      'vnd.ms-powerpoint.slideshow.macroenabled.12',
      'vnd.oasis.opendocument.presentation',
      'vnd.ms-excel',
      'vnd.openxmlformats-officedocument.spreadsheetml.sheet',
      'vnd.oasis.opendocument.spreadsheet',
    );

    preg_match('#^application\/(.+)#', $mime_type, $matchResults);


    if (isset($matchResults[1]) && in_array($matchResults[1], $supportedApplicationTypes)) {
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Get the exec string for transcoding the file.
   *
   * @param array $options
   *   Options array for transcode.
   */
  public static function get_transcode_exec($options) {

    $parameterString = $options['parameter_string'];

    $pattern = '/(-eSelectPdfVersion) (\d)/';
    $replacement = '${1}=${2}';
    $parameterString = preg_replace($pattern, $replacement, $parameterString);

    $execution_string_options = array(
      '@mediafile_source' => $options['location_source_file'],
      '@mediafile_dest' => $options['location_dest_file'] . '.' . $options['file_extension'],
      '@parameter_string' => $parameterString,
      '@status_file' => $options['status_file'],
      '@working_dir' => dirname($options['location_dest_file']),
      '@basename' => basename($options['location_source_file']),
      '@extension' => $options['file_extension'],
    );

    // Execution string.
    return strtr(
      '{ echo "Errors: none"; echo "Status: converting"; echo "Progress: 0.000"; echo "Warnings: none"; } > @status_file;' .
      'export MAGICK_TMPDIR=@working_dir;' .
      'unoconv @parameter_string -f @extension -o @mediafile_dest @mediafile_source;' .
      '{ echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @status_file;',
      $execution_string_options);

  }


}
