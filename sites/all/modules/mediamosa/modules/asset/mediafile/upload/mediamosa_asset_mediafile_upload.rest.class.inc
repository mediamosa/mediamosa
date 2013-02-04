<?php
/**
 * @file
 * REST functions for upload.
 */

/**
 * URI: /mediafile/$mediafile_id/uploadticket/create
 *      /mediafile/$mediafile_id/upload_ticket (deprecated)
 *
 * Method: GET
 */
class mediamosa_rest_call_asset_mediafile_uploadticket_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';

  private $a_metadata_definitions_full = NULL;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'ID of an original file (then it is without transcode) or empty mediafile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User id, determined to check whether the applicant has the rights and quotas to upload the mediafile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Group id, used for any quota check..',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);

    // Create.
    $ticket_id = mediamosa_media_ticket::create_for_upload($app_id, $mediafile_id, $user_id, $group_id);

    // Get serverupload.
    $server_upload = mediamosa_server::get_random_upload();

    if (!$server_upload) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_UPLOAD_NOT_FOUND);
    }

    // Generate unique ID.
    $uuid = rand(1000000, 9999999);

    // Set response.
    $mediamosa->add_item(
      array(
        'action' => strtr(mediamosa_server::get_server_url($server_upload), array('{TICKET}' => $ticket_id, '{PROGRESS_ID}' => $uuid)),
        'uploadprogress_url' => strtr($server_upload[mediamosa_server_db::URI_UPLOAD_PROGRESS], array('{PROGRESS_ID}' => $uuid, '{SERVER_ID}' => $server_upload[mediamosa_server_db::NID])),
        'ticket_id' => $ticket_id,
        'progress_id' => $uuid,
        'server_id' => $server_upload[mediamosa_server_db::NID],
      )
    );
  }
}

///**
// * URI: /mediafile/$mediafile_id/upload/create
// *
// * Method: POST
// */
//class mediamosa_rest_call_asset_mediafile_upload_create extends mediamosa_rest_call {
//  // ------------------------------------------------------------------ Consts.
//  // Rest vars;
//  const MEDIAFILE_ID = 'mediafile_id';
//  const USER_ID = 'user_id';
//  const GROUP_ID = 'group_id';
//
//  private $a_metadata_definitions_full = NULL;
//
//  // ------------------------------------------------------------------ Get Var Setup.
//  public function get_var_setup() {
//    $a_var_setup = array(
//      self::VARS => array(
//        self::MEDIAFILE_ID => array(
//          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
//          self::VAR_DESCRIPTION => 'ID of an original file (then it is without transcode) or empty mediafile.',
//          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
//          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
//        ),
//        self::USER_ID => array(
//          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
//          self::VAR_DESCRIPTION => 'User id, determined to check whether the applicant has the rights and quotas to upload the mediafile.',
//          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
//          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
//        ),
//        self::GROUP_ID => array(
//          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
//          self::VAR_DESCRIPTION => 'Group id, used for any quota check..',
//          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
//        ),
//      )
//    );
//
//    // Enrich with required REST vars.
//    return self::get_var_setup_default($a_var_setup);
//  }
//
//  // ------------------------------------------------------------------ Do Call.
//  public function do_call() {
//    if (module_exists('mediamosa_storage')) {
//      $this->storage_module_call();
//    }
//    else {
//      $this->standard_ticket_call();
//    }
//  }
//
//  private function storage_module_call() {
//    $mediamosa = mediamosa::get();
//
//    // Get the app_id.
//    $app_ids = $this->get_param_value_app();
//    $app_id = reset($app_ids);
//
//    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
//    $user_id = $this->get_param_value(self::USER_ID);
//    $group_id = $this->get_param_value(self::GROUP_ID);
//
//    // Decide which storage class to use
//    $mediafile = mediamosa_storage_profile_app::get_mediafile_object($mediafile_id);
//    $mediafile = mediamosa_storage::create_instance_for_mediafile($mediafile_id);
//
//    // Set response.
//    $mediamosa->add_item($mediafile->get_upload_parameters($app_id, $user_id, $group_id));
//  }
//
//  private function standard_ticket_call() {
//    $mediamosa = mediamosa::get();
//
//    // Get the app_id.
//    $app_ids = $this->get_param_value_app();
//    $app_id = reset($app_ids);
//
//    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
//    $user_id = $this->get_param_value(self::USER_ID);
//    $group_id = $this->get_param_value(self::GROUP_ID);
//
//    // Decide which storage class to use
//
//
//    // Create.
//    $ticket_id = mediamosa_ticket::create_for_upload($app_id, $mediafile_id, $user_id, $group_id);
//
//    // Get serverupload.
//    $server_upload = mediamosa_server::get_random_upload();
//
//    if (!$server_upload) {
//      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_UPLOAD_NOT_FOUND);
//    }
//
//    // Generate unique ID.
//    $uuid = rand(1000000, 9999999);
//
//    // Set response.
//    $mediamosa->add_item(
//    array(
//          'action' => strtr(mediamosa_server::get_server_url($server_upload), array('{TICKET}' => $ticket_id, '{PROGRESS_ID}' => $uuid)),
//          'uploadprogress_url' => strtr($server_upload[mediamosa_server_db::URI_UPLOAD_PROGRESS], array('{PROGRESS_ID}' => $uuid, '{SERVER_ID}' => $server_upload[mediamosa_server_db::NID])),
//          'ticket_id' => $ticket_id,
//          'progress_id' => $uuid,
//          'server_id' => $server_upload[mediamosa_server_db::NID],
//    )
//    );
//  }
//}

/**
 * URI: /mediafile/upload
 *
 * Method: POST
 */
class mediamosa_rest_call_asset_mediafile_upload extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const UPLOAD_TICKET = 'upload_ticket';
  const REDIRECT_URI = 'redirect_uri';
  const TRANSCODE = 'transcode';
  const TRANSCODE_INHERITS_ACL = 'transcode_inherits_acl';
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

  // Watermark.
  const WATERMARK_ID = 'watermark_id';
  const WATERMARK_DST_X = 'watermark_dst_x';
  const WATERMARK_DST_Y = 'watermark_dst_y';
  const WATERMARK_PCT = 'watermark_pct';
  const WATERMARK_V_ALIGN = 'watermark_v_align';
  const WATERMARK_H_ALIGN = 'watermark_h_align';

  const RETRANSCODE = 'retranscode';
  const FILENAME = 'filename';
  const COMPLETED_URL = 'completed_url';
  const COMPLETED_TRANSCODING_URL = 'completed_transcoding_url';
  const STILL_UPLOAD = 'still_upload';
  const ORDER = 'order';
  const DEFAULT_ = 'default';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::UPLOAD_TICKET => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ID that has been given to enable the upload.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_db::HASH_ID_LENGTH,
        ),
        self::REDIRECT_URI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URL_URI,
          self::VAR_DESCRIPTION => 'The URL to redirect when upload has been successful.',
        ),
        self::TRANSCODE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'If one (or more) transcode profiles is in this parameter than the media file will be transcoded after the upload.',
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_DEFAULT_VALUE => array(),
        ),
        self::TRANSCODE_INHERITS_ACL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "If 'true'; All the acl media rights will be copied to the new transcoded media file.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::CREATE_STILL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Specify 'true', if you want a still is generated after the upload and analysis\n\nSee: Still creation parameters (POST) on /mediafile/{mediafile_id}/still/create [POST].",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::STILL_UPLOAD => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Specify TRUE for adding a still to the mediafile.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::ORDER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still order number. Only used when still_upload = true.',
          self::VAR_DEFAULT_VALUE => 0,
        ),
        self::DEFAULT_ => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Is default still? Only used when still_upload = true.',
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
          self::VAR_DESCRIPTION => 'Mediafile tag to set. An empty tag will clear possible set mediafile tag.',
        ),
        self::FRAMETIME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still frametime (in seconds) for still_type NONE and for still upload.',
        ),
        self::WIDTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still width.',
        ),
        self::HEIGHT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Still height.',
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

        self::RETRANSCODE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "if 'retranscode' is not put, all existing transcodes of the original media file will be lost. If 'retranscode' is 'true', then new transcode jobs are created, which will replace the existing transcodes.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The filename of the media file (PUT method only, ignored otherwise).',
          self::VAR_IS_REQUIRED => $this->is_method_put() ? self::VAR_IS_REQUIRED_YES : self::VAR_IS_REQUIRED_NO,
        ),
        self::COMPLETED_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URL,
          self::VAR_DESCRIPTION => "This URL gives opportunity to the APP builder for a status raport; \nupload_ticket\nstatus_code\n\nThe upload_ticket is used to identify the upload. The status_code is a MediaMosa code.\nThe type is URL and not URI, because this parameter http:// or https://.",
        ),
        self::COMPLETED_TRANSCODING_URL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URL,
          self::VAR_DESCRIPTION => "This URL is called after the status of transcoding job changed.",
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Ticket id.
    $ticket_id = $this->get_param_value(self::UPLOAD_TICKET);
    $watermark_id = $this->get_param_value(self::WATERMARK_ID);

    // Get the ticket.
    $ticket = mediamosa_media_ticket::get_valid_upload($ticket_id);
    if (empty($ticket)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_UPLOAD_TICKET);
    }
    // Get the mediafile using the ticket mediafile_id.
    $mediafile = mediamosa_asset_mediafile::must_exists($ticket[mediamosa_media_ticket_db::MEDIAFILE_ID]);
    $app_id = $mediafile[mediamosa_asset_mediafile_db::APP_ID];

    // Check watermark id, if exists.
    if ($watermark_id) {
      mediamosa_asset_mediafile::must_exists_cached($watermark_id, $app_id);
    }

    // Get params.
    $create_still = $this->get_param_value(self::CREATE_STILL);
    $still_parameters = array(
      'still_type' => $this->get_param_value(self::STILL_TYPE),
      'still_per_mediafile' => $this->get_param_value(self::STILL_PER_MEDIAFILE),
      'still_every_second' => $this->get_param_value(self::STILL_EVERY_SECOND),
      'start_frame' => $this->get_param_value(self::START_FRAME),
      'end_frame' => $this->get_param_value(self::END_FRAME),
      'size' => ($this->get_param_value(self::SIZE) ? $this->get_param_value(self::SIZE) : mediamosa_app::get_still_default_size($app_id)),
      'h_padding' => ($this->get_param_value_org(self::H_PADDING) == '' ? NULL : $this->get_param_value(self::H_PADDING)),
      'v_padding' => ($this->get_param_value_org(self::V_PADDING) == '' ? NULL : $this->get_param_value(self::V_PADDING)),
      'tag' => $this->get_param_value(self::TAG),
      'frametime' => $this->get_param_value(self::FRAMETIME),
      'width' => $this->get_param_value(self::WIDTH),
      'height' => $this->get_param_value(self::HEIGHT),
      // Watermark.
      self::WATERMARK_ID => $watermark_id,
      self::WATERMARK_DST_X => $this->get_param_value(self::WATERMARK_DST_X),
      self::WATERMARK_DST_Y => $this->get_param_value(self::WATERMARK_DST_Y),
      self::WATERMARK_PCT => $this->get_param_value(self::WATERMARK_PCT),
      self::WATERMARK_V_ALIGN => $this->get_param_value(self::WATERMARK_V_ALIGN),
      self::WATERMARK_H_ALIGN => $this->get_param_value(self::WATERMARK_H_ALIGN),
    );
    $completed_url = $this->get_param_value(self::COMPLETED_URL);
    $completed_transcoding_url = $this->get_param_value(self::COMPLETED_TRANSCODING_URL);
    $transcode_inherits_acl = $this->get_param_value(self::TRANSCODE_INHERITS_ACL);
    $tag = $this->get_param_value(self::TAG);
    $trancode_profiles = $this->get_param_value(self::TRANSCODE);
    $retranscode = $this->get_param_value(self::RETRANSCODE);

    // Is still upload?
    $still_upload = $this->get_param_value(self::STILL_UPLOAD);
    $order = $this->get_param_value(self::ORDER);
    $default = $this->get_param_value(self::DEFAULT_);

    $filename_put = NULL;

    if ($this->is_method_put()) {
      $filename_put = $this->get_param_value(self::FILENAME); // With PUT only.
    }

    try {
      if ($still_upload) {
        $still_id = mediamosa_asset_mediafile_upload::handle_upload_still($ticket_id, $this->is_method_put(), $tag, $completed_url, $filename_put, $order, $default, $still_parameters, $this->get_param_value(self::FRAMETIME));

        // Add still_id to output.
        $mediamosa->add_item(array('still_id' => $still_id));
      }
      else { // Normal upload.
        mediamosa_asset_mediafile_upload::handle_upload($ticket_id, $this->is_method_put(), $tag, $completed_url, $filename_put, $trancode_profiles, $retranscode, $transcode_inherits_acl, $create_still, $still_parameters, $completed_transcoding_url);
      }
    }
    catch (Exception $e) {

      if ($this->isset_param(self::REDIRECT_URI)) {
        // Any errors need to be redirected.
        throw new mediamosa_exception_redirect_and_exit($this->get_param_value(self::REDIRECT_URI), $e->getCode(), $e->getMessage());
      }

      // Lets create error output then.
      throw $e;
    }

    // Redirect when needed.
    if ($this->isset_param(self::REDIRECT_URI)) {
      throw new mediamosa_exception_redirect_and_exit($this->get_param_value(self::REDIRECT_URI), mediamosa_error::ERRORCODE_OKAY);
    }

    // Set ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/upload
 * Method: PUT
 */
class mediamosa_rest_call_asset_mediafile_upload_put extends mediamosa_rest_call_asset_mediafile_upload {
}


/**
 * URI: mediafile/uploadprogress
 * Method: GET
 */
class mediamosa_rest_call_asset_mediafile_uploadprogress extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ID = 'id';
  const SERVER_ID = 'server_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    // Add ID of the upload.
    $var_setup[self::VARS][self::ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
      self::VAR_DESCRIPTION => 'ID of the upload that has been randomly chosen and provided when the upload was send to the server.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
    );
    // Add ID of the upload.
    $var_setup[self::VARS][self::SERVER_ID] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
      self::VAR_DESCRIPTION => 'The server ID of the upload server. Only required when you have multiple upload servers and with different settings (upload progress).',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
      self::VAR_DEFAULT_VALUE => 0,
    );

    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $id = $this->get_param_value(self::ID);
    $server_id = $this->get_param_value(self::SERVER_ID);

    // Add to response.
    $mediamosa->add_item(json_encode(mediamosa_asset_mediafile_upload::get_uploadprogress($id, $server_id)));
  }
}
