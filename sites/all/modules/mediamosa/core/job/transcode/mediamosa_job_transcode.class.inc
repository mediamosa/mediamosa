<?php
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
  public static function get($job_id) {
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
   * @param string $mediafile_dest
   *   The destination mediafile ID.
   */
  public static function create($app_id, $job_id, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $mediafile_id, $mediafile_dest) {

    // Get the default transcode profile settings when not supplied.
    if (empty($profile_id) && $tool == '') {
      $transcode_profile = mediamosa_transcode_profile::get_default($app_id);
      if (!$transcode_profile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_DEFAULT_TRANSCODE_PROFILE);
      }

      $profile_id = $transcode_profile[mediamosa_transcode_profile_db::ID];
      $tool = $transcode_profile[mediamosa_transcode_profile_db::TOOL];
      $file_extension = $transcode_profile[mediamosa_transcode_profile_db::FILE_EXTENSION];
      $command = $transcode_profile[mediamosa_transcode_profile_db::COMMAND];
    }
    elseif (!empty($profile_id)) {
      $transcode_profile = mediamosa_transcode_profile::get($profile_id);

      if (!$transcode_profile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_DEFAULT_TRANSCODE_PROFILE);
      }

      $tool = $transcode_profile[mediamosa_transcode_profile_db::TOOL];
      $file_extension = $transcode_profile[mediamosa_transcode_profile_db::FILE_EXTENSION];
      $command = $transcode_profile[mediamosa_transcode_profile_db::COMMAND];
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
      $params = array(
        'job_id' => $job_id,
        'mediafile_id' => $mediafile_id,
        'mediafile_dest' => $mediafile_dest,
        // The status code will be filled later, keep at last.
        'status_code' => '',
      );

      // Build the query again.
      $completed_transcoding_url = mediamosa_http::glue_params($completed_transcoding_url, $params);
    }

    // Now create the transcode job.
    $fields = array(
      mediamosa_job_transcode_db::JOB_ID => $job_id,
      mediamosa_job_transcode_db::TRANSCODE_PROFILE_ID => $profile_id,
      mediamosa_job_transcode_db::TOOL => $tool,
      mediamosa_job_transcode_db::FILE_EXTENSION => $file_extension,
      mediamosa_job_transcode_db::COMMAND => $command,
      mediamosa_job_transcode_db::COMPLETED_TRANSCODING_URL => $completed_transcoding_url,
    );

    // Add changed/created values.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_job_transcode_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Create tool transcode
   *
   * @param int $job_id
   * @param string $tool
   * @param string $file_extension
   * @param string $command
   */
  public static function create_tool_transcode($job_id, $tool, $file_extension, $command) {
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
