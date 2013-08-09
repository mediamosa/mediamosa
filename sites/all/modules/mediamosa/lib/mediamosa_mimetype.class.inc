<?php
/**
 * @file
 * Mimetype related functions.
 */

class mediamosa_mimetype {
  // ---------------------------------------------------------------- Functions.
  /**
   * Translate the file extension to a mimetype.
   *
   * This translation back is not reliable as some extensions can have more than
   * 1 mime-type, like webm has an video/webm and audio/webm variant.
   *
   * @param string $extension
   *   The extension to map to mimetype.
   *
   * @return string
   *   Returns the found mimetype or application/octet-stream.
   */
  public static function extension2mimetype($extension) {
    // Re-use the Drupal mimetype library.
    include_once DRUPAL_ROOT . '/includes/file.mimetypes.inc';
    $mapping = file_mimetype_mapping();

    if (isset($mapping['extensions'][$extension])) {
      return $mapping['mimetypes'][$mapping['extensions'][$extension]];
    }

    return 'application/octet-stream';
  }

  /**
   * Try to match mime-type to ext.
   *
   * @param string $mimetype
   *   The mimetype.
   *
   * @return string|boolean
   *    The file extension or FALSE on failure.
   */
  public static function mimetype2extension($mimetype) {
    // Do not convert default.
    if ($mimetype == 'application/octet-stream') {
      return FALSE;
    }

    // Re-use the Drupal mimetype library.
    include_once DRUPAL_ROOT . '/includes/file.mimetypes.inc';
    $mapping = file_mimetype_mapping();

    // Give prio on last ext; use array_keys() in reverse.
    $ext_id = array_search($mimetype, $mapping['mimetypes']);
    if ($ext_id !== FALSE) {
      $mimetypes = array_reverse(array_keys($mapping['extensions'], $ext_id));
      return reset($mimetypes);
    }

    return FALSE;
  }

  /**
   * Determine Mime type of a file.
   *
   * @todo: move to wrapper.
   *
   * @param string $uri
   *   File to get the mimetype of.
   *
   * @return string
   *   The mime type.
   */
  public static function get_mime_type($uri) {
    // Get the local file.
    $realpath = mediamosa_io::realpath($uri);

    $return_var = -1;
    $exec_string = mediamosa_io::exec('file -b -L --mime-type ' . escapeshellarg($realpath), $return_var);
    $file_mime = reset($exec_string);
    if ($file_mime == 'application/octet-stream') {
      $exec_string = mediamosa_io::exec('file -L ' . escapeshellarg($realpath), $return_var);
      $result = reset($exec_string);

      $matches = array();
      $reg = '#^(?P<filename>.*): (?P<filetype>.*)$#';
      if (preg_match($reg, $result, $matches)) {
        // Some specific detection.
        switch ($matches['filetype']) {
          case 'Microsoft ASF':
            return 'video/x-msvideo';

          case 'Macromedia Flash Video':
            return 'video/x-flv';

          case 'video/mp4':
            return 'video/mp4';
        }
      }
    }

    return $file_mime;
  }
}
