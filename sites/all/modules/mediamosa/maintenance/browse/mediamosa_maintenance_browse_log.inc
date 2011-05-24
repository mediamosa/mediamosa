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
 * Shows the last log entries of mediamosa related entries.
 */

/**
 * Show a selection of the watchdog in our own log viewer.
 */
function _mediamosa_maintenance_browse_log_form() {
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  $rows = array();
  $icons = array(
    WATCHDOG_DEBUG     => '',
    WATCHDOG_INFO      => '',
    WATCHDOG_NOTICE    => '',
    WATCHDOG_WARNING   => theme('image', array('path' => 'misc/watchdog-warning.png', 'alt' => t('warning'))),
    WATCHDOG_ERROR     => theme('image', array('path' => 'misc/watchdog-error.png', 'alt' => t('error'))),
    WATCHDOG_CRITICAL  => theme('image', array('path' => 'misc/watchdog-error.png', 'alt' => t('critical'))),
    WATCHDOG_ALERT     => theme('image', array('path' => 'misc/watchdog-error.png', 'alt' => t('alert'))),
    WATCHDOG_EMERGENCY => theme('image', array('path' => 'misc/watchdog-error.png', 'alt' => t('emergency'))),
  );

  $classes = array(
    WATCHDOG_DEBUG     => 'dblog-debug',
    WATCHDOG_INFO      => 'dblog-info',
    WATCHDOG_NOTICE    => 'dblog-notice',
    WATCHDOG_WARNING   => 'dblog-warning',
    WATCHDOG_ERROR     => 'dblog-error',
    WATCHDOG_CRITICAL  => 'dblog-critical',
    WATCHDOG_ALERT     => 'dblog-alert',
    WATCHDOG_EMERGENCY => 'dblog-emerg',
  );

  $header = array(
    '', // Icon column.
    array('data' => t('Type'), 'field' => 'w.type'),
    t('Message'),
    array('data' => t('Severity'), 'field' => 'w.severity', 'sort' => 'desc'),
    array('data' => t('Date'), 'field' => 'w.wid', 'sort' => 'desc'),
  );

  $levels = watchdog_severity_levels();
  $types = array('page not found');

  $query = db_select('watchdog', 'w')->extend('PagerDefault')->extend('TableSort');
  $query->fields('w');
  $query->condition(db_or()->condition('type', $types, 'IN')->condition('type', 'MediaMosa%', 'LIKE'));
  $result = $query
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->orderByHeader($header)
    ->execute();

  foreach ($result as $dblog) {
    $rows[] = array('data' =>
      array(
        // Cells
        $icons[$dblog->severity],
        t($dblog->type),
        truncate_utf8(_mediamosa_maintenance_browse_log_format_message($dblog), 1000, TRUE, TRUE),
        $levels[$dblog->severity],
        theme('mediamosa_maintenance_date', array('timestamp' => $dblog->timestamp)),
      ),
      // Attributes for tr.
      'class' => array('dblog-' . preg_replace('/[^a-z]/i', '-', $dblog->type), $classes[$dblog->severity]),
    );
  }

  if (!$rows) {
    $rows[] = array(array('data' => t('No log messages available.'), 'colspan' => 5));
  }

  $form = array();

  $form['log_pager_top'] = array('#theme' => 'pager');
  $form['log_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('id' => 'admin-dblog'),
  );
  $form['log_pager_bottom'] = array('#theme' => 'pager');

  return $form;
}

/**
 * Formats a log message for display.
 *
 * @param $dblog
 *   An object with at least the message and variables properties
 */
function _mediamosa_maintenance_browse_log_format_message($dblog) {
  // Legacy messages and user specified text
  return $dblog->variables == 'N;' ? $dblog->message : t($dblog->message, unserialize($dblog->variables));
}
