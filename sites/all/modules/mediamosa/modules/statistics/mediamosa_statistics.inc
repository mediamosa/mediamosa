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
 * Common functions.
 */

define('MEDIAMOSA_STATISTICS_FILTER_TYPE_MEDIAFILE_LATEST', 'mediafile_latest');
define('MEDIAMOSA_STATISTICS_FILTER_TYPE_MEDIAFILE_POPULAR', 'mediafile_popular');
define('MEDIAMOSA_STATISTICS_FILTER_TYPE_COLLECTION_LATEST', 'collection_latest');
define('MEDIAMOSA_STATISTICS_FILTER_TYPE_COLLECTION_POPULAR', 'colletion_popular');
define('MEDIAMOSA_STATISTICS_FILTER_TYPE_DISKSPACE', 'diskspace');
define('MEDIAMOSA_STATISTICS_FILTER_TYPE_STREAMREQUESTS', 'streamrequests');
define('MEDIAMOSA_STATISTICS_FILTER_TYPE_POPULARSTREAMS', 'popularstreams');
define('MEDIAMOSA_STATISTICS_FILTER_TYPE_SEARCHEDWORDS', 'searchwords');

/**
 * Form builder; Return form for mediafile filters.
 *
 * @see mediamosa_statistics_list_form()
 */
function mediamosa_statistics_filter_form($form, $form_state) {

  assert(!empty($form_state['build_info']['args'][0]));
  $type = empty($form_state['build_info']['args'][0]) ? '' : $form_state['build_info']['args'][0];

  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filters'),
    '#theme' => 'mediamosa_statistics_filters',
  );
  drupal_add_css(drupal_get_path('module', 'mediamosa_statistics') . '/mediamosa_statistics.css');

  // Get session.
  $session = isset($_SESSION['mediamosa_statistics_filter'][$type]) ? $_SESSION['mediamosa_statistics_filter'][$type] : array();

  switch ($type) {
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_MEDIAFILE_LATEST:
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_MEDIAFILE_POPULAR:
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_COLLECTION_LATEST:
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_COLLECTION_POPULAR:
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_DISKSPACE:
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_POPULARSTREAMS:
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_STREAMREQUESTS:
      // Get all allowed apps.
      $apps = mediamosa_app::collect_allowed_apps();

      $form['filters']['status']['app_id'] = array(
        '#type' => 'select',
        '#options' => array('' => '') + $apps,
        '#title' => 'Application',
        '#default_value' => !empty($session['app_id']) ? $session['app_id'] : '',
      );
      break;
  }

  $years = $months = $days = $hours = array();

  for ($y = 1970; $y <= date('Y'); $y++) {
    $years[$y] = $y;
  }

  for ($m = 1; $m <= 12; $m++) {
    $months[$m] = $m;
  }

  for ($d = 1; $d <= 31; $d++) {
    $days[$d] = $d;
  }

  for ($h = 1; $h <= 24; $h++) {
    $hours[$h] = $h;
  }

  switch ($type) {
    case MEDIAMOSA_STATISTICS_FILTER_TYPE_DISKSPACE:
      $form['filters']['status']['year'] = array(
        '#type' => 'select',
        '#options' => $years,
        '#title' => 'Year',
        '#default_value' => !empty($session['year']) ? $session['year'] : date('Y'),
      );
      $form['filters']['status']['month'] = array(
        '#type' => 'select',
        '#options' => $months,
        '#title' => 'Month',
        '#default_value' => !empty($session['month']) ? $session['month'] : date('n'),
      );

      $types = array(
        mediamosa_statistics_diskspace_used_db::TYPE_CONTAINER => t('Container'),
        mediamosa_statistics_diskspace_used_db::TYPE_GROUP => t('Group'),
        mediamosa_statistics_diskspace_used_db::TYPE_USER => t('User')
      );

      $form['filters']['status']['type'] = array(
        '#type' => 'select',
        '#options' => $types,
        '#title' => 'Type',
        '#default_value' => !empty($session['type']) ? $session['type'] : mediamosa_statistics_diskspace_used_db::TYPE_CONTAINER,
      );

      break;

      case MEDIAMOSA_STATISTICS_FILTER_TYPE_STREAMREQUESTS:
        $form['filters']['status']['year'] = array(
          '#type' => 'select',
          '#options' => $years,
          '#title' => 'Year',
          '#default_value' => !empty($session['year']) ? $session['year'] : date('Y'),
        );
        $form['filters']['status']['month'] = array(
          '#type' => 'select',
          '#options' => $months,
          '#title' => 'Month',
          '#default_value' => !empty($session['month']) ? $session['month'] : date('n'),
        );
        /*
        // Later development.
        $form['filters']['status']['day'] = array(
          '#type' => 'select',
          '#options' => $days,
          '#title' => 'Day',
          '#default_value' => !empty($session['day']) ? $session['day'] : date('j'),
        );
        $form['filters']['status']['hour'] = array(
          '#type' => 'select',
          '#options' => $hours,
          '#title' => 'Hour',
          '#default_value' => !empty($session['hour']) ? $session['hour'] : date('G'),
        );
         */
        break;

      case MEDIAMOSA_STATISTICS_FILTER_TYPE_SEARCHEDWORDS:
      case MEDIAMOSA_STATISTICS_FILTER_TYPE_POPULARSTREAMS:
        $form['filters']['status']['year'] = array(
          '#type' => 'select',
          '#options' => $years,
          '#title' => 'Year',
          '#default_value' => !empty($session['year']) ? $session['year'] : date('Y'),
        );
        $form['filters']['status']['month'] = array(
          '#type' => 'select',
          '#options' => $months,
          '#title' => 'Month',
          '#default_value' => !empty($session['month']) ? $session['month'] : date('n'),
        );

        $year_end = !empty($session['year_end']) ? $session['year_end'] : date('Y');
        $month_end = !empty($session['month_end']) ? $session['month_end'] : date('n') + 1;
        if ($month_end > 12) {
          $month_end = 1;
          $year_end = date('Y') + 1;
        }

        $form['filters']['status']['year_end'] = array(
          '#type' => 'select',
          '#options' => $years,
          '#title' => 'Year end',
          '#default_value' => $year_end,
        );


        $form['filters']['status']['month_end'] = array(
          '#type' => 'select',
          '#options' => $months,
          '#title' => 'Month end',
          '#default_value' => $month_end,
        );
        break;
  }

  $form['filters']['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions', 'container-inline')),
    '#id' => 'mediamosa-statistics-buttons',
  );
  $form['filters']['actions']['submit'] = array('#type' => 'submit', '#value' => t('Filter'));
  $form['filters']['filter_type'] = array('#type' => 'hidden', '#value' => $type);

  $form['#submit'][] = 'mediamosa_statistics_filter_form_submit';

  return $form;
}

function mediamosa_statistics_filter_form_submit($form, &$form_state) {
  $_SESSION['mediamosa_statistics_filter'][$form_state['values']['filter_type']] = $form_state['values'];
}

/**
 * Create pager.
 *
 * @param integer $item_offset
 * @param integer $item_count_total
 * @param integer $item_limit
 * @param integer $element
 */
function _mediamosa_pager($item_offset, $item_count_total, $item_limit = mediamosa_settings::LIMIT_DEFAULT_STATISTICS, $element = 0) {

  $pager_variables['page'] = ceil($item_offset / $item_limit);
  $pager_variables['item_count_total'] = $item_count_total;
  $pager_variables['item_limit'] = $item_limit;
  $pager_variables['quantity'] = $item_limit;
  $pager_variables['element'] = $element;

  return theme('mediamosa_maintenance_pager', $pager_variables);
}
