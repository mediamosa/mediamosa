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

  $limit = $session['limit'];
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $item_count_total = 0;

  $header = array(
    array('field' => 'title', 'data' => t('Title'), 'sort' => 'asc'),
    array('field' => 'app_id', 'data' => t('App ID'), 'class' => array('mmappid')),
    array('field' => 'changed', 'data' => t('Job started'), 'class' => array('mmdate')),
    array('data' => t('Operations'), 'class' => array('mmoperation')),
  );

  $result = _mediamosa_browse_transcode_failures_execute();
  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  $form['transfail']['table'] = array(
    '#theme' => 'mediamosa_browse_transcode_failures',
    '#header' => 'header',
    '#title' => t('Failed transcodings (@total)', array('@total' => $item_count_total)),
    '#description' => t('Below list displays the failed transcodings in the jobs table. Select the desired jobs and select a mass operation to apply.'),
    '#item_count_total' => t('Failed transcodings found: @total', array('@total' => $item_count_total))
  );


  foreach ($result['items'] as $row) {
    $form['transfail']['table']['job_id' .  $row['transcode_job_id']]['job_id' . $row['transcode_job_id']] = array(
      '#type' => 'checkbox',
      '#job_id' => $row['transcode_job_id'],
      '#filename' => $row['transcode_job_filename'],
      '#asset_id' => $row['transcode_asset_id'],
      '#mediafile_id' => $row['transcode_mediafile_id'],
      '#date_started' => theme('mediamosa_maintenance_date', array('datetime' => $row['transcode_job_started'])),
      '#app_id' => $row['transcode_app_id'],
      '#error_description' => $row['transcode_error_description'],
    );
  }

  // Operation buttons.
  if (!isset($form['transfail']['table'])) {
    $form['transfail']['op1'] = array(
      '#type' => 'submit',
      '#value' => t('Re-Transcode'),
    );

    $form['transfail']['op2'] = array(
      '#type' => 'submit',
      '#value' => t('Re-Analyse'),
    );

    $form['transfail']['op3'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
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
    case t('Re-Transcode'):
      _mediamosa_browse_transcode_failures_retranscode($jobs[0]);
      drupal_set_message('re transcode');
      break;
    case t('Re-Analyse'):
      _mediamosa_browse_transcode_failures_reanalyse($jobs[0]);
      drupal_set_message('re analyse');
      break;     
    case t('Delete'):
      _mediamosa_browse_transcode_failures_deletejobs($jobs);
      drupal_set_message('deleting');
      break;
  }
  drupal_set_message(print_r($jobs, true));
}

function _mediamosa_browse_transcode_failures_deletejobs($jobs) {

  $allok = true;

  // Delete all the jobs.
  foreach($jobs as $job_id){
    if(mediamosa_job::get($job_id))
    $allok &= mediamosa_job::delete_job($job_id);
  }

  if($allok){
    drupal_set_message(t('All marked jobs succesfully deleted.'));
  }
  else
  {
    drupal_set_message(t('Error(s) during jobs delete.'));
  }
}

function _mediamosa_browse_transcode_failures_reanalyse($job_id) {

  // Get the job details
  $job = mediamosa_job::get($job_id, array(mediamosa_job_db::ASSET_ID, mediamosa_job_db::MEDIAFILE_ID));

  // REST uri.
  $uri = strtr('mediafile/@mediafile_id/analyse', array('@mediafile_id' => $job['mediafile_id']));

  // Get the asset.
  $asset = mediamosa_asset::get($job['asset_id']);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']);
    $result = mediamosa_response_connector::static_do_restcall_drupal($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      drupal_set_message(t('Analyse started.'));
      drupal_goto('admin/mediamosa/browse/failures/');
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to start re-analyse.'));
  drupal_goto('admin/mediamosa/browse/failures/');
}

function _mediamosa_browse_transcode_failures_retranscode($job_id) {

  // Get the job details
  $job = mediamosa_job::get($job_id, array(mediamosa_job_db::ASSET_ID, mediamosa_job_db::MEDIAFILE_ID));

  // REST uri.
  $uri = strtr('mediafile/@mediafile_id/transcode', array('@mediafile_id' => $job['mediafile_id']));

  // Get the asset.
  $asset = mediamosa_asset::get($job['asset_id']);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']);
    $result = mediamosa_response_connector::static_do_restcall_drupal($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      drupal_set_message(t('Analyse started.'));
      drupal_goto('admin/mediamosa/browse/failures/');
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to start re-transcode.'));
  drupal_goto('admin/mediamosa/browse/failures/');
}


/**
 * Do the REST call and return the response object.
 */
function _mediamosa_browse_transcode_failures_execute() {
  // Get session.
  $session = _mediamosa_browse_transcode_failures_session();

  // What to sort on.
  $sort = $session['sort'];
  $order = $session['order'];
  $limit = $session['limit'];
  $page = $session['page'];

  switch ($order) {
    case t('App ID'):
      $order = 'app_id';
      break;

    case t('Job started'):
      $order = 'changed';
      break;

    case t('Title'):
    default:
      $order = 'title';
      break;
  }

  $params = array();

  $cqls = array();

  $cql_session = _mediamosa_browse_cql_build($session);
  if (!empty($cql_session)) {
    $cqls[] = $cql_session;
  }

  // Add sort.
  $cqls[] = strtr('sortBy @order/@sort', array('@order' => $order, '@sort' => drupal_strtolower($sort) == 'asc' ? 'ascending' : 'descending'));

  // Add params.
  $params['cql'] = implode(' ', $cqls);
  $params['limit'] = $limit;
  $params['offset'] = $page * $limit;

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
    array('data' => t('Job ID'), 'class' => array('simpletest_test')),
    array('data' => t('Asset ID'), 'class' => array('simpletest_test')),
    array('data' => t('Mediafile ID'), 'class' => array('simpletest_test')),
    array('data' => t('Filename'), 'class' => array('simpletest_test')),
    array('data' => t('Date started'), 'class' => array('simpletest_test')),
    array('data' => t('App ID'), 'class' => array('simpletest_test')),
    array('data' => t('Description'), 'class' => array('simpletest_description')),
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

      // Store test title and description so that checkbox won't render them.
      $job_id = $test['#job_id'];
      $filename = $test['#filename'];
      $asset_id = $test['#asset_id'];
      $mediafile_id = $test['#mediafile_id'];
      $app_id = $test['#app_id'];
      $date_started = $test['#date_started'];
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
    return '<h1>No failed transcoding jobs...</h1>';
  }
  else {
    $title = '<h1>' . check_plain($table['#title']) . '</h1>';
    $description = '<p>' . check_plain($table['#description']) . '</p>';

    $buildtable = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'simpletest-form-table')));

    $caption_bottom = '<div align="right">' . check_plain($table['#item_count_total']) . '</div>';
    $complete_table = $title . $description . $buildtable . $caption_bottom;

    return  $complete_table;
  }
}