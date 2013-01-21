<?php
/**
 * @file
 * Batch code for Solr module.
 */

/**
 * Start repopulate queue batch.
 *
 * Will fill reindex table to use cron to index the metadata.
 */
function _mediamosa_solr_batch_reindex_start($purge_solr) {

  // Create the batch.
  $batch = array(
    'operations' => array(
     ),
    'finished' => '_mediamosa_solr_batch_finished',
    'title' => t('Re-indexing the Solr search database'),
    'init_message' => t('Initializing to start to fill re-index queue.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('The re-index process has encounted an error, please check !logs.', array('!logs' => l(t('the logs'), 'admin/reports/dblog'))),
  );

  // Disable sync.
  $batch['operations'][] = array('_mediamosa_solr_batch_disable_queue', array());

  if ($purge_solr) {
    $batch['operations'][] = array('_mediamosa_solr_batch_purge_solr', array());
  }

  $batch['operations'][] = array('_mediamosa_solr_batch_queue', array());

  // Enable sync (when it was on).
  $batch['operations'][] = array('_mediamosa_solr_batch_enable_queue', array());

  // Create batch and run.
  batch_set($batch);

  // Done.
}

/**
 * Purge Solr database.
 *
 * @param array $context
 *   The batch context.
 */
function _mediamosa_solr_batch_disable_queue(&$context) {

  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = 1;
  }

  $context['sandbox']['progress'] = 0;
  $context['sandbox']['current_node'] = 0;
  $context['sandbox']['max'] = 1;
  $context['message'] = t('Disabling the Solr indexing cron');

  // Disable.
  $context['sandbox']['mediamosa_solr_enable_queue'] = variable_get('mediamosa_solr_enable_queue', TRUE);
  variable_set('mediamosa_solr_enable_queue', FALSE);

  // Done.
  $context['sandbox']['progress'] = 1;
}

/**
 * Purge Solr database.
 *
 * @param array $context
 *   The batch context.
 */
function _mediamosa_solr_batch_enable_queue(&$context) {

  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = 1;
  }

  $context['sandbox']['progress'] = 0;
  $context['sandbox']['current_node'] = 0;
  $context['sandbox']['max'] = 1;
  $context['message'] = t('Reverting status on/off Solr indexing cron');

  // Return the previous setting..
  $context['sandbox']['mediamosa_solr_enable_queue'] = isset($context['sandbox']['mediamosa_solr_enable_queue']) ? $context['sandbox']['mediamosa_solr_enable_queue'] : TRUE;
  variable_set('mediamosa_solr_enable_queue', $context['sandbox']['mediamosa_solr_enable_queue']);

  // Done.
  $context['sandbox']['progress'] = 1;
}

/**
 * Purge Solr database.
 *
 * @param array $context
 *   The batch context.
 */
function _mediamosa_solr_batch_purge_solr(&$context) {

  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = 1;
  }

  $context['sandbox']['progress'] = 0;
  $context['sandbox']['current_node'] = 0;
  $context['sandbox']['max'] = 1;
  $context['message'] = t('Purging Solr index');

  // Purge.
  mediamosa_solr::purgeSolr();

  // Done.
  $context['sandbox']['progress'] = 1;
}

function _mediamosa_solr_batch_queue(&$context) {

  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = mediamosa_db::db_select(mediamosa_asset_db::TABLE_NAME, 'a')->countQuery()->execute()->fetchField();
  }

  // Do 50 assets per cycle.
  $limit = 1000;

  $asset_ids = mediamosa_db::db_select(mediamosa_asset_db::TABLE_NAME, 'a')
    ->fields('a', array(mediamosa_asset_db::ID))
    ->range($context['sandbox']['progress'], $limit)
    ->orderBy(mediamosa_asset_db::CREATED, 'ASC') // make sure its order of creation so we dont forget any.
    ->execute()
    ->fetchCol(mediamosa_asset_db::ID);

  // Queue the IDs
  mediamosa_solr::queueAssets($asset_ids);

  // Update progress.
  $context['sandbox']['progress'] += $limit;

  // Inform the batch engine that we are not finished, and provide an estimation
  // of the completion level we reached.
  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    $context['finished'] = 1;
    $context['results'][] = $context['sandbox']['max'];
  }
}

/**
 * The 'finished' callback.
 */
function _mediamosa_solr_batch_finished($success, $results, $operations) {
  if ($success) {
    // Here we do something meaningful with the results.
    $asset_processed = reset($results);

    $message = t('@assets assets queued for re-index.', array('@assets' => (int) $asset_processed));
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
  }

  drupal_set_message($message);
}
