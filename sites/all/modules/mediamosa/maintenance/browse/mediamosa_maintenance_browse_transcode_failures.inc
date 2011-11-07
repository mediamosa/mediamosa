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
 * Failure browser, based on Collection browser.
 */

define('MEDIAMOSA_BROWSE_FILTER_COLLECTION', 'mediafile_browse_collection');

/**
 * Build the form for browsing collections.
 */
function mediamosa_browse_transcode_failures_page($form) {
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  $build = array();

  //$build['collection_filter'] = drupal_get_form('mediamosa_browse_transcode_failures_filter_form');
  $build['failures_list'] = mediamosa_browse_transcode_failures_list_form();

  return $build;
}

/**
 * Form builder; failures listing page.
 *
 */
function mediamosa_browse_transcode_failures_list_form() {
  // Get session.
  $session = _mediamosa_browse_transcode_failures_session();
  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $visibility = $session['visibility'];
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $item_count_total = 0;

  $result = _mediamosa_browse_transcode_failures_execute();

  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  if($session['visibility']=='HIDDEN'){
    $filter_text = "Failed, but hidden, transcodings";
  } else {
    $filter_text = "Failed transcodings";
  }

  $form['transfail']['table'] = array(
    '#theme' => 'mediamosa_browse_transcode_failures',
    '#header' => $header,
    '#showhidehidden' => '',
    '#title' => t($filter_text . '(@total)' , array('@total' => $item_count_total)),
    '#description' => t('Below list displays the failed transcodings in the jobs table. Select the desired jobs and select a mass operation to apply.'),
    '#item_count_total' => t('Failed transcodings found: @total', array('@total' => $item_count_total))
  );
  
  foreach ($result['items'] as $row) {
    $form['transfail']['table']['job_id' .  $row['transcode_job_id']]['job_id' . $row['transcode_job_id']] = array(
      '#type' => 'checkbox',
      '#job_id' => $row['transcode_job_id'],
      '#job_type' => $row['transcode_job_type'],
      '#filename' => substr($row['transcode_job_filename'], 0, 25) . '...',
      '#asset_id' => $row['transcode_job_asset_id'],
      '#mediafile_id' => $row['transcode_job_mediafile_id'],
      '#date_started' => theme('mediamosa_maintenance_date', array('datetime' => $row['transcode_job_started'])),
      '#date_modified' => theme('mediamosa_maintenance_date', array('datetime' => $row['transcode_job_modified'])),
      '#app_id' => $row['transcode_job_app_id'],
      '#error_description' => $row['transcode_job_error_description'],
    );
  }

  // Operation buttons.
  if (isset($result['items'])) {
    $form['transfail']['op1'] = array(
      '#type' => 'submit',
      '#value' => t('Re-Transcode/Analyse'),
    );

    if($session['visibility']=='HIDDEN'){
      $form['transfail']['op3'] = array(
        '#type' => 'submit',
        '#value' => t('Unhide'),
      );
    }
    else
    {
      $form['transfail']['op3'] = array(
        '#type' => 'submit',
        '#value' => t('Hide'),
      );
    }

    $form['transfail']['op4'] = array(
      '#type' => 'submit',
      '#value' => t('ToggleHidden'),
    );
  }

  return $form;
}

function mediamosa_browse_transcode_failures_page_submit($form, $form_state) {

  $jobs = array();
  $op = $form_state['values']['op'];

  foreach($form_state['values'] as $form_key=>$form_value){
    if((substr($form_key, 0, 6) == 'job_id') && ($form_value == 1)){
      $jobs[] = substr($form_key, 6);
    }
  }

  switch ($op) {
    case t('Re-Transcode/Analyse'):
      _mediamosa_browse_transcode_failures_reanalyse_transcode($jobs);
      break;
    case t('Hide'):
      _mediamosa_browse_transcode_failures_hidejobs($jobs);
      break;
    case t('Unhide'):
      _mediamosa_browse_transcode_failures_unhidejobs($jobs);
      break;
    case t('ToggleHidden'):
      _mediamosa_browse_transcode_failures_toggle_hidden($jobs);
      break;
  }
}

function _mediamosa_browse_transcode_failures_unhidejobs($jobs) {
  foreach($jobs as $job_id){
    mediamosa_job_display::unhide($job_id);
  }
}

function _mediamosa_browse_transcode_failures_hidejobs($jobs) {
  foreach($jobs as $job_id){
    mediamosa_job_display::hide($job_id);
  }
}

function _mediamosa_browse_transcode_failures_toggle_hidden() {
  
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION]) ? $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] : array();

  // Toggle hidden attribute
  if($session['visibility'] == 'HIDDEN'){
    $session['visibility'] = null;
  }
  else {
    $session['visibility'] = 'HIDDEN';
  }

  $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] = $session;
}

function _mediamosa_browse_transcode_failures_reanalyse_transcode($jobs = array()) {

  $allok = true;

  foreach($jobs as $job_id){
    // Unhide the job, in case it is hidden.
    mediamosa_job_display::unhide($job_id);
    $allok &= mediamosa_job::set_progress($job_id, 0, false, mediamosa_job_db::JOB_STATUS_WAITING);
  }

  if($allok){
    if(count($jobs)>0)
      drupal_set_message(t('All jobs succesfully set for re-analysis.'));
    else
      drupal_set_message(t('No job(s) selected.'));
  }
  else
  {
    drupal_set_message(t('Error(s) during job(s) delete.'));
  }
}

/**
 * Do the REST call and return the response object.
 */
function _mediamosa_browse_transcode_failures_execute() {
  // Get session.
  $session = _mediamosa_browse_transcode_failures_session();

  $params = array();

  // Add params.
  if((isset($session['visibility'])) && ($session['visibility']=='HIDDEN'))
    $params['visibility'] = $session['visibility'];

  
  // Do the rest call.
  return mediamosa_response_connector::static_do_restcall_drupal('transcode/failures', 'GET', $params);
}

/**
 * Get and set the session.
 */
function _mediamosa_browse_transcode_failures_session($filter_form_state = array()) {
  // Get session.
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION]) ? $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] : array();
  // Get these from _GET if present.
  foreach (array('order', 'sort', 'page') as $param) {
    unset($session[$param]);
    if (isset($_GET[$param])) {
      $session[$param] = $_GET[$param];
    }
  }

  // Set defaults.
  $session += array(
    'order' => t('Title'),
    'sort' => 'asc',
    'page' => 0,
    'visibility' => null,
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
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] = $session;

  return $session;
}

/**
 * Reuse simpletest test table.
 */
function theme_mediamosa_browse_transcode_failures($variables) {

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
    $element = &$table[$key];
    $row = array();
    
    // Cycle through each test within the current group.
    foreach (element_children($element) as $test_name) {
      $test = $element[$test_name];
      $row = array();

      $job_id = $test['#job_id'];
      $job_type = $test['#job_type'];
      $filename = $test['#filename'];
      $asset_id = $test['#asset_id'];
      $mediafile_id = $test['#mediafile_id'];
      $app_id = $test['#app_id'];
      $date_started = $test['#date_started'];
      $date_modified = $test['#date_modified'];
      $error_description = $test['#error_description'];

      $test['#title_display'] = 'invisible';
      unset($test['#description']);

      // Test name is used to determine what tests to run.
      $test['#name'] = $test_name;

      $row[] = array(
        'data' => drupal_render($test),
        'class' => array('simpletest-test-select'),
      );
      $row[] = array(
        'data' => '<div>' . $job_id . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . $job_type . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . $asset_id . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . $mediafile_id . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . $filename . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . $date_started . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . $date_modified . '</label>',
        'class' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div>' . $app_id . '</div>',
        'style' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div class="description">' . $error_description . '</div>',
        'style' => 'width:600px',
      );
      $rows[] = array('data' => $row, 'class' => array($test_class . '-failure', ($collapsed ? 'js-hide' : '')));
    }
  }

  // Add js array of settings.
  drupal_add_js(array('simpleTest' => $js), 'setting');

  if (empty($rows)) {
    return '<h1>No failed transcoding/analyses jobs...</h1>';
  }
  else {
    $title = '<h1>' . check_plain($table['#title']) . '</h1>';
    $description = '<p>' . check_plain($table['#description']) . '</p>';

    $buildtable = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'simpletest-form-table')));
    $caption_top = '<div align="right">' . $table['#showhidehidden'] . '</div>';
    $caption_bottom = '<div align="right">' . check_plain($table['#item_count_total']) . '</div>';
    $complete_table = $title . $description . $caption_top . $buildtable . $caption_bottom;

    return  $complete_table;
  }
}