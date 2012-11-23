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
 * Removed;
 * URI: /still/$still_id (perm. link)
 * URI: /still/ticket/$ticket_id (normal temp. link).
 * URI: /still/[1stletter $still_id]/$still_id (missing perm. link).
 * URI: /still/$still_id/style/$style (perm. link)
 * URI: /still/ticket/$ticket_id/style/$style (normal temp. link).
 * URI: /still/[1stletter $still_id]/$still_id/style/$style (missing perm. link).
 * URI: /download/$ticket_id/$filename (POST/GET).
 * URI: /media/ticket/$app_id/[1st letter ticket]/$ticket_id[,style].ext (3.1)
 * URI: /media/$app_id/[1st letter ticket]/$mediafile_id[,style].ext (3.1)
 *
 * 3.5 uri's;
 * URI: /media/ticket/$app_id/[1st letter ticket]/$ticket_id-$responsetype/filename[,style].ext
 *      /media/$app_id/[1st letter ticket]/$mediafile_id-$responsetype/filename[,style].ext
 *
 * $responsetype:
 * still
 * download
 * play
 *
 * Method: GET / POST
 *
 */
class mediamosa_rest_call_media extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const APP_ID = 'app_id';
  const MEDIAFILE_STR = 'mediafile_str';
  const MEDIATICKET_STR = 'mediaticket_str';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::APP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The app ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIATICKET_STR => array(
          // Mediaticket ID allows style.
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIATICKET_STR,
          self::VAR_DESCRIPTION => 'The ticket ID in extended format; ticket_id[,style_id][.ext]. However, the REST call will not process the call with the ticket ID, as this is always processed by the webserver. See documentation about /media call.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::MEDIAFILE_STR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_STR,
          self::VAR_DESCRIPTION => 'The mediafile ID in extended format; mediafile_id[,style_id][.ext].',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
      )
    );

    // All we need.
    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    // Get the input values.
    $app_id = $this->get_param_value(self::APP_ID);
    $mediafile_str = $this->get_param_value(self::MEDIAFILE_STR);
    $mediaticket_str = $this->get_param_value(self::MEDIATICKET_STR);

    // Either one required.
    if (!$mediaticket_str && !$mediafile_str) {
      throw new mediamosa_exception_error_400(); // Bad request.
    }

    if ($mediaticket_str) {
      // Before 3.1 we always served the mediafile here. Now we don't because
      // the REST call should not have been triggered in the first place with
      // tickets (Apache is handeling this). Need to warn admin that his
      // configuration does not include the correct configuration for his
      // webserver.

      // In any case, if the ticket exists and we where triggered, then the
      // webserver setup is not correct and we should log this problem.
      $file = mediamosa_storage::get_uri_media_ticket_file($app_id, $mediaticket_str);
      if (file_exists($file)) {
        mediamosa_watchdog::log('The /media REST call was triggered while this was not expected. Check your MediaMosa status page for more information.', array(), WATCHDOG_CRITICAL);
        throw new mediamosa_exception_error_501();
      }
    }

    // Handle request.
    try {
      // Tickets are temporary.
      if ($mediaticket_str) {
        mediamosa_asset_mediafile_media::handle_media_request_ticket($app_id, $mediaticket_str);
      }
      else {
        // Non-tickets are temporary.
        mediamosa_asset_mediafile_media::handle_media_request_permanent($app_id, $mediafile_str);
      }
    }
    catch (mediamosa_exception_error_asset_not_found $e) {
      throw new mediamosa_exception_error_404();
    }
    catch (mediamosa_exception_error_mediafile_not_found $e) {
      throw new mediamosa_exception_error_404();
    }
    catch (mediamosa_exception_error_access_denied $e) {
      throw new mediamosa_exception_error_403();
    }
    catch (mediamosa_exception_error_is_unappropriate $e) {
      throw new mediamosa_exception_error_403();
    }
    catch (mediamosa_exception_error_invalid_ticket $e) {
      throw new mediamosa_exception_error_404();
    }
    catch (Exception $e) {
      throw new mediamosa_exception_error_500();
    }
  }
}
