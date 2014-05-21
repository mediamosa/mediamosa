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
 * functions class for statistics.
 *
 * Functions not converted as they are obsolete or empty;
 * _vpx_statistics_log_search_query

 * Opvragen van een overzicht van aangemaakte streams en collecties
 * STATS:5
  function vpx_statistics_get_created_metadata_and_collections() {
  }

 * Opvragen van een overzicht van zoek en bladeracties.
 * STATS:6
 function vpx_statistics_get_search_queries() {
 }
 */

class mediamosa_statistics {
  /**
   * In the log files the lines with this string recognised as a ticket request.
   */
  const LOG_TICKET = 'ticket';
  /**
   * Field line starts with this mark in the log file.
   */
  const LOG_FIELDS = '#Fields: ';
  /**
   * Float to integer conversion number.
   */
  const LOG_FLOAT = 1000;
  /**
   * Ticket clean up time in hours.
   * 30 days. 24x30 = 720.
   */
  const TICKET_TIMEOUT = 720;

  /**
   * Log for statistics.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log($message, array $variables = array(), $severity = WATCHDOG_NOTICE) {
    mediamosa_watchdog::log($message, $variables, $severity, 'statistics');
  }

  /**
   * Log debug for statistics.
   *
   * @param string $message
   * @param array $variables
   */
  public static function log_debug($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'statistics');
  }

  /**
   * Log debug for statistics.
   *
   * @param string $message
   * @param array $variables
   */
  public static function log_debug_high($message, array $variables = array()) {
    mediamosa_debug::log_high($message, $variables, 'statistics');
  }

  /**
   * Create log entry for file upload event.
   *
   * 1.x _vpx_statistics_log_file_upload
   *
   * @param $app_id
   * @param $owner_id
   * @param $group_id
   * @param $file_size
   */
  public static function log_event_file_upload($app_id, $owner_id, $group_id, $file_size) {
    // Data to store.
    $fields = array(
      mediamosa_statistics_file_upload_db::APP_ID => $app_id,
      mediamosa_statistics_file_upload_db::OWNER_ID => $owner_id,
      mediamosa_statistics_file_upload_db::GROUP_ID => $group_id,
      mediamosa_statistics_file_upload_db::FILE_SIZE => (is_null($file_size)? 0 : $file_size),
      mediamosa_statistics_file_upload_db::TIMESTAMP => mediamosa_datetime::utc_current_timestamp_now(),
    );

    // Do the insert.
    return mediamosa_db::db_insert(mediamosa_statistics_file_upload_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Log search query keywords.
   *
   * @param array $keywords
   * @param int $app_id
   */
  public static function log_event_search_query(array $keywords, $app_id) {
    // Is we detect an array, then we are not gonna log it. In this case we
    // are in multi App mode, and we only log during single app mode.
    if (is_array($app_id)) {

      $app_id = reset($app_id);

      // If we are as super user, then dont store the stats.
      if (!$app_id) {
        return;
      }
    }

    $keywords = trim(implode(' ', $keywords));
    if (empty($keywords)) {
      return;
    }

    mediamosa_db::db_insert(mediamosa_statistics_search_request_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_search_request_db::APP_ID => $app_id,
        mediamosa_statistics_search_request_db::KEYWORD => mediamosa_unicode::substr($keywords, 0, mediamosa_statistics_search_request_db::KEYWORD_LENGTH),
        mediamosa_statistics_search_request_db::SEARCHED => mediamosa_datetime::utc_current_timestamp_now(),
      ))
      ->execute();
  }

  /**
   * Log a REST call request.
   *
   * @param $request
   *   The REST call request URI text.
   * @param $process_time
   *   Time it took to execute the REST call.
   * @param $query_count
   *   Amount of queries done during REST call.
   * @param $ip_address
   *   IP address of the client.
   * @param $date
   *   Date and time when REST call was made.
   * @param $app_id
   *   The application ID that executed the REST call.
   */
  public static function log_event_restcall($request, $process_time, $query_count, $ip_address, $date, $app_id) {

    mediamosa_db::db_insert(mediamosa_statistics_rest_log_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_rest_log_db::REQUEST => $request,
        mediamosa_statistics_rest_log_db::PROCESS_TIME => $process_time,
        mediamosa_statistics_rest_log_db::QUERY_COUNT => $query_count,
        mediamosa_statistics_rest_log_db::IP_ADDRESS => $ip_address,
        mediamosa_statistics_rest_log_db::DATE => $date,
        mediamosa_statistics_rest_log_db::APP_ID => $app_id,
      ))
      ->execute();
  }

  /**
   * Calculate and store used diskspace.
   *
   * @param $year
   * @param $month
   */
  public static function calculate_used_diskspace($year, $month) {

    $year = (int) $year;
    $month = (int) $month;

    // Make sure its provide ok.
    if ($year < 2000 || $year > 2099 || $month < 1 || $month > 12) {
      throw new mediamosa_exception_program_error('Invalid input @calculate_used_diskspace');
    }

    mediamosa_db::db_query(
      'DELETE FROM {mediamosa_statistics_diskspace_used} WHERE YEAR(timestamp) = :year AND MONTH(timestamp) = :month',
      array(
        ':year' => $year,
        ':month' => $month,
      )
    );

    // Per usergroup, user and container
    foreach (array('group_id' => 'group', 'owner_id' => 'user', 'app_id' => 'container') as $subject => $name) {
      /*
      // Example queries:
      // group_id => group
      SELECT SUM(m.val_int) / 1024 / 1024 AS diskspace_mb, mf.group_id, mf.app_id, mm.val_char AS container_type
      FROM mediamosa_asset_mediafile_metadata AS m
      LEFT JOIN mediamosa_asset_mediafile_metadata AS mm USING(mediafile_id)
      JOIN mediamosa_asset_mediafile AS mf USING(mediafile_id)
      WHERE m.val_int > 0 AND m.prop_id = 13 AND mm.prop_id = 10
      GROUP BY app_id, group_id, mm.val_char
      // owner_id => user
      SELECT SUM(m.val_int) / 1024 / 1024 AS diskspace_mb, mf.owner_id, mf.app_id, mm.val_char AS container_type
      FROM mediamosa_asset_mediafile_metadata AS m
      LEFT JOIN mediamosa_asset_mediafile_metadata AS mm USING(mediafile_id)
      JOIN mediamosa_asset_mediafile AS mf USING(mediafile_id)
      WHERE m.val_int > 0 AND m.prop_id = 13 AND mm.prop_id = 10
      GROUP BY app_id, owner_id, mm.val_char
      // app_id => container
      SELECT SUM(m.val_int) / 1024 / 1024 AS diskspace_mb, mf.app_id, mf.app_id, mm.val_char AS container_type
      FROM mediamosa_asset_mediafile_metadata AS m
      LEFT JOIN mediamosa_asset_mediafile_metadata AS mm USING(mediafile_id)
      JOIN mediamosa_asset_mediafile AS mf USING(mediafile_id)
      WHERE m.val_int > 0 AND m.prop_id = 13 AND mm.prop_id = 10
      GROUP BY app_id, app_id, mm.val_char
       */
      $result = mediamosa_db::db_query(
        'SELECT SUM(m.#filesize) / 1024 / 1024 AS diskspace_mb, mf.#subject, mf.#app_id, mm.#container_type AS container_type
         FROM {#mediamosa_asset_mediafile_metadata} AS m
         LEFT JOIN {#mediamosa_asset_mediafile_metadata} AS mm USING(#mediafile_id)
         JOIN {#mediamosa_asset_mediafile} AS mf USING(#mediafile_id)
         WHERE m.#filesize > 0 AND m.#prop_id = :prop_id_filesize AND mm.#prop_id = :prop_id_container_type
         GROUP BY #app_id, #subject, mm.#container_type',
        array(
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#filesize' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
          '#subject' => $subject,
          '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
          '#container_type' => mediamosa_asset_mediafile_metadata_db::VAL_CHAR,
          '#mediafile_id' => mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID,
          '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
          ':prop_id_filesize' => mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::FILESIZE),
          ':prop_id_container_type' => mediamosa_asset_mediafile_metadata_property::get_property_id_char(mediamosa_asset_mediafile_metadata::CONTAINER_TYPE),
        )
      );

      // Insert new (or updated) statistics.
      foreach ($result as $data) {
        mediamosa_db::db_query(
          'INSERT INTO {mediamosa_statistics_diskspace_used}
           SET app_id = :app_id, type = :type, keyword = :keyword, container_type = :container_type, diskspace = :diskspace, timestamp = :timestamp',
          array(
            ':app_id' => $data['app_id'],
            ':type' => $name,
            ':keyword' => ($data[$subject] ? $data[$subject] : ''),
            ':container_type' => $data['container_type'],
            ':diskspace' => round($data['diskspace_mb']),
            ':timestamp' => mediamosa_datetime::utc_current_timestamp_now()
          )
        );
      }
    }
  }

  /**
   * Insert job transcode data for the statistics.
   *
   * @param string $mediafile_id
   * @param string $job_id
   */
  public static function insert_job_transcode($mediafile_id, $job_id) {
    // Get data.
    $job = mediamosa_job::get($job_id);
    $job_transcode = mediamosa_job_transcode::get($job_id);
    $profile = mediamosa_transcode_profile::get($job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID]);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_statistics_job_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_job_db::ID => NULL,
        mediamosa_statistics_job_db::APP_ID => $job[mediamosa_job_db::APP_ID],
        mediamosa_statistics_job_db::MEDIAFILE_ID => $mediafile_id,
        mediamosa_statistics_job_db::JOB_TYPE => mediamosa_statistics_job_db::JOB_TYPE_TRANSCODE,
        mediamosa_statistics_job_db::STARTED => $job[mediamosa_job_db::STARTED],
        mediamosa_statistics_job_db::FINISHED => $job[mediamosa_job_db::FINISHED],
        mediamosa_statistics_job_db::TRANSCODE_PROFILE_ID => $job_transcode[mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID],
        mediamosa_statistics_job_db::NID => (isset($profile[mediamosa_transcode_profile_db::NID]) ? $profile[mediamosa_transcode_profile_db::NID] : 0),
        mediamosa_statistics_job_db::VID => (isset($profile[mediamosa_transcode_profile_db::VID]) ? $profile[mediamosa_transcode_profile_db::VID] : 0),
        mediamosa_statistics_job_db::FILESIZE => NULL,
      ))
      ->execute();
  }

  /**
   * Update job transcode filesize data for the statistics.
   *
   * @param string $mediafile_id
   * @param int $filesize
   */
  public static function update_job_transcode_filesize($mediafile_id, $filesize) {
    mediamosa_db::db_update(mediamosa_statistics_job_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_job_db::FILESIZE => $filesize,
      ))
      ->condition(mediamosa_statistics_job_db::MEDIAFILE_ID, $mediafile_id)
      ->execute();
  }

  /**
   * Get the streaming path.
   *
   * @return string $dir
   */
  public static function get_streaming_path() {
    // Get the path.
    $dir = variable_get('mediamosa_statistics_streaming_log_path', '');
    if (drupal_substr($dir, -1) != '/') {
      $dir .= '/';
    }

    return $dir;
  }

  /**
   * Streaming statistics cron REST call.
   */
  public static function cron() {
    // Zlib module is necessary to parse the log file.
    if (!function_exists('gzgets')) {
      return;
    }

    // Get and parse the log file.
    $file = self::streaming_statistics_get_next_file();
    if ($file) {
      self::log('File parser started with file: @file', array('@file' => $file));
      self::streaming_statistics_file_parser($file);
      self::log('File parser ended with file: @file', array('@file' => $file));
    }
  }

  /**
   * Choose a file to process for streaming statistics.
   *
   * @return string $file
   */
  public static function streaming_statistics_get_next_file() {
    $founded_file = NULL;
    $dir = self::get_streaming_path();

    // Search the next log file.
    if (mediamosa_io::is_dir($dir) && mediamosa_io::is_readable($dir)) {
      $directory_handler = opendir($dir);
      if ($directory_handler) {
        while (($file = readdir($directory_handler)) !== false) {
          if (filetype($dir . $file) == 'file') {
            $result = mediamosa_db::db_query("SELECT #file FROM {#mediamosa_statistics_file_helper} WHERE #file = :file", array(
              '#mediamosa_statistics_file_helper' => mediamosa_statistics_file_helper_db::TABLE_NAME,
              '#file' => mediamosa_statistics_file_helper_db::FILE,
              ':file' => $file,
            ));
            if (!$result->rowCount()) {
              $founded_file = $file;
              break;
            }
          }
        }
        closedir($directory_handler);
      }
    }

    return $founded_file;
  }

  /**
   * Streaming statistics file parser.
   * Zlib module is necessary to parse the log file.
   *
   * @param string $file
   */
  public static function streaming_statistics_file_parser($file) {
    $timezone = variable_get('mediamosa_statistics_streaming_log_timezone', 'Europe/Amsterdam');
    $dir = self::get_streaming_path();
    $dir_file = $dir . $file;

    // Occupy the file.
    // TODO: Known issue: It occupy the file on the beginning of the process.
    // If the server is restarted before finishing the parser tasks,
    // the file won't be process again.
    // It will be solved in the next phase of the development.
    // Then we have to store line number too.
    mediamosa_db::db_insert(mediamosa_statistics_file_helper_db::TABLE_NAME)
      ->fields(array(
        mediamosa_statistics_file_helper_db::FILE => $file,
      ))
      ->execute();

    // File must exists.
    if (!mediamosa_io::file_exists($dir_file) || !mediamosa_io::is_readable($dir_file)) {
      self::log_debug('The file @file is not available', array('@file' => $file));
      return;
    }

    // Set the timeout.
    drupal_set_time_limit(mediamosa_settings::STATISTICS_LOG_MAX_TIME);

    // Open the log file.
    $handle = gzopen($dir_file, 'r');
    if ($handle) {
      // Fields in the lines.
      $fields = NULL;
      // Line separator. See later.
      $separator = ' ';

      // Go through the log file.
      while (!gzeof($handle)) {
        // Get a line.
        $buffer = gzgets($handle, 4096);
        // Is the line starts with '#'?
        if ($buffer[0] == '#') {
          // Is the line start with '#Fiels: '?
          if (drupal_substr($buffer, 0, drupal_strlen(self::LOG_FIELDS)) == self::LOG_FIELDS) {
            // Get the name of fields in an array.
            // Separator: Wowza: "\t", Windows media: ' '.
            $separator = (strpos($buffer, "\t") !== FALSE ? "\t" : ' ');
            $fields = explode($separator, drupal_substr($buffer, drupal_strlen('#Fields: ')));
            self::log_debug_high('Fields for file @file: @fields', array('@file' => $file, '@fields' => print_r($fields, TRUE)));
          }
        }
        // If the line starts without '#', and we have fields already, then:
        elseif ($fields) {
          self::streaming_statistics_line_parser($buffer, $fields, $separator, $timezone);
        }
      }

      // Close the log file.
      gzclose($handle);
    }
    else {
      self::log_debug('The file @file is not tar.gz', array('@file' => $file));
    }
  }

  /**
   * Streaming statistics line parser.
   *
   * @param string $buffer
   * @param array $fields
   *   The key of the log line.
   * @param string $separator
   *   Wowza: "\t", Windows media: ' '.
   * @param string $timezone
   */
  public static function streaming_statistics_line_parser($buffer, $fields, $separator = ' ', $timezone = 'Europe/Amsterdam') {
    // Check first with an easy way for performance purpose.
    if (strpos($buffer, variable_get('mediamosa_statistics_streaming_log_ticket', mediamosa_statistics::LOG_TICKET)) === FALSE) {
      return;
    }

    // Parse it.
    $values = explode($separator, $buffer);
    if (count($fields) != count($values)) {
      // Something went wrong. It may be an empty line. Do nothing.
      return;
    }
    $combined = array_combine($fields, $values);

    $stream_path = $combined[isset($combined['x-ctx']) ? 'x-ctx' : 'cs-uri-stem'];

    // Get the lines with app id and ticket id.
    // Pattern. Eg. #vpx/[1-9]?[0-9]*/[0-9a-zA-Z]{24}#
    $pattern = '#' . variable_get('mediamosa_statistics_streaming_log_ticket', mediamosa_statistics::LOG_TICKET) . '/([1-9]?[0-9]*)/([0-9a-zA-Z]{24}(\.[0-9a-zA-Z]*)?)#';
    if (!preg_match($pattern, $stream_path, $matches)) {
      return;
    }

    // Collect the data.
    $query_data = array();

    // App id.
    $query_data['app_id'] = $matches[1];
    if (!is_numeric($query_data['app_id'])) {
      $query_data['app_id'] = 0;
    }

    // Original ticket id.
    // Strip it. Eg. 'vpx/12/G1QUoaSGdYiVNfU57YRotc1q' or '//vpx/5/mfjOXOAKSLhqEkRkYTMrlyN3'.
    $ticket_id = $matches[2];

    // Time.
    // Get the date and time truncated to hours.
    $query_data['time'] = $combined['date'] . ' ' . drupal_substr($combined['time'], 0, 2) . ':00:00';
    // Convert to UTC.
    $query_data['time'] = mediamosa_datetime::app_date2utc_date($query_data['time'], $timezone);

    // Duration.
    $query_data['duration'] = $combined['x-duration'];
    if (!is_numeric($query_data['duration'])) {
      $query_data['duration'] = 0;
    }
    $query_data['duration'] *= self::LOG_FLOAT;

    // Bytes.
    $query_data['bytes'] = $combined['sc-bytes'];
    if (!is_numeric($query_data['bytes'])) {
      $query_data['bytes'] = 0;
    }

    // Get the additional ticket information from the mediamosa_streaming_ticket table.
    self::get_ticket_data($ticket_id, $query_data);

    // Store the data.
    self::streaming_statistics_store($query_data);
  }

  /**
   * Get the ticket information from the mediamosa_streaming_ticket table.
   *
   * @param string $ticket_id
   * @param array &$query_data
   */
  public static function get_ticket_data($ticket_id, &$query_data) {
    if ($ticket_id) {
      $query = mediamosa_db::db_select(mediamosa_streaming_ticket_db::TABLE_NAME, 't');
      $query
        ->condition('t.' . mediamosa_streaming_ticket_db::TICKET_ID, $ticket_id)
        ->fields('t', array(
          mediamosa_streaming_ticket_db::ID,
          mediamosa_streaming_ticket_db::APP_ID,
          mediamosa_streaming_ticket_db::MEDIAFILE_ID,
        ))
        ->range(0, 1);
      $result = $query->execute();
      $record = $result->fetchAssoc();
      // Is there any result?
      if (isset($record[mediamosa_streaming_ticket_db::MEDIAFILE_ID])) {
        $query_data['tid'] = $record[mediamosa_streaming_ticket_db::ID];
        // Use this app id, if exists.
        $query_data['app_id'] = $record[mediamosa_streaming_ticket_db::APP_ID];
        $query_data['mediafile_id'] = $record[mediamosa_streaming_ticket_db::MEDIAFILE_ID];
      }
    }
  }

  /**
   * Store the streaming statistics data.
   *
   * @param array $query_data
   */
  public static function streaming_statistics_store($query_data) {
    // Is there a tid and time (this combination is unique)?
    if (isset($query_data['tid']) && $query_data['tid'] && isset($query_data['time']) && $query_data['time']) {
      // Store the information into the mediamosa_statistics_stream_request table.

      // Is there a row with this tid and time?
      $result = mediamosa_db::db_query("
        SELECT #id
        FROM {#mediamosa_statistics_stream_request}
        WHERE #tid = :tid AND #time = :time
      ", array(
        '#mediamosa_statistics_stream_request' => mediamosa_statistics_stream_request_db::TABLE_NAME,
        '#id' => mediamosa_statistics_stream_request_db::ID,
        '#tid' => mediamosa_statistics_stream_request_db::TID,
        ':tid' => $query_data['tid'],
        '#time' => mediamosa_statistics_stream_request_db::TIME,
        ':time' => $query_data['time'],
      ));
      $id = $result->fetchField();

      // Update or insert?
      if ($id) {
        $num_updated = mediamosa_db::db_query("
          UPDATE {#mediamosa_statistics_stream_request}
          SET
            #duration = IFNULL(#duration, 0) + :duration,
            #bytes = IFNULL(#bytes, 0) + :bytes
          WHERE #id = :id AND #tid = :tid AND #time = :time
        ", array(
          '#mediamosa_statistics_stream_request' => mediamosa_statistics_stream_request_db::TABLE_NAME,
          '#duration' => mediamosa_statistics_stream_request_db::DURATION,
          ':duration' => $query_data['duration'],
          '#bytes' => mediamosa_statistics_stream_request_db::BYTES,
          ':bytes' => $query_data['bytes'],
          '#id' => mediamosa_statistics_stream_request_db::ID,
          ':id' => $id,
          '#tid' => mediamosa_statistics_stream_request_db::TID,
          ':tid' => $query_data['tid'],
          '#time' => mediamosa_statistics_stream_request_db::TIME,
          ':time' => $query_data['time'],
        ));
      }
      else {
        $id = mediamosa_db::db_insert(mediamosa_statistics_stream_request_db::TABLE_NAME)
          ->fields(array(
            mediamosa_statistics_stream_request_db::TID => $query_data['tid'],
            mediamosa_statistics_stream_request_db::TIME => $query_data['time'],
            mediamosa_statistics_stream_request_db::APP_ID => $query_data['app_id'],
            mediamosa_statistics_stream_request_db::MEDIAFILE_ID => $query_data['mediafile_id'],
            mediamosa_statistics_stream_request_db::DURATION => $query_data['duration'],
            mediamosa_statistics_stream_request_db::BYTES => $query_data['bytes'],
          ))
          ->execute();
      }
    }
  }

  /**
   * Create mediamosa_streaming_ticket table row.
   *
   * @param string $ticket_id
   * @param string $ticket_type
   * @param int $app_id
   * @param string $mediafile_id
   */
  public static function create_streaming_ticket($ticket_id, $ticket_type, $app_id, $mediafile_id) {
    // Store the play tickets only.
    if ($ticket_type == mediamosa_media_ticket::TICKET_TYPE_VIEW) {
      // The ticket id should be unique, then just simply store.
      mediamosa_db::db_insert(mediamosa_streaming_ticket_db::TABLE_NAME)
        ->fields(array(
          mediamosa_streaming_ticket_db::TICKET_ID => $ticket_id,
          mediamosa_streaming_ticket_db::APP_ID => $app_id,
          mediamosa_streaming_ticket_db::MEDIAFILE_ID => $mediafile_id,
          mediamosa_streaming_ticket_db::TIMEOUT => mediamosa_datetime::utc_current_timestamp_add_hours(FALSE, self::TICKET_TIMEOUT),
        ))
        ->execute();
    }
  }

  /**
   * Delete mediamosa_streaming_ticket table rows.
   */
  public static function delete_streaming_ticket() {
    $num_deleted = mediamosa_db::db_delete(mediamosa_streaming_ticket_db::TABLE_NAME)
      ->condition(mediamosa_streaming_ticket_db::TIMEOUT, mediamosa_datetime::utc_current_timestamp_now(), '<')
      ->execute();
  }
}
