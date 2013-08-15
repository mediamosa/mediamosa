<?php
/**
 * @file
 * Helper functions for browse pages.
 */

/**
 * Execute a REST call.
 *
 * @param string $uri
 *   The URI of the REST call.
 * @param string $method
 *   The Method (POST/GET/PUT).
 * @param array $data
 *   Array name => value.
 */
function _mediamosa_restcall($uri, $method = 'GET', array $data = array()) {

  // Wrapper.
  return mediamosa_response_connector::static_do_restcall_drupal($uri, $method, $data);
}

function _mediamosa_maintenance_browse_clean_string($string) {
  return drupal_ucfirst(str_replace('_', ' ', $string));
}

/**
 * Create one filter item.
 */
function _mediamosa_browse_searchparameters_helper($x, $values = array('item' => '', 'item_value' => '')) {
  $search = array(
    'title' => t('Title'),
    'description' => t('Description'),
    'owner_id' => t('Owner'),
    'group_id' => t('Group'),
  );

  $form = array();

  $form['item'] = array(
    '#type' => 'select',
    '#title' => t('Search item @n', array('@n' => $x + 1)),
    '#description' => t('Select field to be used for filtering.'),
    '#options' => $search,
    '#default_value' => $values['item'],
  );

  $form['item_value'] = array(
    '#type' => 'textfield',
    '#description' => t('Value for filtering on search item @n, wildcard = *', array('@n' => $x + 1)),
    '#default_value' => $values['item_value'],
  );

  return $form;
}

/**
 * Builds CQL based on session.
 *
 * @param unknown_type $session
 */
function _mediamosa_browse_cql_build($session) {
  $operators = array('or' => ' OR ', 'and' => ' AND ');

  $filters = isset($session['filters']) ? $session['filters'] : array();
  $operator = isset($session['operator']) ? $session['operator'] : 'or';
  $operator = isset($operators[$operator]) ? $operators[$operator] : ' OR ';

  // Build CQL from search items.
  $cql_filter = array();
  foreach ($filters as $filter) {
    if ($filter['item_value'] == '') {
      continue;
    }

    if (mediamosa_unicode::stripos($filter['item_value'], '*') !== FALSE) {
      $cql_filter[] = $filter['item'] . ' == "' . $filter['item_value'] . '"';
    }
    else {
      $cql_filter[] = $filter['item'] . ' == "^' . $filter['item_value'] . '^"';
    }
  }

  return implode($operator, $cql_filter);
}

/**
 * Our asset/collection detail view.
 *
 * @param array $variables
 */
function theme_mediamosa_maintenance_browse_properties($variables) {

  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $build = array();

  if (!empty($variables['description'])) {
    $build['description'] = '<p>' . check_plain($variables['description']) . '</p>';
  }

  if (!empty($variables['properties'])) {
    $header = array(
      t('Property'),
      t('Value'),
    );

    $rows = array();
    foreach ($variables['properties'] as $name => $value) {
      $rows[] = array(
        check_plain($name),
        is_array($value) ? (empty($value['html']) ? check_plain($value['data']) : $value['data']) : check_plain($value),
      );
    }

    $build['properties'] = theme('table', array('header' => $header, 'rows' => $rows));
  }

  if (!empty($variables['metadata_description'])) {
    $build['metadata_description'] = '<p>' . check_plain($variables['metadata_description']) . '</p>';
  }

  if (isset($variables['metadata'])) {
    $header = array(
      t('Property'),
      t('Value'),
    );

    foreach ($variables['metadata'] as $name => $value) {
      if (is_array($value)) {
        foreach ($value as $value_2) {
          $rows[] = array(
            check_plain($name),
            check_plain($value_2),
          );
        }
      }
      else {
        $rows[] = array(
          check_plain($name),
          check_plain($value),
        );
      }
    }

    $build['metadata'] = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('This asset has no metadata.')));
  }


  if (!empty($variables['collection_assets'])) {
    if (!empty($variables['collection_assets']['title'])) {
      $build['collection_assets_title'] = '<h1>' . check_plain($variables['collection_assets']['title']) . '</h1>';
    }

    if (!empty($variables['collection_assets']['description'])) {
      $build['collection_assets_description'] = '<p>' . $variables['collection_assets']['description'] . '</p>';
    }

    $header = array(
      t('Title'),
      array('data' => t('Operations'), 'class' => 'mmoperation'),
    );

    // Get destination.
    $l_options = array('query' => drupal_get_destination());

    $rows = array();
    if (isset($variables['collection_assets']['assets'])) {
      foreach ($variables['collection_assets']['assets'] as $asset) {
        $actions = array(
          l(t('Remove'), 'admin/mediamosa/content/collection/' . rawurlencode($variables['collection_assets']['coll_id']) . '/remove_asset/' . rawurlencode($asset['asset_id']), $l_options),
        );

        $rows[] = array(
          array('data' => theme('l_mediamosa_asset', array('id' => $asset['asset_id'], 'title' => _mediamosa_browse_asset_get_title($asset)))),
          array('data' => implode(' | ', $actions), 'class' => 'mmoperation'),
        );
      }

      $pager_variables['page'] = ceil($variables['collection_assets']['item_offset'] / 10);
      $pager_variables['item_count_total'] = $variables['collection_assets']['item_count_total'];
    }

    $pager_variables['item_limit'] = 10;
    $pager_variables['element'] = 0;
    $pager_variables['quantity'] = 10;

    $build['collection_assets_pager_top'] = theme('mediamosa_maintenance_pager', $pager_variables);

    $build['collection_assets'] = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => isset($variables['collection_assets']['empty']) ? $variables['collection_assets']['empty'] : ''));

    $build['collection_assets_pager_bottom'] = theme('mediamosa_maintenance_pager', $pager_variables);
  }

  return implode('', $build);
}

/**
 * Helper to find best title to show.
 *
 * @param array $row
 */
function _mediamosa_browse_asset_get_title($asset, $default = NULL) {
  $title = $default;

  if (isset($asset['dublin_core']['title'])) {
    $title = reset($asset['dublin_core']['title']);
  }

  if (!empty($asset['mediafile_filename'])) {
    $title = $asset['mediafile_filename'];
  }

  return $title;
}

/**
 * Fix the name for display.
 */
function _mediamosa_browse_fix_metadata_name($name) {
  // Some need some help.
  $fixed = array(
    'hasformat' => 'has format',
    'isreferencedby' => 'is referenced by',
    'isformatof' => 'is format of',
    'videotimestamp' => 'video timestamp',
    'videotimestampmodified' => 'video modified',
    'app_id_search' => 'app id',
    'title_alternative' => 'alternative title',
  );

  return drupal_ucfirst(str_replace('_', ' ', isset($fixed[$name]) ? $fixed[$name] : $name));
}

/**
 * Get Transcode profile data.
 *
 * @param int $transcode_profile_id
 */
function _mediamosa_get_transcode_profile_data($transcode_profile_id) {

  // Our static.
  $transcode_profiles = &drupal_static(__FUNCTION__, array());

  if (isset($transcode_profiles[$transcode_profile_id])) {
    return $transcode_profiles[$transcode_profile_id];
  }

  // Because we need the node ID, we can't use the REST call here.
  $transcode_profiles[$transcode_profile_id] = mediamosa_transcode_profile::get($transcode_profile_id);

  // Return it.
  return $transcode_profiles[$transcode_profile_id];
}
