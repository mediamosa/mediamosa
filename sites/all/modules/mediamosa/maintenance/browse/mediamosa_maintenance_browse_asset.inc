<?php
/**
 * @file
 * Asset browser.
 */

define('MEDIAMOSA_BROWSE_FILTER_ASSET', 'mediafile_browse_asset');

/**
 * Build the form for browsing assets.
 */
function _mediamosa_browse_asset_page() {
  return drupal_get_form('mediamosa_browse_asset_filter_form');
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
  $collections = array_merge(array(0 => t('All collections')), $collections);
  foreach ($collections as $coll_id => $name) {
    if (!$coll_id || trim($name) != '') {
      continue;
    }

    // Set empty titles.
    $collections[$coll_id] = t('<collection has no name> ID: @id', array('@id' => $coll_id));
  }

  $form = array();

  // Include the search results.
  $form['asset_list'] = _mediamosa_browse_asset_list();

  $form['actions']['filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
    '#collapsible' => TRUE,
    '#prefix' => '<div id="searchparameters">',
    '#suffix' => '</div>',
  );

  $form['actions']['filters']['hide_empty_assets'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide empty assets'),
    '#default_value' => isset($session['hide_empty_assets']) ? $session['hide_empty_assets'] : 0,
  );

  $form['actions']['filters']['published'] = array(
    '#type' => 'select',
    '#title' => t('Published state'),
    '#description' => t('Show either publish, not published or all assets.'),
    '#options' => array('ALL' => t('Show all assets'), 'TRUE' => t('Show only published'), 'FALSE' => t('Show only unpublished')),
    '#default_value' => isset($session['published']) ? $session['published'] : 'ALL',
  );

  $form['actions']['filters']['trashcan'] = array(
    '#type' => 'select',
    '#title' => t('Trashcan state'),
    '#description' => t('Show either publish, not published or all assets.'),
    '#options' => array('ALL' => t('Show all assets'), 'TRUE' => t('Show assets in trashcan'), 'FALSE' => t('Show assets not in trashcan')),
    '#default_value' => isset($session['trashcan']) ? $session['trashcan'] : 'ALL',
  );

  $form['actions']['filters']['operator'] = array(
    '#type' => 'select',
    '#title' => t('Search criteria operator'),
    '#description' => t("Filter on all search criteria ('AND') or a least one criterium ('OR')."),
    '#options' => array('and' => t('AND'), 'or' => t('OR')),
    '#default_value' => isset($session['operator']) ? $session['operator'] : 'and',
  );

  if (isset($collections) && is_array($collections) && !empty($collections)) {
    $form['actions']['filters']['coll_id'] = array(
      '#type' => 'select',
      '#title' => t('Search within collection'),
      '#description' => t('Select collection to be searched. Leave empty for all collections.'),
      '#options' => $collections,
      '#default_value' => isset($session['coll_id']) ? $session['coll_id'] : 0,
    );
  }

  $form['actions']['filters']['searchparameters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search criteria'),
  );

  $form['actions']['filters']['searchparameters']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add search item'),
    '#ajax' => array(
      'wrapper' => 'searchparameters',
      'callback' => '_ajax_browse_asset_search_parameters_callback',
      'method' => 'replace',
    ),
    '#weight' => 10,
  );

  $form['actions']['filters']['searchparameters']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#ajax' => array(
      'wrapper' => 'searchparameters',
      'callback' => '_ajax_browse_asset_search_parameters_callback',
      'method' => 'replace',
    ),
    '#weight' => 11,
  );

  $form['actions']['filters']['apply'] = array(
    '#type' => 'submit',
    '#value' => t('Apply filter'),
  );

  // CQL filter.
  $form['actions']['cqlfilter'] = array(
    '#type' => 'fieldset',
    '#title' => t('CQL Filter'),
    '#collapsible' => TRUE,
  );

  $form['actions']['cqlfilter']['cql'] = array(
    '#type' => 'textfield',
    '#description' => t('Enter a CQL search command here. Usage can be found in the Webservice document. Examples: "acl_app==5" or "is_protected == FALSE"'),
    '#default_value' => isset($session['cql']) ? $session['cql'] : '',
  );

  $form['actions']['cqlfilter']['apply_cql'] = array(
    '#type' => 'submit',
    '#value' => t('Apply CQL filter'),
  );

  // CQL filter.
  $form['actions']['actions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Actions'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $result = mediamosa_asset::garbage_cleanup(NULL, TRUE);

  $text = t('Total garbage assets: @result', array('@result' => $result['cleaned']));

  $form['actions']['actions']['summary'] = array(
    '#markup' => $text . '<br/>',
  );

  $form['actions']['actions']['cleanup'] = array(
    '#type' => 'submit',
    '#value' => t('Clean up assets'),
  );

  // Form action, but do not repeat below.
  $form['actions']['#type'] = 'actions';
  $form['actions']['#mediamosa_no_clone'] = TRUE;

  // Show as settings form look.
  $form['#theme'] = 'system_settings_form';

  // Add search parameters.
  _mediamosa_browse_asset_searchparameters($form);

  return $form;
}

/**
 * Process result from asset filter form.
 *
 * @see mediamosa_browse_asset_filter_form()
 */
function mediamosa_browse_asset_filter_form_submit($form, $form_state) {
  $the_session = _mediamosa_browse_asset_session($form_state);

  if ($the_session['op'] === t('Clean up assets')) {
    $result = mediamosa_asset::garbage_cleanup();
    drupal_set_message(t('Cleanup @cleaned assets, failed to remove @failed assets.', array('@cleaned' => $result['cleaned'], '@failed' => $result['cleaned'])));
  }
}

/**
 * Insert the search form into a text area.
 */
function _ajax_browse_asset_search_parameters_callback($form, $form_state) {
  _mediamosa_browse_asset_searchparameters($form, $form_state);
  $form = form_builder('', $form, $form_state);
  return drupal_render($form['actions']['filters']);
}

/**
 * Enrich the form with the filter items.
 *
 * @param array $form
 *   Filter form.
 * @param array $form_state
 *   Drupal form_state.
 */
function _mediamosa_browse_asset_searchparameters(&$form, $form_state = array()) {

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
      $session['hide_empty_assets'] = 1;
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
  unset($form['actions']['filters']['searchparameters']['filters']);
  foreach ($form_state['input']['filters'] as $x => $filter) {
    $form_filter = _mediamosa_browse_searchparameters_helper($x, $filter);

    $form['actions']['filters']['searchparameters']['filters']['filters[' . $x . '][item]'] = $form_filter['item'];
    $form['actions']['filters']['searchparameters']['filters']['filters[' . $x . '][item_value]'] = $form_filter['item_value'];
  }
}

/**
 * Build the asset listing.
 */
function _mediamosa_browse_asset_list() {
  // Get session.
  $session = _mediamosa_browse_asset_session();

  // Include our css.
  drupal_add_css(drupal_get_path('module', 'mediamosa_maintenance') . '/mediamosa_maintenance.css');

  $limit = $session['limit'];
  $page = isset($_GET['page']) ? (int) $_GET['page'] : 0;
  $item_count_total = 0;

  $header = array(
    array('field' => 'title', 'data' => t('Title')),
    array(
      'field' => 'app_id',
      'data' => t('App ID'),
      'class' => array('mmappid'),
    ),
    array(
      'field' => 'changed',
      'data' => t('Last modified'),
      'class' => array('mmdate') ,
      'sort' => 'desc',
    ),
    array('data' => t('Operations'), 'class' => array('mmoperation')),
  );

  $result = _mediamosa_browse_asset_execute();

  $item_count_total = isset($result['header']['item_count_total']) ? $result['header']['item_count_total'] : 0;

  $rows = array();

  // Get destination.
  $l_options = array('query' => drupal_get_destination());

  foreach ($result['items'] as $row) {
    $actions = array(
      l(t('Edit'), 'admin/mediamosa/content/asset/' . $row[mediamosa_asset_db::ID] . '/edit', $l_options),
      l(t('Delete'), 'admin/mediamosa/content/asset/' . $row[mediamosa_asset_db::ID] . '/delete', $l_options),
    );

    $rows[] = array(
      theme('l_mediamosa_asset', array('title' => _mediamosa_browse_asset_get_title($row), 'id' => $row[mediamosa_asset_db::ID])),
      array('data' => check_plain($row['app_id']), 'class' => array('mmappid')),
      array(
        'data' => theme('mediamosa_maintenance_date', array('datetime' => $row[mediamosa_asset_db::VIDEOTIMESTAMP])),
        'class' => array('mmdate'),
      ),
      array('data' => implode(' | ', $actions), 'class' => array('mmoperation')),
    );
  }

  // Our listing.
  $form = array();
  $form['list'] = array(
    '#theme' => 'mediamosa_maintenance_table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No assets found.'),
    // Pager.
    '#page' => $page,
    '#item_count_total' => $item_count_total,
    '#item_limit' => $limit,
    // Top.
    '#title' => t('Assets (@total)', array('@total' => $item_count_total)),
    '#description' => t('Assets matching the search criteria above are listed below. Click on a asset title below to view details.'),
    // Total note.
    '#caption_bottom' => t('Assets found: @total', array('@total' => $item_count_total)),
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
  $published = $session['published'];
  $trashcan = $session['trashcan'];
  $hide_empty_assets = $session['hide_empty_assets'];

  switch ($order) {
    case t('App ID'):
      $order = 'app_id';
      break;

    case t('Title'):
      $order = 'title';
      break;

    case t('Last modified'):
    default:
      $order = mediamosa_asset_db::VIDEOTIMESTAMP;
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
  if ($hide_empty_assets) {
    $params['hide_empty_assets'] = 'TRUE';
  }

  $params['published'] = $published;
  $params['trashcan'] = $trashcan;

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

  if (isset($filter_form_state['input'])) {
    foreach (array('operator', 'coll_id', 'filters', 'cql', 'op', 'published', 'trashcan') as $param) {
      if (isset($filter_form_state['input'][$param])) {
        $session[$param] = $filter_form_state['input'][$param];
      }
    }
    if (isset($filter_form_state['input']['hide_empty_assets'])) {
      $session['hide_empty_assets'] = 1;
    }
    else {
      $session['hide_empty_assets'] = 0;
    }
  }

  // Set defaults.
  $session += array(
    'order' => t('Last modified'),
    'sort' => 'desc',
    'page' => 0,
    'limit' => mediamosa_settings::maintenance_items_per_page(),
    'hide_empty_assets' => 1,
    'op' => t('Apply filter'),
    'published' => 'ALL',
    'trashcan' => 'ALL',
  );

  // Set it again.
  $_SESSION[MEDIAMOSA_BROWSE_FILTER_ASSET] = $session;

  return $session;
}

/**
 * View the details of an asset.
 */
function _mediamosa_browse_asset_page_view($form, $form_state, $asset_id) {
  // Get the asset.
  $params = array(
    'view_hidden_metadata' => 'TRUE',
    'trashcan' => 'TRUE',
  );
  $response = mediamosa_response_connector::static_do_restcall_drupal(strtr('asset/@asset_id', array('@asset_id' => rawurlencode($asset_id))), 'GET', $params);

  if ((int) $response['header']['request_result_id'] != mediamosa_error::ERRORCODE_OKAY) {
    return array('message' => array('#markup' => '<h1>Asset not found</h1>'));
  }

  // Get the asset.
  $asset = reset($response['items']);

  // Create group.
  $form['mediamosa_asset'] = array(
    '#type' => 'vertical_tabs',
  );

  // Properties.
  $form['properties'] = _mediamosa_browse_asset_page_view_properties($asset);

  // Metadata.
  $form['metadata'] = _mediamosa_browse_asset_page_view_metadata($asset);

  // Mediafiles.
  $form['mediafiles'] = _mediamosa_browse_asset_page_view_mediafile_list($asset);

  // Job list.
  $form['joblist'] = _mediamosa_browse_asset_page_view_job_list($asset);

  // Collections.
  $form['collections'] = _mediamosa_browse_asset_page_view_collections_list($asset);

  // Supplements.
  $supplements = _mediamosa_browse_asset_page_view_supplements_list($asset);
  if ($supplements) {
    $form['supplements'] = $supplements;
  }

  return $form;
}

/**
 * Build the properties.
 *
 * @param array $asset
 *   asset info.
 */
function _mediamosa_browse_asset_page_view_properties($asset) {

  $variables = array(
    'properties' => array(
      t('App ID') => $asset[mediamosa_asset_db::APP_ID],
      t('Owner ID') => $asset[mediamosa_asset_db::OWNER_ID],
      t('Group ID') => $asset[mediamosa_asset_db::GROUP_ID],
      t('Created / Changed') => $asset['videotimestamp'] . ' / ' . $asset['videotimestampmodified'],
      t('Reference ID') => $asset['reference_id'],
      t('is private') => $asset['isprivate'],
      t('is empty asset') => $asset['is_empty_asset'],
      t('is protected') => $asset['is_protected'],
      t('is inappropriate') => $asset[mediamosa_asset_db::IS_INAPPROPRIATE],
      t('is favorite') => $asset['is_favorite'],
      t('Asset type') => $asset['asset_type'],
      t('is private') => $asset['isprivate'],
      t('View restriction start') => $asset['play_restriction_start'],
      t('View restriction end') => $asset['play_restriction_end'],
      t('viewed') => $asset['viewed'],
      t('played') => $asset['played'],
    ),
  );

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
      $rows_aut2[] = array(
        _mediamosa_browse_fix_metadata_name($name),
        implode(', ', $values),
      );
    }

    $variables['properties'][t('Autorisation rules')] = array(
      'html' => TRUE,
      'data' => theme('table', array('rows' => $rows_aut2)),
    );
  }

  return array(
    '#title' => t('Properties'),
    '#type' => 'fieldset',
    '#group' => 'mediamosa_asset',
    '#collapsed' => TRUE,
    'properties' => array(
      '#markup' => theme('mediamosa_maintenance_browse_properties', $variables),
    ),
  );
}

/**
 * Show metadata.
 *
 * @param array $asset
 *   asset info.
 */
function _mediamosa_browse_asset_page_view_metadata($asset) {

  // Get all metadata properties for this app.
  $metadatas = mediamosa_asset_metadata_property::get_metadata_properties_full($asset[mediamosa_asset_db::APP_ID]);

  // Collect the metadata groups.
  $metadata_groups = array();
  foreach ($metadatas as $name => $metadata) {
    $metadata_groups[] = $metadata['propgroup_name'];
  }

  $variables = array(
    'metadata' => array(),
  );

  foreach ($metadata_groups as $metadata_group) {
    if (empty($asset[$metadata_group])) {
      continue;
    }

    foreach ($asset[$metadata_group] as $name => $value) {
      if (empty($value)) {
        continue;
      }

      $variables['metadata'][$name . ($metadatas[$name]['propdef_is_hidden'] == mediamosa_asset_metadata_property_db::IS_HIDDEN_TRUE ? ' [' . t('hidden metadata') . ']' : '')] = $value;
    }
  }

  return array(
    '#title' => t('Metadata (@total)', array('@total' => count($variables['metadata']))),
    '#type' => 'fieldset',
    '#group' => 'mediamosa_asset',
    '#collapsed' => TRUE,
    'properties' => array(
      '#markup' => theme('mediamosa_maintenance_browse_properties', $variables),
    ),
  );
}

/**
 * Build the job list.
 *
 * @param array $asset
 *   asset info.
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

  $states = array(
    mediamosa_job_db::JOB_STATUS_WAITING,
    mediamosa_job_db::JOB_STATUS_FAILED,
    mediamosa_job_db::JOB_STATUS_FINISHED,
  );

  // Get Items count.
  $item_count = $result['header']['item_count'];

  if (!$item_count) {
    return '';
  }

  $rows = array();
  foreach ($result['items'] as $item) {
    $delete = (in_array((string) $item['status'], $states)) ? l(t('Delete'), 'admin/mediamosa/asset/' . $asset_id . '/job/' . $item['id'] . '/delete') : '';
    // We will sort jobs by ID.
    $rows[$item['id']] = array(
      $item['id'],
      $item['job_type'],
      $item['status'],
      $item['started_unix'] ? format_date($item['started_unix'], 'small') : '',
      (string) $item['progress'] * 100 . '%',
      (string) $item['owner'],
      (string) $item['error_description'],
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

  return array(
    '#title' => t('Jobs (@count)', array('@count' => $item_count)),
    '#type' => 'fieldset',
    '#group' => 'mediamosa_asset',
    '#collapsed' => TRUE,
    'properties' => array(
      '#markup' => theme('table', array('header' => $header, 'rows' => $rows)),
    ),
  );
}

/**
 * Build the mediafile listing.
 *
 * @param array $asset
 *   asset info.
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

  $header = array(
    t('Property'),
    t('Value'),
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
      $actions[] = l(t('Replace'), strtr('admin/mediamosa/content/asset/@asset_id/mediafile/@mediafile_id/replace', array('@asset_id' => $item['asset_id'], '@mediafile_id' => $item['mediafile_id'])));
    }
    $actions[] = $may_download ? l(t('Download'), strtr('admin/mediamosa/asset/@asset_id/mediafile/@mediafile_id/download', array('@asset_id' => $item['asset_id'], '@mediafile_id' => $item['mediafile_id']))) : t('No download right');
    $caption = implode(' - ', $actions);

    // Mediafile properties.
    $rows = array();
    $rows[] = array(t('Mediafile id'), $item['mediafile_id']);
    $rows[] = array(t('Mount pount uri'), $item['sannas_mount_point']);
    $rows[] = array(t('Owner id'), $item['owner_id']);
    $rows[] = array(t('Group id'), $item['group_id']);
    $rows[] = array(t('MD5'), $item['metadata']['md5']);
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
    $rows[] = array(t('Audio information'), ($item['metadata']['audio_codec']) ? $item['metadata']['audio_codec'] . ' @ ' . $item['metadata']['sample_rate'] . 'Hz' . ' (' . $item['metadata']['channels'] . ')' : '');
    $rows[] = array(t('Protected'), $item['is_protected']);

    if (!empty($item['transcode_profile_id'])) {
      $transcode_profile_data = _mediamosa_get_transcode_profile_data($item['transcode_profile_id']);
      if ($transcode_profile_data) {
        $rows[] = array(
          t('Transcode profile'),
          l($transcode_profile_data[mediamosa_transcode_profile_db::PROFILE], 'admin/mediamosa/config/transcode_profile/' . $transcode_profile_data[mediamosa_transcode_profile_db::NID]),
        );
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
        $rows_aut2[] = array(
          _mediamosa_browse_fix_metadata_name($name), implode(', ', $values),
        );
      }

      $rows[] = array(
        t('Autorisarion rules'),
        theme('table', array('rows' => $rows_aut2)),
      );
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
            t('Still') . '<br />' . ($still_ticket ?
              theme('image',
                array(
                  'path' => $still_ticket,
                  'width' => 200,
                  'height' => NULL,
                ),
                FALSE) : ''),
            theme('table', array('header' => $still_header, 'rows' => $still_rows)),
          );
        }
      }
    }

    $build[] = theme('table', array(
                 'caption' => $caption,
                 'rows' => $rows,
                 'header' => $header,
               )
    );
  }

  // Add text when no mediafiles where found...
  if (!$item_count) {
    $build[] = t('This asset does not contain mediafiles.');
  }

  return array(
    '#title' => t('Mediafiles (@count)', array('@count' => $item_count)),
    '#type' => 'fieldset',
    '#group' => 'mediamosa_asset',
    '#collapsed' => TRUE,
    'properties' => array(
      '#markup' => implode('', $build),
    ),
  );
}

/**
 * Build the collections listing.
 *
 * @param array $asset
 *   asset info.
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
        l(t('Remove'), 'admin/mediamosa/content/collection/' . rawurlencode($item['coll_id']) . '/remove_asset/' . rawurlencode($asset_id), $l_options),
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

  return array(
    '#title' => t('Collections (@count)', array('@count' => $item_count)),
    '#type' => 'fieldset',
    '#group' => 'mediamosa_asset',
    '#collapsed' => TRUE,
    'properties' => array(
      '#markup' => implode('', $build),
    ),
  );
}

/**
 * Build the supplements listing.
 *
 * @param array $asset
 *   asset info.
 *
 * @return bool
 *   FALSE if there are no supplement or themed supplement info.
 *
 * @see _mediamosa_browse_asset_page_view_collections_list
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

  foreach ($result['items'] as $item) {
    $length_encoded = drupal_strlen($item['supplement_base64']);

    // If encoded data is big (more than 16K) we show just approx supplement
    // size. Normally encoded data have 1/3 redundancy.
    $approx_size = $length_encoded > 16 * 1024 ? round($length_encoded * 3 / 4) : drupal_strlen(base64_decode($item['supplement_base64']));

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

  return array(
    '#title' => t('Supplements (@count)', array('@count' => $item_count)),
    '#type' => 'fieldset',
    '#group' => 'mediamosa_asset',
    '#collapsed' => TRUE,
    'properties' => array(
      '#markup' => implode('', $build),
    ),
  );
}

/**
 * Add+Edit form for asset metadata.
 *
 * @see mediamosa_browse_asset_form_validate()
 * @see mediamosa_browse_asset_form_submit()
 */
function mediamosa_browse_asset_form($form, &$form_state, $asset_id) {

  // Do REST call.
  $result_asset = mediamosa_response_connector::static_do_restcall_drupal('asset/' . $asset_id, 'GET', array());

  if (!mediamosa_response_connector::static_result_okay($result_asset)) {
    drupal_set_message(t('Asset with ID @asset_id does not exists.', array('@asset_id' => $asset_id)), 'warning');
    drupal_goto('admin/mediamosa/content/asset');
  }

  $asset = reset($result_asset['items']);

  // Get all metadata properties for this app.
  $metadatas = mediamosa_asset_metadata_property::get_metadata_properties_full();

  // Collect the metadata groups.
  $metadata_groups = array();
  foreach ($metadatas as $metadata) {
    $metadata_groups[$metadata['propgroup_name']] = $metadata['propgroup_id'];
  }

  // Empty form.
  $form = array();

  // Store asset_id.
  $form['asset_id'] = array(
    '#type' => 'hidden',
    '#value' => $asset_id,
  );

  // Build form for editing the metadata.
  foreach (array_keys($metadata_groups) as $group_name) {
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
          '#title' => $name_fixed,
          '#collapsible' => TRUE,
          '#description' => t("Clear an item to delete it from the asset's @name_fixed metadata.", array('@name_fixed' => $name_fixed)),
        );
      }
      $x = 0;
      foreach ($metadata_values as $metadata_value) {
        // Make the edit box.
        // current_ in front of the name makes sure we dont conflict with other
        // names in the form.
        $form[$group_name]['metadata_' . $metadata_name . '[' . $x++ . ']'] = array(
          '#type' => drupal_strlen($metadata_value) > 80 ? 'textarea' : 'textfield',
          '#default_value' => $metadata_value,
          '#title' => _mediamosa_browse_fix_metadata_name($metadata_name),
        );
      }
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
    // No Drupal validation when pressing 'Save changes'.
    '#limit_validation_errors' => array(),
    // Must specify submit hook, else #limit_validation_errors will not work.
    '#submit' => array('mediamosa_browse_asset_form_submit'),
  );

  $options_metadata_groups = array('' => '');

  ksort($metadata_groups);
  ksort($metadatas);
  foreach ($metadata_groups as $metadata_group_name => $metadata_group_id) {
    foreach ($metadatas as $metadata) {
      if ($metadata['propgroup_id'] != $metadata_group_id) {
        continue;
      }

      $options_metadata_groups[_mediamosa_browse_fix_metadata_name($metadata_group_name)][$metadata['propdef_name']] = _mediamosa_browse_fix_metadata_name($metadata['propdef_name']);
    }
  }

  $form['actions']['new_metadata_property'] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#title' => t('Metadata field'),
    '#description' => t('Enter value for the selected metadata field.'),
    '#options' => $options_metadata_groups,
  );

  $form['actions']['new_metadata_value'] = array(
    '#type' => 'textarea',
    '#required' => TRUE,
    '#title' => t('Value'),
    '#description' => t('Enter value for the selected metadata field.'),
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add metadata'),
  );

  // Form action, but do not repeat below.
  $form['actions']['#type'] = 'actions';
  $form['actions']['#mediamosa_no_clone'] = TRUE;

  // Show as settings form look.
  $form['#theme'] = 'system_settings_form';

  return $form;
}

/**
 * Form validation handler for mediamosa_browse_asset_form().
 *
 * @see mediamosa_browse_asset_form()
 * @see mediamosa_browse_asset_form_submit()
 */
function mediamosa_browse_asset_form_validate($form, &$form_state) {
  // Save metadata edit.
  if ($form_state['values']['op'] == t('Save changes')) {
    // Get the metadata list.
    $metadatas = mediamosa_asset_metadata_property::get_metadata_properties_full();

    foreach ($metadatas as $name => $metadata) {
      // So it matches.
      $name_current = 'metadata_' . $name;

      // Check if $name is a valid metadata field.
      // We also skip checks on non-changed values.
      if (!isset($form_state['values'][$name_current]) || $form_state['values'][$name_current] == '') {
        continue;
      }

      // Now validate the value, empty is allowed.
      switch ($metadata['propdef_type']) {
        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
          break;

        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          try {
            $form_state['values'][$name_current] = mediamosa_type::check($name, mediamosa_sdk::TYPE_DATETIME, $form_state['values'][$name_current]);
          }
          catch (mediamosa_exception $e) {
            form_set_error($name_current, t('Invalid date, must be in format YYYY-MM-DD or YYYY-MM-DD 00:00:00'));
          }
          break;

        case mediamosa_asset_metadata_property_db::TYPE_INT:
          if (!is_numeric($form_state['values'][$name_current])) {
            form_set_error($name_current, t('This value must be numeric.'));
          }
          break;

        default:
          assert(0);
          drupal_set_message(t('Unknown/unsupported metadata type.'), 'error');
          break;
      }
    }
  }
}

/**
 * Form submission handler for mediamosa_browse_asset_form().
 *
 * @see mediamosa_browse_asset_form()
 * @see mediamosa_browse_asset_form_validate()
 */
function mediamosa_browse_asset_form_submit($form, &$form_state) {

  // Difference between submit hook and button pressed with submit hook.
  $values = empty($form_state['input']) ? $form_state['values'] : $form_state['input'];

  // Get the asset_id.
  $asset_id = $values['asset_id'];

  // Start empty.
  $params = array();

  if ($form_state['values']['op'] == t('Save changes')) {
    foreach ($values as $name => $value) {
      // Must start with current_.
      if (drupal_substr($name, 0, 9) != 'metadata_') {
        continue;
      }

      // Get the original name.
      $params[drupal_substr($name, 9)] = $value;
    }
  }
  elseif ($form_state['values']['op'] == t('Add metadata')) {
    if (!empty($values['new_metadata_property'])) {
      $params[$values['new_metadata_property']] = $values['new_metadata_value'];
      // Use 'append' instead of 'replace'.
      $params['action'] = 'append';
    }
  }

  // Make sure we got something.
  if (!empty($params)) {
    // Get the asset.
    $asset = mediamosa_asset::get($asset_id);

    // Needed.
    $params['app_id'] = $asset['app_id'];
    $params['user_id'] = $asset['owner_id'];

    $uri = strtr('asset/@asset_id/metadata', array('@asset_id' => $asset_id));

    // Execute the metadata post.
    $result = mediamosa_response_connector::static_do_restcall($uri, 'POST', $params);

    if (mediamosa_response_connector::static_result_okay($result)) {
      drupal_set_message(t('The changes to the asset metadata have been saved.'));
      drupal_goto('admin/mediamosa/content/asset/' . rawurlencode($asset_id) . '/edit');
    }
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
    t('Are you sure you want to delete this asset?'),
    // Cancel path:
    mediamosa_settings::get_url_asset($asset_id),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
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
    drupal_goto('admin/mediamosa/content/asset');
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
    drupal_goto('admin/mediamosa/content/asset');
  }

  drupal_goto(mediamosa_settings::get_url_asset($asset_id));
}
