<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * URI: /download
 * Method: GET / POST
 *
 * replacement for download/index.php.
 */
class mediamosa_rest_call_download_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const TICKET_ID = 'ticket_id';
  const FILENAME = 'filename';

    // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::TICKET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ticket ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The filename.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        )
      )
    );

    // All we need.
    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the ticket ID.
    $ticket_id = $this->get_param_value(self::TICKET_ID);
    $filename = $this->get_param_value(self::FILENAME);

    // Get filename.
    $path = mediamosa_storage::get_uri_media_download_ticket() . $ticket_id;
    $file = $path . DIRECTORY_SEPARATOR . $filename;

    // Make sure the file exists we want to download.
    if (!is_dir($path) || !file_exists($file)) {
      throw new mediamosa_exception(0, 'Invalid download ticket');
    }

    // Set headers.
    header('Pragma: public');
    header('Expires: 0');
    header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
    header('Cache-Control: private, false');
    header('Content-Description: Download');
    header('Content-Type: application/force-download'); // alternative: application/octet-stream
    header('Content-Length: ' . filesize($file));

    // This has been tested with filename plus'q %20uote.mp4 under IE8 and
    // FF3.6, chrome 6.
    header('Content-Disposition: attachment; filename="' . $filename . '"'); // force a save dialog.
    header('Content-Transfer-Encoding: binary');

    $handle = fopen($file, 'rb');

    if (!$handle) {
      throw new mediamosa_exception(0, 'Unable to open file!');
    }

    // Make sure we send the header now.
    ob_clean();
    flush();

    // No time-out here.
    set_time_limit(0);

    // Until end of file, get the file and output.
    while (!feof($handle)) {
      $buffer = fread($handle, mediamosa_settings::DOWNLOAD_CHUNK_SIZE);

      print $buffer;

      ob_flush();
      flush();
    }

    // Close the file.
    fclose($handle);

    // Set the result, reponse object will know what to do.
    $mediamosa->set_result(mediamosa_response::SUCCESS, 200, $file);
  }
}
