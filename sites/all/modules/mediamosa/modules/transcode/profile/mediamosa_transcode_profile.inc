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
  * MediaMosa transcode profile function.
  */

/**
 * Collect the client apps as app_id => title.
 */
function _mediamosa_get_apps() {
  // Get the apps for the drop down listing.
  $apps = array(0 => t('Global profile (no owner)'));

  // Gather all apps
  foreach (mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME), mediamosa_app_db::APP_NAME, 'ASC') as $app) {
    if (!user_access(mediamosa_permission::ADMIN_MEDIAMOSA) && !_mediamosa_user_access(mediamosa_permission::ADMIN_MEDIAMOSA . '|app|' . $app[mediamosa_app_db::APP_ID])) {
      continue;
    }

    $apps[$app[mediamosa_app_db::APP_ID]] = $app[mediamosa_app_db::APP_NAME];
  }

  return $apps;
}

/**
 * Show the listing of the current transcode profiles.
 */
function _mediamosa_transcode_profile_list() {

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  // Get the apps.
  $apps = _mediamosa_get_apps();

  $app_ids = array();

  // Only allow either all apps (admin) or limited to global and owned.
  if (!user_access(mediamosa_permission::ADMIN_MEDIAMOSA)) {
    $app_ids[] = 0; // Global profiles (allowed to read).
    foreach ($apps as $app_id => $app) {
      $app_ids[] = $app_id;
    }
  }

  $header = array(
    array('data' => t('Profile'), 'field' => 'tp.' . mediamosa_transcode_profile_db::PROFILE),
    array('data' => t('Transcode profile id'), 'field' => 'tp.' . mediamosa_transcode_profile_db::ID),
    array('data' => t('Is Default'), 'field' => 'tp.' . mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE),
    array('data' => t('Tool'), 'field' => 'tp.' . mediamosa_transcode_profile_db::TOOL),
    array('data' => t('Owner'), 'field' => 'tp.' . mediamosa_transcode_profile_db::APP_ID),
    t('Operations')
  );

  $query = mediamosa_db::db_select(mediamosa_transcode_profile_db::TABLE_NAME, 'tp')->extend('PagerDefault')->extend('TableSort');
  $query->fields('tp');

  // Limit apps?
  if (!empty($app_ids)) {
    $query->condition(mediamosa_transcode_profile_db::APP_ID, $app_ids, 'IN');
  }

  $transcode_profiles = $query->orderByHeader($header)
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->execute()->fetchAll();

  // Collect the node IDs.
  $nids = array();
  foreach ($transcode_profiles as $transcode_profile) {
    $nids[] = $transcode_profile[mediamosa_app_db::NID];
  }

  // Get the nodes.
  $nodes = node_load_multiple($nids);

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  $rows = array();
  foreach ($transcode_profiles as $transcode_profile) {
    $actions = array();
    if (node_access('update', $nodes[$transcode_profile[mediamosa_transcode_profile_db::NID]])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/transcode_profile/' . $transcode_profile[mediamosa_transcode_profile_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $nodes[$transcode_profile[mediamosa_transcode_profile_db::NID]])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/transcode_profile/' . $transcode_profile[mediamosa_transcode_profile_db::NID] . '/delete', $l_options);
    }

    $owner = isset($apps[$transcode_profile[mediamosa_transcode_profile_db::APP_ID]]) ? $apps[$transcode_profile[mediamosa_transcode_profile_db::APP_ID]] : t('Unknown owner');
    $rows[] = array(
      'title' => array('data' => l($transcode_profile[mediamosa_transcode_profile_db::PROFILE], 'admin/mediamosa/config/transcode_profile/' . $transcode_profile[mediamosa_transcode_profile_db::NID]), 'field' => $transcode_profile[mediamosa_transcode_profile_db::PROFILE]),
      'transcode_profile_id' => $transcode_profile[mediamosa_transcode_profile_db::ID],
      'description' => $transcode_profile[mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE],
      'tool' => $transcode_profile[mediamosa_transcode_profile_db::TOOL],
      'owner' => $owner,
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $output = array();
  $output[] = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no transcode profiles. Click on the <em>Add</em> tab to create one.')));
  $output[] = theme('pager');
  return implode('', $output);
}
