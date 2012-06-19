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
 * Failure browser, based on Collection browser.
 */

define('MEDIAMOSA_BROWSE_JOB_FAILURES', 'mediafile_browse_job_failures');
define('MEDIAMOSA_BROWSE_JOB_FAILURES_HIDDEN', 'HIDDEN');
define('MEDIAMOSA_BROWSE_JOB_FAILURES_PREFIX', 'job_id');

/**
 * Build the form for browsing collections.
 */
function mediamosa_browse_job_failures_page($form) {
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  $build = array();

  //$build['collection_filter'] = drupal_get_form('mediamosa_browse_job_failures_filter_form');
  $build['failures_list'] = _mediamosa_browse_job_failures_list_form();

  return $build;
}

/**
 * Form builder; failures listing page.
 *
 */
function _mediamosa_browse_job_failures_list_form() {
  // Get session.
  $session = _mediamosa_browse_job_failures_session();
  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $visibility = $session['visibility'];
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $limit = mediamosa_settings::maintenance_items_per_page();

  $result = _mediamosa_browse_job_failures_execute($page, $limit);

  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  if ($session['visibility'] == MEDIAMOSA_BROWSE_JOB_FAILURES_HIDDEN) {
    $filter_text = "Failed, but hidden";
  } else {
    $filter_text = "Failed jobs";
  }

  $form['jobfail']['table'] = array(
    '#theme' => 'mediamosa_browse_job_failures',
    '#header' => '',
    '#showhidehidden' => '',
    '#title' => t($filter_text . ' (@total)' , array('@total' => $item_count_total)),
    '#description' => t('Below list displays the failed jobs in the jobs table. Select the desired jobs and select a mass operation to apply.'),
    '#item_count_total_text' => t('Failed jobs found: @total', array('@total' => $item_count_total)),
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
  );

  foreach ($result['items'] as $row) {
    $form['jobfail']['table']['job_id' .  $row['job_id']]['job_id' . $row['job_id']] = array(
      '#type' => 'checkbox',
      '#job_id' => $row['job_id'],
      '#job_type' => $row['job_type'],
      '#filename' => substr($row['job_filename'], 0, 25) . '...',
      '#asset_id' => $row['job_asset_id'],
      '#mediafile_id' => $row['job_mediafile_id'],
      '#date_started' => theme('mediamosa_maintenance_date', array('datetime' => $row['job_started'])),
      '#date_modified' => theme('mediamosa_maintenance_date', array('datetime' => $row['job_modified'])),
      '#app_id' => $row['job_app_id'],
      '#error_description' => $row['job_error_description'],
    );
  }

  // Operation buttons.
  if (isset($result['items'])) {
    $form['jobfail']['op1'] = array(
      '#type' => 'submit',
      '#value' => t('Retranscode/Reanalyse'),
    );

    if ($session['visibility'] == MEDIAMOSA_BROWSE_JOB_FAILURES_HIDDEN) {
      $form['jobfail']['op3'] = array(
        '#type' => 'submit',
        '#value' => t('Unhide'),
      );
    }
    else {
      $form['jobfail']['op3'] = array(
        '#type' => 'submit',
        '#value' => t('Hide'),
      );
    }

    $form['jobfail']['op4'] = array(
      '#type' => 'submit',
      '#value' => t('ToggleHidden'),
    );
  }

  return $form;
}

function mediamosa_browse_job_failures_page_submit($form, $form_state) {

  $jobs = array();
  $op = $form_state['values']['op'];

  $prefix = MEDIAMOSA_BROWSE_JOB_FAILURES_PREFIX;
  $prefix_length = drupal_strlen($prefix);

  foreach($form_state['values'] as $form_key => $form_value) {
    if (drupal_substr($form_key, 0, $prefix_length) == $prefix && $form_value == 1) {
      $jobs[] = substr($form_key, $prefix_length);
    }
  }

  switch ($op) {
    case t('Retranscode/Reanalyse'):
      _mediamosa_browse_job_failures_reanalyse_transcode($jobs);
      break;
    case t('Hide'):
      _mediamosa_browse_job_failures_hidejobs($jobs);
      break;
    case t('Unhide'):
      _mediamosa_browse_job_failures_unhidejobs($jobs);
      break;
    case t('ToggleHidden'):
      _mediamosa_browse_job_failures_toggle_hidden($jobs);
      break;
  }
}

function _mediamosa_browse_job_failures_unhidejobs($jobs) {
  foreach($jobs as $job_id) {
    mediamosa_job_display::unhide($job_id);
  }
}

function _mediamosa_browse_job_failures_hidejobs($jobs) {
  foreach($jobs as $job_id) {
    mediamosa_job_display::hide($job_id);
  }
}

function _mediamosa_browse_job_failures_toggle_hidden() {
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_JOB_FAILURES]) ? $_SESSION[MEDIAMOSA_BROWSE_JOB_FAILURES] : array();

  // Toggle hidden attribute
  if ($session['visibility'] == MEDIAMOSA_BROWSE_JOB_FAILURES_HIDDEN) {
    $session['visibility'] = NULL;
  }
  else {
    $session['visibility'] = MEDIAMOSA_BROWSE_JOB_FAILURES_HIDDEN;
  }

  $_SESSION[MEDIAMOSA_BROWSE_JOB_FAILURES] = $session;
}

function _mediamosa_browse_job_failures_reanalyse_transcode($jobs = array()) {
  $all_ok = TRUE;

  foreach($jobs as $job_id) {
    // Unhide the job, in case it is hidden.
    mediamosa_job_display::unhide($job_id);

    // Get the job details.
    $job = mediamosa_job::get($job_id);
    $job_type = $job[mediamosa_job_db::JOB_TYPE];
    $owner_id = $job[mediamosa_job_db::OWNER_ID];
    $asset_id = $job[mediamosa_job_db::ASSET_ID];
    $mediafile_id = $job[mediamosa_job_db::MEDIAFILE_ID];
    $app_id = $job[mediamosa_job_db::APP_ID];
    $hint = $job[mediamosa_job_db::HINT];
    $mediafile_dest = $job[mediamosa_job_db::MEDIAFILE_DEST];
    $is_app_admin = TRUE;
    $group_id = '';
    $create_still = FALSE;
    $still_parameters = array();

    // Create the job.
    if ($job_type == mediamosa_job_db::JOB_TYPE_ANALYSE) {
      mediamosa_job::create_analyse_job($owner_id, $asset_id, $mediafile_id, $app_id);
    }
    elseif ($job_type == mediamosa_job_db::JOB_TYPE_TRANSCODE) {
      $job_transcode = mediamosa_job_transcode::get($job_id);
      $file_extension = $job_transcode[mediamosa_job_transcode_db::FILE_EXTENSION];
      $command = $job_transcode[mediamosa_job_transcode_db::COMMAND];
      $tool = $job_transcode[mediamosa_job_transcode_db::TOOL];
      $profile_id = $job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID];
      $completed_transcoding_url = $job_transcode[mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL];
      $priority = NULL;

      $new_job_id = mediamosa_job::create_job_transcode($app_id, $owner_id, $group_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $priority, $hint, $mediafile_dest);
      if ($all_ok) {
        $all_ok = (is_numeric($new_job_id) && $new_job_id > 0);
      }
    }
  }

  if ($all_ok) {
    if (count($jobs) > 0) {
      drupal_set_message(t('All jobs succesfully set.'));
    }
    else {
      drupal_set_message(t('No job(s) selected.'));
    }
  }
  else {
    drupal_set_message(t('Error(s) during job(s) create.'));
  }
}

/**
 * Do the REST call and return the response object.
 *
 * @param $page
 * @param $limit
 *
 * @return $result
 */
function _mediamosa_browse_job_failures_execute($page, $limit) {
  // Get session.
  $session = _mediamosa_browse_job_failures_session();

  $params = array(
    'visibility' => isset($session['visibility']) && $session['visibility'] == MEDIAMOSA_BROWSE_JOB_FAILURES_HIDDEN ? mediamosa_job_display_db::VISIBILITY_HIDDEN : mediamosa_job_display_db::VISIBILITY_DISPLAY,
    'limit' => $limit,
    'offset' => $page * $limit,
    'order_by' => 'job_id',
    'order_direction' => 'desc',
  );

  // Do the rest call.
  $result = mediamosa_response_connector::static_do_restcall_drupal('job/failures', 'GET', $params);

  return $result;
}

/**
 * Get and set the session.
 */
function _mediamosa_browse_job_failures_session($filter_form_state = array()) {
  // Get session.
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_JOB_FAILURES]) ? $_SESSION[MEDIAMOSA_BROWSE_JOB_FAILURES] : array();

  // Get these from _GET if present.
  foreach (array('order', 'sort', 'page') as $param) {
    unset($session[$param]);
    if (isset($_GET[$param])) {
      $session[$param] = $_GET[$param];
    }
  }

  // Set defaults.
  $session += array(
    'order' => 'job_id',
    'sort' => 'asc',
    'page' => 0,
    'visibility' => NULL,
    'limit' => mediamosa_settings::maintenance_items_per_page(),
  );

  // Process filter.
  if (!empty($filter_form_state['input']['operator'])) {
    $session['operator'] = $filter_form_state['input']['operator'];
  }

  if (!empty($filter_form_state['input']['filters'])) {
    $session['filters'] = $filter_form_state['input']['filters'];
  }

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_JOB_FAILURES] = $session;

  return $session;
}

/**
 * Reuse simpletest test table.
 */
function theme_mediamosa_browse_job_failures($variables) {

  $table = $variables['table'];

  drupal_add_css(drupal_get_path('module', 'simpletest') . '/simpletest.css');
  drupal_add_js(drupal_get_path('module', 'simpletest') . '/simpletest.js');
  drupal_add_js('misc/tableselect.js');

  // Create header for test selection table.
  $header = array(
    array('class' => array('select-all')),
    array('data' => t('Job ID'), 'class' => array('mmjobid')),
    array('data' => t('Job type'), 'class' => array('mmjobtype')),
    array('data' => t('Asset ID'), 'class' => array('mmassetid')),
    array('data' => t('Mediafile ID'), 'class' => array('mmmediafileid')),
    array('data' => t('Filename'), 'class' => array('mmfilename')),
    array('data' => t('Date started'), 'class' => array('mmdate')),
    array('data' => t('Date modified'), 'class' => array('mmdate')),
    array('data' => t('App ID'), 'class' => array('mmappid')),
    array('data' => t('Description'), 'class' => array('mmdescription')),
  );

   // Cycle through each failure and create a row.
  $rows = array();
  foreach (element_children($table) as $key) {
    $element = $table[$key];

    // Cycle through each test within the current group.
    foreach (element_children($element) as $this_name) {
      $details = $element[$this_name];
      $row = array();

      $job_id = $details['#job_id'];
      $job_type = $details['#job_type'];
      $filename = $details['#filename'];
      $asset_id = $details['#asset_id'];
      $mediafile_id = $details['#mediafile_id'];
      $app_id = $details['#app_id'];
      $date_started = $details['#date_started'];
      $date_modified = $details['#date_modified'];
      $error_description = $details['#error_description'];

      $details['#title_display'] = 'invisible';
      unset($details['#description']);

      // Test name is used to determine what tests to run.
      $details['#name'] = $this_name;

      $row[] = array(
        'data' => drupal_render($details),
        'class' => array('simpletest-test-select'),
      );
      $row[] = array(
        'data' => '<div>' . check_plain($job_id) . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . check_plain($job_type) . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . l(check_plain($asset_id), 'admin/mediamosa/browse/asset/' . $asset_id) . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . check_plain($mediafile_id) . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . check_plain($filename) . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . check_plain($date_started) . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . check_plain($date_modified) . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . check_plain($app_id) . '</div>',
        'style' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div class="description">' . check_plain($error_description) . '</div>',
        'style' => 'width:600px',
      );
      $rows[] = array('data' => $row);
    }
  }

  if (empty($rows)) {
    return '<h1>No failed transcoding/analyses jobs...</h1>';
  }
  else {
    $title = '<h1>' . check_plain($table['#title']) . '</h1>';
    $description = '<p>' . check_plain($table['#description']) . '</p>';

    $buildtable = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'simpletest-form-table')));
    $caption_top = '<div align="right">' . $table['#showhidehidden'] . '</div>';
    $caption_bottom = '<div align="right">' . check_plain($table['#item_count_total_text']) . '</div>';

    $pager_variables = array(
      'page' => $table['#page'],
      'item_count_total' => $table['#item_count_total'],
      'item_limit' => $table['#item_limit'],
    );
    $pager = theme('mediamosa_maintenance_pager', $pager_variables);

    $complete_table = $title . $description . $pager . $caption_top . $buildtable . $caption_bottom . $pager;

    return  $complete_table;
  }
}
