<?php
/**
 * @file
 * MediaMosa admin code.
 */

/**
 * Provide a single block from the administration menu as a page.
 * This function is often a destination for these blocks.
 * For example, 'admin/structure/types' needs to have a destination to be valid
 * in the Drupal menu system, but too much information there might be
 * hidden, so we supply the contents of the block.
 *
 * @return
 *   The output HTML.
 */
function _mediamosa_maintenance_admin_page($path) {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  drupal_set_breadcrumb($breadcrumb);

  $content = _mediamosa_maintenance_admin_menu_block(menu_get_item($path));
  if ($content) {
    $output = theme('admin_block_content', array('content' => $content));
  }
  else {
    $output = t('You do not have any administrative items.');
  }
  return $output;
}

/**
 * Provide a single block on the administration overview page.
 *
 * Cloned from system_admin_menu_block().
 *
 * @param $item
 *   The menu item to be displayed.
 */
function _mediamosa_maintenance_admin_menu_block($item) {
  $cache = &drupal_static(__FUNCTION__, array());
  if (!isset($item['mlid'])) {
    $item += db_query("SELECT mlid, menu_name FROM {menu_links} ml WHERE ml.router_path = :path AND module = 'system'", array(':path' => $item['path']))->fetchAssoc();
  }

  if (isset($cache[$item['mlid']])) {
    return $cache[$item['mlid']];
  }

  $content = array();
  $query = db_select('menu_links', 'ml', array('fetch' => PDO::FETCH_ASSOC));
  $query->join('menu_router', 'm', 'm.path = ml.router_path');
  $query
    ->fields('ml')
    // Weight should be taken from {menu_links}, not {menu_router}.
    ->fields('m', array_diff(drupal_schema_fields_sql('menu_router'), array('weight')))
    ->condition('ml.plid', $item['mlid'])
    ->condition('ml.menu_name', $item['menu_name'])
    ->condition('ml.hidden', 0);

  foreach ($query->execute() as $link) {
    _menu_link_translate($link);
    if ($link['access']) {
      // The link description, either derived from 'description' in
      // hook_menu() or customized via menu module is used as title attribute.
      if (!empty($link['localized_options']['attributes']['title'])) {
        $link['description'] = $link['localized_options']['attributes']['title'];
        unset($link['localized_options']['attributes']['title']);
      }
      // Prepare for sorting as in function _menu_tree_check_access().
      // The weight is offset so it is always positive, with a uniform 5-digits.
      $key = (50000 + $link['weight']) . ' ' . drupal_strtolower($link['title']) . ' ' . $link['mlid'];
      $content[$key] = $link;
    }
  }
  ksort($content);
  $cache[$item['mlid']] = $content;
  return $content;
}

/**
 * Our default themed date.
 *
 * @param array $variables
 */
function theme_mediamosa_maintenance_date($variables) {
  $type = $variables['type'];
  $datetime = isset($variables['datetime']) ? $variables['datetime'] : NULL;
  $timestamp = isset($variables['timestamp']) ? $variables['timestamp'] : NULL;

  switch ($type) {
    case 'short':
    case 'medium':
    case 'long':
      break;

    default:
      assert(0);
      break;
  }

  $timestamp = isset($datetime) ? mediamosa_lib::iso8601date_to_unix($datetime) : $timestamp;

  return check_plain(empty($timestamp) ? t('<empty date>') : format_date($timestamp, $type));
}

/**
 * Our default themed table.
 *
 * @param array $variables
 */
function theme_mediamosa_maintenance_table($variables) {
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $pager_variables = array();
  foreach (array('page', 'item_count_total', 'item_limit', 'element', 'quantity') as $var) {
    if (isset($variables[$var])) {
      $pager_variables[$var] = $variables[$var];
    }
  }

  if (!empty($pager_variables['item_count_total'])) {
    $build['pager_top'] = theme('mediamosa_maintenance_pager', $pager_variables);
  }

  if (!empty($variables['title'])) {
    $build['title'] = '<h1>' . check_plain($variables['title']) . '</h1>';
  }

  if (!empty($variables['description'])) {
    $build['description'] = '<p>' . check_plain($variables['description']) . '</p>';
  }

  // Show summary.
  if (isset($variables['summary']) && is_array($variables['summary']) && !empty($variables['summary'])) {
    foreach ($variables['summary'] as $summary_item) {
      if (isset($summary_item['title']) && isset($summary_item['data'])) {
        $variables['rows'][] = array(
          array('data' => $summary_item['title'], 'class' => 'mm-table-summary-right', 'colspan' => count($variables['header'])-1),
          array('data' => $summary_item['data'], 'class' => 'mm-table-summary'),
        );
      }
    }
  }

  $build['table'] = theme('table', $variables);

  if (!empty($variables['note'])) {
    $build['note'] = '<div align="right"><sup><small>' . check_plain($variables['note']) . '</small></sup></div>';
  }

  if (!empty($variables['caption_bottom'])) {
    $build['caption_bottom'] = '<div align="right">' . check_plain($variables['caption_bottom']) . '</div>';
  }

  if (!empty($pager_variables['item_count_total'])) {
    $build['pager_bottom'] = theme('mediamosa_maintenance_pager', $pager_variables);
  }

  return implode('', $build);
}

/**
 * Our pager wrapper.
 *
 * @param array $variables
 */
function theme_mediamosa_maintenance_pager($variables) {
  global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;

  $page = $variables['page'];
  $item_count_total = $variables['item_count_total'];
  $item_limit = $variables['item_limit'];
  $element = $variables['element'];
  $quantity = $variables['quantity'];

  $pager_page_array = array($element => $page);

  // Total items.
  $pager_total_items[$element] = $item_count_total;

  // Number of pagelinks.
  $pager_total[$element] = ceil($item_count_total / $item_limit);

  // Page limit.
  $pager_limits[$element] = $item_limit;

  return theme('pager', array('quantity' => $quantity));
}

/**
 * Create a link to a asset.
 *
 * @param $variables
 */
function theme_l_mediamosa_asset($variables) {
  return mediamosa_lib::l_asset($variables['id'], empty($variables['title']) ? t('@asset_id (asset has no title)', array('@asset_id' => $variables['id'])) : $variables['title']);
}

/**
 * Create a link to a collection.
 *
 * @param $variables
 */
function theme_l_mediamosa_collection($variables) {
  return mediamosa_lib::l_collection($variables['id'], empty($variables['title']) ? t('<collection has no title>') : $variables['title']);
}

/**
 * Confirm deletion of mediafile.
 */
function _mediamosa_maintenance_mediafile_delete($form, &$form_state, $asset_id, $mediafile_id) {
  return confirm_form(
    array(
      'asset_id' => array(
        '#type' => 'hidden',
        '#value' => $asset_id,
      ),
      'mediafile_id' => array(
        '#type' => 'hidden',
        '#value' => $mediafile_id,
      ),
    ), // form
    t('Are you sure you want to delete this mediafile?'),
    mediamosa_settings::get_url_asset($asset_id),
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

/**
 * This function deletes an asset and optionally all mediafiles within it
 */
function _mediamosa_maintenance_mediafile_delete_submit($form, &$form_state) {

  // Get ids.
  $asset_id = $form_state['values']['asset_id'];
  $mediafile_id = $form_state['values']['mediafile_id'];

  // REST uri.
  $uri = strtr('mediafile/@mediafile_id/delete', array('@mediafile_id' => $mediafile_id));

  // Get the asset.
  $asset = mediamosa_asset::get($asset_id);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']);
    $result = mediamosa_response_connector::static_do_restcall($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      drupal_set_message(t('Mediafile deleted.'));
      drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id));
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to delete mediafile.'));
}

/**
 * Start a analyse of a mediafile.
 *
 * @param $asset_id
 * @param $mediafile_id
 */
function _mediamosa_maintenance_mediafile_analyse($asset_id, $mediafile_id) {

  // REST uri.
  $uri = strtr('mediafile/@mediafile_id/analyse', array('@mediafile_id' => $mediafile_id));

  // Get the asset.
  $asset = mediamosa_asset::get($asset_id);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']);
    $result = mediamosa_response_connector::static_do_restcall_drupal($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      drupal_set_message(t('Analyse started.'));
      drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id));
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to start analyse.'));
  drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id));
}

/**
 * Start a retranscode of a mediafile.
 *
 * @param $asset_id
 * @param $mediafile_id
 */
function mediamosa_maintenance_mediafile_retranscode($asset_id, $mediafile_id) {

  // REST uri.
  $uri = strtr('mediafile/@mediafile_id/retranscode', array('@mediafile_id' => $mediafile_id));

  // Get the asset.
  $asset = mediamosa_asset::get($asset_id);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']);
    $result = mediamosa_response_connector::static_do_restcall_drupal($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      drupal_set_message(t('Retranscode started.'));
      drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id));
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to start retranscode.'));
  drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id));
}

/**
 * start download proxy call.
 */
function _mediamosa_maintenance_mediafile_download($asset_id, $mediafile_id) {

  // REST uri.
  $uri = strtr('asset/@asset_id/play', array('@asset_id' => $asset_id));

  // Get the asset.
  $asset = mediamosa_asset::get($asset_id);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id'], 'mediafile_id' => $mediafile_id, 'response' => 'download', 'is_app_admin' => 'true');
    $result = mediamosa_response_connector::static_do_restcall($uri, 'GET', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      $item = reset($result['items']);
      drupal_goto($item['output']);
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to start download.'));
  drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id));
}

/**
 * Confirm deletion of job.
 */
function _mediamosa_maintenance_job_delete($form, &$form_state, $asset_id, $job_id) {
  return confirm_form(
    array(
      'asset_id' => array(
        '#type' => 'hidden',
        '#value' => $asset_id,
      ),
      'job_id' => array(
        '#type' => 'hidden',
        '#value' => $job_id,
      ),
    ), // form
    t('Are you sure you want to delete this job?'),
    mediamosa_settings::get_url_asset($asset_id),
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

/**
 * This function deletes an asset and optionally all mediafiles within it
 */
function _mediamosa_maintenance_job_delete_submit($form, &$form_state) {

  // Get ids.
  $asset_id = $form_state['values']['asset_id'];
  $job_id = $form_state['values']['job_id'];

  // REST uri.
  $uri = strtr('job/@job_id/delete', array('@job_id' => $job_id));

  // Get the asset.
  $asset = mediamosa_asset::get($asset_id);

  if ($asset) {
    $params = array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']);
    $result = mediamosa_response_connector::static_do_restcall($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      drupal_set_message(t('Job deleted.'));
      drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id));
    }
  }

  // Set failed.
  drupal_set_message(t('Failed to delete job.'));
}
