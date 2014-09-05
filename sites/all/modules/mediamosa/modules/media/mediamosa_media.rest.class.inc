<?php
/**
 * @file
 * Media Play functions
 */

/**
 * URI: /asset/$asset_id/view
 *      /asset/$asset_id/play (deprecated)
 *      /video/$asset_id/play (deprecated)
 * Method: GET
 */
class mediamosa_rest_call_media_view extends mediamosa_rest_call {
  // --------------------------------------------------------0---------- Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const ORIGINAL_MEDIAFILE_ID = 'original_mediafile_id';
  const STILL_ID = 'still_id';
  const RESPONSE = 'response';
  const DOMAIN = 'domain';
  const REALM = 'realm';
  const ACL_GROUP_ID = 'acl_group_id';
  const ACL_DOMAIN = 'acl_domain';
  const ACL_REALM = 'acl_realm';
  const PROFILE_ID = 'profile_id';
  const WIDTH = 'width';
  const HEIGHT = 'height';
  const START = 'start';
  const DURATION = 'duration';
  const AUTOSTART = 'autostart';
  const SIZE = 'size';
  const FORMAT = 'format';
  const RANGE = 'range';
  const TAG = 'tag';
  const COUNT_PLAY = 'count_play';

  // Aliases, do NOT use in code(!).
  const ALIAS_AUT_GROUP_ID = 'aut_group_id';
  const ALIAS_AUT_DOMAIN = 'aut_domain';
  const ALIAS_AUT_REALM = 'aut_realm';

  // ---------------------------------------------------------------- Functions.
  /**
   * Implements get_var_setup()
   */
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID and owner of the asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID, is not required when profile_id is used or when response type is still. When response type is still, then mediafile ID is optional and can be used to retrieve a listing of stills that have this Mediafile as parent (stills created from).',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        // TODO: Group id is deprecated. Should be removed in 3.5.
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Deprecated. Use acl_group_id instead. Group ID of the person who wants to play the video, can be used for authorization.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::ORIGINAL_MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The original mediafile ID.',
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'The still ID.',
        ),
        self::RESPONSE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The response type.',
          self::VAR_ALLOWED_VALUES => array(
            mediamosa_media::RESPONSE_DOWNLOAD,
            mediamosa_media::RESPONSE_METAFILE,
            mediamosa_media::RESPONSE_OBJECT,
            mediamosa_media::RESPONSE_STILL,
            mediamosa_media::RESPONSE_URI
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_media::RESPONSE_URI
        ),
        self::ACL_GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'Group ID of the person who wants to play the video, can be used for authorization.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_GROUP_ID),
        ),
        self::ACL_DOMAIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'The domain for which the application is made. If specified it is used for authorization. The domain is checked against the acl rules of the media files, if the domain appears in the / acl / domain list, the video is played.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_DOMAIN),
        ),
        self::ACL_REALM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
          self::VAR_DESCRIPTION => 'If specified it is used for authorization.',
          self::VAR_ALIAS_FOR => array(self::ALIAS_AUT_REALM),
        ),
        self::DOMAIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DOMAIN,
          self::VAR_DESCRIPTION => 'The domain for which the application is made. If specified it is used for authorization. The domain is checked against the acl rules of the media files, if the domain appears in the / acl / domain list, the video is played.',
          self::VAR_IS_HIDDEN => self::VAR_IS_HIDDEN_YES,
        ),
        self::REALM => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_REALM,
          self::VAR_DESCRIPTION => 'If specified it is used for authorization.',
          self::VAR_IS_HIDDEN => self::VAR_IS_HIDDEN_YES,
        ),
        self::PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => "Play the mediafile which is transcode with {profile_id}.\n\n(new since 1.7.0) If {original_mediafile_id} is also given, plays the transcoded file with profile_id= {profile_id} which was transcoded from source file mediafile_id = {original_mediafile_id}",
        ),
        self::WIDTH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The width of the video.',
        ),
        self::HEIGHT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The height of the video.',
        ),
        self::START => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Start time of the video clip in milliseconds. Must be less than 24 hours (86,400,000)',
          self::VAR_RANGE_START => 0,
          // (24 uur in msec)-1 msec.
          self::VAR_RANGE_END => 86399999,
        ),
        self::DURATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Playing length of the video clip in milliseconds. Must be less than 24 hours (86,400,000).',
          self::VAR_RANGE_START => 0,
          // (24 uur in msec)-1 msec.
          self::VAR_RANGE_END => 86399999,
        ),
        self::AUTOSTART => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Adjust the value of the autostart in object code.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
        self::SIZE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'If response type = still, then size parameter may filter the response (eg. 150x120).',
        ),
        self::FORMAT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "If response type = still, then it is possble to filter the response to format (eg. 'jpeg').",
        ),
        self::RANGE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "If response type = still, then it is possible to filter the response to range (to the still order value) (eg. '3', or '3,5,6', or '3, 5-8').",
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "Tag to indentify and search for the mediafile to play.",
        ),
        self::COUNT_PLAY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "Specifing 'FALSE' will prevent increasing the play count on the asset. Default 'TRUE'.",
          self::VAR_DEFAULT_VALUE => 'TRUE',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  /**
   * Implements process_rest_args()
   */
  protected function process_rest_args(array $var_setup) {
    $response_type = $this->get_param_value_global(self::RESPONSE);
    $profile_id = $this->get_param_value_global(self::PROFILE_ID);

    // Is profile ID is set or response type is still then mediafile_id is not required.
    if (isset($profile_id) || $response_type == mediamosa_media::RESPONSE_STILL) {
      $var_setup[self::VARS][self::MEDIAFILE_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;
    }

    return parent::process_rest_args($var_setup);
  }

  /**
   * Implements do_call()
   *
   * @throws mediamosa_exception_error
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Fix renames.
    if ($this->isset_given_param(self::DOMAIN)) {
      $this->set_param_value(self::ACL_DOMAIN, $this->get_param_value(self::DOMAIN));
    }
    if ($this->isset_given_param(self::REALM)) {
      $this->set_param_value(self::ACL_REALM, $this->get_param_value(self::REALM));
    }

    // FIXME: hack, fixed again by checking if response was not given.
    if ($this->is_internal() && !$this->isset_given_param(self::RESPONSE)) {
      $this->set_param_value(self::RESPONSE, mediamosa_media::RESPONSE_STILL);
    }

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $original_mediafile_id = $this->get_param_value(self::ORIGINAL_MEDIAFILE_ID);

    $response_type = $this->get_param_value(self::RESPONSE);
    $profile_id = $this->get_param_value(self::PROFILE_ID);

    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);
    $user_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $acl_group_id = $this->get_param_value(self::ACL_GROUP_ID);
    $acl_domain = $this->get_param_value(self::ACL_DOMAIN);
    $acl_realm = $this->get_param_value(self::ACL_REALM);

    $tag = $this->get_param_value(self::TAG);

    // Group id is deprecated.
    if (!$acl_group_id && $group_id) {
      $acl_group_id = $group_id;
    }

    // Get the asset.
    $asset = mediamosa_asset::must_exists($asset_id);

    // Test if access on unappropriate flag.
    mediamosa_asset::is_inappropriate($app_ids, $asset_id, $user_id, $is_app_admin, TRUE);

    // Check play restrictions.
    if ($asset['play_restriction_start'] != '' || $asset['play_restriction_end'] != '') {
      mediamosa_media::check_time_restrictions(
        strtotime($asset['play_restriction_start']),
        strtotime($asset['play_restriction_end'])
      );
    }

    // All other response have mediafile_id as required.
    if ($response_type != mediamosa_media::RESPONSE_STILL) {
      if ($this->isset_given_param(self::PROFILE_ID)) {
        $mediafile_id = mediamosa_asset_mediafile::get_mediafile_id_by_profile($asset_id, $profile_id, $original_mediafile_id);
        if (!$mediafile_id) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_MEDIAFILE_FOUND_FOR_PROFILE_ID);
        }
      }
      elseif (!$mediafile_id && $this->isset_given_param(self::TAG)) {
        $mediafile_id = mediamosa_asset_mediafile::get_mediafile_id_by_tag($asset_id, $tag);
        if (!$mediafile_id) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_MEDIAFILE_FOUND_FOR_TAG);
        }
      }

      // At this point, mediafile_id must be set.
      if (!$mediafile_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array('@param' => self::MEDIAFILE_ID, '@type' => mediamosa_sdk::TYPE_MEDIAFILE_ID));
      }
    }

    // Mediafile ID is only optional when response is still.
    if (!empty($mediafile_id)) {
      // Get mediafile.
      $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

      // Make sure asset_id matches.
      if ($mediafile[mediamosa_asset_mediafile_db::ASSET_ID] != $asset_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_ASSET_MEDIAFILE_COMBINATION);
      }

      // Check if able to play.
      mediamosa_asset_mediafile_metadata::is_playable($mediafile_id);

      // Check access.
      mediamosa_acl::access_check_on_object(
        mediamosa_acl::ACL_TYPE_MEDIAFILE,
        $asset_id,
        $mediafile_id,
        $app_id,
        $user_id,
        $acl_group_id,
        $acl_domain,
        $acl_realm,
        $is_app_admin
      );
    }

    switch ($response_type) {
      case mediamosa_media::RESPONSE_DOWNLOAD:
        $response = mediamosa_media::do_response_download($app_id, $user_id, $is_app_admin, $mediafile);
        break;

      case mediamosa_media::RESPONSE_METAFILE:
        $autostart = $this->get_param_value(self::AUTOSTART);
        $start = $this->get_param_value(self::START);
        $duration = $this->get_param_value(self::DURATION);
        $response = mediamosa_media::do_response_metafile($app_id, $user_id, $is_app_admin, $mediafile, $autostart, $start, $duration);
        break;

      case mediamosa_media::RESPONSE_OBJECT:
        $width = $this->get_param_value(self::WIDTH);
        $height = $this->get_param_value(self::HEIGHT);
        $autostart = $this->get_param_value(self::AUTOSTART);
        $start = $this->get_param_value(self::START);
        $duration = $this->get_param_value(self::DURATION);
        $response = mediamosa_media::do_response_object($app_id, $user_id, $is_app_admin, $mediafile, $width, $height, $autostart, $start, $duration);
        break;

      case mediamosa_media::RESPONSE_STILL:
        // We use still ID (optional).
        $still_id = $this->get_param_value(self::STILL_ID);

        // Still parameters.
        $width = $this->get_param_value(self::WIDTH);
        $height = $this->get_param_value(self::HEIGHT);
        $size = $this->get_param_value(self::SIZE);
        $range = $this->get_param_value(self::RANGE);
        $format = $this->get_param_value(self::FORMAT);

        // Get a list of filtered stills based on $size, $width, $height, $range
        // and $height.
        $filtered_stills = mediamosa_media::collect_stills($app_id, $asset_id, $mediafile_id, $still_id, $size, $width, $height, $range, $format, $tag);

        // Did any still pass our filters? For output we should at least find one.
        if (empty($filtered_stills)) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_FOUND, array('@asset_id' => $asset_id));
        }

        // Set default still for still response. This still can also be in $filter_stills.
        if ($still_id) {
          $still = mediamosa_asset_mediafile::get($still_id);
        }
        else {
          $still = mediamosa_asset_mediafile_still::find_default($asset_id);

          // No default?
          if (empty($still)) {
            // Has stills, grab the first.
            $still = reset($filtered_stills);
          }

          // Change the still ID to the default.
          $still_id = $still[mediamosa_asset_mediafile_db::ID];
        }

        // Make sure asset_id matches.
        if ($still[mediamosa_asset_mediafile_db::ASSET_ID] != $asset_id) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_ASSET_MEDIAFILE_COMBINATION);
        }

        // Check access on still.
        mediamosa_acl::access_check_on_object(
          mediamosa_acl::ACL_TYPE_MEDIAFILE,
          $asset_id,
          $still_id,
          $app_id,
          $user_id,
          $acl_group_id,
          $acl_domain,
          $acl_realm,
          $is_app_admin
        );

        // Get response.
        $response = mediamosa_media::do_response_still($app_id, $user_id, $is_app_admin, $still, $filtered_stills);
        break;

      case mediamosa_media::RESPONSE_URI:
        $response = mediamosa_media::do_response_uri($app_id, $user_id, $is_app_admin, $mediafile);
        break;

      default:
        throw new mediamosa_exception_program_error('Unsupported response type; @response_type', array('@response_type' => $response_type));
    }

    // If play count is allowed, increase the play counter with one.
    if ($this->get_param_value(self::COUNT_PLAY)) {
      // All ok, now set played + 1
      mediamosa_asset::asset_played($asset_id);
    }

    // Add response.
    $mediamosa->add_item($response);

    // All ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * Removed;
 * URI: /still/$still_id (perm. link)
 * URI: /still/ticket/$ticket_id (normal temp. link).
 * URI: /still/[1stletter $still_id]/$still_id (missing perm. link).
 * URI: /still/$still_id/style/$style (perm. link)
 * URI: /still/ticket/$ticket_id/style/$style (normal temp. link).
 * URI: /still/[1stletter $still_id]/$still_id/style/$style (missing perm. link).
 * URI: /download/$ticket_id/$filename (POST/GET).
 * URI: /media/ticket/$app_id/[1st letter ticket]/$ticket_id[,style].ext (3.1)
 * URI: /media/$app_id/[1st letter ticket]/$mediafile_id[,style].ext (3.1)
 *
 * 3.5 uri's;
 * URI:
 * View (play response):
 * /media/$app_id/[1st letter ticket]/$mediafile_id/filename[,style].ext
 * /media/ticket/$app_id/[1st letter ticket]/$ticket_id/filename[,style].ext
 *
 * Still (still response)
 * /media/$app_id/[1st letter ticket]/$mediafile_id-still/filename[,style].ext
 * /media/ticket/still/$app_id/[1st letter ticket]/$ticket_id-still/filename[,style].ext
 *
 * Not supported, is supported by /download REST call;
 * download (download response)
 * /media/download/$app_id/[1st letter ticket]/$mediafile_id-download/filename[,style].ext
 * /media/ticket/download/$app_id/[1st letter ticket]/$ticket_id-download/filename[,style].ext
 *
 * Method: GET / POST
 */
class mediamosa_rest_call_media extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const APP_ID = 'app_id';
  const MEDIAFILE_ID_PREFIX = 'mediafile_id_prefix';
  const MEDIAFILE_ID = 'mediafile_id';
  const TICKET_ID_PREFIX = 'ticket_id_prefix';
  const TICKET_ID = 'ticket_id';
  const FILENAME = 'filename';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::APP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The app ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID_PREFIX => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID_PREFIX,
          self::VAR_DESCRIPTION => 'One letter index of the mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'The filename.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // All we need.
    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    // Get the input values.
    $app_id = $this->get_param_value(self::APP_ID);
    $filename = $this->get_param_value(self::FILENAME);

    $mediafile_id = NULL;
    if ($this->isset_given_param(self::MEDIAFILE_ID)) {
      $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    }

    $ticket_id = NULL;
    if ($this->isset_given_param(self::TICKET_ID)) {
      $ticket_id = $this->get_param_value(self::TICKET_ID);
    }

    // Either one required.
    if (!$mediafile_id && !$ticket_id) {
      throw new mediamosa_exception_error_400(); // Bad request.
    }

    if ($ticket_id) {
      // Before 3.1 we always served the mediafile here. Now we don't because
      // the REST call should not have been triggered in the first place with
      // tickets (Your webserver is handeling this). Need to warn admin that his
      // configuration does not include the correct configuration for his
      // webserver.

      // In any case, if the ticket exists and we where triggered, then the
      // webserver setup is not correct and we should log this problem.
      $file = mediamosa_storage::get_uri_media_ticket_file($app_id, $ticket_id, $filename);
      if (mediamosa_io::file_exists($file)) {
        mediamosa_watchdog::log('The /media REST call was triggered while this was not expected. Check your MediaMosa status page for more information.', array(), WATCHDOG_CRITICAL);
        throw new mediamosa_exception_error_501();
      }
    }

    // Handle request.
    try {
      // Tickets are temporary.
      if ($ticket_id) {
        mediamosa_media::handle_media_request_ticket($app_id, $ticket_id, $filename);
      }
      else {
        // Non-tickets are permanent.
        mediamosa_media::handle_media_request_permanent($app_id, $mediafile_id, $filename);
      }
    }
    catch (mediamosa_exception_error_asset_not_found $e) {
      throw new mediamosa_exception_error_404();
    }
    catch (mediamosa_exception_error_mediafile_not_found $e) {
      throw new mediamosa_exception_error_404();
    }
    catch (mediamosa_exception_error_access_denied $e) {
      throw new mediamosa_exception_error_403();
    }
    catch (mediamosa_exception_error_is_inappropriate $e) {
      throw new mediamosa_exception_error_403();
    }
    catch (mediamosa_exception_error_invalid_ticket $e) {
      throw new mediamosa_exception_error_404();
    }
    catch (Exception $e) {
      throw new mediamosa_exception_error_500();
    }
  }
}

/**
 * 3.5 media uri's;
 * URI:
 * View (play response):
 * /media/$app_id/[1st letter ticket]/$mediafile_id/filename[,style].ext
 * /media/ticket/$app_id/[1st letter ticket]/$ticket_id/filename[,style].ext
 *
 * still (still response)
 * /media/$app_id/[1st letter ticket]/$mediafile_id-still/filename[,style].ext
 * /media/ticket/still/$app_id/[1st letter ticket]/$ticket_id-still/filename[,style].ext
 *
 * Method: GET / POST
 *
 * @see mediamosa_rest_call_media
 */
class mediamosa_rest_call_media_ticket extends mediamosa_rest_call_media {
  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::APP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The app ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::TICKET_ID_PREFIX => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID_PREFIX,
          self::VAR_DESCRIPTION => 'One letter index of the mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::TICKET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'The filename.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // All we need.
    return $var_setup;
  }
}

/**
 * URI: /download
 * Method: GET / POST
 *
 * Serve the mediafile for download.
 */
class mediamosa_rest_call_media_download extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  // Rest vars;
  const APP_ID = 'app_id';
  const MEDIAFILE_ID_PREFIX = 'mediafile_id_prefix';
  const MEDIAFILE_ID = 'mediafile_id';
  const TICKET_ID_PREFIX = 'ticket_id_prefix';
  const TICKET_ID = 'ticket_id';
  const FILENAME = 'filename';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::APP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The app ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID_PREFIX => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID_PREFIX,
          self::VAR_DESCRIPTION => 'One letter index of the mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ticket ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'The filename.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // All we need.
    return $var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the input values.
    $app_id = $this->get_param_value(self::APP_ID);
    $filename = $this->get_param_value(self::FILENAME);

    $mediafile_id = NULL;
    if ($this->isset_given_param(self::MEDIAFILE_ID)) {
      $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    }

    $ticket_id = NULL;
    if ($this->isset_given_param(self::TICKET_ID)) {
      $ticket_id = $this->get_param_value(self::TICKET_ID);
    }

    // Either one required.
    if (!$mediafile_id && !$ticket_id) {
      throw new mediamosa_exception_error_400(); // Bad request.
    }

    if (isset($ticket_id)) {
      // Get filename.
      $file = mediamosa_storage::get_realpath_media_ticket_file($app_id, $ticket_id, $filename, mediamosa_media_ticket::TICKET_TYPE_DOWNLOAD);
    }
    else {
      $file = mediamosa_storage::get_realpath_mediafile($mediafile_id);
    }

    // Make sure the file exists we want to download.
    if (!mediamosa_io::file_exists($file) || !mediamosa_io::is_file($file)) {
      throw new mediamosa_exception_error_404(); // File not found.
    }

    // Set headers.
    header('Pragma: public');
    header('Expires: 0');
    header('Cache-Control: must-revalidate, post-check=0, pre-check=0');
    header('Cache-Control: private, false');
    header('Content-Description: Download');
    header('Content-Type: application/octet-stream');
    header('Content-Length: ' . filesize($file));

    // Force a save dialog.
    header('Content-Disposition: attachment; filename="' . mime_header_encode($filename) . '"');
    header('Content-Transfer-Encoding: binary');

    $handle = fopen($file, 'rb');

    if (!$handle) {
      throw new mediamosa_exception(0, 'Unable to open file!');
    }

    // Make sure we send the header now.
    ob_clean();
    flush();

    // Max download time is 1 day.
    drupal_set_time_limit(3600);

    // Until end of file, get the file and output.
    while (!feof($handle)) {
      $buffer = fread($handle, mediamosa_settings::DOWNLOAD_CHUNK_SIZE);

      print $buffer;

      ob_flush();
      flush();
    }

    // Close the file.
    fclose($handle);

    // Set the result, reponse object will know what to do.
    $mediamosa->set_result(mediamosa_response::SUCCESS, 200, $file);
  }
}

/**
 * 3.5 uri's;
 * /download/$app_id/[1st letter ticket]/$mediafile_id-download/filename[,style].ext
 * /download/ticket/$app_id/[1st letter ticket]/$ticket_id-download/filename[,style].ext
 *
 * Method: GET / POST
 *
 * @see mediamosa_rest_call_media
 */
class mediamosa_rest_call_media_download_ticket extends mediamosa_rest_call_media_download {
  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::APP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_APP_ID,
          self::VAR_DESCRIPTION => 'The app ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::TICKET_ID_PREFIX => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID_PREFIX,
          self::VAR_DESCRIPTION => 'One letter index of the ticket ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::TICKET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ticket ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'The filename.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // All we need.
    return $var_setup;
  }
}
