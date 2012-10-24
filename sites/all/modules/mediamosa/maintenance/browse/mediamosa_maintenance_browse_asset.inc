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
 * Asset browser.
 */

define('MEDIAMOSA_BROWSE_FILTER_ASSET', 'mediafile_browse_asset');

/**
 * Build the form for browsing assets.
 */
function _mediamosa_browse_asset_page($op = 'list') {

  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  // Get/set session.
  $session = _mediamosa_browse_asset_session();

  $build = array();

  $build['asset_filter'] = drupal_get_form('mediamosa_browse_asset_filter_form');
  $build['asset_list'] = drupal_get_form('mediamosa_browse_asset_list_form');

  return $build;
}

/**
 * Form builder; Return form for asset filters.
 *
 * @see mediamosa_browse_asset_filter_form_submit()
 */
function mediamosa_browse_asset_filter_form() {
  // Get session.
  $session = _mediamosa_browse_asset_session();

  // Get all collections.
  $collections = mediamosa_collection::getAll(array(mediamosa_collection_db::ID, mediamosa_collection_db::TITLE))->fetchAllKeyed();
  foreach ($collections as $coll_id => $name) {
    if (!$coll_id || trim($name) != '') {
      continue;
    }

    // Set empty titles.
    $collections[$coll_id] = t('<collection has no name> ID: @id', array('@id' => $coll_id));
  }

  $form = array();

  $form['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
    '#collapsible' => TRUE,
    '#prefix' => '<div id="searchparameters">',
    '#suffix' => '</div>',
  );

  $form['filters']['operator'] = array(
    '#type' => 'select',
    '#title' => t('Search criteria operator'),
    '#description' => t("Filter on all search criteria ('AND') or a least one criterium ('OR')."),
    '#options' => array('and' => t('AND'), 'or' => t('OR')),
    '#default_value' => isset($session['operator']) ? $session['operator'] : 'and',
  );

  if (!empty($collections) && is_array($collections)) {
    $form['filters']['coll_id'] = array(
      '#type' => 'select',
      '#title' => t('Search within collection'),
      '#description' => t("Select collection to be searched. Leave empty for all collections."),
      '#options' => $collections,
      '#default_value' => isset($session['coll_id']) ? $session['coll_id'] : 0,
    );
  }

  $form['filters']['searchparameters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search criteria'),
  );

  $form['filters']['searchparameters']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add search item'),
    '#ajax' => array(
      'event' => 'click',
      'wrapper' => 'searchparameters',
      'path' => 'admin/mediamosa/browse/js/asset/searchparameters',
      'method' => 'replace',
    ),
    '#weight' => 10,
  );

  $form['filters']['searchparameters']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#ajax' => array(
      'event' => 'click',
      'wrapper' => 'searchparameters',
      'path' => 'admin/mediamosa/browse/js/asset/searchparameters',
      'method' => 'replace',
    ),
    '#weight' => 11,
  );

  $form['filters']['apply'] = array(
    '#type' => 'submit',
    '#value' => t('Apply filter'),
  );

  // CQL filter.
  $form['cqlfilter'] = array(
    '#type' => 'fieldset',
    '#title' => t('CQL Filter'),
    '#collapsible' => TRUE,
  );

  $form['cqlfilter']['cql'] = array(
    '#type' => 'textfield',
    //'#title' => t('CQL:'),
    '#description' => t('Enter a CQL search command here. Usage can be found in the Webservice document. Examples: "acl_app==5" or "is_protected == FALSE"'),
    '#default_value' => isset($session['cql']) ? $session['cql'] : '',
  );

  $form['cqlfilter']['apply_cql'] = array(
    '#type' => 'submit',
    '#value' => t('Apply CQL filter'),
  );

  $form = _mediamosa_browse_asset_searchparameters($form);

  return $form;
}

/**
 * Insert the search form into a text area.
 */
function _ajax_browse_asset_search_parameters() {

  // Load the form based upon the $_POST data sent via the ajax call.
  list($form, $form_state) = ajax_get_form();

  // Fix drupal notice.
  $form_state['values'] = isset($form_state['values']) ? $form_state['values'] : array();
  $form = form_builder('', _mediamosa_browse_asset_searchparameters($form, $form_state), $form_state);

  $commands[] = ajax_command_replace('', drupal_render($form['filters']));

  return array('#type' => 'ajax', '#commands' => $commands, '#header' => FALSE);
}

/**
 * Enrich the form with the filter items.
 *
 * @param array $form
 * @param array $form_state
 */
function _mediamosa_browse_asset_searchparameters($form, $form_state = array()) {

  // Get session.
  $session = _mediamosa_browse_asset_session($form_state);

  if (!isset($form_state['input']['filters'])) {
    $form_state['input']['filters'] = isset($session['filters']) ? $session['filters'] : array();
  }

  $op = isset($form_state['input']['op']) ? $form_state['input']['op'] : '';

  // Ajax input.
  if (!empty($form_state['input']['_triggering_element_name']) && $form_state['input']['_triggering_element_name'] == 'op') {
    $op = $form_state['input']['_triggering_element_value'];
  }

  switch ($op) {
    case t('Reset'):
      $session['operator'] = 'and';
      $session['coll_id'] = 0;
      $session['filters'] = array();
      $form_state['input']['filters'] = array();
      // No break.

    case t('Add search item'):
      $form_state['input']['filters'][] = array('item' => '', 'item_value' => '');
      break;

    default:
      if (empty($form_state['input']['filters'])) {
        $form_state['input']['filters'][] = array('item' => '', 'item_value' => '');
      }
      break;

    case t('Apply filter'):
      break;
  }

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET] = $session;

  unset($form['filters']['searchparameters']['filters']);
  foreach ($form_state['input']['filters'] as $x => $filter) {
    $form_filter = _mediamosa_browse_searchparameters_helper($x, $filter);

    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item]'] = $form_filter['item'];
    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item_value]'] = $form_filter['item_value'];
  }

  return $form;
}

/**
 * Process result from asset filter form.
 */
function mediamosa_browse_asset_filter_form_submit($form, $form_state) {
  _mediamosa_browse_asset_session($form_state);
}

/**
 * Form builder; asset listing page.
 *
 */
function mediamosa_browse_asset_list_form() {
  // Get session.
  $session = _mediamosa_browse_asset_session();

  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $limit = $session['limit'];
  $page = isset($_GET['page']) ? (int) $_GET['page'] : 0;
  $item_count_total = 0;

  $header = array(
    array('field' => 'title', 'data' => t('Title'), 'sort' => 'asc'),
    array('field' => 'app_id', 'data' => t('App ID'), 'class' => array('mmappid')),
    array('field' => 'changed', 'data' => t('Last modified'), 'class' => array('mmdate')),
    array('data' => t('Operations'), 'class' => array('mmoperation')),
  );

  $result = _mediamosa_browse_asset_execute();
  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  foreach ($result['items'] as $row) {
    $actions = array(
      l(t('Edit'), 'admin/mediamosa/browse/asset/' . $row[mediamosa_asset_db::ID] . '/edit', $l_options),
      l(t('Delete'), 'admin/mediamosa/browse/asset/' . $row[mediamosa_asset_db::ID] . '/delete', $l_options),
    );

    $rows[] = array(
      theme('l_mediamosa_asset', array('title' => _mediamosa_browse_asset_get_title($row), 'id' => $row[mediamosa_asset_db::ID])),
      array('data' => check_plain($row['app_id']), 'class' => array('mmappid')),
      array('data' => theme('mediamosa_maintenance_date', array('datetime' => $row[mediamosa_asset_db::VIDEOTIMESTAMP])), 'class' => array('mmdate')),
      array('data' => implode(' | ', $actions), 'class' => array('mmoperation')),
    );
  }

  // Our listing.
  $form['list'] = array(
    '#theme' => 'mediamosa_maintenance_table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No assets found.'),
    // Pager.
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
    // Top
    '#title' => t('Assets (@total)', array('@total' => $item_count_total)),
    '#description' => t('Assets matching the search criteria above are listed below. Click on a asset title below to view details.'),
    // Total note.
    '#caption_bottom' => t('Assets found: @total', array('@total' => $item_count_total))
  );

  return $form;
}

/**
 * Do the REST call and return the response object.
 */
function _mediamosa_browse_asset_execute() {
  // Get session.
  $session = _mediamosa_browse_asset_session();

  // What to sort on.
  $sort = $session['sort'];
  $order = $session['order'];
  $limit = $session['limit'];
  $page = $session['page'];
  $op = $session['op'];

  switch ($order) {
    case t('App ID'):
      $order = 'app_id';
      break;

    case t('Last modified'):
      $order = mediamosa_asset_db::VIDEOTIMESTAMP;
      break;

    case t('Title'):
    default:
      $order = 'title';
      break;
  }

  $params = array();

  $cqls = array();
  $add_sort_cql = TRUE;

  switch ($op) {
    case t('Apply filter'):
    default:
      $cql_session = _mediamosa_browse_cql_build($session);
      if (!empty($cql_session)) {
        $cqls[] = '(' . $cql_session . ')';
      }

      $coll_id = isset($session['coll_id']) ? $session['coll_id'] : 0;
      if ($coll_id) {
        $cqls[] = strtr('!ANDcoll_id == "^@coll_id^"', array('!AND' => (empty($cqls) ? '' : 'AND '), '@coll_id' => $coll_id));
      }
      break;

    case t('Apply CQL filter'):
      $cqls[] = $session['cql'];

      // Can't add sorting when its in users cql.
      $add_sort_cql = stripos($session['cql'], 'sortby ') === FALSE;
      break;
  }

  if ($add_sort_cql) {
    // Add sort.
    $cqls[] = strtr('sortBy !order/!sort', array('!order' => $order, '!sort' => drupal_strtolower($sort) == 'asc' ? 'ascending' : 'descending'));
  }

  // Add params.
  $params['cql'] = implode(' ', $cqls);
  $params['limit'] = $limit;
  $params['offset'] = $page * $limit;

  // Do the rest call.
  return mediamosa_response_connector::static_do_restcall_drupal('asset', 'GET', $params);
}

/**
 * Get and set the session.
 */
function _mediamosa_browse_asset_session($filter_form_state = array()) {
  // Get session.
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET]) ? $_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET] : array();

  // Get these from _GET if present.
  foreach (array('order', 'sort', 'page') as $param) {
    unset($session[$param]);
    if (isset($_GET[$param])) {
      $session[$param] = $_GET[$param];
    }
  }

  foreach (array('operator', 'coll_id', 'filters', 'cql', 'op') as $param) {
    if (isset($filter_form_state['input'][$param])) {
      $session[$param] = $filter_form_state['input'][$param];
    }
  }

  // Set defaults.
  $session += array(
    'order' => t('Title'),
    'sort' => 'asc',
    'page' => 0,
    'limit' => mediamosa_settings::maintenance_items_per_page(),
    'op' => t('Apply filter'),
  );

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET] = $session;

  return $session;
}

/**
 * View the details of an asset.
 */
function _mediamosa_browse_asset_page_view($asset_id) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Asset browser'), 'admin/mediamosa/browse/asset');
  drupal_set_breadcrumb($breadcrumb);

  // Need it for app_id.
  //$asset = mediamosa_asset::get($asset_id);

  // Get the asset.
  $params = array(
    'view_hidden_metadata' => 'TRUE',
  );
  $response =  mediamosa_response_connector::static_do_restcall_drupal(strtr('asset/@asset_id', array('@asset_id' => rawurlencode($asset_id))) , 'GET', $params);

  if ((int) $response['header']['request_result_id'] != mediamosa_error::ERRORCODE_OKAY) {
    return '<h1>Asset not found</h1>';
  }

  // Get the asset.
  $asset = reset($response['items']);

  // properties.
  $build['properties'] = _mediamosa_browse_asset_page_view_properties($asset);

  // job list.
  $build['joblist'] = _mediamosa_browse_asset_page_view_job_list($asset);

  // Mediafiles.
  $build['mediafiles'] = _mediamosa_browse_asset_page_view_mediafile_list($asset);

  // Collections.
  $build['collections'] = _mediamosa_browse_asset_page_view_collections_list($asset);

  // Supplements.
  $supplements = _mediamosa_browse_asset_page_view_supplements_list($asset);
  if ($supplements) {
    $build['supplements'] = $supplements;
  }

  return implode('', $build);
}

/**
 * Build the properties.
 *
 * @param array $asset
 */
function _mediamosa_browse_asset_page_view_properties($asset) {

  // Get all metadata properties for this app.
  $metadatas = mediamosa_asset_metadata_property::get_metadata_properties_full($asset[mediamosa_asset_db::APP_ID]);

  // Collect the metadata groups.
  $metadata_groups = array();
  foreach ($metadatas as $name => $metadata) {
    $metadata_groups[] = $metadata['propgroup_name'];
  }

  $variables = array(
    'description' => t('Details for this asset are shown below.'),
    'properties' => array(
      t('App ID') => $asset[mediamosa_asset_db::APP_ID],
      t('Owner ID') => $asset[mediamosa_asset_db::OWNER_ID],
      t('Group ID') => $asset[mediamosa_asset_db::GROUP_ID],
    ),
    'metadata_description' => t('Metadata properties for the asset are shown below.'),
    'metadata' => array(
      t('Created / Changed') => $asset['videotimestamp'] . '/' . $asset['videotimestampmodified']
    ),
  );

  foreach ($metadata_groups as $metadata_group) {
    if (empty($asset[$metadata_group])) {
      continue;
    }

    foreach ($asset[$metadata_group] as $name => $value) {
      if (empty($value)) {
        continue;
      }

      $variables['metadata'][t($name) . ($metadatas[$name]['propdef_is_hidden'] == mediamosa_asset_metadata_property_db::IS_HIDDEN_TRUE ? ' [' . t('hidden metadata') . ']' : '')] = $value;
    }
  }

  // Uri REST call.
  $uri = strtr('asset/!asset_id/acl', array('!asset_id' => $asset['asset_id']));

  $result = mediamosa_response_connector::static_do_restcall($uri, 'GET', array('user_id' => $asset['owner_id'], 'app_id' => $asset['app_id']));
  if (mediamosa_response_connector::static_result_okay($result)) {
    $rows_aut = array();
    foreach ($result['items'] as $item_values) {
      foreach ($item_values as $name => $value) {
        $rows_aut[$name][] = $value;
      }
    }

    $rows_aut2 = array();
    foreach ($rows_aut as $name => $values) {
      $rows_aut2[] = array(t(_mediamosa_browse_fix_metadata_name($name)), implode(', ', $values));
    }

    $variables['properties'][t('Autorisarion rules')] = array('html' => TRUE, 'data' => theme('table', array('rows' => $rows_aut2)));
  }

  return theme('mediamosa_maintenance_browse_properties', $variables);
}


/**
 * Build the job list.
 *
 * @param array $asset
 */
function _mediamosa_browse_asset_page_view_job_list($asset) {

  $asset_id = $asset[mediamosa_asset_db::ID];

  $params = array(
    'user_id' => $asset[mediamosa_asset_db::OWNER_ID],
    'app_id' => $asset[mediamosa_asset_db::APP_ID],
  );

  // Get job list uri.
  $uri = strtr('asset/@asset_id/joblist', array('@asset_id' => $asset_id));

  // Do REST call.
  $result = mediamosa_response_connector::static_do_restcall($uri, 'GET', $params);

  $states = array(mediamosa_job_db::JOB_STATUS_WAITING, mediamosa_job_db::JOB_STATUS_FAILED, mediamosa_job_db::JOB_STATUS_FINISHED);

  // Get Items count.
  $item_count = $result['header']['item_count'];

  if (!$item_count) {
    return '';
  }

  $rows = array();
  foreach ($result['items'] as $item) {
    $delete = (in_array((string)$item['status'], $states)) ? l(t('Delete'), 'admin/mediamosa/asset/' . $asset_id . '/job/' . $item['id'] . '/delete') : '';
    // We will sort jobs by ID.
    $rows[$item['id']] = array(
      $item['id'],
      $item['job_type'],
      $item['status'],
      $item['started_unix'] ? format_date($item['started_unix'], 'small') : '',
      (string)$item['progress'] * 100 . '%',
      (string)$item['owner'],
      (string)$item['error_description'],
      $delete,
    );
  }
  ksort($rows);
  $header = array(
    t('ID'),
    t('Type'),
    t('Status'),
    t('Started'),
    t('Progress'),
    t('Owner'),
    t('Description'),
    t('Action'),
  );
  $output[] = '<p><h2>' . t('Jobs (@count)', array('@count' => $item_count)) . '</h2>';
  $output[] = '<div>' . t('Jobs started for this asset.') . '</div>';
  $output[] = theme('table', array('header' => $header, 'rows' => $rows)) . '</p>';

  return implode('', $output);
}

/**
 * Build the mediafile listing.
 *
 * @param array $asset
 */
function _mediamosa_browse_asset_page_view_mediafile_list($asset) {

  // Get the asset_id.
  $asset_id = $asset[mediamosa_asset_db::ID];

  // Need app_id to get mediafiles.
  $app_id = $asset[mediamosa_asset_db::APP_ID];

  // Get job list uri.
  $uri = strtr('asset/@asset_id/mediafile', array('@asset_id' => $asset_id));

  // Do REST call.
  $result = mediamosa_response_connector::static_do_restcall($uri, 'GET', array('app_id' => $app_id));

  // Get Items count.
  $item_count = count($result['items']);

  // Cache rights.
  $may_delete = $may_analyse = $may_retranscode = $may_download = _mediamosa_user_access_asset($asset_id);

  $allowed_still_fields = array(
    'mediafile_id',
    'file_extension',
    'created',
    'width',
    'height',
    'filesize',
    'mime_type',
    'still_time_code',
    'still_order',
    'still_format',
    'still_type',
    'still_default',
    'still_ticket',
  );

  $build = array();
  foreach ($result['items'] as $item) {
    if (isset($item['#value'])) {
      $item = $item['#value'];
    }

    $actions = array();

    $actions[] = '<b>' . trim($item['filename']) . '</b>';
    $actions[] = $may_delete ? l(t('Delete'), strtr('admin/mediamosa/asset/@asset_id/mediafile/@mediafile_id/delete', array('@asset_id' => $item['asset_id'], '@mediafile_id' => $item['mediafile_id']))) : t('No delete right');
    $actions[] = $may_analyse ? l(t('Analyze'), strtr('admin/mediamosa/asset/@asset_id/mediafile/@mediafile_id/analyse', array('@asset_id' => $item['asset_id'], '@mediafile_id' => $item['mediafile_id']))) : t('No analyse right');
    // Retranscode.
    if ($item[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE && $item[mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID]) {
      $actions[] = $may_retranscode ? l(t('Retranscode'), strtr('admin/mediamosa/asset/@asset_id/mediafile/@mediafile_id/retranscode', array('@asset_id' => $item['asset_id'], '@mediafile_id' => $item['mediafile_id']))) : t('No retranscode right');
    }
    else {
      $actions[] = t('Original file');
    }
    $actions[] = $may_download ? l(t('Download'), strtr('admin/mediamosa/asset/@asset_id/mediafile/@mediafile_id/download', array('@asset_id' => $item['asset_id'], '@mediafile_id' => $item['mediafile_id']))) : t('No download right');
    $caption = implode(' - ', $actions);

    // Mediafile properties.
    $rows = array();
    $rows[] = array(t('Mediafile id'), $item['mediafile_id']);
    $rows[] = array(t('Owner id'), $item['owner_id']);
    $rows[] = array(t('Group id'), $item['group_id']);
    $rows[] = array(t('Mime Type'), $item['metadata']['mime_type']);
    $rows[] = array(t('Container'), $item['metadata']['container_type'] ? $item['metadata']['container_type'] . ' @ ' . $item['metadata']['bitrate'] . 'kbps' : '');
    $rows[] = array(t('Width * Height @ fps'), $item['metadata']['width'] ? $item['metadata']['width'] . 'px * ' . $item['metadata']['height'] . 'px @ ' . $item['metadata']['fps'] . 'fps' : '');
    $rows[] = array(t('Created / Changed'), $item['created'] . ' / ' . $item['changed']);
    $rows[] = array(t('Duration'), $item['metadata']['file_duration']);
    $rows[] = array(t('Filesize'), ($item['metadata']['filesize']) ? format_size($item['metadata']['filesize']) : '');
    $rows[] = array(t('Video information'), ($item['metadata']['video_codec']) ? $item['metadata']['video_codec'] . ' @ ' . $item['metadata']['bitrate'] . 'kbps (' . $item['metadata']['colorspace'] . ')' : '');
    $rows[] = array(t('MP4 hinted'), $item['metadata']['is_hinted']);
    $rows[] = array(t('FLV metadated'), $item['metadata']['is_inserted_md']);
    $rows[] = array(t('Original'), $item['is_original_file']);
    $rows[] = array(t('Downloadable'), $item['is_downloadable']);
    $rows[] = array(t('Audio information'), ($item['metadata']['audio_codec']) ? $item['metadata']['audio_codec'] . ' @ ' . $item['metadata']['sample_rate'] . 'Hz' . ' (' . $item['metadata']['channels'] . ')': '');
    $rows[] = array(t('Protected'), $item['is_protected']);

    if (!empty($item['transcode_profile_id'])) {
      $transcode_profile_data = _mediamosa_get_transcode_profile_data($item['transcode_profile_id']);
      if ($transcode_profile_data) {
        $rows[] = array(t('Transcode profile'), l($transcode_profile_data[mediamosa_transcode_profile_db::PROFILE], 'admin/mediamosa/config/transcode_profile/' . $transcode_profile_data[mediamosa_transcode_profile_db::NID]));
      }
      else {
         $rows[] = array(t('Transcode profile'), t('Transcode profile no longer exists.'));
      }
    }

    // Uri REST call.
    $uri = strtr('mediafile/@mediafile_id/acl', array('@mediafile_id' => $item['mediafile_id']));

    $result = mediamosa_response_connector::static_do_restcall($uri, 'GET', array('user_id' => $item['owner_id'], 'app_id' => $item['app_id']));
    if (mediamosa_response_connector::static_result_okay($result)) {
      $rows_aut = array();
      foreach ($result['items'] as $item_values) {
        foreach ($item_values as $name => $value) {
          $rows_aut[$name][] = $value;
        }
      }

      $rows_aut2 = array();
      foreach ($rows_aut as $name => $values) {
        $rows_aut2[] = array(t(_mediamosa_browse_fix_metadata_name($name)), implode(', ', $values));
      }

      $rows[] = array(t('Autorisarion rules'), theme('table', array('rows' => $rows_aut2)));
    }

    // Uri REST call.
    $uri = strtr('mediafile/@mediafile_id', array('@mediafile_id' => $item['mediafile_id']));

    $still_result = mediamosa_response_connector::static_do_restcall($uri, 'GET', array('app_id' => $item['app_id']));
    if (mediamosa_response_connector::static_result_okay($still_result)) {
      $mediafile = reset($still_result['items']);

      if (isset($mediafile['still'])) {
        foreach ($mediafile['still'] as $still_item) {
          $still_header = array();
          $still_rows = array();
          $still_ticket = NULL;

          foreach ($still_item as $still_key => $still_det) {

            if (in_array($still_key, $allowed_still_fields)) {
              $still_rows[] = array(
                $still_key == 'mediafile_id' ? 'still_id' : $still_key,
                $still_det,
              );

              if ($still_key == 'still_ticket') {
                $still_ticket = $still_det;
              }
            }
          }

          $rows[] = array(
            t('Still') . '<br />' . ($still_ticket ? theme('image', array('path' => $still_ticket, 'width' => 200, 'height' => NULL), FALSE) : ''),
            theme('table', array('header' => $still_header, 'rows' => $still_rows)),
          );
        }
      }
    }

    $build[] = theme('table', array('caption' => $caption, 'rows' => $rows));
  }

  // Add text when no mediafiles where found...
  if (!$item_count) {
    $build[] = t('This asset does not contain mediafiles.');
  }

  return '<p><h2>' . t('Mediafiles (@count)', array('@count' => $item_count)) . '</h2>' . implode('', $build) . '</p>';
}

/**
 * Build the collections listing.
 *
 * @param array $asset
 */
function _mediamosa_browse_asset_page_view_collections_list($asset) {

  // Get the asset_id.
  $asset_id = $asset[mediamosa_asset_db::ID];

  // Need app_id to get collections.
  $app_id = $asset[mediamosa_asset_db::APP_ID];

  // Get job list uri.
  $uri = strtr('asset/@asset_id/collection', array('@asset_id' => $asset_id));

  // Do REST call.
  $result = mediamosa_response_connector::static_do_restcall($uri, 'GET', array('app_id' => $app_id));

  // Get Items count.
  $item_count = count($result['items']);

  $build = array();
  if (!$item_count) {
    $build[] = t('This asset was not found in any collection.');
  }
  else {
    $rows = array();

    $build[] = t('This asset was found in the following collections:');
    $header = array(
      array('data' => t('Title')),
      array('data' => t('Operations'), 'class' => array('mmoperation')),
    );

    // Get destination.
    $l_options = array('query' => drupal_get_destination());

    foreach ($result['items'] as $item) {
      $actions = array(
        l(t('Remove'), 'admin/mediamosa/browse/collection/' . rawurlencode($item['coll_id']) . '/remove_asset/' . rawurlencode($asset_id), $l_options),
      );

      $title = trim($item['title']);
      $title = $title == '' ? '<collection has no title>' : $title;

      $rows[] = array(
        l($title, mediamosa_settings::get_url_collection($item['coll_id'])),
        array('data' => implode(' | ', $actions), 'class' => 'mmoperation'),
      );
    }

    $build[] = theme('table', array('header' => $header, 'rows' => $rows));
  }

  return '<p><h2>' . t('Collections (@count)', array('@count' => $item_count)) . '</h2>' . implode('', $build) . '</p>';
}

/**
 * Build the supplements listing.
 *
 * @param array $asset
 *
 * @return FALSE if there are no supplement or themed supplement info.
 *
 * @see _mediamosa_browse_asset_page_view_collections_list
 *
 */
function _mediamosa_browse_asset_page_view_supplements_list($asset) {

  // Get the asset_id.
  $asset_id = $asset[mediamosa_asset_db::ID];

  // Get job list uri.
  $uri = strtr('asset/@asset_id/supplement', array('@asset_id' => $asset_id));

  // Do REST call.
  $result = mediamosa_response_connector::static_do_restcall($uri, 'GET');

  // Get Items count.
  $item_count = count($result['items']);

  $build = array();
  if (!$item_count) {
    return FALSE;
  }

  $rows = array();

  $build[] = t('This asset have the following supplements:');
  $header = array(
    array('data' => t('ID')),
    array('data' => t('Created')),
    array('data' => t('Last modified')),
    array('data' => t('Approximate size')),
  );

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  foreach ($result['items'] as $item) {
    $length_encoded = drupal_strlen($item['supplement_base64']);

    // If encoded data is big (more than 16K) we show just approx supplement size.

    // Normally encoded data have 1/3 redundancy.
    $approx_size = $length_encoded > 16 * 1024 ? round( $length_encoded * 3 / 4 ) : drupal_strlen(base64_decode($item['supplement_base64']));

    $approx_size = format_size($approx_size);
    $rows[$item['created']] = array(
      $item['supplement_id'],
      $item['created'],
      $item['changed'],
      $approx_size,
    );
  }

  ksort($rows);

  $build[] = theme('table', array('header' => $header, 'rows' => $rows));

  return '<p><h2>' . t('Supplements (@count)', array('@count' => $item_count)) . '</h2>' . implode('', $build) . '</p>';
}

/**
 * Add+Edit form
 */
function mediamosa_browse_asset_form($form, &$form_state, $asset_id) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Asset browser'), 'admin/mediamosa/browse/asset');
  drupal_set_breadcrumb($breadcrumb);

  // Get the asset REST call.
  $url = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

  // Do REST call.
  $result_asset = mediamosa_response_connector::static_do_restcall_drupal('asset/' . $asset_id, 'GET', array());

  if (!mediamosa_response_connector::static_result_okay($result_asset)) {
    drupal_set_message(t('Asset with ID @asset_id does not exists.', array('@asset_id' => $asset_id)), 'warning');
    drupal_goto('admin/mediamosa/browse/asset');
  }

  $asset = reset($result_asset['items']);

  // Get all metadata properties for this app.
  $metadatas = mediamosa_asset_metadata_property::get_metadata_properties_full();

  // Collect the metadata groups.
  $metadata_groups = array();
  foreach ($metadatas as $name => $metadata) {
    $metadata_groups[$metadata['propgroup_name']] = $metadata['propgroup_id'];
  }

  // Empty form.
  $form = array();

  // Store asset_id.
  $form['asset_id'] = array(
    '#type' => 'hidden',
    '#value' => $asset_id,
  );

  // Build form for editing the metdata.
  foreach ($metadata_groups as $group_name => $group_id) {
    if (empty($asset[$group_name])) {
      continue;
    }

    // Add fieldset for metadata group.
    $name_fixed = _mediamosa_browse_fix_metadata_name($group_name);

    // Walk through the properties.
    foreach ($asset[$group_name] as $metadata_name => $metadata_values) {
      if (empty($metadata_values)) {
        continue;
      }

      // Set fieldset if not already.
      if (!isset($form[$group_name])) {
        $form[$group_name] = array(
          '#type' => 'fieldset',
          '#title' => t($name_fixed),
          '#collapsible' => TRUE,
          '#description' => t('Clear an item to delete it from the asset\'s @name_fixed metadata.', array('@name_fixed' => $name_fixed))
        );
      }

      foreach ($metadata_values as $metadata_value) {
        // Make the edit box.
        // rpl_/old_ in front of the name makes sure we dont conflict with other
        // names in the form.
        $form[$group_name]['rpl_' . $metadata_name] = array(
          '#type' => drupal_strlen($metadata_value) > 80 ? 'textarea' : 'textfield',
          '#default_value' => $metadata_value,
          '#title' => t(_mediamosa_browse_fix_metadata_name($metadata_name)),
        );
      }
    }
  }

  $options_metadata_groups = array('' => '');

  ksort($metadata_groups);
  ksort($metadatas);
  foreach ($metadata_groups as $metadata_group_name => $metadata_group_id) {
    foreach ($metadatas as $name => $metadata) {
      if ($metadata['propgroup_id'] != $metadata_group_id) {
        continue;
      }

      $options_metadata_groups[_mediamosa_browse_fix_metadata_name($metadata_group_name)][$metadata['propdef_name']] = _mediamosa_browse_fix_metadata_name($metadata['propdef_name']);
    }
  }


  $form['add'] = array(
    '#type' => 'fieldset',
    '#title' => t('New metadata field'),
    '#collapsible' => TRUE,
    '#description' => t('Add a new metadata field to the asset\'s metadata.')
  );

  $form['add']['new_metadata_property'] = array(
    '#type' => 'select',
    '#title' => t('Metadata field'),
    '#description' => t('Enter value for the selected metadata field.'),
    '#options' => $options_metadata_groups,
  );

  $form['add']['new_metadata_value'] = array(
    '#type' => 'textarea',
    '#title' => t('Value'),
    '#description' => t('Enter value for the selected metadata field.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit changes'),
  );

  return $form;
}

/**
 * Submit form.
 */
function mediamosa_browse_asset_form_validate($form, &$form_state) {

  if ((empty($form_state['values']['new_metadata_property']) && $form_state['values']['new_metadata_value'] != '') || (!empty($form_state['values']['new_metadata_property']) && $form_state['values']['new_metadata_value'] == '')) {
    if (empty($form_state['values']['new_metadata_property'])) {
      form_set_error('new_metadata_property', t('Select the metadata property.'));
    }
    else {
      form_set_error('new_metadata_value', t('give the selected metadata property a value.'));
    }
  }

  // Get the metadata list.
  $metadatas = mediamosa_asset_metadata_property::get_metadata_properties_full();

  foreach ($metadatas as $name => $metadata) {
    // So it matches.
    $name_rpl = 'rpl_' . $name;

    // Check if $name is a valid metadata field.
    // We also skip checks on non-changed values.
    if (!isset($form_state['values'][$name_rpl]) || $form_state['values'][$name_rpl] == '') {
      continue;
    }

    // Now validate the value, empty is allowed.
    switch ($metadata['propdef_type']) {
      case mediamosa_asset_metadata_property_db::TYPE_CHAR:
        break;

      case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
        try {
          $form_state['values'][$name_rpl] = mediamosa_type::check($name, mediamosa_sdk::TYPE_DATETIME, $form_state['values'][$name_rpl]);
        }
        catch (mediamosa_exception $e) {
          form_set_error($name_rpl, t('Invalid date, must be in format YYYY-MM-DD or YYYY-MM-DD 00:00:00'));
        }
        break;

      case mediamosa_asset_metadata_property_db::TYPE_INT:
        if (!is_numeric($form_state['values'][$name_rpl])) {
          form_set_error($name_rpl, t('This value must be numeric.'));
        }
        break;

      default:
        assert(0);
        drupal_set_message(t('Unknown/unsupported metadata type.'), 'error');
        break;
    }
  }
}

/**
 * Submit form.
 */
function mediamosa_browse_asset_form_submit($form, &$form_state) {

  $asset_id = $form_state['values']['asset_id'];

  // Get the asset REST call.
  $url = strtr('asset/@asset_id', array('@asset_id' => $asset_id));

  // Do REST call.
  $result_asset = mediamosa_response_connector::static_do_restcall_drupal('asset/' . $asset_id, 'GET', array());

  if (!mediamosa_response_connector::static_result_okay($result_asset)) {
    drupal_set_message(t('Asset with ID @asset_id does not exists.', array('@asset_id' => $asset_id)), 'warning');
    drupal_goto('admin/mediamosa/browse/asset');
  }

  // Get the asset from the result.
  $asset = reset($result_asset['items']);

  // Collect was what made empty.
  $empty_names = array();
  $changed_names = array();

  foreach ($form_state['values'] as $name_rpl => $value) {

    // Must start with rpl_.
    if (drupal_substr($name_rpl, 0, 4) != 'rpl_') {
      continue;
    }

    // Get the original name.
    $name = drupal_substr($name_rpl, 4);
    $changed_names[$name] = $value;
  }

  // Save the changes.
  $params = array(
    'app_id' => $asset['app_id'], // Need to this under the app of the asset.
    'user_id' => $asset['owner_id'], // Need the owner.
  );

  $params = array_merge($changed_names, $params);

  if (!empty($form_state['values']['new_metadata_property']) && $form_state['values']['new_metadata_value'] != '') {
    $params[$form_state['values']['new_metadata_property']] = $form_state['values']['new_metadata_value'];
  }

  $uri = strtr('asset/@asset_id/metadata', array('@asset_id' => $asset_id));

  // Execute the metadata post.
  $result = mediamosa_response_connector::static_do_restcall($uri, 'POST', $params);

  if (mediamosa_response_connector::static_result_okay($result)) {
    drupal_set_message(t('The changes to the asset metadata have been saved.'));
    drupal_goto('admin/mediamosa/browse/asset/' . rawurlencode($asset_id));
  }

  // Set failed.
  drupal_set_message(t('Failed to save changes to the asset metadata.'));
}

/**
 * This function returns a confirmation form to delete a asset.
 */
function mediamosa_browse_asset_delete_confirm_form($form, &$form_state, $asset_id) {
  $form['asset_id'] = array(
    '#type' => 'hidden',
    '#value' => $asset_id,
  );

  $form['cascade'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove all mediafiles from this asset before deleting it.'),
  );

  return confirm_form($form,
                      t('Are you sure you want to delete this asset?'), // question
                      mediamosa_settings::get_url_asset($asset_id), // cancel path
                      t('This action cannot be undone.'),
                      t('Delete'), t('Cancel')
  );
}

/**
 * This function deletes an asset and optionally all mediafiles within it.
 */
function mediamosa_browse_asset_delete_confirm_form_submit($form, &$form_state) {

  // Get asset_id.
  $asset_id = $form_state['values']['asset_id'];

  // Get the asset.
  $asset = mediamosa_asset::get($asset_id);

  if (!$asset) {
    drupal_set_message(t('Unable to find asset.'), 'warning');

    // Failed.
    drupal_goto('admin/mediamosa/browse/asset');
  }

  // Delete uri REST call.
  $uri = strtr('asset/@asset_id/delete', array('@asset_id' => $asset_id));

  $params = array(
    'user_id' => $asset['owner_id'],
    'app_id' => $asset['app_id'],
  );

  if ($form_state['values']['cascade'] == 1) {
    $params['delete'] = 'cascade';
  }

  // Do the REST call.
  $result = mediamosa_response_connector::static_do_restcall_drupal($uri, 'POST', $params);

  if (mediamosa_response_connector::static_result_okay($result)) {
    drupal_goto('admin/mediamosa/browse/asset');
  }

  drupal_goto(mediamosa_settings::get_url_asset($asset_id));
}
