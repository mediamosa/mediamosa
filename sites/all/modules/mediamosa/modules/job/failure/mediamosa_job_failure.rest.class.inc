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
 * Job failures REST calls.
 */


/**
 * URI: /job/failures
 * Method: GET
 *
 *
 */
class mediamosa_rest_call_job_failures extends mediamosa_rest_call {

  // Rest vars;
  const VISIBILITY = 'visibility';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::VISIBILITY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The visibility attribute.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_job_display_db::VISIBILITY_DISPLAY,
            mediamosa_job_display_db::VISIBILITY_HIDDEN,
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_job_display_db::VISIBILITY_DISPLAY,
        ),
      )
    );

    // Include limit, offset vars.
    $var_setup = self::get_var_setup_range($var_setup);

    // Include order by.
    $var_setup = self::get_var_setup_order_by($var_setup);

    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the visibility attribute.
    $visibility = $this->get_param_value(self::VISIBILITY);

    // Limit, offset.
    $limit = $this->get_param_value(self::LIMIT);
    $offset = $this->get_param_value(self::OFFSET);

    // Order by, order direction.
    $order_by = $this->get_param_value(self::ORDER_BY);
    $order_direction = $this->get_param_value(self::ORDER_DIRECTION);

    // Get the listing.
    $result_array = mediamosa_job::get_failed($visibility, $limit, $offset, $order_by, $order_direction);

    // Total rows.
    $mediamosa->item_count_total = $result_array['item_count_total'];

    $job_failures = $result_array['result'];
    foreach ($job_failures as $job_failure) {
      $item = array();
      $item['job_id'] = $job_failure[mediamosa_job_db::ID];
      $item['job_asset_id'] = $job_failure[mediamosa_job_db::ASSET_ID];
      $item['job_mediafile_id'] = $job_failure[mediamosa_job_db::MEDIAFILE_ID];
      $item['job_app_id'] = $job_failure[mediamosa_job_db::APP_ID];
      $item['job_type'] = $job_failure[mediamosa_job_db::JOB_TYPE];
      $item['job_started'] = $job_failure[mediamosa_job_db::STARTED];
      $item['job_modified'] = $job_failure[mediamosa_job_db::CHANGED];
      $item['job_filename'] = $job_failure[mediamosa_asset_mediafile_db::FILENAME];
      $item['job_error_description'] = $job_failure[mediamosa_job_db::ERROR_DESCRIPTION];

      $mediamosa->add_item($item);
    }
  }
}

