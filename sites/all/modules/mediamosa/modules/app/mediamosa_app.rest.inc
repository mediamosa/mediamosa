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
 * App rest calls.
 */


/**
 * URI: /preview_profile_id
 * Method: GET
 *
 * 1.x: transcoding_profiles_get_default_transcode_profile.
 *
 */
class mediamosa_rest_call_app_get_default_transcode_profile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    // Get the app.
    $a_app = mediamosa_app::get_by_appid($app_id);

    // Add to response.
    if ($a_app) {
      $o_mediamosa->add_item(array(mediamosa_app_db::PREVIEW_PROFILE_ID => $a_app[mediamosa_app_db::PREVIEW_PROFILE_ID]));
    }
  }
}
