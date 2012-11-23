<?php
/**
 * @file
 * Class for /media REST call.
 */

class mediamosa_asset_mediafile_media {
  // ---------------------------------------------------------------- Functions.
  /**
   * Split up the mediafile_str into it parts.
   *
   * @param $mediafile_str
   *   The mediafile string; mediafile_id[,style_id][.extension]
   *
   * @return
   *   A array containing;
   *     'mediafile_id' (always)
   *     'style_id' (always or 0)
   *     'extension'
   */
  public static function mediafile_str_parse($mediafile_str) {
    $matches = array();
    if (preg_match('@^([a-zA-Z0-9]+),?([0-9]+)?(\.[a-zA-Z0-9]+)?$@', $mediafile_str, $matches)) {
      return array($matches[1], empty($matches[2]) ? 0 : $matches[2], empty($matches[3]) ? '' : mediamosa_unicode::substr($matches[3], 1));
    }

    // If its not in our format, then return it as is.
    return array($mediafile_str, 0, '');
  }

  /**
   * Build the mediafile_str.
   *
   * @param $mediafile_id
   *   The mediafile ID.
   * @param $extension
   *   The file extension (without dot).
   * @param $style_id
   *   The style_id (optional).
   *
   * @return
   *   The created mediafile_str.
   */
  public static function mediafile_str_build($mediafile_id, $extension, $style_id = '') {
    return $mediafile_id . (empty($style_id) ? '' : ',' . $style_id) . '.' . $extension;
  }

  /**
   * Split up the mediaticket_str into it parts.
   *
   * @param $mediaticket_str
   *   The mediafile string; mediafile_id[,style_id][.extension]
   *
   * @return
   *   A array containing;
   *     'mediaticket_id' (always)
   *     'style_id' (always or 0)
   *     'ext'
   */
  public static function mediaticket_str_parse($mediaticket_str) {
    $matches = array();
    if (preg_match('@^([a-zA-Z0-9]+),?([0-9]+)?(\.[a-zA-Z0-9]+)?$@', $mediaticket_str, $matches)) {
      return array($matches[1], empty($matches[2]) ? 0 : $matches[2], empty($matches[3]) ? '' : mediamosa_unicode::substr($matches[3], 1));
    }

    // If its not in our format, then return it as is.
    return array($mediaticket_str, 0, '');
  }

  /**
   * Build the ticket_str.
   *
   * @param $ticket_id
   *   The ticket ID.
   * @param $extension
   *   The file extension (without dot).
   * @param $style_id
   *   The style_id (optional).
   *
   * @return
   *   The created ticket_str.
   */
  public static function mediaticket_str_build($ticket_id, $extension, $style_id = '') {
    return $ticket_id . (empty($style_id) ? '' : ',' . $style_id) . '.' . $extension;
  }

  /**
   * Create the symlink for either mediaticket or mediafile.
   *
   * @param $target
   *   The target of the symlink.
   * @param $link
   *   The link of the symlink.
   *
   * @throws
   *   mediamosa_exception_error();
   *     ERRORCODE_DIR_NOT_FOUND
   *     ERRORCODE_DIR_NOT_WRITABLE
   *     ERRORCODE_FILE_NOT_FOUND
   *     ERRORCODE_UNABLE_TO_CREATE_SYMLINK
   *
   */
  protected static function handle_media_symlink($target, $link) {
    // Create the symlink, if not already. In most cases, its either already
    // been created (mean while during this call) or its the first call.
    if (mediamosa_io::file_exists($link)) {
      return;
    }

    // Get the path.
    $path = mediamosa_io::dirname($link);

    // Create directory.
    mediamosa_io::mkdir($path);

    // Make sure the location is a directory.
    if (!is_dir($path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_FOUND, array('@location' => $path));
    }

    // Must be able to write there.
    if (!mediamosa_io::is_writable($path)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_WRITABLE, array('@location' => $path));
    }

    // Now create the symlink.
    if (!mediamosa_io::symlink($target, $link)) {
      // Might happen that the symlink fails because it was created during
      // our code between this symlink and !file_exists() above.
      if (!mediamosa_io::file_exists($link)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK);
      }
    }
  }

  /**
   * Serve the file by returning it as file using http.
   *
   * @param $file
   *   The file location to serve.
   * @param $mimetype
   *   The mimetype of the file.
   */
  protected static function serve_media($file, $mimetype) {
    // Build up the default headers.
    $headers = mediamosa_io::file_get_content_headers($file, $mimetype);

    // Serve file.
    mediamosa_io::file_transfer($file, $headers);
  }

  /**
   * Proccess the media request using the ticket ID for /media REST call.
   *
   * We only process the ticket when style is provided AND the ticket does
   * exists. The style is only used on stills.
   *
   * @param $app_id
   *   The application ID.
   * @param $mediaticket_str
   *   The ticket ID in extended form (ticket_id[,style_id][.extension]).
   *
   * @throws mediamosa_exception_error_mediafile_not_found
   */
  public static function handle_media_request_ticket($app_id, $mediaticket_str) {
    // Get the parts.
    list($ticket_id, $style_id, $extension) = self::mediaticket_str_parse($mediafile_str);

    // Get the mediafile_id.
    $mediafile_id = mediamosa_ticket::get_mediafile_id($ticket_id);

    // No mediafile? Invalid ticket.
    if (empty($mediafile_id)) {
      throw new mediamosa_exception_error_invalid_ticket(array('@ticket_id' => $ticket_id));
    }

    // Get the mimetype from the technical metadata.
    $mimetype = mediamosa_asset_mediafile::get_mimetype($mediafile_id);

    // Now use the mimetype to match ext.
    $mimetype_extension = mediamosa_mimetype::extension2mimetype($extension);

    // The mimetypes must match.
    if ($mimetype_extension != $mimetype) {
      throw new mediamosa_exception_error_invalid_ticket(array('@ticket_id' => $ticket_id));
    }

    // We can trust the extension.

    // Only when the ticket does not exists and a style was defined, then allow
    // to generate the ticket link with style. In any other case, we don't
    // create ticket links.

    // Need the original.
    $original_mediaticket_str = self::mediaticket_str_build($ticket_id, $extension);

    // Get the ticket location. It must exist. If it does not, then either the
    // ticket is invalid or we don't have access.
    $original_mediaticket_file = mediamosa_storage::get_uri_media_ticket_file($app_id, $original_mediaticket_str);
    if (!file_exists($original_mediaticket_file)) {
      throw new mediamosa_exception_error_invalid_ticket(array('@ticket_id' => $ticket_id));
    }

    // Any style?
    if (!$style_id) {
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id));
    }

    // Ticket exists and we have a style.

    // Get (and create) the still derivative using the given style.
    $target = mediamosa_asset_mediafile_still::image_style_create_derivative($mediafile_id, $style_id);

    // Now create ticket symlink to the style file. Also the ticket symlink must
    // have the same create date as the original ticket so its cleaned up at the
    // same time as the master ticket.

    // Symlink Link.
    $link = mediamosa_storage::get_uri_media_file($app_id, $mediaticket_str);

    // Create the symlink.
    self::handle_media_symlink($target, $link);

    // Serve the file.
    self::serve_media($link, $mimetype);
  }

  /**
   * Process the media request call for /media REST call.
   *
   * Only intended for stills atm.
   *
   * @param $app_id
   *   The application ID.
   * @param $mediafile_str
   *   The mediafile_str is a extended filename version of the mediafile ID.
   */
  public static function handle_media_request_permanent($app_id, $mediafile_str) {

    // Get the parts.
    list($mediafile_id, $style_id, $extension) = self::mediafile_str_parse($mediafile_str);

    // Check access. Permanent can not have any ACL protection.
    mediamosa_asset_mediafile::is_mediafile_protected($mediafile_id);

    // Get the mimetype from the technical metadata.
    $mimetype = mediamosa_asset_mediafile::get_mimetype($mediafile_id);

    // Now use the mimetype to match ext.
    $mimetype_extension = mediamosa_mimetype::extension2mimetype($extension);

    // Log it.
    mediamosa_debug::log('Mediafile content-type; @mimetype, matching with extension @extension, @mimetype_extension', array('@mimetype' => $mimetype, '@extension' => $extension, '@mimetype_extension' => $mimetype_extension));

    // The mimetypes must match.
    if ($mimetype_extension != $mimetype) {
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id));
    }

    // We can trust the extension.

    // At this point the mediafile exists, and we have access.

    // Any style_id?
    if ($style_id) {
      // Handle still style, create it as target for our symlink.
      $target = mediamosa_asset_mediafile_still::image_style_create_derivative($mediafile_id, $style_id);
    }
    else {
      // Get the source file (has no extension) as our symlink target.
      $target = mediamosa_storage::get_uri_data_file($mediafile_id);
    }

    // Symlink Link.
    $link = mediamosa_storage::get_uri_media_file($app_id, $mediafile_str);

    // Create the symlink.
    self::handle_media_symlink($target, $link);

    // Serve the file.
    self::serve_media($link, $mimetype);
  }

  /**
   * Test if a permanent mediafile link can be made.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile to test.
   *
   * @throws mediamosa_exception_error_is_unappropriate
   */
  public static function allow_permanent($mediafile_id, $is_app_admin = FALSE) {
    // Protected mediafiles (stills) can not be permanent.
    try {
      return !mediamosa_asset_mediafile::is_mediafile_protected($mediafile_id, $is_app_admin);
    }
    catch (mediamosa_exception_error_is_unappropriate $e) {
      throw $e;
    }
    catch (mediamosa_exception $e) {
      return FALSE;
    }
  }
}
