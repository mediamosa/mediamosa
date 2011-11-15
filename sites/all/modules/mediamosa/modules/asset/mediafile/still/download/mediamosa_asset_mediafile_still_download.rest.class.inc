<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * REST call for downloading mediafiles created by tickets.
 */

/**
 * URI: /still/$still_id (perm. link)
 * URI: /still/ticket/$ticket_id (normal temp. link).
 * URI: /still/[1stletter $still_id]/$still_id (missing perm. link).
 *
 * URI: /still/$still_id/style/$style (perm. link)
 * URI: /still/ticket/$ticket_id/style/$style (normal temp. link).
 * URI: /still/[1stletter $still_id]/$still_id/style/$style (missing perm. link).
 *
 * Method: GET / POST
 *
 * replacement for still/index.php.
 */
class mediamosa_rest_call_download_mediafile_still extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const STILL_ID = 'still_id';
  const TICKET_ID = 'ticket_id';
  const STYLE = 'style';

  // ------------------------------------------------------------------ Get Var Setup.
  /**
   * Match URI.
   *
   * @param string $uri
   *
   * @return boolean
   *   TRUE, if match.
   */
  public static function match_uri($uri) {
    // $delta = 1, if we are not in a simpletest.
    $delta = (mediamosa::get()->in_simpletest_sandbox() ? 2 : 1);
    $uri_parts = explode('/', $uri);

    // Match if there isn't second argument, or the first is "ticket", or the length of the first is 1.
    // This is necessary, because of REST calls, like: still/$still_id/watermark.
    return
      // still/$still_id
      (empty($uri_parts[$delta + 1]) ||
      // still/$still_id/style/$style
      (!empty($uri_parts[$delta + 1]) && $uri_parts[$delta + 1] == 'style') ||
      // still/ticket/$ticket_id and still/ticket/$ticket_id/style/$style
      (!empty($uri_parts[$delta]) && $uri_parts[$delta] == 'ticket') ||
      // still/[1stletter $still_id]/$still_id and still/[1stletter $still_id]/$still_id/style/$style
      (!empty($uri_parts[$delta]) && drupal_strlen($uri_parts[$delta]) == 1));
  }

  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'The still ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::TICKET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ticket ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::STYLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Image style.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
      )
    );

    // All we need.
    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the ticket ID.
    $still_id = $this->get_param_value(self::STILL_ID);
    $ticket_id = $this->get_param_value(self::TICKET_ID);
    $style = $this->get_param_value(self::STYLE);


    if (!$ticket_id && !$still_id) {
      throw new mediamosa_exception_error(mediamosa_error::HTTP_FORBIDDEN);
    }

    if ($ticket_id) {
      //
      // Temporary link.
      //

      // Get filename.
      $file = mediamosa_configuration_storage::ticket_still_location_get() . $ticket_id;

      if (!file_exists($file)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_STILL_TICKET, array('@ticket_id' => $ticket_id));
      }
    }
    elseif ($still_id) {
      //
      // Permanent link.
      //

      // Check if the permanent link is here. If not, then we re-eval the access.

      // Get symlink to still.
      $file = mediamosa_configuration_storage::ticket_still_location_get() . mediamosa_configuration_storage::objectid_to_location($still_id);

      // No perm link? Then check access, based on that create a perm link.
      if (!file_exists($file)) {

        // Check access.
        try {
          mediamosa_asset_mediafile::is_mediafile_protected($still_id);
        }
        catch (mediamosa_exception_error_asset_not_found $e) {
          throw new mediamosa_exception_error_404();
        }
        catch (mediamosa_exception_error_mediafile_not_found $e) {
          throw new mediamosa_exception_error_404();
        }
        catch (mediamosa_exception $e) {
          throw new mediamosa_exception_error_403();
        }

        // Create the permanent link.
        $path = mediamosa_configuration_storage::ticket_still_location_get() . $still_id[0];

        // Create directory.
        mediamosa_io::mkdir($path, TRUE);

        // Make sure the location is a directory.
        if (!is_dir($path)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_FOUND, array('@location' => $path));
        }

        // Must be able to write there.
        if (!mediamosa_io::is_writable($path)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_DIR_NOT_WRITABLE, array('@location' => $path));
        }

        // Target symlink.
        $target = mediamosa_configuration_storage::mount_point_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::still_location_get() . DIRECTORY_SEPARATOR . mediamosa_configuration_storage::objectid_to_location($still_id);

        // Create the symlink ticket.
        if (!is_file($target)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_FILE_NOT_FOUND, array('@filename' => $target));
        }

        if (!mediamosa_io::symlink($target, $file)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_UNABLE_TO_CREATE_SYMLINK);
        }
      }
    }

    // Handle still style.
    $file = mediamosa_asset_mediafile_still::handle_still_style_request($file, $style, $ticket_id, $still_id);

    $handle = fopen($file, 'rb');

    if (!$handle) {
      throw new mediamosa_exception(mediamosa_error::HTTP_FORBIDDEN, 'Unable to open file!');
    }

    // Set headers.
    $type = mediamosa_asset_mediafile::get_mimetype($still_id);
    $type = isset($type) ? $type : 'image/jpeg';
    header('Content-Type: ' . $type);

    // Set the cache header in case of permanent stills.
    if ($still_id) {
      header('Cache-Control: ' . (isset($response->headers['cache-control']) ? $response->headers['cache-control'] : 'public, max-age=' . mediamosa_settings::STILL_REST_CACHE_SECOND . ', must-revalidate'));
      header('Last-Modified: ' . gmdate('D, d M Y H:i:s', filemtime($file)) . ' GMT');
      header('Expires: ' . (isset($response->headers['expires']) ? $response->headers['expires'] : gmdate('D, d M Y H:i:s', time() + mediamosa_settings::STILL_REST_CACHE_SECOND) . " GMT"));
    }

    // Make sure we send the header now.
    ob_clean();
    flush();

    // Until end of file, get the file and output.
    while (!feof($handle)) {
      $buffer = fread($handle, mediamosa_settings::DOWNLOAD_CHUNK_SIZE);

      print $buffer;

      ob_flush();
      flush();
    }

    // Close the file.
    fclose($handle);
    die;

    // Set the result, reponse object will know what to do.
    $mediamosa->set_result(mediamosa_response::SUCCESS, 200, $file);
  }
}
