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
 * Job Transcode functions.
 */
class mediamosa_job_transcode {

  /**
   * Get the job transcode data.
   * @param integer $job_id
   *  The job ID.
   */
  static public function get($job_id) {
    return mediamosa_db::db_select(mediamosa_job_transcode_db::TABLE_NAME, 'jt')
      ->fields('jt')
      ->condition(mediamosa_job_transcode_db::JOB_ID, $job_id)
      ->execute()
      ->fetchAssoc();
  }


  /**
   * Add new transcode job.
   *
   * Based on the selected tool, file_extension and possible extra parameters.
   *
   * When the tool is windows, a extra job is created for transcoding to
   * raw format.
   *
   * 1.x: _vpx_jobs_create_new_job_transcode
   *
   * @param integer $app_id
   *  The application ID.
   * @param integer $job_id
   *  The job ID.
   * @param string $file_extension
   *  File extension of the file to transcode.
   * @param string $command
   *  Set of commands.
   * @param string $tool
   *  Name of tool to use.
   * @param integer $profile_id
   *  Profile to use.
   * @param string $completed_transcoding_url
   *  Triggered when transcoding is complete.
   * @param string $mediafile_id
   *  The mediafile ID.
   */
  static public function create($app_id, $job_id, $file_extension, $command, $tool = '', $profile_id = '', $completed_transcoding_url = '', $mediafile_id = NULL) {

    // Get the default transcode profile settings when not supplied.
    if (empty($profile_id) && $tool == '') {
      $a_transcode_profile = mediamosa_transcode_profile::get_default($app_id);
      if (!$a_transcode_profile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_DEFAULT_TRANSCODE_PROFILE);
      }

      $profile_id = $a_transcode_profile[mediamosa_transcode_profile_db::ID];
      $tool = $a_transcode_profile[mediamosa_transcode_profile_db::TOOL];
      $file_extension = $a_transcode_profile[mediamosa_transcode_profile_db::FILE_EXTENSION];
      $command = $a_transcode_profile[mediamosa_transcode_profile_db::COMMAND];
    }
    elseif (!empty($profile_id)) {
      // FIXME: should we add app_id here?
      $a_transcode_profile = mediamosa_transcode_profile::get($profile_id);

      if (!$a_transcode_profile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_DEFAULT_TRANSCODE_PROFILE);
      }

      $tool = $a_transcode_profile[mediamosa_transcode_profile_db::TOOL];
      $file_extension = $a_transcode_profile[mediamosa_transcode_profile_db::FILE_EXTENSION];
      $command = $a_transcode_profile[mediamosa_transcode_profile_db::COMMAND];
    }
    else {
      $profile_id = NULL;
    }

    // Both settings are required.
    if ($tool == '' || $file_extension == '') {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_TRANSCODE_PARAMETERS);
    }

    // Check if fileextension is ok.
    try {
      // TODO: Is it necessary?
      //mediamosa_transcode_profile::check_command_parameters($tool, 'internal_file_extension:' . $file_extension);
    }
    catch (mediamosa_exception_error $e) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOB_TRANSCODE_PARAMETER_COMBINATION, array('@tool' => $tool, '@file_extension' => $file_extension));
    }

    // Check if given parameter is ok for tool.
    mediamosa_transcode_profile::check_command_parameters($tool, $command);

    // Build the completed_transcoding_url.
    if (!empty($completed_transcoding_url)) {
      $a_query = array();

      // Lets see if there is a http query.
      $pos = mediamosa_unicode::strpos($completed_transcoding_url, '?');
      if ($pos !== FALSE) {
        if (mediamosa_unicode::strlen($completed_transcoding_url) > $pos + 1) {
          parse_str(mediamosa_unicode::substr($completed_transcoding_url, $pos + 1), $a_query);
        }

        // Remove the query string.
        $completed_transcoding_url = mediamosa_unicode::substr($completed_transcoding_url, 0, $pos);
      }

      // Now add our parameters.
      $a_query['job_id'] = $job_id;
      $a_query['mediafile_id'] = $mediafile_id;

      // The status code will be filled later
      $a_query['status_code'] = '';

      // Build the query again.
      $completed_transcoding_url .= '?' . http_build_query($a_query, '', '&');
    }

    // Now create the transcode job.
    $a_fields = array(
      mediamosa_job_transcode_db::JOB_ID => $job_id,
      mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID => $profile_id,
      mediamosa_job_transcode_db::TOOL => $tool,
      mediamosa_job_transcode_db::FILE_EXTENSION => $file_extension,
      mediamosa_job_transcode_db::COMMAND => $command,
      mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL => $completed_transcoding_url,
    );

    // Add changed/created values.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_transcode_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();

    // See if the tool is windows. If so, we need to create a raw job.
    if ($tool == mediamosa_job::JOBWINDOWS_TOOL) {
      mediamosa_job::create_raw($job_id);
    }
  }

  /**
   * Create tool transcode
   *
   * @param int $job_id
   * @param string $tool
   * @param string $file_extension
   * @param string $command
   */
  static public function create_tool_transcode($job_id, $tool, $file_extension, $command) {
    $profile_id = 0;
    $completed_transcoding_url = '';

    // Now create the transcode job.
    $fields = array(
      mediamosa_job_transcode_db::JOB_ID => $job_id,
      //mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID => $profile_id,
      mediamosa_job_transcode_db::TOOL => $tool,
      mediamosa_job_transcode_db::FILE_EXTENSION => $file_extension,
      mediamosa_job_transcode_db::COMMAND => $command,
      //mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL => $completed_transcoding_url,
    );

    // Add changed/created values.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_transcode_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }
}
