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
 * Statistics Diskspace usage.
 */

// Include default.
module_load_include('inc', 'mediamosa_statistics', 'mediamosa_statistics');

/**
 * List the latest mediafiles.
 */
function mediamosa_statistics_diskspaceused_form() {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Statistics'), 'admin/mediamosa/statistics');
  drupal_set_breadcrumb($breadcrumb);

  $build = array();

  $build['mediamosa_statistics_filter'] = drupal_get_form('mediamosa_statistics_filter_form', MEDIAMOSA_STATISTICS_FILTER_TYPE_DISKSPACE);
  $build['mediamosa_statistics_list'] = drupal_get_form('mediamosa_statistics_diskspaceused_list_form');

  return $build;
}

function mediamosa_statistics_diskspaceused_list_form() {

  // Get session.
  $session = isset($_SESSION['mediamosa_statistics_filter'][MEDIAMOSA_STATISTICS_FILTER_TYPE_DISKSPACE]) ? $_SESSION['mediamosa_statistics_filter'][MEDIAMOSA_STATISTICS_FILTER_TYPE_DISKSPACE] : array();

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
  $type = empty($session['type']) ? mediamosa_statistics_diskspace_used_db::TYPE_CONTAINER : $session['type'];

  // Limit, page.
  $limit = mediamosa_settings::LIMIT_DEFAULT_STATISTICS;
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;

  $params = array(
    'app_id' => $app_ids,
    'month' => $month,
    'year' => $year,
    'type' => $type,
    'limit' => $limit,
    'offset' => $page * $limit,
  );

  // Do the rest call.
  $result = mediamosa_response_connector::static_do_restcall_drupal('statistics/datausagevideo', 'GET', $params);

  $summary_data = NULL;
  if (isset($result['header']['statistics']['total_disk_usage_mb'])) {
    $summary_data = $result['header']['statistics']['total_disk_usage_mb'];
  }

  // Our header.
  $header = array(
    array('data' => t('App ID')),
    array('data' => t('Container type')),
    array('data' => t('Type')),
    array('data' => t('Used diskspace MB')),
  );

  $rows = array();

  foreach ($result['items'] as $row) {
    $title = empty($row['title']) ? t('<this object has no title>') : $row['title'];
    $rows[] = array(
      $row['app_id'],
      check_plain($row['container_type']),
      check_plain($row['type']),
      check_plain($row['diskspace_mb']) . ' MB',
    );
  }

  // Set item_count_total.
  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  // Our listing.
  $form['list'] = array(
    '#theme' => 'mediamosa_maintenance_table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No diskspace usage found.'),
    // Pager.
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
    // Total note.
    //'#caption_bottom' => t('Entries found: @total', array('@total' => $item_count_total)),
    // Summary.
    '#summary' => array(
      array(
        'title' => t('Entries found:'),
        'data' => $item_count_total,
      ),
      array(
        'title' => t('Disk usage:'),
        'data' => (is_numeric($summary_data) ? format_size(1024 * 1024 * $summary_data) : t('N/A')),
      ),
    ),
  );

  return $form;
}
