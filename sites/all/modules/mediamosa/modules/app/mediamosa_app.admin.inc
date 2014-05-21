<?php

/**
 * @file
 * Admin page callbacks for the MediaMosa App module.
 */

/**
 * Show a listing of apps.
 */
function mediamosa_app_listing_form() {

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
    t('Operations'),
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

  // Get all diskspace used.
  $diskspace_used = mediamosa_webservice_app::diskspace_used_get_all_status();

  // Get destination.
  $options = array('query' => drupal_get_destination());

  $rows = array();
  foreach ($apps as $app) {
    $app_id = $app[mediamosa_app_db::APP_ID];

    $actions = array();
    if (node_access('update', $nodes[$app[mediamosa_app_db::NID]])) {
      $actions[] = l(t('Edit'), 'admin/mediamosa/config/app/' . $app[mediamosa_app_db::NID] . '/edit', $options);
      $actions[] = l(t('Metadata'), 'admin/mediamosa/config/app/' . $app[mediamosa_app_db::NID] . '/metadata/tag', $options);
      $actions[] = l(t('Storage'), 'admin/mediamosa/config/app/' . $app[mediamosa_app_db::NID] . '/storage', $options);
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
    if (!isset($diskspace_used[$app[mediamosa_app_db::APP_ID]])) {
      $diskspace_used[$app[mediamosa_app_db::APP_ID]] = 0;
    }
    $perc = round($diskspace_used[$app[mediamosa_app_db::APP_ID]] / ($app[mediamosa_app_db::QUOTA] * 1024 * 1024 * 1024) * 100);
    $percentagebar = '<div class="progressbar" style="width:100px;height:10px" rel="' . $perc . '"></div>';
    $rows[] = array(
      $app[mediamosa_app_db::APP_ID],
      array(
        'data' => l($app[mediamosa_app_db::APP_NAME], 'admin/mediamosa/config/app/' . $app[mediamosa_app_db::NID]),
        'field' => $app[mediamosa_app_db::APP_NAME],
      ),
      $percentagebar . ' (' . format_size($diskspace_used[$app[mediamosa_app_db::APP_ID]]) . ' / ' . $app[mediamosa_app_db::QUOTA] . ' GB)',
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
    '#empty' => t('You have no client applications. Click on the <em>Add</em> tab to create one.'),
  );
  $form['pager_bottom'] = array('#theme' => 'pager');

  drupal_add_library('system', 'ui.progressbar');
  drupal_add_js("
  (function($) {
    $(function() {
      $('.progressbar').each(function() {
        var val = $(this).attr('rel');
        $(this).progressbar({ value: parseInt(val) });
      });
    });
  })(jQuery);",
    'inline'
  );

  return $form;
}

/**
 * Show the storage location listing.
 *
 * The listing is used to show current mediafiles location(s) and choose to
 * move them to the current storage location.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 * @param object $node
 *   The application node.
 * @param string is_still
 *   Either 'stills' or ''.
 *
 * @return array
 *   The form.
 */
function mediamosa_app_storage_form($form, $form_state, $node) {

  // Make sure its the right node type.
  if ($node->type != 'mediamosa_app') {
    drupal_set_message('Node object not an client application', 'error');
    drupal_goto('admin/mediamosa/config/app');
  }

  // Collect the storage information.
  $storage_mf = mediamosa_storage::app_collect_storage_locations($node->{mediamosa_app_db::APP_ID}, FALSE);
  $storage_stills = mediamosa_storage::app_collect_storage_locations($node->{mediamosa_app_db::APP_ID}, TRUE);

  // Our header.
  $header = array(
    array('data' => t('Name'), 'class' => 'mediamosa-app-storage-name'),
    array('data' => t('Description'), 'class' => 'mediamosa-app-storage-description'),
    array('data' => t('Location'), 'class' => 'mediamosa-app-storage-location'),
    array('data' => t('Valid location'), 'class' => 'mediamosa-app-storage-valid'),
    array('data' => t('Count'), 'class' => 'mediamosa-app-storage-count'),
  );

  $form = array();
  $form['storage'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['storage_mediafiles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mediafiles (@count)', array('@count' => $storage_mf['total'])),
    '#collapsible' => FALSE,
    '#group' => 'storage',
  );

  $rows = array();
  foreach ($storage_mf['data'] as $uri => $item) {
    $rows[] = array(
      array('data' => $item['name'] . '<br />' . $item['path']),
      array('data' => $item['description']),
      array('data' => $item['is_local'] ?  t('@path (local)', array('@path' => $item['path'])) : t('@uri (external)', array('@uri' => $uri))),
      array('data' => $item['wrapper_instance'] ? t('Yes') : '<b>' . t('No') . '</b>', 'class' => 'item-center'),
      array('data' => $item['count'], 'class' => 'item-center'),
    );
  }

  // Build the table.
  $form['storage_mediafiles']['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('Did not find any mediafiles on any location for this application.')
  );

  $count_move_jobs = mediamosa_job::count_application_jobs_by_type($node->{mediamosa_app_db::APP_ID}, array(mediamosa_job_db::JOB_TYPE_TRANSFER_MEDIA_MOVE));

  // Add submit when we have items to move.
  $wrapper_info = mediamosa_io_streamwrapper::get_wrapper_info($storage_mf['app_uri']);
  if ($storage_mf['total_to_move']) {
    if (!empty($wrapper_info)) {
      if ($count_move_jobs) {
        $form['storage_mediafiles']['help'] = array(
          '#prefix' => '<div class="mediamosa-vtabs-submit-text">',
          '#suffix' => '</div>',
          '#markup' => t('There are @count move jobs in the job queue for this client application, please wait till these are processed. Make sure at least one of your job processor servers has file transfer enabled.', array('@count' => $count_move_jobs)),
        );
      }
      else {
        $form['storage_mediafiles']['help'] = array(
          '#prefix' => '<div class="mediamosa-vtabs-submit-text">',
          '#suffix' => '</div>',
          '#markup' => t('Press the move button to move @count mediafile(s) to @name (@description).', array('@count' => $storage_mf['total_to_move'], '@name' => $wrapper_info['name'], '@description' => trim($wrapper_info['description'], '.'))),
        );
        $form['storage_mediafiles']['submit'] = array(
          '#prefix' => '<div class="form-actions form-wrapper">',
          '#suffix' => '</div>',
          '#type' => 'submit',
          '#value' => t('Move mediafiles'),
        );
      }

    }
    else {
      $form['storage_mediafiles']['help'] = array(
        '#value' => t('Can not move file(s) to storage location defined by client application.'),
      );
    }
  }

  $form['storage_stills'] = array(
    '#type' => 'fieldset',
    '#title' => t('Stills (@count)', array('@count' => $storage_stills['total'])),
    '#collapsible' => FALSE,
    '#group' => 'storage',
  );

  $rows = array();
  foreach ($storage_stills['data'] as $uri => $item) {
    $rows[] = array(
      array('data' => $item['name'] . '<br />' . $item['path']),
      array('data' => $item['description']),
      array('data' => $item['is_local'] ?  t('@path (local)', array('@path' => $item['path'])) : t('@uri (external)', array('@uri' => $uri))),
      array('data' => $item['wrapper_instance'] ? t('Yes') : '<b>' . t('No') . '</b>', 'class' => 'item-center'),
      array('data' => $item['count'], 'class' => 'item-center'),
    );
  }

  // Build the table.
  $form['storage_stills']['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('Did not find any stills on any location for this application.')
  );

  // Add submit when we have items to move.
  $wrapper_info = mediamosa_io_streamwrapper::get_wrapper_info($storage_stills['app_uri']);
  if ($storage_stills['total_to_move']) {
    if (!empty($wrapper_info)) {
      if ($count_move_jobs) {
        $form['storage_mediafiles']['help'] = array(
          '#prefix' => '<div class="mediamosa-vtabs-submit-text">',
          '#suffix' => '</div>',
          '#markup' => t('There are @count move jobs in the job queue for this client application, please wait till these are processed. Make sure at least one of your job processor servers has file transfer enabled.', array('@count' => $count_move_jobs)),
        );
      }
      else {
        $form['storage_stills']['help'] = array(
          '#prefix' => '<div class="mediamosa-vtabs-submit-text">',
          '#suffix' => '</div>',
          '#markup' => t('Press the move button to move @count stills(s) to @name (@description).', array('@count' => $storage_stills['total_to_move'], '@name' => $wrapper_info['name'], '@description' => trim($wrapper_info['description'], '.'))),
        );

        $form['storage_stills']['submit'] = array(
          '#prefix' => '<div class="form-actions form-wrapper">',
          '#suffix' => '</div>',
          '#type' => 'submit',
          '#value' => t('Move stills'),
        );
      }
    }
    else {
      $form['storage_stills']['help'] = array(
        '#value' => t('Can not move file(s) to storage location defined by client application.'),
      );
    }
  }

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  return $form;
}

/**
 * Implentation hook_submit().
 */
function mediamosa_app_storage_form_submit($form, $form_state) {
  // Get form submit.
  $values = $form_state['values'];
  $nid = $values['nid'];

  switch ($values['op']) {
    case t('Move mediafiles'):
      $destination = 'admin/mediamosa/config/app/' . $nid . '/storage';
      if (isset($_GET['destination'])) {
        $destination = $_GET['destination'];
        unset($_GET['destination']);
      }
      drupal_goto('admin/mediamosa/config/app/' . $nid . '/storage/mediafiles/confirm', array('query' => array('destination' => $destination)));

    case t('Move stills'):
      $destination = 'admin/mediamosa/config/app/' . $nid . '/storage';
      if (isset($_GET['destination'])) {
        $destination = $_GET['destination'];
        unset($_GET['destination']);
      }
      drupal_goto('admin/mediamosa/config/app/' . $nid . '/storage/stills/confirm', array('query' => array('destination' => $destination)));
  }

  drupal_goto('admin/mediamosa/config/app/' . $nid . '/storage');
}

/**
 * Show confirm for moving mediafiles/stills.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 * @param object $node
 *   The application node.
 * @param bool is_still
 *   TRUE if mediafile is still.
 *
 * @return array
 *   The confirm form.
 */
function mediamosa_app_storage_form_confirm($form, $form_state, $node, $type) {
  // Collect the storage information.
  $storage = mediamosa_storage::app_collect_storage_locations($node->{mediamosa_app_db::APP_ID}, ($type == 'stills'));
  $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/mediamosa/config/app/' . $node->nid . '/storage';

  $form = array();
  $form['is_still'] = array(
    '#type' => 'value',
    '#value' => $type == 'stills',
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  $form['destination_uri'] = array(
    '#type' => 'value',
    '#value' => $storage['app_uri'],
  );

  return confirm_form(
    $form,
    t("Are you sure you want to move @count @type?", array('@count' => $storage['total_to_move'], '@type' => $type == 'stills' ? 'stills' : 'mediafiles')),
    $destination,
    t('The file move will be done by (transfer) jobs and will take some time to complete. Pressing confirm will create all necessary jobs for the transfer.'),
    t('Confirm move'),
    t('Cancel')
  );
}

/**
 * Confirm submitted for moving mediafiles/stills.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   An associative array containing the current state of the form.
 */
function mediamosa_app_storage_form_confirm_submit($form, $form_state) {

  $values = $form_state['values'];
  if ($values['op'] != t('Confirm move')) {
    drupal_goto('admin/mediamosa/config/app/' . $values['nid'] . '/storage');
  }

  // Values we need.
  $nid = $values['nid'];
  $is_still = $values['is_still'] ? TRUE : FALSE;
  $destination_uri = $values['destination_uri'];

  // Load node.
  $node = node_load($nid);

  // Make sure its the right node type.
  if ($node->type != 'mediamosa_app') {
    drupal_set_message('Node object not an client application', 'error');
    drupal_goto('admin/mediamosa/config/app');
  }

  // Check access. Although this would not be possible.
  if (!node_access('update', $node)) {
    drupal_set_message('Access denied', 'error');
    drupal_goto('admin/mediamosa/config/app');
  }

  // Collect the storage information.
  $storage = mediamosa_storage::app_collect_storage_locations($node->{mediamosa_app_db::APP_ID}, $is_still);
  $wrapper_info = mediamosa_io_streamwrapper::get_wrapper_info($destination_uri);

  // Create the batch.
  $batch = array(
    'operations' => array(
     ),
    'finished' => '_mediamosa_app_storage_move_finished',
    'title' => t(
      'Populating jobs; moving @count @type to @name (@description).',
      array(
        '@count' => $storage['total_to_move'],
        '@type' => $is_still ? 'stills' : 'mediafiles',
        '@name' => $wrapper_info['name'],
        '@description' => trim($wrapper_info['description'], '.')
      )
    ),
    'file' => drupal_get_path('module', 'mediamosa_app') . '/mediamosa_app.admin.inc',
    'init_message' => t('Initializing to start job population.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('The job population process has encounted an error, please check !logs.', array('!logs' => l(t('the logs'), 'admin/reports/dblog'))),
  );

  foreach ($storage['data'] as $sannas_mount_point => $item) {
    if ($item['movable']) {
      // Create per uri a batch.
      $batch['operations'][] = array('_mediamosa_app_storage_move', array($node->{mediamosa_app_db::APP_ID}, $is_still, $sannas_mount_point, $destination_uri));
    }
  }

  // Create batch and run.
  batch_set($batch);
}

function _mediamosa_app_storage_move($app_id, $is_still, $sannas_mount_point, $destination_uri, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->condition(mediamosa_asset_mediafile_db::APP_ID, $app_id)
      ->condition(mediamosa_asset_mediafile_db::IS_STILL, $is_still ? mediamosa_asset_mediafile_db::IS_STILL_TRUE : mediamosa_asset_mediafile_db::IS_STILL_FALSE)
      ->condition(mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT, $sannas_mount_point)
      ->countQuery()
      ->execute()
      ->fetchField();
  }

  // Get stream wrapper.
  $wrapper = mediamosa_io::require_stream_wrapper_instance_by_uri($destination_uri);

  // Per 10.
  $limit = 10;
  $mediafiles = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
    ->fields(
      'mf',
      array(
        mediamosa_asset_mediafile_db::ID,
        mediamosa_asset_mediafile_db::OWNER_ID,
      )
    )
    ->condition(mediamosa_asset_mediafile_db::APP_ID, $app_id)
    ->condition(mediamosa_asset_mediafile_db::IS_STILL, $is_still ? mediamosa_asset_mediafile_db::IS_STILL_TRUE : mediamosa_asset_mediafile_db::IS_STILL_FALSE)
    ->condition(mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT, $sannas_mount_point)
    ->range($context['sandbox']['progress'], $limit)
    ->execute();

  // Now add transfer jobs.
  foreach ($mediafiles as $mediafile) {
    mediamosa_job::create_job_media_move(
      $app_id,
      $mediafile[mediamosa_asset_mediafile_db::OWNER_ID],
      TRUE,
      $mediafile[mediamosa_asset_mediafile_db::ID],
      $wrapper->get_storage_profile_id(),
      $wrapper->get_path(),
      // Positive number means less prio.
      50
    );

    $context['sandbox']['progress']++;
  }

  $context['results'] = $context['sandbox']['max'];

  // Inform the batch engine that we are not finished, and provide an estimation
  // of the completion level we reached.
  if ($mediafiles->rowCount() > 0 || $context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    $context['finished'] = 1;
  }
}

/**
 * The 'finished' callback.
 */
function _mediamosa_app_storage_move_finished($success, $results, $operations) {
  if ($success) {
    // Here we do something meaningful with the results.
    $message = t('@jobs moving jobs queued.', array('@jobs' => (int) $results));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
  }

  drupal_set_message($message);
}
