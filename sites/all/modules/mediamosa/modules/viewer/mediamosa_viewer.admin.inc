<?php
/**
 * @file
 * Administration page callbacks for the MediaMosa viewer module.
 */

function _mediamosa_viewer_list() {
  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  $rows = array();

  $header = array(
    array('data' => t('Name'), 'field' => 'ms.' . mediamosa_viewer_db::NAME, 'sort' => 'asc'),
    array('data' => t('Description'), 'field' => 'ms.' . mediamosa_viewer_db::DESCRIPTION),
    t('Operations'),
  );

  // Get the servers.
  $viewers = mediamosa_db::db_select(mediamosa_viewer_db::TABLE_NAME, 'ms')->extend('PagerDefault')->extend('TableSort')
    ->fields('ms')
    ->limit(mediamosa_settings::maintenance_items_per_page())
    ->orderByHeader($header)
    ->execute()
    ->fetchAll();

  // Collect the node IDs.
  $nids = array();
    foreach ($viewers as $viewer) {
    $nids[] = $viewer[mediamosa_viewer_db::NID];
  }

  // Get the nodes.
  $nodes = node_load_multiple($nids);

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  foreach ($viewers as $viewer) {
    $actions = array();
    if (node_access('update', $nodes[$viewer[mediamosa_viewer_db::NID]])) {
      $actions[] = l(t('Edit'), 'node/' . $viewer[mediamosa_viewer_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $nodes[$viewer[mediamosa_viewer_db::NID]])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/viewer/' . $viewer[mediamosa_viewer_db::NID] . '/delete', $l_options);
    }

    $rows[] = array('data' =>
      array(
        // Cells
        $viewer[mediamosa_viewer_db::NAME],
        $viewer[mediamosa_viewer_db::DESCRIPTION],
        empty($actions) ? '-' : implode(' | ', $actions),
      ),
    );
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no viewer specified. Click on the <em>Add</em> tab to create one.')));
  $output .= theme('pager');

  return $output;
}
