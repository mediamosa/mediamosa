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
 * REST calls for the status module.
 */

/**
 * URI: /external/status
 * Method: GET
 */
class mediamosa_rest_call_status extends mediamosa_rest_call {

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get status from cache.
    $statuses = module_invoke_all('mediamosa_status_collect', FALSE);

    // Get only what we need.
    foreach ($statuses as $section_part => $section) {
      $item = array();

      $item[$section_part]['title'] = $section['title'];
      $failures = 0;

      // fill it.
      foreach ($section['results'] as $part => $result) {
        if (!isset($result['severity']) || $result['severity'] == MEDIAMOSA_MAINTENANCE_STATUS_RESULT_INFO) {
          continue;
        }

        if ($result['severity'] == MEDIAMOSA_MAINTENANCE_STATUS_RESULT_ERROR) {
          $failures++;
        }

        $item[$section_part][$part]['failure'] = ($result['severity'] == MEDIAMOSA_MAINTENANCE_STATUS_RESULT_ERROR ? 'TRUE' : 'FALSE');
      }

      $item[$section_part]['failure'] = $failures ? 'TRUE' : 'FALSE';

      // we add runtime (unixtimestamp) to verify if cron is still running.
      $last_time = variable_get('mediamosa_status_last_time', 0);
      $item['time'] = $last_time;

      $mediamosa->add_item($item);
    }
  }
}
