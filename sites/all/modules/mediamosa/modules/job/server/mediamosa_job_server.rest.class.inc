<?php
/**
 * @file
 * The job server module.
 */

/**
 * URI: /server/joblist
 * Method: GET
 *
 * Internal only.
 */
class mediamosa_rest_call_jobserver_job_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $jobs = mediamosa_job_server::search();

    foreach ($jobs as $job) {
      $mediamosa->add_item($job);
    }
  }
}

/**
 * URI: /server/jobstart
 * Method: POST
 *
 * Internal only.
 */
class mediamosa_rest_call_jobserver_job_start extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const JOB_ID = 'job_id';
  const JOB_TYPE = 'job_type';
  const MEDIAFILE_SRC = 'mediafile_src';
  const TOOL = 'tool';
  const FILE_EXTENSION = 'file_extension';
  const COMMAND = 'command';
  const FRAMETIME = 'frametime';
  const SIZE = 'size';
  const H_PADDING = 'h_padding';
  const V_PADDING = 'v_padding';
  const TAG = 'tag';
  const BLACKSTILL_CHECK = 'blackstill_check';
  const STILL_TYPE = 'still_type';
  const STILL_PER_MEDIAFILE = 'still_per_mediafile';
  const STILL_EVERY_SECOND = 'still_every_second';
  const START_FRAME = 'start_frame';
  const END_FRAME = 'end_frame';
  const VIDEO_DURATION= 'video_duration';
  const FPS = 'fps';

  // Watermark.
  const WATERMARK_ID = 'watermark_id';
  const WATERMARK_DST_X = 'watermark_dst_x';
  const WATERMARK_DST_Y = 'watermark_dst_y';
  const WATERMARK_PCT = 'watermark_pct';
  const WATERMARK_V_ALIGN = 'watermark_v_align';
  const WATERMARK_H_ALIGN = 'watermark_h_align';

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
        self::JOB_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_JOB,
          self::VAR_DESCRIPTION => 'The job type.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_SRC => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The mediafile destination filename.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
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
        self::FRAMETIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still frametime.',
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
        self::BLACKSTILL_CHECK => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Check if result end with a empty still (black).',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::STILL_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The still type.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_PICTURE,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_SCENE,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_SECOND,
            mediamosa_asset_mediafile_metadata::STILL_TYPE_NORMAL,
            '',
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
        ),
        self::STILL_PER_MEDIAFILE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills per mediafile (Used when still_type is NORMAL).',
        ),
        self::STILL_EVERY_SECOND => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Stills every second (Used when still_type is SECOND).',
        ),
        self::START_FRAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still start frame (in seconds, used when still_type is NORMAL or SECOND).',
        ),
        self::END_FRAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still end frame (in seconds, used when still_type is NORMAL or SECOND).',
        ),
        self::VIDEO_DURATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Video duration.',
        ),
        self::FPS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Frame per second.',
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

      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $job_id = $this->get_param_value(self::JOB_ID);
    $job_type = $this->get_param_value(self::JOB_TYPE);
    $mediafile_id_src = $this->get_param_value(self::MEDIAFILE_SRC);
    $watermark_id = $this->get_param_value(self::WATERMARK_ID);

    // Webservice must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_JOBS, $a_app_ids);

    // Based on type, call job creation.
    switch ($job_type) {
      case mediamosa_job_server_db::JOB_TYPE_ANALYSE:
        mediamosa_job_server::create_job_analyse($job_id, $mediafile_id_src);
        break;

      case mediamosa_job_server_db::JOB_TYPE_STILL:
        $still_parameters = array(
          'still_type' => $this->get_param_value(self::STILL_TYPE),
          'still_per_mediafile' => $this->get_param_value(self::STILL_PER_MEDIAFILE),
          'still_every_second' => $this->get_param_value(self::STILL_EVERY_SECOND),
          'start_frame' => $this->get_param_value(self::START_FRAME),
          'end_frame' => $this->get_param_value(self::END_FRAME),
          'tag' => $this->get_param_value(self::TAG),
          'video_duration' => $this->get_param_value(self::VIDEO_DURATION),
          'fps' => $this->get_param_value(self::FPS),
          // Watermark.
          self::WATERMARK_ID => $watermark_id,
          self::WATERMARK_DST_X => $this->get_param_value(self::WATERMARK_DST_X),
          self::WATERMARK_DST_Y => $this->get_param_value(self::WATERMARK_DST_Y),
          self::WATERMARK_PCT => $this->get_param_value(self::WATERMARK_PCT),
          self::WATERMARK_V_ALIGN => $this->get_param_value(self::WATERMARK_V_ALIGN),
          self::WATERMARK_H_ALIGN => $this->get_param_value(self::WATERMARK_H_ALIGN),
        );

        // Get params.
        $frametime = $this->get_param_value(self::FRAMETIME);
        $size = ($this->get_param_value(self::SIZE) ? $this->get_param_value(self::SIZE) : mediamosa_app::get_still_default_size($app_id));
        $h_padding = $this->get_param_value(self::H_PADDING);
        $v_padding = $this->get_param_value(self::V_PADDING);
        $blackstill_check = $this->get_param_value(self::BLACKSTILL_CHECK);

        mediamosa_job_server::create_job_still($job_id, $mediafile_id_src, $frametime, $size, $h_padding, $v_padding, $blackstill_check, $still_parameters);
        break;

      case mediamosa_job_server_db::JOB_TYPE_TRANSCODE:
        $tool = $this->get_param_value(self::TOOL);
        $file_extension = $this->get_param_value(self::FILE_EXTENSION);
        $command = $this->get_param_value(self::COMMAND);

        mediamosa_job_server::create_job_transcode($job_id, $mediafile_id_src, $tool, $file_extension, $command);
        break;

      default:
        // Create job.
        mediamosa_job_server::create_job($job_id, $job_type, $mediafile_id_src);
        break;
    }

    // Set ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /server/$job_id/delete
 * Method: POST
 *
 * Internal only.
 */
class mediamosa_rest_call_jobserver_job_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const JOB_ID = 'job_id';
  const KILLJOB = 'killjob';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::JOB_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The job ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::KILLJOB => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'The job ID.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    // Collect parameter values.
    $job_id = $this->get_param_value(self::JOB_ID);
    $killjob = $this->get_param_value(self::KILLJOB);

    // Kill the job.
    mediamosa_job_server::delete_job($job_id, $killjob);
  }
}
