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
 * Collection browser.
 */

define('MEDIAMOSA_BROWSE_FILTER_COLLECTION', 'mediafile_browse_collection');

/**
 * Build the form for browsing collections.
 */
function _mediamosa_browse_collection_page($op = 'list') {
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  drupal_set_breadcrumb($breadcrumb);

  $build = array();

  $build['collection_filter'] = drupal_get_form('mediamosa_browse_collection_filter_form');
  $build['collection_list'] = drupal_get_form('mediamosa_browse_collection_list_form');

  return $build;
}

/**
 * Form builder; Return form for collection filters.
 *
 * @see mediamosa_browse_collection_filter_form_submit()
 */
function mediamosa_browse_collection_filter_form() {

  // Get session.
  $session = _mediamosa_browse_collection_session();

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
      'path' => 'admin/mediamosa/browse/js/collection/searchparameters',
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
      'path' => 'admin/mediamosa/browse/js/collection/searchparameters',
      'method' => 'replace',
    ),
    '#weight' => 11,
  );


  $form['filter']['apply'] = array(
    '#type' => 'submit',
    '#value' => t('Apply filter'),
  );

  $form = _mediamosa_browse_collection_searchparameters($form);

  return $form;
}

/**
 * Insert the search form into a text area.
 */
function _ajax_browse_collection_search_parameters() {

  // Load the form based upon the $_POST data sent via the ajax call.
  list($form, $form_state) = ajax_get_form();

  // Fix drupal notice.
  $form_state['values'] = isset($form_state['values']) ? $form_state['values'] : array();
  $form = form_builder('', _mediamosa_browse_collection_searchparameters($form, $form_state), $form_state);

  $commands[] = ajax_command_replace('', drupal_render($form['filters']));

  return array('#type' => 'ajax', '#commands' => $commands, '#header' => FALSE);
}

/**
 * Enrich the form with the filter items.
 *
 * @param array $form
 * @param array $form_state
 */
function _mediamosa_browse_collection_searchparameters($form, $form_state = array()) {

  // Get session.
  $session = _mediamosa_browse_collection_session($form_state);

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
      $session['filters'] = array();
      $form_state['input']['filters'] = array();

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
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] = $session;

  unset($form['filters']['searchparameters']['filters']);
  foreach ($form_state['input']['filters'] as $x => $filter) {
    $form_filter = _mediamosa_browse_searchparameters_helper($x, $filter);

    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item]'] = $form_filter['item'];
    $form['filters']['searchparameters']['filters']['filters[' . $x . '][item_value]'] = $form_filter['item_value'];
  }

  return $form;
}

/**
 * Process result from collection filter form.
 */
function mediamosa_browse_collection_filter_form_submit($form, $form_state) {
  _mediamosa_browse_collection_session($form_state);
}

/**
 * Form builder; collection listing page.
 *
 */
function mediamosa_browse_collection_list_form() {
  // Get session.
  $session = _mediamosa_browse_collection_session();

  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $limit = $session['limit'];
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $item_count_total = 0;

  $header = array(
    array('field' => 'title', 'data' => t('Title'), 'sort' => 'asc'),
    array('field' => 'app_id', 'data' => t('App ID'), 'class' => array('mmappid')),
    array('field' => 'changed', 'data' => t('Last modified'), 'class' => array('mmdate')),
    array('data' => t('Operations'), 'class' => array('mmoperation')),
  );

  $result = _mediamosa_browse_collection_execute();
  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  foreach ($result['items'] as $row) {
    $actions = array(
      l(t('Edit'), 'admin/mediamosa/browse/collection/' . $row[mediamosa_collection_db::ID] . '/edit', $l_options),
      l(t('Delete'), 'admin/mediamosa/browse/collection/' . $row[mediamosa_collection_db::ID] . '/delete', $l_options),
    );

    $title = empty($row['title']) ? NULL : check_plain($row['title']);
    $rows[] = array(
      theme('l_mediamosa_collection', array('title' => $title, 'id' => $row[mediamosa_collection_db::ID])),
      array('data' => check_plain($row['app_id']), 'class' => array('mmappid')),
      array('data' => theme('mediamosa_maintenance_date', array('datetime' => $row[mediamosa_collection_db::CHANGED])), 'class' => array('mmdate')),
      array('data' => implode(' | ', $actions), 'class' => array('mmoperation')),
    );
  }

  // Our listing.
  $form['list'] = array(
    '#theme' => 'mediamosa_maintenance_table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No collections found.'),
    // Pager.
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
    // Top
    '#title' => t('Collections (@total)', array('@total' => $item_count_total)),
    '#description' => t('Collections matching the search criteria above are listed below. Click on a collection title below to view details.'),
    // Total note.
    '#caption_bottom' => t('Collections found: @total', array('@total' => $item_count_total))
  );

  return $form;
}

/**
 * Do the REST call and return the response object.
 */
function _mediamosa_browse_collection_execute() {
  // Get session.
  $session = _mediamosa_browse_collection_session();

  // What to sort on.
  $sort = $session['sort'];
  $order = $session['order'];
  $limit = $session['limit'];
  $page = $session['page'];

  switch ($order) {
    case t('App ID'):
      $order = 'app_id';
      break;

    case t('Last modified'):
      $order = 'changed';
      break;

    case t('Title'):
    default:
      $order = 'title';
      break;
  }

  $params = array();

  $cqls = array();

  $cql_session = _mediamosa_browse_cql_build($session);
  if (!empty($cql_session)) {
    $cqls[] = $cql_session;
  }

  // Add sort.
  $cqls[] = strtr('sortBy @order/@sort', array('@order' => $order, '@sort' => drupal_strtolower($sort) == 'asc' ? 'ascending' : 'descending'));

  // Add params.
  $params['cql'] = implode(' ', $cqls);
  $params['limit'] = $limit;
  $params['offset'] = $page * $limit;

  // Do the rest call.
  return mediamosa_response_connector::static_do_restcall_drupal('collection', 'GET', $params);
}

/**
 * Get and set the session.
 */
function _mediamosa_browse_collection_session($filter_form_state = array()) {
  // Get session.
  $session = isset($_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION]) ? $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] : array();

  // Get these from _GET if present.
  foreach (array('order', 'sort', 'page') as $param) {
    unset($session[$param]);
    if (isset($_GET[$param])) {
      $session[$param] = $_GET[$param];
    }
  }

  // Set defaults.
  $session += array(
    'order' => t('Title'),
    'sort' => 'asc',
    'page' => 0,
    'limit' => mediamosa_settings::maintenance_items_per_page(),
  );

  // Process filter.
  if (!empty($filter_form_state['input']['operator'])) {
    $session['operator'] = $filter_form_state['input']['operator'];
  }

  if (!empty($filter_form_state['input']['filters'])) {
    $session['filters'] = $filter_form_state['input']['filters'];
  }

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_COLLECTION] = $session;

  return $session;
}

/**
 * View the details of an collection.
 */
function _mediamosa_browse_collection_page_view($coll_id) {
  // If we are set as Home page, the breadcrumb isnt set, lets do it here.
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), NULL);
  $breadcrumb[] = l(t('Administration'), 'admin');
  $breadcrumb[] = l(t('MediaMosa'), 'admin/mediamosa');
  $breadcrumb[] = l(t('Browse'), 'admin/mediamosa/browse');
  $breadcrumb[] = l(t('Collection browser'), 'admin/mediamosa/browse/collection');
  drupal_set_breadcrumb($breadcrumb);

  // Get the collection.
  $response =  mediamosa_response_connector::static_do_restcall_drupal(strtr('collection/@coll_id', array('@coll_id' => rawurlencode($coll_id))) , 'GET');

  if ((int) $response['header']['request_result_id'] != mediamosa_error::ERRORCODE_OKAY) {
    return '<h1>Collection not found</h1>';
  }

  $collection = reset($response['items']);

  $variables = array(
    'description' => t('Details for this collection are shown below.'),
    'properties' => array(
      t('Title') => $collection[mediamosa_collection_db::TITLE],
      t('Description') => $collection[mediamosa_collection_db::DESCRIPTION],
      t('App ID') => $collection[mediamosa_collection_db::APP_ID],
      t('Owner ID') => $collection[mediamosa_collection_db::OWNER_ID],
      t('Group ID') => $collection[mediamosa_collection_db::GROUP_ID],
    ),
  );

  $page = isset($_GET['page']) ? $_GET['page'] : 0;

  // Get the possible assets in this collection.
  $params = array('limit' => 10, 'offset' => $page * 10);
  $response =  mediamosa_response_connector::static_do_restcall_drupal(strtr('collection/@coll_id/asset', array('@coll_id' => rawurlencode($coll_id))) , 'GET', $params);

  if (in_array((int) $response['header']['request_result_id'], array(mediamosa_error::ERRORCODE_OKAY, mediamosa_error::ERRORCODE_EMPTY_RESULT))) {
    $variables['collection_assets']['title'] = t('Assets (@total)', array('@total' => $response['header']['item_count_total']));
    $variables['collection_assets']['coll_id'] = $coll_id;

    if ($response['header']['item_count_total'] > 0) {
      $variables['collection_assets']['description'] = t('Collection <em>@name</em> contains the following assets.', array('@name' => $collection[mediamosa_collection_db::TITLE]));

      $variables['collection_assets']['item_count_total'] = $response['header']['item_count_total'];
      $variables['collection_assets']['item_offset'] = $response['header']['item_offset'];
      $variables['collection_assets']['page'] = $page;
      $variables['collection_assets']['assets'] = array();

      foreach ($response['items'] as $asset) {
        $variables['collection_assets']['assets'][] = $asset;
      }
    }
    else {
      $variables['collection_assets']['empty'] = t('Collection <em>@name</em> does not contain any assets.', array('@name' => $collection[mediamosa_collection_db::TITLE]));
    }
  }

  $build['properties'] = theme('mediamosa_maintenance_browse_properties', $variables);

  return implode('', $build);
}

/**
 * Add/Edit form
 */
function mediamosa_browse_collection_form($form, &$form_state, $coll_id = NULL) {

  $collection = NULL;
  if (isset($coll_id)) {
    $collection = mediamosa_collection::get($coll_id);

    if (!$collection) {
      $form['collection']['#markup'] = t('Collection was not found.');
    }
  }

  $form['collection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Collection properties'),
    '#collapsible' => TRUE,
  );

  // You can not move collection to other app, for now.
  if (!isset($collection)) {
    // Get the apps.
    $apps = mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::APP_NAME), mediamosa_app_db::APP_NAME, 'ASC')->fetchAllKeyed();
    // no array_merge, will lose Ids in apps.
    $apps = array('' => '') + $apps;

    $form['collection']['app_id'] = array(
      '#type' => 'select',
      '#title' => t('Application'),
      '#description' => t('Select for which application the collection is made.'),
      '#required' => TRUE,
      '#options' => $apps,
    );
  }
  else {
    $form['coll_id'] = array(
      '#type' => 'hidden',
      '#value' => $collection[mediamosa_collection_db::ID],
    );
  }

  $form['collection']['owner_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Owner'),
    '#description' => t('The owner of the collection.'),
    '#default_value' => isset($collection) ? $collection[mediamosa_collection_db::OWNER_ID] : '',
    '#required' => TRUE,
  );

  $form['collection']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#description' => t('The title of the collection.'),
    '#default_value' => isset($collection) ? $collection[mediamosa_collection_db::TITLE] : '',
    '#required' => TRUE,
  );

  $form['collection']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('Short description of the collection.'),
    '#default_value' => isset($collection) ? $collection[mediamosa_collection_db::DESCRIPTION] : '',
    '#required' => TRUE,
  );

  if (!isset($collection)) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add collection'),
    );
  }
  else {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save collection'),
    );
  }

  return $form;
}

/**
 * Submit form handler.
 */
function mediamosa_browse_collection_form_submit($form, &$form_state) {

  switch ($form_state['values']['op']) {

    case t('Add collection'):
      // Create the collection.
      $coll_id = mediamosa_collection::create($form_state['values']['app_id'], $form_state['values']['owner_id'], $form_state['values']['title'], $form_state['values']['description']);
      break;

    case t('Save collection'):
      // Update the collection.
      $coll_id = $form_state['values']['coll_id'];
      mediamosa_collection::update($coll_id, $form_state['values']['owner_id'], $form_state['values']['title'], $form_state['values']['description']);
      break;

    default:
      drupal_goto('admin/mediamosa2');
  }

  drupal_goto(mediamosa_settings::get_url_collection($coll_id));
}

/**
 * Deletion confirm.
 *
 * @param string $coll_id
 */
function mediamosa_browse_collection_delete_confirm_form($form, &$form_state, $coll_id) {

  // Get the collection.
  $collection = mediamosa_collection::get($coll_id);

  if (!$collection) {
    drupal_set_message(t('Unable to find collection.'), 'warning');

    // Failed.
    drupal_goto('admin/mediamosa/browse/collection');
  }

  $form['coll_id'] = array(
    '#type' => 'hidden',
    '#value' => $coll_id,
  );

  $form['truncate'] = array(
    '#type' => 'checkbox',
    '#title' => 'Remove all assets from this collection before deleting it',
    '#description' => t('This option does not delete any assets, it just removes them from this collection. When unchecked, the deletion will fail if the collection contains assets.'),
  );

  return confirm_form($form,
                      t('Are you sure you want to delete this collection?'),
                      mediamosa_settings::get_url_collection($coll_id),
                      t('This action cannot be undone.'),
                      t('Delete collection'), t('Cancel')
  );
}

/**
 * Submit handler for deletion collection.
 *
 * @see mediamosa_browse_collection_delete_confirm_form()
 */
function mediamosa_browse_collection_delete_confirm_form_submit($form, &$form_state) {

  // Remove all links
  $coll_id = $form_state['values']['coll_id'];

  // Get the collection.
  $collection = mediamosa_collection::get($coll_id);

  // Check if the user also wants to remove all assets from the collection first
  if ($form_state['values']['truncate'] === 1) {

    // Get all asset_ids.
    $asset_ids = mediamosa_asset_collection::get_asset_ids_by_coll_ids(array($coll_id))->fetchCol();

    if (!empty($asset_ids)) {
      $url = strtr('collection/@coll_id/asset_relation/delete', array('@coll_id' => $coll_id));

      $asset_ids_todo = array();

      // As long we got ids, we'll continue.
      while (count($asset_ids)) {
        for ($x = 0; $x < 5000 && count($asset_ids); $x++) {
          $asset_ids_todo[] = array_pop($asset_ids);
        }

        // Do request to get total of assets in collection.
        mediamosa_response_connector::static_do_restcall_drupal($url, 'POST', array('app_id' => $collection[mediamosa_collection_db::APP_ID], 'asset_id' => $asset_ids_todo, 'user_id' => $collection[mediamosa_collection_db::OWNER_ID]));
      }
    }
  }

  // Delete the collection.
  $url = strtr('collection/@coll_id/delete', array('@coll_id' => $coll_id));
  $result = mediamosa_response_connector::static_do_restcall_drupal($url, 'POST', array('app_id' => $collection[mediamosa_collection_db::APP_ID], 'user_id' => $collection[mediamosa_collection_db::OWNER_ID]));

  if (in_array((int)$result->request_result_id, array(mediamosa_error::ERRORCODE_OKAY, mediamosa_error::ERRORCODE_EMPTY_RESULT))) {
    drupal_set_message(t('The collection has been deleted.'));
    drupal_goto('admin/mediamosa/browse/collection');
  }

  drupal_goto('admin/mediamosa/browse/collection/' . rawurlencode($coll_id));
}

/**
 * Remove from collection confirm.
 *
 * @param string $coll_id
 */
function mediamosa_browse_collection_remove_asset_confirm_form($form, &$form_state, $coll_id, $asset_id) {

  // Get the collection.
  $collection = mediamosa_collection::get($coll_id);

  if (!$collection) {
    drupal_set_message(t('Unable to find collection.'), 'warning');

    // Failed.
    drupal_goto('admin/mediamosa/browse/collection');
  }

  $form['coll_id'] = array(
    '#type' => 'hidden',
    '#value' => $coll_id,
  );

  $form['asset_id'] = array(
    '#type' => 'hidden',
    '#value' => $asset_id,
  );

  return confirm_form($form,
                      t("Are you sure you want to remove this asset from the collection '@collection'?", array('@collection' => $collection[mediamosa_collection_db::TITLE])),
                      mediamosa_settings::get_url_collection($coll_id),
                      t('This action cannot be undone.'),
                      t('Remove relation'), t('Cancel'));
}

/**
 * Submit handler for removing asset from collection.
 *
 * @see mediamosa_browse_collection_remove_asset_confirm_form()
 */
function mediamosa_browse_collection_remove_asset_confirm_form_submit($form, &$form_state) {

  // Remove all links
  $coll_id = $form_state['values']['coll_id'];
  $asset_id = $form_state['values']['asset_id'];

  // Get the collection.
  $collection = mediamosa_collection::get($coll_id);

  $url = strtr('collection/@coll_id/asset_relation/delete', array('@coll_id' => $coll_id));

  // Do request to get total of assets in collection.
  $result = mediamosa_response_connector::static_do_restcall_drupal($url, 'POST', array('app_id' => $collection[mediamosa_collection_db::APP_ID], 'asset_id' => array($asset_id), 'user_id' => $collection[mediamosa_collection_db::OWNER_ID]));

  if (in_array((int)$result->request_result_id, array(mediamosa_error::ERRORCODE_OKAY, mediamosa_error::ERRORCODE_EMPTY_RESULT))) {
    drupal_set_message(t('The asset-collection relation has been removed.'));
  }

  drupal_goto('admin/mediamosa/browse/collection/' . rawurlencode($coll_id));
}
