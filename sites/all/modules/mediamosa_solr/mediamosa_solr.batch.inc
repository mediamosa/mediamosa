<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Batch code for Solr extension.
 */

/**
 * Start reindex batch.
 *
 * Will fill reindex table to use cron to index the metadata.
 */
function _mediamosa_solr_batch_reindex_start($purge_index) {

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

  // Add the operations.
  if ($purge_index) {
    $batch['operations'][] = array('_mediamosa_solr_batch_purge', array());
  }

  $batch['operations'][] = array('_mediamosa_solr_batch_reindex', array());

  // Create batch and run.
  batch_set($batch);

  // Done.
}

function _mediamosa_solr_batch_purge(&$context) {

  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = 1;
  }

  $context['sandbox']['progress'] = 0;
  $context['sandbox']['current_node'] = 0;
  $context['sandbox']['max'] = 1;
  $context['message'] = t('Purging Solr index');

  // Purge.
  mediamosa_solr::purge_solr_index();

  // Done.
  $context['sandbox']['progress'] = 1;
}

function _mediamosa_solr_batch_reindex(&$context) {

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
  mediamosa_solr::queue_assets($asset_ids);

  // Update progress.
  $context['sandbox']['progress'] += $limit;

// Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
  else {
    $context['finished'] = 1;
    $context['results'][] = $context['sandbox']['max'];
  }
}

/**
 * 'finished' callback.
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
