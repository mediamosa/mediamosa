<?php
/**
 * @file
 * Administration page callbacks for the MediaMosa server module.
 */

/**
 * Show the mediamosa servers.
 */
function _mediamosa_server_list() {
  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  $rows = array();

  $header = array(
    array('data' => t('Server Type'), 'field' => 'ms.' . mediamosa_server_db::SERVER_TYPE, 'sort' => 'asc'),
    array('data' => t('Title'), 'field' => 'ms.' . mediamosa_server_db::SERVER_NAME),
    array('data' => t('URI'), 'field' => 'ms.' . mediamosa_server_db::SERVER_URI),
    array('data' => t('Path'), 'field' => 'ms.' . mediamosa_server_db::SERVER_PATH),
    array('data' => t('Status'), 'field' => 'ms.' . mediamosa_server_db::SERVER_STATUS),
    t('Operations'),
  );

  // Get the servers.
  $servers = mediamosa_db::db_select(mediamosa_server_db::TABLE_NAME, 'ms')->extend('PagerDefault')->extend('TableSort')
    ->fields('ms')
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->orderByHeader($header)
    ->execute()
    ->fetchAll();

  // Collect the node IDs.
  $nids = array();
  foreach ($servers as $server) {
    $nids[] = $server[mediamosa_server_db::NID];
  }

  // Get the nodes.
  $nodes = node_load_multiple($nids);

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  foreach ($servers as $server) {
    $actions = array();
    if (node_access('update', $nodes[$server[mediamosa_server_db::NID]])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/server/' . $server[mediamosa_server_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $nodes[$server[mediamosa_server_db::NID]])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/server/' . $server[mediamosa_server_db::NID] . '/delete', $l_options);
    }
    if (_mediamosa_user_access_server_viewer(mediamosa_permission::UPDATE_CONFIG_VIEWERS, $nodes[$server[mediamosa_server_db::NID]])) {
      $actions[] = l(t('Viewers'), 'admin/mediamosa/config/server/' . $server[mediamosa_server_db::NID] . '/viewers', $l_options);
    }

    $statuses = array(
      mediamosa_server_db::SERVER_STATUS_ON => t('On'),
      mediamosa_server_db::SERVER_STATUS_CLOSE => t('Close'),
      mediamosa_server_db::SERVER_STATUS_OFF => t('Off'),
    );

    $rows[] = array('data' =>
      array(
        // Cells
        t($server[mediamosa_server_db::SERVER_TYPE]),
       'title' => array('data' => l($server[mediamosa_server_db::SERVER_NAME], 'admin/mediamosa/config/server/' . $server[mediamosa_server_db::NID]), 'field' => mediamosa_server_db::SERVER_NAME),
        t($server[mediamosa_server_db::SERVER_URI]),
        t($server[mediamosa_server_db::SERVER_PATH]),
        $statuses[$server[mediamosa_server_db::SERVER_STATUS]],
        empty($actions) ? '-' : implode(' | ', $actions),
      ),
    );
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no servers specified. Click on the <em>Add</em> tab to create one.')));
  $output .= theme('pager');

  return $output;
}

/**
 * Form callback: the selection of viewer for server.
 *
 * @param $form
 *   An associative array containing the structure of the form.
 * @param $form_state
 *   An associative array containing the current state of the form.
 * @param $node
 *   The server node.
 *
 * @return
 *   An array representing the form definition.
 *
 * @see mediamosa_server_viewer_form_submit()
 */
function mediamosa_server_viewer_form($form, &$form_state, $node) {
  drupal_add_js(drupal_get_path('module', 'mediamosa_server') . '/mediamosa_server.js');

  $viewers = db_select(mediamosa_viewer_db::TABLE_NAME, 'v')
    ->fields('v')
    ->execute()->fetchAllAssoc(mediamosa_viewer_db::NID, PDO::FETCH_ASSOC);

  $form = array(
    '#tree' => TRUE,
    '#weight' => -20,
    'enabled' => array(),
    'disabled' => array(),
  );

  $viewer_statuses = array(
    mediamosa_server_viewer_db::STATUS_DISABLED => t('Disabled'),
    mediamosa_server_viewer_db::STATUS_ENABLED => t('Enabled'),
  );

  $statusses = array();
  foreach (array_keys($viewer_statuses) as $status) {
    $statusses[$status] = db_select(mediamosa_server_viewer_db::TABLE_NAME, 'sv')
      ->fields('sv', array(mediamosa_server_viewer_db::NID_VIEWER))
      ->condition(mediamosa_server_viewer_db::NID_SERVER, $node->nid)
      ->condition(mediamosa_server_viewer_db::STATUS, $status)
      ->execute()
      ->fetchAllAssoc(mediamosa_server_viewer_db::NID_VIEWER);
  }

  foreach ($viewers as $nid => $viewer) {
    $key = $node->nid . '-' . $nid;

    $status = empty($statusses[mediamosa_server_viewer_db::STATUS_ENABLED][$nid]) ? mediamosa_server_viewer_db::STATUS_DISABLED : mediamosa_server_viewer_db::STATUS_ENABLED;
    $form['viewers'][$status][$key]['name']['#markup'] = check_plain($viewer[mediamosa_viewer_db::NAME]);
    $form['viewers'][$status][$key]['description']['#markup'] = check_plain($viewer[mediamosa_viewer_db::DESCRIPTION]);
    $form['viewers'][$status][$key]['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#delta' => 50,
      '#default_value' => $viewer['weight'],
      '#attributes' => array('class' => array('viewer-weight', 'viewer-weight-' . strtolower($status))),
    );
    $form['viewers'][$status][$key]['status'] = array(
      '#type' => 'select',
      '#title' => t('Status'),
      '#options' => $viewer_statuses,
      '#default_value' => $status,
      '#attributes' => array('class' => array('viewer-status-select', 'viewer-status-' . strtolower($status))),
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#access' => !empty($viewers),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );

  return $form;
}

/**
 * Implements hook_submit().
 */
function mediamosa_server_viewer_form_submit($form, &$form_state) {

  $node_updated = array();
  foreach ($form_state['values']['viewers'] as $viewers) {
    foreach ($viewers as $ids => $viewer) {
      list($nid_server, $nid_viewer) = explode('-', $ids, 2);
      $mediamosa_server_viewer = db_select(mediamosa_server_viewer_db::TABLE_NAME, 'sv')
        ->fields('sv', array(mediamosa_server_viewer_db::NID_VIEWER, mediamosa_server_viewer_db::NID_SERVER))
        ->condition(mediamosa_server_viewer_db::NID_VIEWER, $nid_viewer)
        ->condition(mediamosa_server_viewer_db::NID_SERVER, $nid_server)
        ->execute()
        ->fetchAssoc();

      $node_updated[$nid_server] = $nid_server;
      if ($mediamosa_server_viewer) {
        db_update(mediamosa_server_viewer_db::TABLE_NAME)
          ->fields(array(mediamosa_server_viewer_db::STATUS => $viewer['status'], mediamosa_server_viewer_db::WEIGHT => $viewer['weight']))
          ->condition(mediamosa_server_viewer_db::NID_VIEWER, $nid_viewer)
          ->condition(mediamosa_server_viewer_db::NID_SERVER, $nid_server)
          ->execute();
      }
      else {
        db_insert(mediamosa_server_viewer_db::TABLE_NAME)
          ->fields(array(
            mediamosa_server_viewer_db::NID_VIEWER => $nid_viewer,
            mediamosa_server_viewer_db::NID_SERVER => $nid_server,
            mediamosa_server_viewer_db::STATUS => $viewer['status'],
            mediamosa_server_viewer_db::WEIGHT => $viewer['weight'],
          ))
          ->execute();
      }
    }
  }
  $nodes = node_load_multiple($node_updated);

  foreach ($nodes as $node) {
    $t_args = array('@type' => node_type_get_name($node), '%title' => $node->title);
    drupal_set_message(t('@type %title has been updated.', $t_args));
  }
}

/**
 * Returns HTML for a sync listing form.
 *
 * FIXME: Dragging does not change disabled/enabled status when target is empty.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @see bamopen_plugin_list()
 */
function theme_mediamosa_server_viewer_form($variables) {

  $form = $variables['form'];
  $viewer_statuses = array(
    mediamosa_server_viewer_db::STATUS_DISABLED => t('Disabled'),
    mediamosa_server_viewer_db::STATUS_ENABLED => t('Enabled'),
  );
  $viewers_by_status = array(
    mediamosa_server_viewer_db::STATUS_ENABLED => element_children($form['viewers'][mediamosa_server_viewer_db::STATUS_ENABLED]),
    mediamosa_server_viewer_db::STATUS_DISABLED => element_children($form['viewers'][mediamosa_server_viewer_db::STATUS_DISABLED]),
  );
  // Do not add any rows to the table if there are no viewers to display.
  $statuses = empty($viewers_by_status[mediamosa_server_viewer_db::STATUS_ENABLED]) && empty($viewers_by_status[mediamosa_server_viewer_db::STATUS_DISABLED]) ? array() : array_keys($viewers_by_status);

  $rows = array();
  foreach ($statuses as $status) {
    drupal_add_tabledrag('viewers', 'match', 'sibling', 'viewer-status-select');
    drupal_add_tabledrag('viewers', 'order', 'sibling', 'viewer-weight');
    $rows[] = array(
      'data' => array(array(
        'colspan' => 3,
        'data' => '<strong>' . $viewer_statuses[$status] . '</strong>',
      )),
      'class' => array('viewer-status-' . strtolower($status)),
    );

    foreach ($viewers_by_status[$status] as $key) {
      $viewer = &$form['viewers'][$status][$key];
      $row = array();
      $row[] = drupal_render($viewer['name']);
      $row[] = drupal_render($viewer['weight']);
      $row[] = drupal_render($viewer['status']);
      $row[] = drupal_render($viewer['description']);
      $rows[] = array(
        'data' => $row,
        'class' => array('draggable'),
      );
    }
  }

  $header = array(t('Name'), t('Weight'), t('Status'), t('Description'));
  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'viewers'), 'empty' => t('No viewers available.', array())));
  $output .= drupal_render($form['actions']);
  $output = drupal_render_children($form) . $output;
  return $output;
}
