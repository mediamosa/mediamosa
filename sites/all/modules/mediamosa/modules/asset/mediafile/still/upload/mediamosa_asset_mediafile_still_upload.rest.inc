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
 * URI: /asset/$asset_id/still/upload
 * Method: POST
 */
class mediamosa_rest_call_upload_mediafile_still extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const UPLOAD_TICKET = 'upload_ticket';
  const REDIRECT_URI = 'redirect_uri';
  const COMPLETED_URL = 'completed_url';
  const MEDIAFILE_ID = 'mediafile_id';
  const ORDER = 'order';
  const DEFAULT_ = 'default';
  const TAG = 'tag';
  const FILENAME = 'filename';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
        ),
        self::UPLOAD_TICKET => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ID that has been given to enable the upload.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
        ),
        self::ORDER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still order number.',
          self::VAR_DEFAULT_VALUE => 0,
        ),
        self::DEFAULT_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Is default still?',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::REDIRECT_URI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URL_URI,
          self::VAR_DESCRIPTION => 'The URL to redirect when upload has been successful.',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Still tag.',
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The filename of the media file (PUT method only, ignored otherwise).',
          self::VAR_IS_REQUIRED => $this->is_method_put() ? self::VAR_IS_REQUIRED_YES : self::VAR_IS_REQUIRED_NO,
        ),
        self::COMPLETED_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URL,
          self::VAR_DESCRIPTION => "This URL gives opportunity to the APP builder for a status raport; \nupload_ticket\nstatus_code\n\nThe upload_ticket is used to identify the upload. The status_code is a MediaMosa code.\nThe type is URL and not URI, because this parameter http:// or https://.",
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get params.
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $ticket_id = $this->get_param_value(self::UPLOAD_TICKET);
    $completed_url = $this->get_param_value(self::COMPLETED_URL);
    $tag = $this->get_param_value(self::TAG);
    $order = $this->get_param_value(self::ORDER);
    $default = $this->get_param_value(self::DEFAULT_);

    $filename_put = NULL;

    if ($this->is_method_put()) {
      $filename_put = $this->get_param_value(self::FILENAME); // With PUT only.
    }

    // Check the mediafile.
    mediamosa_asset_mediafile::must_exists($mediafile_id);

    try {
      $still_id = mediamosa_asset_mediafile_upload::handle_upload_still($ticket_id, $this->is_method_put(), $tag, $completed_url, $filename_put, $order, $default);
    }
    catch (Exception $e) {
      if ($this->isset_param(self::REDIRECT_URI)) {
        // Any errors need to be redirected.
        throw new mediamosa_exception_redirect_and_exit($this->get_param_value(self::REDIRECT_URI), $e->getCode(), $e->getMessage());
      }

      // Lets create error output then.
      throw $e;
    }

    // Redirect when needed.
    if ($this->isset_param(self::REDIRECT_URI)) {
      throw new mediamosa_exception_redirect_and_exit($this->get_param_value(self::REDIRECT_URI), mediamosa_error::ERRORCODE_OKAY);
    }

    // Add still_id to output.
    $mediamosa->add_item(array('still_id' => $still_id));
  }
}
