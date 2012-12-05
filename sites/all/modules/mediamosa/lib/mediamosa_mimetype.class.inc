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

    $ext_id = array_search($mimetype, $mapping['mimetypes']);
    if ($ext_id !== FALSE) {
      return array_search($ext_id, $mapping['extensions']);
    }

    return FALSE;
  }
}
