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
 * Deviant length transcoding browser.
 */

define('MEDIAMOSA_BROWSE_TRANSCODE_LENGTH_DEVIATION_PREFIX', 'mediafile_id-');

/**
 * Build the form for browsing transcode length deviation.
 */
function mediamosa_browse_transcode_length_deviation_page($form) {
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  $build = array();

  $build['deviant_length_transcoding_list'] = _mediamosa_browse_transcode_length_deviation_list_form();

  return $build;
}

/**
 * Form builder; deviant length listing page.
 */
function _mediamosa_browse_transcode_length_deviation_list_form() {

  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $limit = mediamosa_settings::maintenance_items_per_page();

  $result_array = _mediamosa_browse_transcode_length_deviation_execute($page, $limit);
  $item_count_total = $result_array['found_rows'];
  $result = $result_array['result'];

  $rows = array();

  $form['devlen']['table'] = array(
    '#theme' => 'mediamosa_browse_transcode_length_deviation',
    '#header' => '',
    '#showhidehidden' => '',
    '#title' => t('Deviant length transcodings (@total)' , array('@total' => $item_count_total)),
    '#description' => t('Below list displays transcodings in the jobs table that deviate 10% or more to their original.'),
    '#item_count_total_text' => t('Deviant length transcodings found: @total', array('@total' => $item_count_total)),
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
  );

  foreach ($result as $row) {
    $form['devlen']['table'][$row['asset_id']]['mediafile_id-' . $row['trans_mediafile_id']] = array(
      '#type' => 'checkbox',
      '#org_sec' => $row['org_sec'],
      '#trans_sec' => $row['trans_sec'],
      '#asset_id' => $row['asset_id'],
      '#mediafile_id' => $row['mediafile_id'],
      '#trans_mediafile_id' => $row['trans_mediafile_id'],
    );
  }

  // Retranscode buttons.
  if (isset($result)) {
    $form['devlen']['retranscode'] = array(
      '#type' => 'submit',
      '#value' => t('Retranscode'),
    );
  }

  return $form;
}

function mediamosa_browse_transcode_length_deviation_page_submit($form, $form_state) {
  $mediafiles = array();
  $op = $form_state['values']['op'];

  $prefix = MEDIAMOSA_BROWSE_TRANSCODE_LENGTH_DEVIATION_PREFIX;
  $prefix_length = drupal_strlen($prefix);

  foreach ($form_state['values'] as $form_key => $form_value) {
    if (drupal_substr($form_key, 0, $prefix_length) == $prefix && $form_value == 1) {
      $mediafiles[] = substr($form_key, $prefix_length);
    }
  }

  switch ($op) {
    case t('Retranscode'):
      _mediamosa_browse_transcode_length_deviation_retranscode($mediafiles);
      break;
  }
}

function _mediamosa_browse_transcode_length_deviation_retranscode($mediafiles = array()) {
  $all_ok = TRUE;

  foreach ($mediafiles as $mediafile_id) {
    $mediafile_details = mediamosa_asset_mediafile::get($mediafile_id);
    $asset_id = $mediafile_details['asset_id'];
    if ($all_ok) {
      $all_ok = _mediamosa_browse_transcode_length_deviation_process_retranscode($asset_id, $mediafile_id);
    }
  }

  if ($all_ok) {
    if (count($mediafiles) > 0) {
      drupal_set_message(t('All selected mediafile succesfully set for re-transcode.'));
    }
    else {
      drupal_set_message(t('No mediafile(s) selected.'));
    }
  }
  else {
    drupal_set_message(t('Error(s) during setting of retranscode.'));
  }
}

function _mediamosa_browse_transcode_length_deviation_process_retranscode($asset_id, $mediafile_id) {
  // REST uri.
  $uri = strtr('mediafile/@mediafile_id/retranscode', array('@mediafile_id' => $mediafile_id));

  // Get the asset.
  $asset = mediamosa_asset::get($asset_id);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']);
    $result = mediamosa_response_connector::static_do_restcall_drupal($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      return TRUE;
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to start retranscode for mediafile @mediafile_id', array('@mediafile_id' => $mediafile_id,)));
  return FALSE;
}

/**
 * Execute the query.
 *
 * @param $page
 *
 * @return array($found_rows, $result)
 */
function _mediamosa_browse_transcode_length_deviation_execute($page, $limit) {
  // Prop ids.
  $prop_id_file_duration = mediamosa_asset_mediafile_metadata_property::get_property_id_char('file_duration');

  // Query.
  $result = mediamosa_db::db_query_range("
    SELECT
      SQL_CALC_FOUND_ROWS
      org.#asset_id AS asset_id,
      org.#mediafile_id AS mediafile_id,
      (SUBSTR(meta.#val_char,1,2) * 3600) + (SUBSTR(meta.#val_char,4,2) * 60) + SUBSTR(meta.#val_char,7,2) AS org_sec,
      trans.#mediafile_id as trans_mediafile_id,
      (SUBSTR(meta2.#val_char,1,2) * 3600) + (SUBSTR(meta2.#val_char,4,2) * 60) + SUBSTR(meta2.#val_char,7,2) AS trans_sec

    FROM {#mediamosa_asset_mediafile} AS org
    JOIN {#mediamosa_asset_mediafile_metadata} AS meta ON (meta.#mediafile_id_metadata = org.#mediafile_id AND meta.#prop_id = :prop_id)
    JOIN {#mediamosa_asset_mediafile} AS trans ON (trans.#asset_id = org.#asset_id AND trans.#mediafile_id_source = org.#mediafile_id)
    LEFT JOIN {#mediamosa_asset_mediafile_metadata} AS meta2 ON (trans.#mediafile_id = meta2.#mediafile_id_metadata AND meta2.#prop_id = :prop_id)

    WHERE org.#is_original_file = :is_original_file_true AND trans.#is_original_file = :is_original_file_false AND org.#is_still = :is_still_false AND trans.#is_still = :is_still_false AND meta.#val_char IS NOT NULL
    HAVING trans_sec IS NULL OR ABS(org_sec - trans_sec) > (org_sec / 10)
    ORDER BY ABS(org_sec - trans_sec) DESC
  ", $page * $limit, $limit, array(
    '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
    '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
    '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
    '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
    '#mediafile_id_metadata' => mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
    '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
    '#val_char' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
    '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
    '#is_original_file' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
    '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
    ':is_original_file_true' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
    ':is_original_file_false' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE,
    ':is_still_false' => mediamosa_asset_mediafile_db::IS_STILL_FALSE,
    ':prop_id' => $prop_id_file_duration,
  ));

  $found_rows = mediamosa_db::db_found_rows();

  return array(
    'found_rows' => $found_rows,
    'result' => $result,
  );
}

/**
 * Theme.
 */
function theme_mediamosa_browse_transcode_length_deviation($variables) {

  $table = $variables['table'];

  drupal_add_css(drupal_get_path('module', 'simpletest') . '/simpletest.css');
  drupal_add_js(drupal_get_path('module', 'simpletest') . '/simpletest.js');
  drupal_add_js('misc/tableselect.js');

  // Create header for test selection table.
  $header = array(
    array('class' => array('select-all')),
    array('data' => t('Asset ID'), 'class' => array('mmjobid')),
    array('data' => t('Mediafile ID'), 'class' => array('mmjobtype')),
    array('data' => t('Transcoded Mediafile ID'), 'class' => array('mmjobtype')),
    array('data' => t('Original time'), 'class' => array('mmassetid')),
    array('data' => t('Transcoded time'), 'class' => array('mmmediafileid')),
  );

   // Cycle through each deviant and create rows.
  $rows = array();
  foreach (element_children($table) as $key) {
    $element = $table[$key];

    // Cycle through each result within the current group.
    foreach (element_children($element) as $deviant_name) {
      $deviant = $element[$deviant_name];
      $row = array();

      $org_sec = $deviant['#org_sec'];
      $trans_sec = $deviant['#trans_sec'];
      $asset_id = $deviant['#asset_id'];
      $mediafile_id = $deviant['#mediafile_id'];
      $trans_mediafile_id = $deviant['#trans_mediafile_id'];

      $deviant['#title_display'] = 'invisible';
      unset($deviant['#description']);

      // Test name is used to determine what tests to run.
      $deviant['#name'] = $deviant_name;

      $row[] = array(
        'data' => drupal_render($deviant),
        'class' => array('simpletest-test-select'),
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
        'data' => '<div>' . check_plain($trans_mediafile_id) . '</div>',
        'style' => array('simpletest-test-label'),
      );
      $row[] = array(
        'data' => '<div class="description">' . check_plain($org_sec) . '</div>',
        'style' => 'width:100px',
      );
      $row[] = array(
        'data' => '<div class="description">' . check_plain($trans_sec) . '</div>',
        'style' => 'width:100px',
      );
      $rows[] = array('data' => $row);
    }
  }

  if (empty($rows)) {
    return '<h1>No deviant length transcoding jobs...</h1>';
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