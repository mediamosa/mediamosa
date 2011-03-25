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
  * MediaMosa tool params function.
  */

/**
 * Show the listing of the current tool params.
 */
function _mediamosa_tool_params_list() {

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  $header = array(
    array('data' => t('Name'), 'field' => 'tp.' . mediamosa_tool_params_db::NICE_PARAMETER),
    array('data' => t('Tool'), 'field' => 'tp.' . mediamosa_tool_params_db::TOOL),
    array('data' => t('Tool params id'), 'field' => 'tp.' . mediamosa_tool_params_db::ID, 'sort' => 'asc'),
    array('data' => t('Tool parameter'), 'field' => 'tp.' . mediamosa_tool_params_db::TOOL_PARAMETER),
    array('data' => t('Min value'), 'field' => 'tp.' . mediamosa_tool_params_db::MIN_VALUE),
    array('data' => t('Max value'), 'field' => 'tp.' . mediamosa_tool_params_db::MAX_VALUE),
    array('data' => t('Default value'), 'field' => 'tp.' . mediamosa_tool_params_db::DEFAULT_VALUE),
    array('data' => t('Required'), 'field' => 'tp.' . mediamosa_tool_params_db::REQUIRED),
    array('data' => t('Type'), 'field' => 'tp.' . mediamosa_tool_params_db::TYPE_PARAMETER),
    t('Operations')
  );

  $query = mediamosa_db::db_select(mediamosa_tool_params_db::TABLE_NAME, 'tp')->extend('PagerDefault')->extend('TableSort');
  $result = $query
    ->fields('tp')
    ->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  $rows = array();
  foreach ($result as $row) {
    $actions = array();
    if (node_access('update', $row[mediamosa_tool_params_db::NID])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/tool_params/' . $row[mediamosa_tool_params_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $row[mediamosa_tool_params_db::NID])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/tool_params/' . $row[mediamosa_tool_params_db::NID] . '/delete', $l_options);
    }

    $rows[] = array(
      'nice_parameter' => array('data' => l($row[mediamosa_tool_params_db::NICE_PARAMETER], 'admin/mediamosa/config/tool_params/' . $row[mediamosa_tool_params_db::NID]), 'field' => $row[mediamosa_tool_params_db::NICE_PARAMETER]),
      'tool' => $row[mediamosa_tool_params_db::TOOL],
      'tool_params_id' => $row[mediamosa_tool_params_db::ID],
      'tool_parameter' => $row[mediamosa_tool_params_db::TOOL_PARAMETER],
      'min_value' => $row[mediamosa_tool_params_db::MIN_VALUE],
      'max_value' => $row[mediamosa_tool_params_db::MAX_VALUE],
      'default_value' => $row[mediamosa_tool_params_db::DEFAULT_VALUE],
      'required' => $row[mediamosa_tool_params_db::REQUIRED],
      'type' => $row[mediamosa_tool_params_db::TYPE_PARAMETER],
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no tool params. Click on the <em>Add</em> tab to create one.')));
  $output .= theme('pager');
  return $output;
}
