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
 * Play Proxy functions
 */

/**
 * URI: /asset/$asset_id/play
 *      /video/$asset_id/play (deprecated)
 * Method: GET
 */
class mediamosa_rest_call_asset_mediafile_play extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
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

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
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
          self::VAR_DESCRIPTION => 'The mediafile ID, is not required when profile_id is used or when response type is still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,// Check it later.
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
            mediamosa_asset_mediafile_play_proxy::RESPONSE_DOWNLOAD,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_METAFILE,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_OBJECT,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL,
            mediamosa_asset_mediafile_play_proxy::RESPONSE_URI
          ),
          self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_play_proxy::RESPONSE_URI
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
          self::VAR_RANGE_END => 86399999, // (24 uur in msec)-1 msec
        ),
        self::DURATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'Playing length of the video clip in milliseconds. Must be less than 24 hours (86,400,000).',
          self::VAR_RANGE_START => 0,
          self::VAR_RANGE_END => 86399999, // (24 uur in msec)-1 msec
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
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // Override for some cases.
  protected function process_rest_args(array $a_var_setup) {
    $response_type = $this->get_param_value_global(self::RESPONSE);
    $profile_id = $this->get_param_value_global(self::PROFILE_ID);

    // Is reponse is set or response type is still then mediafile_id is not required.
    if (isset($profile_id) || $response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL) {
      $a_var_setup[self::VARS][self::MEDIAFILE_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;
    }

    return parent::process_rest_args($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Fix renames.
    if ($this->isset_given_param(self::DOMAIN)) {
      $this->set_param_value(self::ACL_DOMAIN, $this->get_param_value(self::DOMAIN));
    }
    if ($this->isset_given_param(self::REALM)) {
      $this->set_param_value(self::ACL_REALM, $this->get_param_value(self::REALM));
    }

    // FIXME: hack, fixed again by checking if response was not given.
    if ($this->is_internal() && !$this->isset_given_param(self::RESPONSE)) {
      $this->set_param_value(self::RESPONSE, mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL);
    }

    $user_id = $this->get_param_value(self::USER_ID);
    $response_type = $this->get_param_value(self::RESPONSE);
    $profile_id = $this->get_param_value(self::PROFILE_ID);
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $original_mediafile_id = $this->get_param_value(self::ORIGINAL_MEDIAFILE_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $acl_group_id = $this->get_param_value(self::ACL_GROUP_ID);
    $acl_domain = $this->get_param_value(self::ACL_DOMAIN);
    $acl_realm = $this->get_param_value(self::ACL_REALM);
    $still_id = $this->get_param_value(self::STILL_ID);
    $tag = $this->get_param_value(self::TAG);

    // Group id is deprecated.
    if (!$acl_group_id && $group_id) {
      $acl_group_id = $group_id;
    }

    if ($this->isset_given_param(self::PROFILE_ID)) {
      $mediafile_id = mediamosa_asset_mediafile_play_proxy::get_mediafile_id_on_profile($asset_id, $profile_id, $original_mediafile_id);

      if (!$mediafile_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_MEDIAFILE_FOUND_FOR_PROFILE_ID);
      }
    }

    if ($response_type != mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL && !$mediafile_id && $this->isset_given_param(self::TAG)) {
      $mediafile_id = mediamosa_asset_mediafile_play_proxy::get_mediafile_id_on_tag($asset_id, $tag);

      if (!$mediafile_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_MEDIAFILE_FOUND_FOR_TAG);
      }
    }

    // Still response does not need mediafile_id.
    if ($response_type != mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL && !$mediafile_id) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_REQUIRED_PARAMETER, array('@param' => self::MEDIAFILE_ID, '@type' => mediamosa_sdk::TYPE_MEDIAFILE_ID));
    }

    $mediafile_ext = array();

    // Get the asset.
    $asset = mediamosa_asset::must_exists($asset_id);

    if ($mediafile_id) {
      // Check play restrictions
      if ($asset['play_restriction_start'] != '' || $asset['play_restriction_end'] != '') {
        $result = mediamosa_asset_mediafile_play_proxy::check_time_restrictions(
          strtotime($asset['play_restriction_start']),
          strtotime($asset['play_restriction_end'])
        );
      }

      // Get mediafile.
      $mediafile_ext = mediamosa_asset_mediafile::must_exists($mediafile_id);

      // Make sure asset_id matches.
      if ($mediafile_ext[mediamosa_asset_mediafile_db::ASSET_ID] != $asset_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_ASSET_MEDIAFILE_COMBINATION);
      }

      // Get the metadata.
      $mediafile_ext['metadata'] = mediamosa_asset_mediafile_metadata::get_with_mediafileid($mediafile_id);

      // Test if access on unappropriate flag.
      mediamosa_asset::is_unappropriate($asset_id, $app_ids, $user_id, $is_app_admin, TRUE);

      // Check if allowed to play.
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
    else {
      $mediafile_ext['asset_id'] = $asset_id;
    }

    // Create ticket ID.
    $ticket = mediamosa_db::uuid($app_id);

    if (empty($mediafile_ext[mediamosa_asset_mediafile_db::URI]) && $response_type != mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL) {
      // FIXME: Hmmm not still response type, but we do supply still_id?
      $ticket = mediamosa_ticket::process_play_ticket($ticket, $mediafile_ext, $response_type, $app_id, $user_id, $still_id);
    }

    // If response type is still, we get all information of all stills with all details.
    $stills = array();
    if ($response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL) {
      // Test if access on unappropriate flag.
      mediamosa_asset::is_unappropriate($asset_id, $app_ids, $user_id, $is_app_admin, TRUE);

      // Set default still.
      if (empty($mediafile_ext[mediamosa_asset_mediafile_db::URI])) {
        $still_id_default = ($still_id ? $still_id : mediamosa_asset_mediafile_still::find_default($asset_id));

        if (!empty($still_id_default)) {
          // Generate ticket or perm link?
          if (mediamosa_asset_mediafile_still::is_still_permanent($still_id_default, $is_app_admin)) {
            $ticket =  $still_id_default . '.' . mediamosa_asset_mediafile::get_file_extension($still_id_default);
          }
          else {
            $ticket = mediamosa_settings::SAN_NAS_TICKET_FOLDER . '/' . mediamosa_ticket::process_play_ticket($ticket, $mediafile_ext, $response_type, $app_id, $user_id, $still_id_default);
          }
        }
      }

      $width = $this->get_param_value(self::WIDTH);
      $height = $this->get_param_value(self::HEIGHT);
      $size = $this->get_param_value(self::SIZE);
      $format = $this->get_param_value(self::FORMAT);
      $range = $this->get_param_value(self::RANGE);

      if (!empty($size)) {
        list($size_width, $size_height) = explode('x', $size, 2);
      }

      if (!isset($width) && !isset($height) && isset($size_width) && $size_width >= 0 && is_numeric($size_width) && isset($size_height) && is_numeric($size_height) && $size_height >= 0) {
        $width = $size_width;
        $height = $size_height;
      }

      $order = array();
      if (isset($range)) {
        if (is_numeric($range)) {
          $order[] = $range;
        }
        else {
          $ranges = explode(',', $range);
          foreach ($ranges as $range_2) {
            if (is_numeric($range_2)) {
              $order[] = $range_2;
            }
            else {
              $pos = strpos($range_2, '-', 1);
              if ($pos !== FALSE) {
                // Think to the negative numbers, so change the separator
                $range_2[$pos] = '!';
                $range_2 = explode('!', $range_2);
                if (is_array($range_2) && isset($range_2[0]) && is_numeric($range_2[0]) && isset($range_2[1]) && is_numeric($range_2[1]) && !isset($range_2[2]) && $range_2[0] <= $range_2[1]) {
                  for ($i = $range_2[0]; $i <= $range_2[1]; $i++) {
                    $order[] = $i;
                  }
                }
              }
            }
          }
        }
      }

      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'm');
      $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mm', strtr('mm.#mediafile_id = m.#mediafile_id', array('#mediafile_id' => mediamosa_asset_mediafile_db::ID)));
      $query->join(
        mediamosa_asset_mediafile_db::TABLE_NAME,
        'v',
        strtr(
          "v.#mediafile_id = m.#mediafile_id_source AND v.#is_still = '#is_still_false'",
          array(
            '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
            '#mediafile_id_source' => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
            '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
            '#is_still_false' => mediamosa_asset_mediafile_db::IS_STILL_FALSE,
          )
        )
      );
      $query->addField('m', mediamosa_asset_mediafile_db::ID, 'still_id');
      $query->addField('v', mediamosa_asset_mediafile_db::ID, 'mediafile_id');
      $query->addField('m', mediamosa_asset_mediafile_db::ASSET_ID, 'asset_id');
      $query->fields('m',
        array(
          mediamosa_asset_mediafile_db::APP_ID,
          mediamosa_asset_mediafile_db::OWNER_ID,
          mediamosa_asset_mediafile_db::FILENAME,
          mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
          mediamosa_asset_mediafile_db::TAG,
        )
      );
      $query->condition('m.' . mediamosa_asset_mediafile_db::ASSET_ID, $asset_id);
      $query->condition('m.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE);
      $query->condition('m.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);
      if (!empty($still_id)) {
        $query->condition('m.' . mediamosa_asset_mediafile_db::ID, $still_id);
      }
      if (isset($mediafile_id)) {
        $query->condition('v.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
      }
      if (isset($tag)) {
        $query->condition('m.' . mediamosa_asset_mediafile_db::TAG, $tag);
      }
      $query->condition('mm.' . mediamosa_asset_mediafile_metadata_db::PROP_ID, mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::STILL_ORDER));
      $query->orderBy('m.' . mediamosa_asset_mediafile_db::ASSET_ID, 'ASC');
      $query->orderBy('mm.' . mediamosa_asset_mediafile_metadata_db::VAL_INT, 'ASC');

      if (!empty($still_id)) {
        $result = $query->range(0, 1)->execute()->fetchAssoc();
        if ($this->metadata_values($result, $width, $height, $format, $order)) {
          $result['ticket'] = $ticket;
          $stills[] = $result;
        }
      }
      else {
        $result = $query->execute();

        foreach ($result as $t_stills) {
          // Enrich the t_stills with the still parameters.
          if ($this->metadata_values($t_stills, $width, $height, $format, $order)) {

            // Get file ext.
            $file_ext = mediamosa_asset_mediafile::get_file_extension($t_stills['still_id']);

            // Generate ticket_id.
            $ticket_still = mediamosa_db::uuid($app_id);

            // make a play or download symlink.
            if (empty($mediafile_ext[mediamosa_asset_mediafile_db::URI])) {
              // Generate ticket or perm link?
              if (mediamosa_asset_mediafile_still::is_still_permanent($t_stills['still_id'], $is_app_admin)) {
                $ticket_still = $t_stills['still_id'] . '.' . $file_ext;
              }
              else {
                $ticket_still = mediamosa_settings::SAN_NAS_TICKET_FOLDER . '/' . mediamosa_ticket::process_play_ticket($ticket_still, $mediafile_ext, $response_type, $app_id, $user_id, $t_stills['still_id']);
              }
            }
            else {
              $ticket_still .= '.' . $file_ext;
            }

            $t_stills['ticket'] = $ticket_still;
            $stills[] = $t_stills;
          }
        }
      }
    }

    // If the request is for a still but we have no stills, then we fail.
    if (($response_type == mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL) && (empty($stills))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    // Collect the metadata of the asset.
    $asset_metadata = mediamosa_asset_metadata::metadata_get($asset_id);

    // Create the response.
    $response = mediamosa_asset_mediafile_play_proxy::create_response(
      $response_type,
      $app_id,
      $ticket,
      $asset_id,
      $this->get_param_value(self::AUTOSTART),
      $this->get_param_value(self::WIDTH),
      $this->get_param_value(self::HEIGHT),
      $this->get_param_value(self::START),
      $this->get_param_value(self::DURATION),
      $this->get_param_value(self::IS_APP_ADMIN),
      $asset_metadata,
      $mediafile_ext,
      $stills
    );

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

  /**
   * Metadata values.
   *
   * @param &result
   *
   * @return is_skip
   */
  private function metadata_values(&$result, $width, $height, $format, $order) {

    // Get the technical metadata of the still.
    $values = mediamosa_asset_mediafile_metadata::get_all_mediafile_metadata($result['still_id']);
    $values_to_still = array();

    $fields = array(
      mediamosa_asset_mediafile_metadata::WIDTH,
      mediamosa_asset_mediafile_metadata::HEIGHT,
      mediamosa_asset_mediafile_metadata::FILESIZE,
      mediamosa_asset_mediafile_metadata::MIME_TYPE,
      mediamosa_asset_mediafile_metadata::STILL_TIME_CODE,
      mediamosa_asset_mediafile_metadata::STILL_ORDER,
      mediamosa_asset_mediafile_metadata::STILL_FORMAT,
      mediamosa_asset_mediafile_metadata::STILL_TYPE,
      mediamosa_asset_mediafile_metadata::STILL_DEFAULT,
    );

    // Collect the values we need.
    foreach ($fields as $field) {
      $values_to_still[$field] = (empty($values[$field]) ? NULL : $values[$field]);
    }

    $result = array_merge($result, $values_to_still);
    if (isset($width) && $width != $values_to_still[mediamosa_asset_mediafile_metadata::WIDTH]) {
      return FALSE;
    }
    if (isset($height) && $height != $values_to_still[mediamosa_asset_mediafile_metadata::HEIGHT]) {
      return FALSE;
    }
    if (isset($format) && $format != $values_to_still[mediamosa_asset_mediafile_metadata::STILL_FORMAT]) {
      return FALSE;
    }
    if (count($order) && !in_array($values_to_still[mediamosa_asset_mediafile_metadata::STILL_ORDER], $order)) {
      return FALSE;
    }

    return TRUE;
  }
}
