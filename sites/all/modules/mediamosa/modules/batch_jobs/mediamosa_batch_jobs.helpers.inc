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
 * Helper functions for batch-jobs.
 */

/**
 * Parse the job table based on given type (queue or history).
 * @param $files array()
 * @param $type string: 'queue', 'history'
 * @return array($headers, $table)
 */
function mediamosa_batch_jobs_parse_table($files = array(), $type = 'queue') {
  $table = array(); // Table data.
  $headers = array(); // Table header.
  $dom = new DOMDocument(); // XML document vuf.
  $dom_metadata = new DOMDocument(); // XML document metadata.
  $record = array(); // Table row data.

  // Loop through all files, parse the xml and add data to table array.
  foreach ($files as $file) {

    // Log if the file is parsed, if no videos are found in the xml,
    // the file should still appear in the table.
    $file_parsed = FALSE;

    if (mediamosa_io::file_exists($file->uri)) {

      if (@$dom->load($file->uri)) {

        if ($dom->getElementsByTagName('video')->length > 0) {

          // Loop through video tags in XML.
          foreach ($dom->getElementsByTagName('video') as $video) {

            foreach ($video->getElementsByTagName('formats') as $format) {

              switch ($type) {
                case 'queue':
                  $record['user_id'] = mediamosa_unicode::substr(basename(dirname($file->uri)), 0, -3);
                  $record['app_id'] = ltrim(mediamosa_unicode::substr(dirname($file->uri), -3), '0');

                  $metadatafile = dirname($file->uri) . '/' . $video->getElementsByTagName('metadataFile')->item(0)->nodeValue;

                  // Load metadata.
                  if (mediamosa_io::file_exists($metadatafile)) {
                    @$dom_metadata->load($metadatafile);
                    $record['title'] = (isset($dom_metadata->getElementsByTagName('title')->item(0)->nodeValue) ? $dom_metadata->getElementsByTagName('title')->item(0)->nodeValue : '');
                  }
                  else {
                    $record['title'] = '';
                  }
                  break;

                case 'history':
                  $record['processed'] = mediamosa_unicode::format_date(filectime($file->uri), 'small');
                  break;
              }

              $usr = mediamosa_unicode::substr(basename(dirname($file->uri)), 0, -3) . '_' . ltrim(mediamosa_unicode::substr(dirname($file->uri), -3), '0');

              $record['vuf'] = basename($file->uri);
              $record['email'] = (isset($dom->getElementsByTagName('email')->item(0)->nodeValue) ? l($dom->getElementsByTagName('email')->item(0)->nodeValue, 'mailto:' . $dom->getElementsByTagName('email')->item(0)->nodeValue) : '');
              // Reference id (with asset id)
              $record['referenceId'] = '';
              if (isset($video->getElementsByTagName('referenceId')->item(0)->nodeValue)) {
                $record['referenceId'] = $video->getElementsByTagName('referenceId')->item(0)->nodeValue;
              }
              if (isset($video->getElementsByTagName('assetid')->item(0)->nodeValue)) {
                $record['referenceId'] = (isset($record['referenceId']) && $record['referenceId'] ? $record['referenceId'] . ' ' : '') . 'asset_id => ' . $video->getElementsByTagName('assetid')->item(0)->nodeValue;
              }
              if (isset($video->getElementsByTagName('assetId')->item(0)->nodeValue)) {
                $record['referenceId'] = (isset($record['referenceId']) && $record['referenceId'] ? $record['referenceId'] . ' ' : '') . 'asset_id => ' . $video->getElementsByTagName('assetId')->item(0)->nodeValue;
              }
              $record['action'] = (isset($video->getElementsByTagName('action')->item(0)->nodeValue) ? $video->getElementsByTagName('action')->item(0)->nodeValue : '');
              $record['mediaFile'] = (isset($format->getElementsByTagName('mediaFile')->item(0)->nodeValue) ? $format->getElementsByTagName('mediaFile')->item(0)->nodeValue : '');
              $record['status'] = l(t('Log'), 'admin/mediamosa/tools/ftp_batch/log/' . basename($file->uri) . '/' . $usr);

              // Delete doesn't work because the apache user has insufficient rights.
              // @todo: rewrite canceling of queued jobs.
              /*
              if ($type == 'queue') {
                $record['remove'] = l(t('Delete'), 'admin/mediamosa/tools/ftp_batch/queue/delete/'. urlencode(basename($file->uri)));
              }
              */
              if (!empty($record)) {
                // Add row to table and empty array.
                $table[] = $record;
                $file_parsed = TRUE;
                $record = array();
              }
            }
          }
        }
        else { // No video tags in XML file.
          $messsage = t('No video tags in XML file.');
        }
      }
      else { // DOM not loaded.
        $messsage = t('Could not load XML file.');
      }
    }
    else { // File doesn't exist.
      $messsage = t('File not found.');
    }

    // Create a row for files without valid XML data.
    if (!$file_parsed) {
      switch ($type) {
        case 'queue':
          $record['user_id'] = mediamosa_unicode::substr(basename(dirname($file->uri)), 0, -3);
          $record['app_id'] = ltrim(mediamosa_unicode::substr(dirname($file->uri), -3), '0');
          $record['title'] = '';
          break;
        case 'history':
          $record['processed'] = mediamosa_unicode::format_date(filectime($file->uri), 'small');
          break;
      }

      $record['vuf'] = basename($file->uri);
      $record['email'] = '';
      $record['referenceId'] = !empty($messsage) ? $messsage : t('n.a.');
      $record['action'] = '';
      $record['mediaFile'] = '';
      if ($type == 'queue') {
        //@todo: link to correct file.
        $record['status'] = '';
      }
      else {
        $usr = mediamosa_unicode::substr(basename(dirname($file->uri)), 0, -3) . '_' . ltrim(mediamosa_unicode::substr(dirname($file->uri), -3), '0');

        $record['status'] = l(t('Log'), 'admin/mediamosa/tools/ftp_batch/log/' . basename($file->uri) . '/' . $usr);
      }

      // Delete doesn't work because the apache user has insufficient rights.
      // @todo: rewrite canceling of queued jobs.
      /*
      if ($type == 'queue') {
        $record['remove'] = l(t('Delete'), 'admin/mediamosa/tools/ftp_batch/queue/delete/'. urlencode(basename($file->uri)));
      }
      */

      // Add row to table and empty array.
      $table[] = $record;
      $record = array();
    }
  }

  if (!empty($table)) {
    // Convert array keys to table headers.
    $headers = array_keys($table[0]);
    foreach ($headers as &$header) {
      $header = ucwords(str_replace('_', ' ', $header));
    }
  }

  return array($headers, $table);
}

/**
 * Sort the files returned by file_scan_directory().
 *
 * @param $files
 * @param $function
 * @return array $files
 */
function mediamosa_batch_jobs_sort_files($files, $function) {
  $i = 0;
  foreach ($files as $key => $file) {
    unset($files[$key]);
    $files[$function($file->uri) . $i++] = $file;
  }
  krsort($files, SORT_NUMERIC);
  return $files;
}
