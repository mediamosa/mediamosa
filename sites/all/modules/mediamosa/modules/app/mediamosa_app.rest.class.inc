<?php
/**
 * @file
 * Client Application rest calls.
 */

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
 * URI: /preview_profile_id (deprecated)
 *      /transcode/profile/default
 *
 * Method: GET
 */
class mediamosa_rest_call_app_get_default_transcode_profile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Get the app.
    $app = mediamosa_app::get_by_appid($app_id);

    // Add to response.
    if ($app) {
      $mediamosa->add_item(array(mediamosa_app_db::PREVIEW_PROFILE_ID => $app[mediamosa_app_db::PREVIEW_PROFILE_ID]));
    }
  }
}

/**
 * URI: /app/quota
 * Method: GET
 */
class mediamosa_rest_call_app_get_quota extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Get the app.
    $totals = mediamosa_app::get_by_appid($app_id);

    // Get diskspace used.
    $used = mediamosa_user::get_diskspace('app', $app_id, $app_id);

    // Add application quota to response.
    if ($totals && $used) {
      $quota = $totals[mediamosa_app_db::QUOTA] * 1024;
      $mediamosa->add_item(array(
        'app_quota_mb' => $quota,
        'app_diskspace_used_mb' => $used['diskspace_used_mb'],
        'quota_available_mb' => ($quota - $used['diskspace_used_mb']),
      ));
    }
  }
}
