<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
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
 * Theming functions maintenance
 */

/**
 * Returns HTML for the MediaMosa status report.
 *
 * @param $table_name
 *   A string with the name of the table
 * @param $results
 *   An associative array containing:
 *   - result: An array of results.
 *     * 'title': the name of the result.
 *     * 'value': the current value.
 *     * 'description': description of the result.
 *     * 'severity': the result's severity level, one of:
 *       MEDIAMOSA_MAINTENANCE_STATUS_RESULT_INFO: For info only.
 *       MEDIAMOSA_MAINTENANCE_STATUS_RESULT_OK: The result is satisfied.
 *       MEDIAMOSA_MAINTENANCE_STATUS_RESULT_WARNING: The result failed with a warning.
 *       MEDIAMOSA_MAINTENANCE_STATUS_RESULT_ERROR: The result failed with an error.
 *
 * @ingroup themeable
 */
function theme_mediamosa_maintenance_status_report($variables) {
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance_status') . '/mediamosa_maintenance_status.css');

  $table_name = $variables['table_name'];
  $results = $variables['results'];

  $output = '';
  $i = 0;

  $output .= '<h2>' . $table_name . '</h2>';
  $output .= '<table class="system-status-report">';
  foreach ($results as $result) {
    $class = ++$i % 2 == 0 ? 'even' : 'odd';

    $classes = array(
      MEDIAMOSA_MAINTENANCE_STATUS_RESULT_INFO => 'mediamosa-status-info',
      MEDIAMOSA_MAINTENANCE_STATUS_RESULT_OK => 'mediamosa-status-ok',
      MEDIAMOSA_MAINTENANCE_STATUS_RESULT_WARNING => 'mediamosa-status-warning',
      MEDIAMOSA_MAINTENANCE_STATUS_RESULT_ERROR => 'mediamosa-status-error',
    );
    $class = $classes[isset($result['severity']) ? (int) $result['severity'] : 0];// . ' ' . $class;

    // Output table row(s)
    if (empty($result['description'])) {
      $output .= '<tr class="' . $class . '"><td class="mediamosa-status-part">' . $result['title'] . '</td><td>' . $result['value'];
    }
    else {
      $output .= '<tr class="' . $class . ' merge-down"><td class="mediamosa-status-part">' . $result['title'] . '</td><td>' . $result['value'] . '</td></tr>';
      $output .= '<tr class="' . $class . ' merge-up"><td colspan="2">' . $result['description'];
    }
    $output .= '</td></tr>';
  }

  $output .= '</table>';
  return $output;
}
