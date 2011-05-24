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
  * Tool functions.
  */

class mediamosa_tool_params {

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Get tool_param listing.
   */
  static public function search() {
    return mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'tm')
      ->fields('tm')
      ->execute();
  }

  /**
   * Get a specific tool parameter.
   *
   * @param $tool
   * @param $nice_parameter
   */
  static public function get_by_tool($tool, $nice_parameter = NULL) {

    $query = mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'tm')->fields('tm');

    if ($nice_parameter) {
      $query->condition(mediamosa_tool_params_db::NICE_PARAMETER, $nice_parameter);
      return $query->condition(mediamosa_tool_params_db::TOOL, $tool)
        ->execute()
        ->fetchAssoc();
    }

    return $query->condition(mediamosa_tool_params_db::TOOL, $tool)
      ->execute();
  }

  /**
   * Check value against tools parameter settings.
   *
   * @param string $tool
   *  Tool used.
   * @param string $nice_parameter
   *  The param to check.
   * @param mixed $value
   *  The value to check.
   */
  static public function check_mapping($tool, $nice_parameter, $value) {

    // Get the mapping param.
    $a_transcode_mapping = self::get_by_tool($tool, $nice_parameter);

    if (!$a_transcode_mapping) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FOUND, array('@key' => $nice_parameter));
    }

    // Check if value is in range and right type.
    if (!is_null($a_transcode_mapping[mediamosa_tool_params_db::MIN_VALUE]) || !is_null($a_transcode_mapping[mediamosa_tool_params_db::MAX_VALUE])) {
      if (!is_numeric($value)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT, array('@key' => $nice_parameter, '@value' => $value));
      }

      if (!is_null($a_transcode_mapping[mediamosa_tool_params_db::MIN_VALUE])) {
        if ($a_transcode_mapping[mediamosa_tool_params_db::MIN_VALUE] > $value) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT, array('@key' => $nice_parameter, '@min_value' => $a_transcode_mapping[mediamosa_tool_params_db::MIN_VALUE], '@value' => $value));
        }

        if ($a_transcode_mapping[mediamosa_tool_params_db::MAX_VALUE] < $value) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT, array('@key' => $nice_parameter, '@max_value' => $a_transcode_mapping[mediamosa_tool_params_db::MAX_VALUE], '@value' => $value));
        }
      }
    }

    // If allowed values is set, then make sure its one of them.
    if (!empty($a_transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE])) {
      $a_allowed_values = unserialize($a_transcode_mapping[mediamosa_tool_params_db::ALLOWED_VALUE]);
      if (!in_array($value, $a_allowed_values)) {
        return new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_WRONG_VALUE, array('@key' => $nice_parameter, '@value' => $value));
      }
    }
  }
}
