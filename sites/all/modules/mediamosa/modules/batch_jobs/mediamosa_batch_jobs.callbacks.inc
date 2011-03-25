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
 * Callbacks for batch_jobs.
 */

/**
 * Callback function to display the queued jobs.
 */
function mediamosa_batch_jobs_queue() {
  $output = '';
  $files = mediamosa_batch_jobs_sort_files(
    mediamosa_io::file_scan_directory(
      mediamosa_configuration_storage::get_full_ftp_path(),
      '/\.' . mediamosa_settings::FTP_QUEUE_EXTENTION . '$/',
      array('nomask' => '/history/')
    ),
    'filemtime'
  );

  list($headers, $table) = mediamosa_batch_jobs_parse_table($files, 'queue');
  if ($table) {
    $output .= t('List of jobs queued to be processed.');
    $output .= theme('table', array('header' => $headers, 'rows' => $table));
  }
  else {
    drupal_set_message(t('No queued jobs found.'), 'status', FALSE);
  }
  return $output;
}

/**
 * Callback function to display the current job.
 */
function mediamosa_batch_jobs_current() {
  $output = '';
  if (file_exists(mediamosa_configuration_storage::current_job_path())) {
    $handle = fopen(mediamosa_configuration_storage::current_job_path(), 'r');
    $filename = trim(fread($handle, filesize(mediamosa_configuration_storage::current_job_path())));
    fclose($handle);

    if (file_exists($filename)) {
      $file = new stdClass;
      $file->uri = $filename;
      list($headers, $table) = mediamosa_batch_jobs_parse_table(array($file), 'queue');
    }
  }
  if (isset($table) && $table) {
    $output .= t('Job which is currently processed.');
    $output .= theme('table', array('header' => $headers, 'rows' => $table));
  }
  else {
    drupal_set_message(t('Currently, there is no active job.'), 'status', FALSE);
  }
  return $output;
}

/**
 * Callback function to display the job history.
 */
function mediamosa_batch_jobs_history() {
  $output = '';
  $files = mediamosa_batch_jobs_sort_files(
    mediamosa_io::file_scan_directory(
      mediamosa_configuration_storage::get_full_history_path(),
      '/\.' . mediamosa_settings::FTP_HISTORY_EXTENTION . '$/'
    ),
    'filemtime'
  );

  list($headers, $table) = mediamosa_batch_jobs_parse_table($files, 'history');

  if ($table) {
    $output .= t('Jobs which have been processed.');
    $output .= theme('table', array('header' => $headers, 'rows' => $table));
  }
  else {
    drupal_set_message(t('No history found.'), 'status', FALSE);
  }
  return $output;
}

/**
 * Callback function to display a confirmation form for deletion.
 * @param <array> $form_state
 * @return <array>
 */
function mediamosa_batch_jobs_delete_confirm(&$form_state) {
  $value_vuf = check_plain(arg(6));
  $form['batch'] = array(
    '#type' => 'value',
    '#value' => $value_vuf,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete the following batch: %vuf?', array('%vuf' => $value_vuf)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/mediamosa/browse/ftp_batch/queue',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Execute batch deletion.
 * @param <string> $form
 * @param <array> $form_state
 */
function mediamosa_batch_jobs_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $filename = mediamosa_configuration_storage::get_full_ftp_path() . '/' . urldecode($form_state['values']['batch']);
    if (file_exists($filename)) {
      unlink($filename);
    }
  }

  $form_state['redirect'] = 'admin/mediamosa/browse/ftp_batch/queue';
}

/**
 * Callback function to display the contents of a log.
 * @return <string>
 */
function mediamosa_batch_jobs_log() {
  $filename = check_plain(urldecode(arg(5)));
  $usr = check_plain(urldecode(arg(6)));
  $path = mediamosa_configuration_storage::get_full_history_path() . DIRECTORY_SEPARATOR . basename($filename, '.' . mediamosa_settings::FTP_HISTORY_EXTENTION) . '.log';
  if (!file_exists($path)) {
    $path = mediamosa_configuration_storage::get_full_history_path() . DIRECTORY_SEPARATOR . $usr . '_' . basename($filename, '.' . mediamosa_settings::FTP_QUEUE_EXTENTION) . '.log';
  }

  drupal_set_title(t('Log') . ' ' . $filename);

  $body = nl2br(htmlentities(file_get_contents($path)));
  $body .= l(t('Return to batch job queue'), 'admin/mediamosa/browse/ftp_batch/queue');

  return $body;
}
