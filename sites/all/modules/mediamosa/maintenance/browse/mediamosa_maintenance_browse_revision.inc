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
 * Browse the history of all mediamosa node objects changes using Drupals
 * node revisions.
 */

/**
 * node_revision_delete($revision_id)
 * Delete a node revision (not the latest).
 *
 * node_revision_list($node)
 * Returns a array with all revisions of one node.
 *
 * node_revision_overview($node)
 * Generate an overview table of older revisions of a node.
 */

/**
 * Show listing of the revisions (of our nodes).
 */
function mediamosa_maintenance_browse_revision_form() {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Content'), 'admin/mediamosa/content');
  drupal_set_breadcrumb($breadcrumb);

  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $header = array(
    array('data' => t('ID'), 'field' => 'mnr.vid', 'sort' => 'desc'),
    array('data' => t('User'), 'field' => 'nr.uid'),
    array('data' => t('Type'), 'field' => 'n.type'),
    array('data' => t('Title'), 'field' => 'n.title'),
    array('data' => t('Date'), 'field' => 'nr.timestamp', 'class' => 'mmdate'),
    array('data' => t('Type')),
  );

  $query = mediamosa_db::db_select(mediamosa_node_revision_db::TABLE_NAME, 'mnr')->extend('PagerDefault')->extend('TableSort');
  $query->join('node', 'n', 'n.nid = mnr.nid');
  $query->join('node_revision', 'nr', 'nr.nid = mnr.nid AND nr.vid = mnr.vid');
  $query->fields('mnr');
  $query->fields('nr');
  $query->fields('n');
  $query->orderByHeader($header);
  $query->limit(mediamosa_settings::maintenance_items_per_page());
  $result = $query->execute()->fetchAll();

  // Get the user ids.
  $types = $uids = array();
  foreach ($result as $revision) {
    $uids[$revision['uid']] = $revision['uid'];
    $types[$revision['type']] = $revision['type'];
  }

  // Now get the names.
  $names = mediamosa_db::db_select('users', 'u')
    ->fields('u', array('uid', 'name'))
    ->condition('uid', $uids, 'IN')
    ->execute()
    ->fetchAllKeyed();

  // Now get the node types.
  $node_types = mediamosa_db::db_select('node_type', 'nt')
    ->fields('nt', array('type', 'name'))
    ->condition('type', $types, 'IN')
    ->execute()
    ->fetchAllKeyed();

  $rows = array();

  // Build the rows.
  foreach ($result as $revision) {

    // Build link to object view.
    switch ($revision['type']) {
      case mediamosa_node::MEDIAMOSA_NODE_TYPE_SETTINGS:
        $link = l($node_types[$revision['type']], 'admin/mediamosa/config/global');
        break;
      case mediamosa_node::MEDIAMOSA_NODE_TYPE_APP:
        $link = l($node_types[$revision['type']], 'admin/mediamosa/config/app/' . $revision['nid']);
        break;
      case mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER:
        $link = l($node_types[$revision['type']], 'admin/mediamosa/config/server/' . $revision['nid']);
        break;
      case mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE:
        $link = l($node_types[$revision['type']], 'admin/mediamosa/config/storage_profile/' . $revision['nid']);
        break;
      case mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE:
        $link = l($node_types[$revision['type']], 'admin/mediamosa/config/transcode_profile/' . $revision['nid']);
        break;
      case mediamosa_node::MEDIAMOSA_NODE_TYPE_TOOL_PARAMS:
        $link = l($node_types[$revision['type']], 'admin/mediamosa/config/tool_params/' . $revision['nid']);
        break;
    }

    $rows[] = array('data' =>
      array(
        l($revision['vid'], strtr('admin/mediamosa/content/revision/@node/@revision', array('@node' => $revision['nid'], '@revision' => $revision['vid']))),
        check_plain($names[$revision['uid']]),
        $link,
        check_plain($revision['title']),
        array('data' => theme('mediamosa_maintenance_date', array('timestamp' => $revision['timestamp'])), 'class' => 'mmdate'),
        'change'
      ),
    );
  }

  $form['pager_top'] = array('#theme' => 'pager');
  $form['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('id' => 'admin-dblog'),
    '#empty' => t('There are no revisions.')
  );
  $form['pager_bottom'] = array('#theme' => 'pager');

  return $form;
}

/**
 * Show details of a revision.
 *
 * @param $vid
 *  The revision number.
 */
function _mediamosa_maintenance_view_revision($nid, $vid) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Content'), 'admin/mediamosa/content');
  $breadcrumb[] = l(t('Revision'), 'admin/mediamosa/content/revision');
  drupal_set_breadcrumb($breadcrumb);

  $node_current = node_load($nid);
  $node_revision = node_load($nid, $vid);

  // Get all vids from this node so we can figure out the previous node revision.
  $nids = db_select('node_revision', 'nr')
    ->fields('nr', array('vid'))
    ->condition('nid', $nid)
    ->orderBy('vid', 'ASC')
    ->execute()
    ->fetchCol();

  $key = array_search($vid, $nids);
  $node_previous_revision = array();
  if (isset($nids[$key - 1])) {
    $node_previous_revision = node_load($nid, $nids[$key - 1]);
  }

  // Get the fields we want to show.
  $fields_to_show = mediamosa_node::get_fields($node_current);
  $fields_to_skip = array('created', 'changed', 'command', 'vid', 'nid', 'transcode_profile_id', 'version');

  $header = array(
    array('data' => t('Key')),
    array('data' => t('Value before change (Revision @vid)', array('@vid' => (isset($node_previous_revision->vid) ? $node_previous_revision->vid : 'none'))) ),
    array('data' => t('Value after change (Revision @vid)', array('@vid' => (isset($node_revision->vid) ? $node_revision->vid : 'none')))),
    array('data' => t('Current Value (Revision @vid)', array('@vid' => (isset($node_current->vid) ? $node_current->vid : 'none')))),
  );

  $rows = array();

  foreach ($fields_to_show as $field) {
    if (in_array($field, $fields_to_skip)) {
      continue;
    }

    // Show values in.
    if (is_array($node_current->{$field})) {
      $fields = $node_current->{$field};
      foreach ($fields as $field2 => $value) {
        $rows[] = _mediamosa_maintenance_revision_line(
          $field2,
          isset($node_previous_revision->{$field}[$field2]) ? $node_previous_revision->{$field}[$field2] : '',
          isset($node_revision->{$field}[$field2]) ? $node_revision->{$field}[$field2] : '',
          isset($node_current->{$field}[$field2]) ? $node_current->{$field}[$field2] : ''
        );
      }
    }
    else {
      $rows[] = _mediamosa_maintenance_revision_line(
        $field,
        isset($node_previous_revision->{$field}) ? $node_previous_revision->{$field} : '',
        isset($node_revision->{$field}) ? $node_revision->{$field} : '',
        isset($node_current->{$field}) ? $node_current->{$field} : ''
      );
    }
  }

  // Load the user.
  $user = user_load($node_revision->uid);

  // Build output.
  $output = theme('mediamosa_maintenance_browse_revision_info', array('node' => $node_revision, 'user' => $user));
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('Unable to find changes.')));
  $output .= theme('pager');

  return $output;
}

/**
 * Helper for row line.
 *
 * @param string $field
 * @param string $previous
 * @param string $revision
 * @param string $current
 */
function _mediamosa_maintenance_revision_line($field, $previous, $revision, $current) {
  return array(
    $field,
    ($previous != $revision ? '<b>' : '') . check_plain($previous) . ($previous != $revision ? '</b>' : ''),
    ($previous != $revision ? '<b>' : '') . check_plain($revision) . ($previous != $revision ? '</b>' : ''),
    check_plain($current),
  );
}

/**
 * Show information of revision.
 *
 * @param $node
 */
function theme_mediamosa_maintenance_browse_revision_info($variables) {

  $name = 'unknown admin';
  if (isset($variables['user'])) {
    $name = $variables['user']->name;
  }

  $log = trim($variables['node']->log);

  $output[] = t('View details of a change to MediaMosa configuration settings. Differences compared to current values are displayed <b>bold</b>.');
  $output[] = t('<b>Date change</b>:') . ' ' . theme('mediamosa_maintenance_date', array('timestamp' => $variables['node']->created));
  $output[] = t('<b>Log message</b>:') . ' ' . (empty($log) ? t('no log message found.') : check_plain($log));
  $output[] = t('<b>User</b>:') . ' ' . check_plain($name);
  return '<p>' . implode('</p><p>', $output) . '</p>';
}
