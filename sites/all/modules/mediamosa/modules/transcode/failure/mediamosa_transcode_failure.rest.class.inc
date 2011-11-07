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
 * Transcode profile REST calls.
 */


/**
 * URI: /transcode/failures
 * Method: GET
 *
 *
 */
class mediamosa_rest_call_transcode_failures extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Get the listing.
    $a_transcode_failures = mediamosa_job_transcode::getFailed();

    foreach ($a_transcode_failures as $a_transcode_failure) {
      $a_item = array();
      $a_item['transcode_job_id'] = $a_transcode_failure[mediamosa_job_db::ID];
      $a_item['transcode_asset_id'] = $a_transcode_failure[mediamosa_job_db::ASSET_ID];
      $a_item['transcode_mediafile_id'] = $a_transcode_failure[mediamosa_job_db::MEDIAFILE_ID];
      $a_item['transcode_app_id'] = $a_transcode_failure[mediamosa_job_db::APP_ID];
      $a_item['transcode_job_started'] = $a_transcode_failure[mediamosa_job_db::STARTED];
      $a_item['transcode_job_filename'] = $a_transcode_failure[mediamosa_asset_mediafile_db::FILENAME];
      $a_item['transcode_error_description'] = $a_transcode_failure[mediamosa_job_db::ERROR_DESCRIPTION];
      $a_item['command'] = $a_transcode_failure[mediamosa_job_transcode_db::COMMAND];

      $o_mediamosa->add_item($a_item);
    }
  }
}

