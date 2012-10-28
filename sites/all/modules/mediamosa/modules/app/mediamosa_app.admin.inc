<?php

/**
 * @file
 * Admin page callbacks for the MediaMosa App module.
 */

/**
 * Show a listing of apps.
 */
function _mediamosa_app_list() {

  global $user;

  // Get all allowed apps.
  $app_allowed = mediamosa_app::collect_allowed_apps();
  $access_app_ids = array_keys($app_allowed);

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  // Our header.
  $header = array(
    array('data' => t('App ID'), 'field' => 'a.' . mediamosa_app_db::APP_ID),
    array('data' => t('Name'), 'field' => 'a.' . mediamosa_app_db::APP_NAME),
    array('data' => t('Quota'), 'field' => 'a.' . mediamosa_app_db::QUOTA),
    t('Offline webservices'),
    array('data' => t('Owner'), 'field' => 'a.' . mediamosa_app_db::OWNER),
    array('data' => t('Status'), 'field' => 'a.' . mediamosa_app_db::ACTIVE),
    t('Operations')
  );

  // Get the app nodes.
  $query = mediamosa_db::db_select(mediamosa_app_db::TABLE_NAME, 'a')->extend('PagerDefault')->extend('TableSort')
    ->fields('a')
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->orderByHeader($header);

  if (!empty($access_app_ids)) {
    $query->condition(mediamosa_app_db::APP_ID, $access_app_ids, 'IN');
  }
  // @FIXME: use roles etc and do not fix this with a condition.
  elseif ($user->uid != 1) { // if not admin and no rights show nothing.
    $query->condition('0', '1');
  }

  $apps = $query->execute()->fetchAll();

  // Collect the node IDs.
  $nids = array();
  foreach ($apps as $app) {
    $nids[] = $app[mediamosa_app_db::NID];
  }

  // Get the nodes.
  $nodes = node_load_multiple($nids);

  // Get all statuses of webservices per app.
  $webservice_statuses = mediamosa_webservice_app::webservice_get_all_status();

  // Get destination.
  $options = array('query' => drupal_get_destination());

  $rows = array();
  foreach ($apps as $app) {
    $app_id = $app[mediamosa_app_db::APP_ID];

    $actions = array();
    if (node_access('update', $nodes[$app[mediamosa_app_db::NID]])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/app/' . $app[mediamosa_app_db::NID] . '/edit', $options);
    }
    if (node_access('delete', $nodes[$app[mediamosa_app_db::NID]])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/app/' . $app[mediamosa_app_db::NID] . '/delete', $options);
    }

    $offline_webservices = array();
    if (isset($webservice_statuses[$app_id])) {
      foreach ($webservice_statuses[$app_id] as $handle => $status) {
        if (!$status) {
          $offline_webservices[] = $handle;
        }
      }
    }

    $rows[] = array(
      $app[mediamosa_app_db::APP_ID],
      'title' => array('data' => l($app[mediamosa_app_db::APP_NAME], 'admin/mediamosa/config/app/' . $app[mediamosa_app_db::NID]), 'field' => $app[mediamosa_app_db::APP_NAME]),
      $app[mediamosa_app_db::QUOTA],
      implode(', ', $offline_webservices),
      $app[mediamosa_app_db::OWNER],
      $app[mediamosa_app_db::ACTIVE] == mediamosa_app_db::ACTIVE_TRUE ? t('Enabled') : t('<b>Disabled</b>'),
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $form = array();
  $form['pager_top'] = array('#theme' => 'pager');
  $form['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('You have no client applications. Click on the <em>Add</em> tab to create one.')
  );
  $form['pager_bottom'] = array('#theme' => 'pager');

  return $form;
}
