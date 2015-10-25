<?php
/**
 * @file
 * Drush integration of MediaMosa Solr module.
 */

/**
 * Implements hook_drush_help().
 */
function mediamosa_solr_drush_help($section) {
  switch ($section) {
    case 'meta:mediamosa_solr:title':
      return dt('MediaMosa Solr module.');

    case 'meta:mediamosa_solr:summary':
      return dt('Drush commands for the MediaMosa Solr module.');
  }
}


/**
 * Implements hook_drush_command().
 */
function mediamosa_solr_drush_command() {
  $items = array();
  $items['mediamosa-solr-info'] = array(
    'description' => dt('Show the complete Solr information. Schema version information retrieval will only work on Solr 4.x and higher.'),
    'aliases' => array('mm-solr-info'),
  );

  $items['mediamosa-solr-info-queue'] = array(
    'description' => dt('Show the Solr queue data.'),
    'aliases' => array('mm-solr-info-queue'),
  );

  $items['mediamosa-solr-ping'] = array(
    'description' => dt('Ping the Solr instance'),
    'aliases' => array('mm-solr-ping'),
  );

  $items['mediamosa-solr-queue-all'] = array(
    'description' => '.',
    'arguments' => array(
      'asset_id' => dt('Re-queue all assets'),
    ),
    'options' => array(
      'purge' => dt('Purge the Solr index before re-queuing Solr. This is only required when Solr contains assets that no longer are in MediaMosa database or when schema has been changed.'),
    ),
    'aliases' => array('mm-solr-queue-all'),
  );

  $items['mediamosa-solr-queue-asset'] = array(
    'description' => '.',
    'arguments' => array(
      'asset_id' => dt('The ID of the asset to queue (queue needs to be processed before the asset will be in Solr).'),
    ),
    'aliases' => array('mm-solr-queue-asset'),
  );

  $items['mediamosa-solr-index-asset'] = array(
    'description' => '.',
    'arguments' => array(
      'asset_id' => dt('The ID of the asset to insert (no queue, will insert asset immediately into Solr).'),
    ),
    'aliases' => array('mm-solr-index-asset'),
  );

  $items['mediamosa-solr-queue-disable'] = array(
    'description' => dt('Disable the queue.'),
    'aliases' => array('mm-solr-queue-disable'),
  );

  $items['mediamosa-solr-queue-enable'] = array(
    'description' => dt('Enable the queue.'),
    'aliases' => array('mm-solr-queue-enable'),
  );

  return $items;
}

/**
 * Callback function for mediamosa-solr-info command.
 */
function drush_mediamosa_solr_info() {

  try {
    drush_print(dt('Url: @url', array('@url' => mediamosa_solr_apache_solr_service::mediamosaGetUrl())));
    $schema = mediamosa_solr_apache_solr_service::solrServiceGetSchemaInfo();

    if ($schema['success']) {
      drush_print(dt('Schema used: @schema', array('@schema' => $schema['name'])));
    }
    else {
      drush_print(dt('Unable to get schema version.'));
    }
  }
  catch (Exception $e) {
    drush_print(dt("Caught exception: '@exception'", array('@exception' => $e->getMessage())));
  }

  drush_mediamosa_solr_info_queue();
}

/**
 * Callback function for mediamosa-solr-info-queue command.
 */
function drush_mediamosa_solr_info_queue() {

  try {
    $count = mediamosa_solr::queueGetCount();
    $enabled = variable_get('mediamosa_solr_enable_queue', TRUE);
    $running = !lock_may_be_available(mediamosa_solr::LOCK_NAME_PROCESSING);
    $cron_last = variable_get('mediamosa_solr_queue_cron_last', NULL);

    $failures = variable_get('mediamosa_solr_queue_failures', NULL);
    $failures = $failures ? array(dt('Warning: indexer generated an unrecoverable error; ') . check_plain(var_export($failures, TRUE))) : array();
    $stats = variable_get('mediamosa_solr_queue_stats', array());
    $average_stats = 0;
    foreach ($stats as $stat) {
      $average_stats += $stat;
    }

    $queue_message[] = $enabled ? dt('Enabled') : dt('Queue is disabled!');

    if ($count) {
      $queue_message[] = !empty($running) ? t('is processing') : t('waiting for next interval');
    }

    $queue_message[] = $count ? format_plural($count, 'one asset queued for indexing', '@count assets queued for indexing') : t('queue is empty');
    $queue_message[] = !$running && (!$average_stats || empty($stats)) ? t('no assets indexed for the last @count runs', array('@count' => mediamosa_solr::TAKE_AVERAGE_LAST_X_MIN)) : t('indexing @stats assets per run', array('@stats' => round($average_stats / count($stats), 2)));

    // Has not run OR has run more than 5 minutes ago last time.
    if ($cron_last < (REQUEST_TIME - (5 *60))) {
      $failures[] = t('Warning: the solr queue has not run for more than 5 minutes.');
    }
    elseif (empty($cron_last)) {
      $failures[] = t('Warning: the solr queue has not run yet.');
    }

    drush_print(dt("Queue status: @queue_message", array('@queue_message' => implode(', ', $queue_message))));
    drush_print(dt("Failures reported: @failures", array('@failures' => empty($failures) ? 'Nothing reported.' : implode(', ', $failures))));
  }
  catch (Exception $e) {
    drush_print(dt("Caught exception: '@exception'", array('@exception' => $e->getMessage())));
  }
}

/**
 * Callback function for mediamosa-solr-ping command.
 */
function drush_mediamosa_solr_ping() {

  $ping_success = false;
  try {
    drush_print(dt("Pinging server on @url.", array('@url' => mediamosa_solr_apache_solr_service::mediamosaGetUrl())));
    $ping_success = mediamosa_solr_apache_solr_service::solrServicePing();
  }
  catch (Exception $e) {
    drush_print(dt("Caught exception: '@exception'", array('@exception' => $e->getMessage())));
  }

  if ($ping_success) {
    drush_print(dt('Ping successful!'));
  }
  else {
    drush_print(dt('Ping Failed!'));
  }
}

/**
 * Callback function for mediamosa-solr-queue_all command.
 */
function drush_mediamosa_solr_queue_all() {
  module_load_include('inc', 'mediamosa_solr', 'mediamosa_solr');

  $purge = drush_get_option('purge', FALSE);

  $batch = _mediamosa_solr_batch_get($purge);
  $batch['progressive'] = FALSE;
  batch_set($batch);

  drush_backend_batch_process();
}

/**
 * Callback function for mediamosa-solr-queue-asset command.
 */
function drush_mediamosa_solr_queue_asset($asset_id) {
  if (_drush_mediamosa_solr_asset_exists($asset_id)) {
    $errors = array();
    mediamosa_solr::queueAssets(array($asset_id), $errors);

    if (empty($errors)) {
      drush_print(dt('Asset queued.'));
    }
    else {
      drush_print(dt('Asset failed to queue, error log below.'));
      foreach ($errors as $error) {
        drush_print($error);
      }
    }
  }
}

/**
 * Callback function for mediamosa-solr-index-asset command.
 */
function drush_mediamosa_solr_index_asset($asset_id) {

  if (_drush_mediamosa_solr_asset_exists($asset_id)) {
    $errors = array();
    mediamosa_solr::insertAssets(array($asset_id), $errors);

    if (empty($errors)) {
      drush_print(dt('Asset inserted.'));
    }
    else {
      drush_print(dt('Asset failed to insert, error log below.'));
      foreach ($errors as $error) {
        drush_print($error);
      }
    }
  }
}

/**
 * Callback function for mediamosa-solr-queue-enable command.
 */
function drush_mediamosa_solr_queue_enable() {
  if (variable_get('mediamosa_solr_enable_queue', TRUE)) {
    drush_print(dt('Queue is already enabled, there are @count asset(s) queued.', array('@count' => mediamosa_solr::queueGetCount())));
  }
  else {
    variable_set('mediamosa_solr_enable_queue', TRUE);
    drush_print(dt('Queue enabled, there are @count asset(s) queued.', array('@count' => mediamosa_solr::queueGetCount())));
  }
}

/**
 * Callback function for mediamosa-solr-queue-enable command.
 */
function drush_mediamosa_solr_queue_disable() {
  if (!variable_get('mediamosa_solr_enable_queue', TRUE)) {
    drush_print(dt('Queue is already disabled, there are @count asset(s) queued.', array('@count' => mediamosa_solr::queueGetCount())));
  }
  else {
    variable_set('mediamosa_solr_enable_queue', FALSE);
    drush_print(dt('Queue disabled, there are @count asset(s) queued.', array('@count' => mediamosa_solr::queueGetCount())));
  }
}

/**
 * Helper for testing if asset exists.
 *
 * @param string $asset_id
 *   The asset ID to test.
 *
 * @return bool
 *   TRUE when exists, FALSE otherwise.
 */
function _drush_mediamosa_solr_asset_exists($asset_id) {
  try {
    mediamosa_asset::must_exists($asset_id);
  }
  catch (mediamosa_exception_error $e) {
    if ($e->getCode() == mediamosa_error::ERRORCODE_ASSET_NOT_FOUND) {
      drush_print(dt('Asset not found.'));
    }
    else {
      drush_print($e->getMessage());
    }
    return FALSE;
  }

  return TRUE;
}
