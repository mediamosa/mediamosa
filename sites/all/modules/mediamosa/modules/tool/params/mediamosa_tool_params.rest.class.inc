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
 * Transcode parameter REST calls.
 */

/**
 * URI: /transcode/parameter
 * Method: GET
 *
 * 1.x: _transcoding_profiles_transcode_parameter.
 *
 */
class mediamosa_rest_call_tool_params_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

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
    $a_transcode_mappings = mediamosa_tool_params::search();

    // Add to response.
    foreach ($a_transcode_mappings as $transcode_mapping) {
      // TODO: MediaMosa supports now a ";" separated list, but it is possible, that some allowed values has ";".
      $transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE] = implode(';', (array)unserialize($transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE]));
      $transcode_mapping[mediamosa_tool_params_db::CREATED] = mediamosa::utcdate2appdate($transcode_mapping[mediamosa_tool_params_db::CREATED]);
      $transcode_mapping[mediamosa_tool_params_db::CHANGED] = mediamosa::utcdate2appdate($transcode_mapping[mediamosa_tool_params_db::CHANGED]);

      $o_mediamosa->add_item($transcode_mapping);
    }
  }
}
