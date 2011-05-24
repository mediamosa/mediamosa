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
 * Statistics popular streams.
 */

// Include default.
module_load_include('inc', 'mediamosa_statistics', 'mediamosa_statistics');

/**
 * List the latest mediafiles.
 */
function mediamosa_statistics_popularstreams_form() {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Statistics'), 'admin/mediamosa/statistics');
  drupal_set_breadcrumb($breadcrumb);

  $build = array();

  $build['mediamosa_statistics_filter'] = drupal_get_form('mediamosa_statistics_filter_form', MEDIAMOSA_STATISTICS_FILTER_TYPE_POPULARSTREAMS);
  $build['mediamosa_statistics_list'] = drupal_get_form('mediamosa_statistics_popularstreams_list_form');

  return $build;
}

function mediamosa_statistics_popularstreams_list_form() {

  // Get session.
  $session = isset($_SESSION['mediamosa_statistics_filter'][MEDIAMOSA_STATISTICS_FILTER_TYPE_POPULARSTREAMS]) ? $_SESSION['mediamosa_statistics_filter'][MEDIAMOSA_STATISTICS_FILTER_TYPE_POPULARSTREAMS] : array();

  // Get app owners.
  $app_owners = mediamosa_app::collect_app_owners();

  $app_ids = array();
  if (!empty($session['app_id'])) {
    $app_ids[] = $session['app_id'];
  }
  if (!empty($session['app_owner']) && isset($app_owners[$session['app_owner']])) {
    $app_ids += $app_owners[$session['app_owner']];
  }
  $year = empty($session['year']) ? date('Y') : $session['year'];
  $month = empty($session['month']) ? date('n') : $session['month'];
  $month_end = empty($session['month_end']) ? ($month + 1) : $session['month_end'];
  if ($month_end > 12) {
    $month_end = 1;
    $year_end = $year + 1;
  }
  $year_end = empty($session['year_end']) ? (isset($year_end) ? $year_end : $year) : $session['year_end'];

  $params = array(
    'app_id' => $app_ids,
    'begindate' => sprintf('%02d-%02d', $year, $month),
    'enddate' => sprintf('%02d-%02d', $year_end, $month_end),
  );

  // Do the rest call.
  $result = mediamosa_response_connector::static_do_restcall_drupal('statistics/streams', 'GET', $params);

  // Our header.
  $header = array(
    array('data' => t('Played')),
    array('data' => t('App ID')),
    array('data' => t('Filename')),
    array('data' => t('Owner ID')),
  );

  $rows = array();
  foreach ($result['items'] as $row) {
    $title = empty($row['filename']) ? $row['mediafile_id'] : $row['filename'];

    $rows[] = array(
      $row['requested'] . 'x',
      $row['app_id'],
      mediamosa_lib::l_asset($row['asset_id'], $title),
      check_plain($row['owner_id']),
    );
  }

  /**
   * No total count, pager not possible.
   */

  //$form['pager_top'] = array('#theme' => 'pager');
  $form['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No stream requests found.')
  );
  //$form['pager_bottom'] = array('#theme' => 'pager');

  return $form;
}
