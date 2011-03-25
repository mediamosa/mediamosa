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
 * MediaMosa Migration.
 */

module_load_include('inc', 'mediamosa_migration', 'mediamosa_migration');

/**
 * The page to execute a REST call.
 */
function mediamosa_migration_page_form() {
  $build = array();

  $build['restcall'] = drupal_get_form('_mediamosa_migration_form');

  return $build;
}

function _mediamosa_migration_form() {
  $form = array();

  $form['migration'] = array(
    '#type' => 'fieldset',
    '#title' => t('MediaMosa 1.7.x migration'),
  );
  // Get the status.
  $status = variable_get('mediamosa_migration_status');
  if (!is_array($status) && $status == 'done') {
    $form['migration']['text'] = array(
      '#markup' => '<p>' . t('Migration has been completed, you can turn off this module.') . '</p>',
    );
  }
  else {
    // Get possible reasons not to migrate.
    $reasons = _mediamosa_requirements_test();
    $form['migration']['text_passed_intro'] = array(
      '#markup' => '<p>' . t('To use your MediaMosa 1.7.x database, you need to migrate your 1.7.x database to MediaMosa. Your original 1.7.x database is not changed. Warning: Your old database must be installed on the same mysql server as the new MediaMosa 2.x database. Also note that your MediaMosa 2.x mysql user requires SELECT access on your 1.7.x database. If for some reason migration fails, please report on !mediamosaorg. Make sure you are migrating with version MediaMosa 1.7.3 or higher. We also advice you to turn off the cron maintenance during the migration.', array('!mediamosaorg' => l(t('mediamosa.org'), 'http://mediamosa.org'))) . '</p>'
    );

    if (count($reasons)) {
      $form['migration']['text'] = array(
        '#markup' => '<p>' . t('Solve these problems before you can migrate;') . '</p><p><b>' . implode('</b></p><p><b>', $reasons) . '</b></p>',
      );
    }
    else {
      $form['migration']['text_passed_status'] = array(
        '#markup' => '<p>' . t('All prechecks passed.') . '</p>'
      );
      $form['migration']['text_push_button'] = array(
        '#markup' => '<p>' . t('Press migration button to start migration.') . '</p>'
      );

      $form['migration']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Start migration'),
      );
    }
  }
  return $form;
}

function _mediamosa_migration_form_submit($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t('Start migration'):
      $batch = array(
        'operations' => array(
          array('_mediamosa_migration_process', array('_mediamosa_migration_vpx_ftp', t('FTP tables (optional)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_error', t('error code table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_client_applications', t('client applications table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_sannas', t('san nas settings'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_asset', t('asset table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_asset_supplement', t('asset supplement table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_mediafile', t('mediafile table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_kill_child_assets', t('move child assets from possible parents, parent_id in asset table is no longer used in MediaMosa'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_mediafile_metadata', t('mediafile metadata table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_asset_metadata_property_group', t('asset metadata property group table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_asset_metadata_property', t('asset metadata property table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_asset_metadata', t('asset metadata property table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_collection', t('collection table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_aut', t('acl/aut table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_asset_delete', t('asset delete table(s)'))),
          //array('_mediamosa_migration_process', array('_mediamosa_migration_ticket', t('ticket table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_user', t('(mediamosa) user/group table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_ftp_batch', t('ftp batch table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_webservice', t('webservice table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_create_server_nodes', t('server table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_job', t('job table(s)'))),
          //array('_mediamosa_migration_process', array('_mediamosa_migration_jobserver_all', t('job server table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_transcode', t('transcode table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_settings', t('settings table(s)'))),
          array('_mediamosa_migration_process', array('_mediamosa_migration_statistics', t('statistic table(s)'))),
          ),
        'finished' => '_mediamosa_migration_batch_finished',
        'title' => t('Migrating database'),
        'init_message' => t('Migration is starting.'),
        'progress_message' => t('Processed @current out of @total.'),
        'error_message' => t('The migration has encounted an error, please check logs.'),
      );
      batch_set($batch);
      break;
  }
}

function _mediamosa_migration_process($callback, $part, &$context) {

  $status = variable_get('mediamosa_migration_status', array());

  if ((!is_array($status) && $status == 'done') || (is_array($status) && isset($status[$callback]))) {
    $context['sandbox']['progress'] = 1;
    return;
  }

  $context['sandbox']['progress'] = 0;
  $context['sandbox']['current_node'] = 0;
  $context['sandbox']['max'] = 1;
  $context['message'] = t('Migrating: @part', array('@part' => $part));

  // Run callback.
  $callback();

  $context['results'][] = t('Succesfully migrated: @part', array('@part' => $part));
  $context['sandbox']['progress'] = 1;

  // Set whats done.
  $status[$callback] = $callback;
  variable_set('mediamosa_migration_status', $status);
}

/**
 * 'finished' callback.
 */
function _mediamosa_migration_batch_finished($success, $results, $operations) {
  if ($success) {
    // Here we do something meaningful with the results.
    $message = count($results) . ' processed.';

    if (module_exists('mediamosa_solr')) {
      $message .= t('You now need to re-index the Solr database, you can do that !here.', array('!here' => l('here', '')));
    }

    // Mark that migration was successful.
    variable_set('mediamosa_migration_status', 'done');
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
  }
  drupal_set_message($message);
}
