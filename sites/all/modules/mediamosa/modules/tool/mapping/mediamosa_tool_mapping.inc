<?php

/**
 * @file
 * MediaMosa tool mapping function.
 */

/**
 * Show the listing of the current tool mapping.
 */
function _mediamosa_tool_mapping_list() {

  // Reset the breadcrum.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Configuration'), 'admin/mediamosa/config');
  drupal_set_breadcrumb($breadcrumb);

  $header = array(
    array('data' => t('Name'), 'field' => 'tp.' . mediamosa_tool_mapping_db::PARAMETER),
    array('data' => t('Tool'), 'field' => 'tp.' . mediamosa_tool_mapping_db::TOOL),
    array('data' => t('Asset metadata')),
    t('Operations')
  );

  $query = mediamosa_db::db_select(mediamosa_tool_mapping_db::TABLE_NAME, 'tp')->extend('PagerDefault')->extend('TableSort');
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
    if (node_access('update', $row[mediamosa_tool_mapping_db::NID])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/tool_mapping/' . $row[mediamosa_tool_mapping_db::NID] . '/edit', $l_options);
    }
    if (node_access('delete', $row[mediamosa_tool_mapping_db::NID])) {
      $actions[] = l(t('Delete'), 'admin/mediamosa/config/tool_mapping/' . $row[mediamosa_tool_mapping_db::NID] . '/delete', $l_options);
    }

    $rows[] = array(
      'parameter' => array('data' => l($row[mediamosa_tool_mapping_db::PARAMETER], 'admin/mediamosa/config/tool_mapping/' . $row[mediamosa_tool_mapping_db::NID]), 'field' => $row[mediamosa_tool_mapping_db::PARAMETER]),
      'tool' => $row[mediamosa_tool_mapping_db::TOOL],
      'asset_metadata' => mediamosa_tool_mapping::get_asset_metadata_display_name($row[mediamosa_tool_mapping_db::PROP_ID]),
      empty($actions) ? '-' : implode(' | ', $actions),
    );
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('You have no tool mapping. Click on the <em>Add</em> tab to create one.')));
  $output .= theme('pager');
  return $output;
}
