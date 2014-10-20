<?php
/**
 * @file
 * Jobs Module REST calls.
 */

/**
 * URI: /job/$job_id/status
 * Method: POST
 *
 * Internal only.
 */
class mediamosa_rest_call_job_set_status extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const JOB_ID = 'job_id';
  const STATUS = 'status';
  const PROGRESS = 'progress';
  const ERROR_DESCRIPTION = 'error_description';
  const CREATE_STILL = 'create_still';
  const STILL_TYPE = 'still_type';
  const STILL_PER_MEDIAFILE = 'still_per_mediafile';
  const STILL_EVERY_SECOND = 'still_every_second';
  const START_FRAME = 'start_frame';
  const END_FRAME = 'end_frame';
  const SIZE = 'size';
  const H_PADDING = 'h_padding';
  const V_PADDING = 'v_padding';
  const TAG = 'tag';
  const FRAMETIME = 'frametime';
  const WIDTH = 'width';
  const HEIGHT ='height';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The job ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::STATUS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The status.',
          self::VAR_ALLOWED_VALUES => array(mediamosa_job_db::JOB_STATUS_WAITING, mediamosa_job_db::JOB_STATUS_INPROGRESS, mediamosa_job_db::JOB_STATUS_FINISHED, mediamosa_job_db::JOB_STATUS_FAILED),
        ),
        self::PROGRESS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The progress of the job from 0.000 to 1.000.',
        ),
        self::ERROR_DESCRIPTION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The possible error description log.',
        ),
        self::CREATE_STILL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'The create still switch after upload.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::STILL_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The still type.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
            //mediamosa_asset_mediafile_metadata::STILL_TYPE_PICTURE,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_SCENE,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_SECOND,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_NORMAL,
            '',
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
        ),
        self::STILL_PER_MEDIAFILE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills per mediafile. (Used when still_type is NORMAL).',
        ),
        self::STILL_EVERY_SECOND => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills every second. (Used when still_type is SECOND).',
        ),
        self::START_FRAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still start frame. (in seconds, used when still_type is NORMAL or SECOND).',
        ),
        self::END_FRAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still end frame.',
        ),
        self::SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Size of the still.',
        ),
        self::H_PADDING => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still horizontal padding.',
        ),
        self::V_PADDING => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still vertical padding.',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Still tag.',
        ),
        self::FRAMETIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still frametime.',
        ),
        self::WIDTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still width.',
        ),
        self::HEIGHT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still height.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Get params.
    $job_id = $this->get_param_value(self::JOB_ID);
    $progress = $this->get_param_value(self::PROGRESS);
    $status = $this->get_param_value(self::STATUS);
    $error_description = $this->get_param_value(self::ERROR_DESCRIPTION);

    /*
     * The extra params are not used atm, because code has been turned off that
     * uses them.
     * FIXME: need to think about removing redudent params.
     * its an internal, so should not be a problem.
     */

    // Job must exist.
    mediamosa_job::must_exists($job_id);

    // Get complete job.
    $job_ext = mediamosa_job::get_job_ext($job_id);

    // Update the job.
    mediamosa_job::update_status($job_ext, $status, $progress, $error_description);

    // Set Ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/analyse
 * Method: POST
 */
class mediamosa_rest_call_job_analyse_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';

  const USER_ID = 'user_id';
  const HINT = 'hint';
  const NO_HINT = 'no_hint';
  const PRIORITY = 'priority';
  const PRIORITY_RANGE_START = -100;
  const PRIORITY_RANGE_END = 100;

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user id, must be the owner of the mediafile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::HINT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Setting to TRUE or FALSE overrules the global ega-setting. possible values: TRUE, FALSE, NULL. Default: NULL',
          self::VAR_DEFAULT_VALUE => NULL,
        ),
        self::NO_HINT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'When true and "hint" parameter is not defined then, regardless of the ega setting, do not add hinting to the mediafile. Deprecated.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::PRIORITY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Priority of the job.',
          self::VAR_RANGE_START => self::PRIORITY_RANGE_START,
          self::VAR_RANGE_END => self::PRIORITY_RANGE_END,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Get params.
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $priority = $this->get_param_value(self::PRIORITY);

    $hint = $this->get_param_value(self::HINT);
    $no_hint = $this->get_param_value(self::NO_HINT);
    // If hint is null, and no_hint is true, then do the old way.
    if (is_null($hint) && $no_hint === TRUE) {
      $hint = FALSE;
    }

    // Create it.
    $job_id = mediamosa_job::create_job_analyse($app_id, $user_id, $is_app_admin, $mediafile_id, $priority, $hint);

    // Add job_id.
    $mediamosa->add_item(array('job_id' => $job_id));
  }
}

/**
 * URI: /job/$job_id/progress
 * Method: POST
 *
 * Internal only.
 */
class mediamosa_rest_call_job_set_upload_progress extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const JOB_ID = 'job_id';
  const UPLOADED_FILE_SIZE = 'uploaded_file_size';
  const CREATE_STILL = 'create_still';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The job ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::UPLOADED_FILE_SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The uploaded file size.',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $job_id = $this->get_param_value(self::JOB_ID);
    $uploaded_file_size = $this->get_param_value(self::UPLOADED_FILE_SIZE);

    // Must exist.
    mediamosa_job::must_exists($job_id);

    // Update it.
    mediamosa_job::update_progress_upload($job_id, $uploaded_file_size);

    // Set Okay.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /job/create
 * Method: POST
 *
 * Internal only.
 */
class mediamosa_rest_call_job_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const JOB_TYPE = 'job_type';
  const CREATE_STILL = 'create_still';
  const STILL_TYPE = 'still_type';
  const STILL_PER_MEDIAFILE = 'still_per_mediafile';
  const STILL_EVERY_SECOND = 'still_every_second';
  const START_FRAME = 'start_frame';
  const START_TIME = 'start_time';
  const END_FRAME = 'end_frame';
  const END_TIME = 'end_time';
  const SIZE = 'size';
  const H_PADDING = 'h_padding';
  const V_PADDING = 'v_padding';
  const TAG = 'tag';
  const FRAMETIME = 'frametime';

  // Watermark.
  const WATERMARK_ID = 'watermark_id';
  const WATERMARK_DST_X = 'watermark_dst_x';
  const WATERMARK_DST_Y = 'watermark_dst_y';
  const WATERMARK_PCT = 'watermark_pct';
  const WATERMARK_V_ALIGN = 'watermark_v_align';
  const WATERMARK_H_ALIGN = 'watermark_h_align';

  const COMPLETED_TRANSCODING_URL = 'completed_transcoding_url';
  const PROFILE_ID = 'profile_id';
  const TOOL = 'tool';
  const FILE_EXTENSION = 'file_extension';
  const COMMAND = 'command';
  const PRIORITY = 'priority';
  const PRIORITY_RANGE_START = -100;
  const PRIORITY_RANGE_END = 100;
  const FILE_SIZE = 'file_size';
  const RETRANSCODE = 'retranscode';
  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner of the mediafile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_JOB,
          self::VAR_DESCRIPTION => 'The job type.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::CREATE_STILL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Create still after job run.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::STILL_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The still type.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_SCENE,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_SECOND,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_NORMAL,
            '',
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
        ),
        self::STILL_PER_MEDIAFILE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills per mediafile. (Used when still_type is NORMAL).',
        ),
        self::STILL_EVERY_SECOND => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills every second. (Used when still_type is SECOND).',
        ),
        self::START_FRAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still start frame (in seconds, used when still_type is NORMAL or SECOND). Deprecrated, use start_time instead.',
        ),
        self::START_TIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still start time (in seconds, used when still_type is NORMAL or SECOND).',
        ),
        self::END_FRAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still end frame (in seconds, used when still_type is NORMAL or SECOND). Deprecrated, use end_time instead.',
        ),
        self::END_TIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still end time (in seconds, used when still_type is NORMAL or SECOND).',
        ),
        self::SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Size of the still in WxH format (e.g. 640x320).',
        ),
        self::H_PADDING => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still horizontal padding.',
        ),
        self::V_PADDING => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still vertical padding.',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Still tag.',
        ),
        self::FRAMETIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still frametime (in seconds) for still_type NONE.',
        ),
        // Watermark.
        self::WATERMARK_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The watermark ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::WATERMARK_DST_X => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'X-coordinate of destination point of watermark.',
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_still::WATERMARK_DEFAULT_DST_X,
        ),
        self::WATERMARK_DST_Y => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Y-coordinate of destination point of watermark.',
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_still::WATERMARK_DEFAULT_DST_Y,
        ),
        self::WATERMARK_PCT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Alpha transparency of watermark (watermark visibility in percentage [0 - 100%]).',
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_still::WATERMARK_DEFAULT_PCT,
          self::VAR_RANGE_START => 0,
          self::VAR_RANGE_END => 100,
        ),
        self::WATERMARK_V_ALIGN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Vertical align of watermark.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_asset_mediafile_still::WATERMARK_VALUE_LEFT,
            mediamosa_asset_mediafile_still::WATERMARK_VALUE_RIGHT,
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_still::WATERMARK_DEFAULT_V_ALIGN,
        ),
        self::WATERMARK_H_ALIGN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Horizontal align of watermark.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_asset_mediafile_still::WATERMARK_VALUE_TOP,
            mediamosa_asset_mediafile_still::WATERMARK_VALUE_BOTTOM,
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_still::WATERMARK_DEFAULT_H_ALIGN,
        ),

        self::COMPLETED_TRANSCODING_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URL,
          self::VAR_DESCRIPTION => 'Triggered when transcoding is completed.',
        ),
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Profile ID, used with transcode job.',
        ),
        self::TOOL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Tool used for transcode, used with transcode job.',
        ),
        self::FILE_EXTENSION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'file extension of the file, used with transcode job.',
        ),
        self::COMMAND => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COMMAND,
          self::VAR_DESCRIPTION => 'Command used with transcode, used with transcode job.',
        ),
        self::PRIORITY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Priority of the job.',
          self::VAR_RANGE_START => self::PRIORITY_RANGE_START,
          self::VAR_RANGE_END => self::PRIORITY_RANGE_END,
        ),
        self::FILE_SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'File size.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO, // Only when type is upload.
        ),
        self::RETRANSCODE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Retranscode after upload, used with upload jobs.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------- Progress Rest Args.
  /**
   * Process the supplied parameters.
   *
   * @param array $var_setup
   */
  protected function process_rest_args(array $var_setup) {
    $var_setup = parent::process_rest_args($var_setup);

    $job_type = $this->get_param_value(self::JOB_TYPE);

    // Make File size required when uploading.
    if ($job_type == mediamosa_job_db::JOB_TYPE_UPLOAD) {
      $var_setup[self::VARS][self::FILE_SIZE][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      $var_setup[self::VARS][self::GROUP_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
    }

    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $job_type = $this->get_param_value(self::JOB_TYPE);
    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $completed_transcoding_url = $this->get_param_value(self::COMPLETED_TRANSCODING_URL);
    $file_extension = $this->get_param_value(self::FILE_EXTENSION);
    $command = $this->get_param_value(self::COMMAND);
    $tool = $this->get_param_value(self::TOOL);
    $profile_id = $this->get_param_value(self::PROFILE_ID);
    $file_size = $this->get_param_value(self::FILE_SIZE);
    $priority = $this->get_param_value(self::PRIORITY);
    $retranscode = $this->get_param_value(self::RETRANSCODE);
    $watermark_id = $this->get_param_value(self::WATERMARK_ID);

    $create_still = $this->get_param_value(self::CREATE_STILL);
    $still_parameters = array(
      'still_type' => $this->get_param_value(self::STILL_TYPE),
      'still_per_mediafile' => $this->get_param_value(self::STILL_PER_MEDIAFILE),
      'still_every_second' => $this->get_param_value(self::STILL_EVERY_SECOND),
      'start_frame' => $this->get_param_value(self::START_FRAME),
      'end_frame' => $this->get_param_value(self::END_FRAME),
      'size' => ($this->get_param_value(self::SIZE) ? $this->get_param_value(self::SIZE) : mediamosa_app::get_still_default_size($app_id)),
      'h_padding' => $this->get_param_value(self::H_PADDING),
      'v_padding' => $this->get_param_value(self::V_PADDING),
      'tag' => $this->get_param_value(self::TAG),
      'frametime' => $this->get_param_value(self::FRAMETIME),
      // Watermark.
      self::WATERMARK_ID => $watermark_id,
      self::WATERMARK_DST_X => $this->get_param_value(self::WATERMARK_DST_X),
      self::WATERMARK_DST_Y => $this->get_param_value(self::WATERMARK_DST_Y),
      self::WATERMARK_PCT => $this->get_param_value(self::WATERMARK_PCT),
      self::WATERMARK_V_ALIGN => $this->get_param_value(self::WATERMARK_V_ALIGN),
      self::WATERMARK_H_ALIGN => $this->get_param_value(self::WATERMARK_H_ALIGN),
    );

    // Webservice must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_JOBS, $app_ids);

    // Make sure the mediafile exists, returns the mediafile.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);

    // Check watermark id, if exists.
    if ($watermark_id) {
      mediamosa_asset_mediafile::must_exists_cached($watermark_id, $app_id);
    }

    if (in_array($job_type, array(mediamosa_job_db::JOB_TYPE_TRANSCODE, mediamosa_job_db::JOB_TYPE_UPLOAD))) {
      mediamosa_user::must_have_user_quota($app_id, $user_id, $group_id);
    }

    if (!empty($mediafile[mediamosa_asset_mediafile_db::URI])) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CHANGE_URI_AND_FILE, array('@mediafile_id' => $mediafile_id));
    }

    switch ($job_type) {
      case mediamosa_job_db::JOB_TYPE_STILL:
        $job_id = mediamosa_job::create_job_still($app_id, $user_id, $is_app_admin, $mediafile_id, $still_parameters, $priority);
        break;

      case mediamosa_job_db::JOB_TYPE_TRANSCODE:
        $job_id = mediamosa_job::create_job_transcode($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $priority);
        break;

      case mediamosa_job_db::JOB_TYPE_RETRANSCODE:
        $job_id = mediamosa_job::create_job_retranscode($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $file_extension, $command, $tool, $profile_id, $completed_transcoding_url, $priority);
        break;

      case mediamosa_job_db::JOB_TYPE_UPLOAD:
        $job_id = mediamosa_job::create_job_upload($app_id, $user_id, $group_id, $is_app_admin, $mediafile_id, $file_size, $retranscode, $create_still, $still_parameters, $priority);
        break;

      default:
        $job_id = mediamosa_job::create_job($job_type, $app_id, $user_id, $is_app_admin, $mediafile_id, $create_still, $still_parameters, $priority);
        break;
    }

    // Return the created job_id.
    $mediamosa->add_item(array('job_id' => $job_id));
  }
}

/**
 * URI: /user/$user_id/joblist
 * Method: GET
 *
 * Return list of jobs based on user as owner.
 */
class mediamosa_rest_call_job_user_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const USER_ID = 'user_id';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $user_id = $this->get_param_value(self::USER_ID);

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_id, $user_id, $is_app_admin);

    // Get the count.
    $mediamosa->item_count_total = $result->found_rows;

    foreach ($result as $row) {
      $started_unix = isset($row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($row[mediamosa_job_db::STARTED]) : '';

      $job = array();

      $job['id'] = $row[mediamosa_job_db::ID];
      $job['owner'] = $row[mediamosa_job_db::OWNER_ID];
      $job['status'] = $row[mediamosa_job_db::JOB_STATUS];
      $job['progress'] = $row[mediamosa_job_db::PROGRESS];
      $job['priority'] = $row[mediamosa_job_db::PRIORITY];
      $job['job_type'] = $row[mediamosa_job_db::JOB_TYPE];
      $job['started'] = $started_unix ? format_date($started_unix, 'custom', 'd-m-Y H:i') : '';
      $job['started_unix'] = $started_unix;
      $job['error_description'] = $row[mediamosa_job_db::ERROR_DESCRIPTION];

      $mediamosa->add_item($job);
    }
  }
}

/**
 * URI: /asset/$asset_id/joblist
 * Method: GET
 *
 * Return list of jobs based on asset_id
 */
class mediamosa_rest_call_job_asset_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const ASSET_ID = 'asset_id';
  const CQL = 'cql';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::CQL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_CQL_JOB,
          self::VAR_DESCRIPTION => 'The CQL search string, used for extended and complex search options.',
        ),
      )
    );

    // Set limit and offset.
    $var_setup = self::get_var_setup_range($var_setup, mediamosa_settings::LIMIT_MAX_JOBLIST, mediamosa_settings::LIMIT_DEFAULT_JOBLIST);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ---------------------------------------------- Override Validate Rest Args.
  // Override for checking CQL string.
  protected function validate_rest_args(array $var_setup) {

    // Validate first.
    parent::validate_rest_args($var_setup);

    $cql = $this->get_param_value(self::CQL);
    $app_ids = $this->get_param_value_app();
    $error_text = '';

    if ($cql != '' && !mediamosa_core_cql::verify_job($cql, $app_ids, $error_text)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CQL_ERROR, array('@error' => $error_text));
    }
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);
    $app_ids = $this->get_param_value_app();
    $first_app_id = reset($app_ids);

    $user_id = $this->get_param_value(self::USER_ID);
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $cql = $this->get_param_value(self::CQL);
    $limit = $this->get_param_value_limit();
    $offset =  $this->get_param_value_offset();

    // Must exist.
    mediamosa_asset::must_exists($asset_id);

    // We show all data is it is app_admin or first element in application ID array is zero.
    $is_full_access = $is_app_admin || ($first_app_id == 0);

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_ids, $user_id, $is_full_access, FALSE, $asset_id, '', $cql, $limit, $offset);

    // Get the count.
    $mediamosa->item_count_total = $result->found_rows;

    foreach ($result as $row) {
      $started_unix = isset($row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($row[mediamosa_job_db::STARTED]) : '';

      $job = array();

      $job['id'] = $row[mediamosa_job_db::ID];
      $job['owner'] = $row[mediamosa_job_db::OWNER_ID];
      $job['status'] = $row[mediamosa_job_db::JOB_STATUS];
      $job['progress'] = $row[mediamosa_job_db::PROGRESS];
      $job['priority'] = $row[mediamosa_job_db::PRIORITY];
      $job['job_type'] = $row[mediamosa_job_db::JOB_TYPE];
      $job['started'] = $started_unix ? format_date($started_unix, 'custom', 'd-m-Y H:i') : '';
      $job['started_unix'] = $started_unix;
      $job['error_description'] = $row[mediamosa_job_db::ERROR_DESCRIPTION];

      $mediamosa->add_item($job);
    }
  }
}

/**
 * URI: /mediafile/$mediafile_id/joblist
 * Method: GET
 *
 * Return list of jobs based on mediafile_id
 */
class mediamosa_rest_call_job_mediafile_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $user_id = $this->get_param_value(self::USER_ID);
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);

    // Must exists.
    mediamosa_asset_mediafile::must_exists_cached($mediafile_id);

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_id, $user_id, $is_app_admin, FALSE, FALSE, $mediafile_id);

    // Get the count.
    $mediamosa->item_count_total = $result->found_rows;

    // Build result.
    foreach ($result as $row) {
      $started_unix = isset($row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($row[mediamosa_job_db::STARTED]) : '';

      $job = array();

      $job['id'] = $row[mediamosa_job_db::ID];
      $job['owner'] = $row[mediamosa_job_db::OWNER_ID];
      $job['status'] = $row[mediamosa_job_db::JOB_STATUS];
      $job['progress'] = $row[mediamosa_job_db::PROGRESS];
      $job['priority'] = $row[mediamosa_job_db::PRIORITY];
      $job['job_type'] = $row[mediamosa_job_db::JOB_TYPE];
      $job['started'] = $started_unix ? format_date($started_unix, 'custom', 'd-m-Y H:i') : '';
      $job['started_unix'] = $started_unix;
      $job['error_description'] = $row[mediamosa_job_db::ERROR_DESCRIPTION];

      $mediamosa->add_item($job);
    }
  }
}

/**
 * URI: /job/$job_id/status
 *      /video/transcode/$job_id/status (deprecated)
 *      /mediafile/transcode/$job_id/status
 *
 * Method: GET
 *
 * Return list of jobs based on job_id
 */
class mediamosa_rest_call_job_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const JOB_ID = 'job_id';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The job ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = FALSE; //$this->get_param_value(self::IS_APP_ADMIN); (not supplied).

    $user_id = $this->get_param_value(self::USER_ID);
    $job_id = $this->get_param_value(self::JOB_ID);

    // Must exists.
    mediamosa_job::must_exists($job_id);

    // Get the jobs from the user.
    $result = mediamosa_job::get_job_list($app_id, $user_id, $is_app_admin, $job_id);

    // Get the count.
    $mediamosa->item_count_total = $result->found_rows;

    // Build result.
    foreach ($result as $row) {
      $started_unix = isset($row[mediamosa_job_db::STARTED]) ? mediamosa_lib::iso8601date_to_unix($row[mediamosa_job_db::STARTED]) : '';

      $job = array();

      $job['id'] = $row[mediamosa_job_db::ID];
      $job['owner'] = $row[mediamosa_job_db::OWNER_ID];
      $job['status'] = $row[mediamosa_job_db::JOB_STATUS];
      $job['progress'] = $row[mediamosa_job_db::PROGRESS];
      $job['priority'] = $row[mediamosa_job_db::PRIORITY];
      $job['job_type'] = $row[mediamosa_job_db::JOB_TYPE];
      $job['started'] = $started_unix ? format_date($started_unix, 'custom', 'd-m-Y H:i') : '';
      $job['started_unix'] = $started_unix;
      $job['error_description'] = $row[mediamosa_job_db::ERROR_DESCRIPTION];

      $mediamosa->add_item($job);
    }
  }
}

/**
 * URI: /video/$mediafile_id/transcode (deprecated)
 *      /mediafile/$mediafile_id/transcode
 *
 * Method: POST
 *
 * Create a transcode job.
 */
class mediamosa_rest_call_job_create_transcode extends mediamosa_rest_call_job_create {
  // ------------------------------------------------------------------- Consts.

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = parent::get_var_setup();

    unset($var_setup[self::VARS][parent::JOB_TYPE]);

    return $var_setup;
  }

  // ------------------------------------------------------- Progress Rest Args.
  /**
   * Process the supplied parameters.
   *
   * @param array $var_setup
   */
  protected function process_rest_args(array $var_setup) {

    // Set job type and give it fixed value.
    $var_setup[self::VARS][self::JOB_TYPE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_JOB,
      self::VAR_DESCRIPTION => 'The job type.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_OVERRIDE_VALUE => mediamosa_job_db::JOB_TYPE_TRANSCODE,
    );

    // GET / POST to array.
    $var_setup = parent::process_rest_args($var_setup);

    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    parent::do_call();
  }
}

/**
 * URI: /mediafile/$mediafile_id/retranscode
 *
 * Method: POST
 *
 * Create a retranscode job.
 */
class mediamosa_rest_call_job_create_retranscode extends mediamosa_rest_call_job_create {
  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = parent::get_var_setup();

    unset($var_setup[self::VARS][parent::JOB_TYPE]);

    return $var_setup;
  }

  // ------------------------------------------------------- Progress Rest Args.
  /**
   * Process the supplied parameters.
   *
   * @param array $var_setup
   */
  protected function process_rest_args(array $var_setup) {

    // Set job type and give it fixed value.
    $var_setup[self::VARS][self::JOB_TYPE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_JOB,
      self::VAR_DESCRIPTION => 'The job type.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_OVERRIDE_VALUE => mediamosa_job_db::JOB_TYPE_RETRANSCODE,
    );

    // GET / POST to array.
    $var_setup = parent::process_rest_args($var_setup);

    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    parent::do_call();
  }
}

/**
 * URI: /asset/$asset_id/still/create
 * Method: POST
 *
 * Create a still job.
 */
class mediamosa_rest_call_job_create_still extends mediamosa_rest_call_job_create {
  // ------------------------------------------------------------------- Consts.
  // The asset ID.
  const ASSET_ID = 'asset_id';

  // Only used here, converted to size.
  const WIDTH = 'width';
  const HEIGHT = 'height';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = parent::get_var_setup();

    unset($var_setup[self::VARS][parent::JOB_TYPE]);

    // Although its required, its not used(!).
    // note: mediamosa_rest_call_job_create_still_for_mediafile() depends on
    // this non-requirement and non-usage.
    $var_setup[self::VARS][self::ASSET_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
      self::VAR_DESCRIPTION => 'The asset ID of the medafile.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
    );

    $var_setup[self::VARS][self::WIDTH] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
      self::VAR_DESCRIPTION => 'Still width. Deprecated, use size instead.',
    );
    $var_setup[self::VARS][self::HEIGHT] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
      self::VAR_DESCRIPTION => 'Still height. Deprecated, use size instead.',
    );

    return $var_setup;
  }

  // ------------------------------------------------------- Progress Rest Args.
  /**
   * Process the supplied parameters.
   *
   * @param array $var_setup
   */
  protected function process_rest_args(array $var_setup) {

    // Set job type and give it fixed value.
    $var_setup[self::VARS][self::JOB_TYPE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_JOB,
      self::VAR_DESCRIPTION => 'The job type.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_OVERRIDE_VALUE => mediamosa_job_db::JOB_TYPE_STILL,
    );

    // GET / POST to array.
    $var_setup = parent::process_rest_args($var_setup);

    // Fix problems with usage of obsolete FRAMETIME, WIDTH and HEIGHT.
    $frametime = $this->get_param_value(self::FRAMETIME);
    $start_frame = $this->get_param_value(self::START_FRAME);
    $end_frame = $this->get_param_value(self::END_FRAME);
    if (!$start_frame && $frametime) {
      $this->set_param_value(self::START_FRAME, $frametime);
    }

    $width = $this->get_param_value(self::WIDTH);
    $height = $this->get_param_value(self::HEIGHT);

    if (!$start_frame && $frametime) {
      $this->set_param_value(self::START_FRAME, $frametime);
    }

    // Start/end time replaces start/end frame.
    $start_time = $this->get_param_value(self::START_TIME);
    if (!$start_frame && $start_time) {
      $this->set_param_value(self::START_FRAME, $start_time);
    }

    $end_time = $this->get_param_value(self::END_TIME);
    if (!$end_frame && $end_time) {
      $this->set_param_value(self::END_FRAME, $end_time);
    }

    // If size is not given, but width and height are, use width, height.
    if (!$this->isset_given_param(self::SIZE) && $width && $height) {
      $this->set_param_value(self::SIZE, $width . 'x' . $height);
    }
    // If there isn't size, we get the still default size of application.
    if (!$this->isset_given_param(self::SIZE)) {
      $app_ids = $this->get_param_value_app();
      $app_id = reset($app_ids);
      $this->set_param_value(self::SIZE, mediamosa_app::get_still_default_size($app_id));
    }

    unset($var_setup[self::VARS][self::FRAMETIME]);
    unset($var_setup[self::VARS][self::WIDTH]);
    unset($var_setup[self::VARS][self::HEIGHT]);

    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    parent::do_call();
  }
}

/**
 * URI: /mediafile/$mediafile_id/still/create
 *      /video/$mediafile_id/still/create (deprecated)
 *
 * Method: POST
 *
 * Create a still job.
 *
 * This class is more a dummy for mediamosa_rest_call_job_create_still,
 * I could choose to set the URL directly on the class, but for better
 * view on code, lets keep it like own class for now. The only difference
 * is that mediafile_id comes from the URI, which is not a problem for the
 * MediaMosa REST interface. Doesn't matter if mediafile_id comes from
 * URI / GET / POST.
 */
class mediamosa_rest_call_job_create_still_for_mediafile extends mediamosa_rest_call_job_create_still {
  // ------------------------------------------------------------------- Consts.

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = parent::get_var_setup();

    // Unset the asset_id (we dont use it).
    unset($var_setup[self::VARS][self::ASSET_ID]);

    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    parent::do_call();
  }
}

/**
 * URI: /job/$job_id/delete
 *
 * Method: POST
 *
 * Cancel job.
 */
class mediamosa_rest_call_job_cancel extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const JOB_ID = 'job_id';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID for owner.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_JOB_ID,
          self::VAR_DESCRIPTION => 'The job ID to search on.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $user_id = $this->get_param_value(self::USER_ID);
    $job_id = $this->get_param_value(self::JOB_ID);

    // Cancel it.
    mediamosa_job::cancel_job($app_id, $job_id, $user_id);

    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /cron/jobscheduler
 * Method: GET
 */
class mediamosa_rest_call_cron_jobscheduler extends mediamosa_rest_call {

  // ---------------------------------------------------------------- Functions.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Run them.
    mediamosa_job_scheduler::run_parse_queue();

    // Set ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /cron/jobserver
 * Method: GET
 */
class mediamosa_rest_call_cron_jobserver extends mediamosa_rest_call {

  // ---------------------------------------------------------------- Functions.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Run them.
    mediamosa_job_server::run_parse_queue();

    // Set ok.
    $mediamosa->set_result_okay();
  }
}
