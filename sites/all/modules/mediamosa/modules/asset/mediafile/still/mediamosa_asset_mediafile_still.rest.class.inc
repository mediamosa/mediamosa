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
  * Still REST calls.
  */

/**
 * URI: /asset/$asset_id/still/delete
 * Method: POST
 *
 * 1.x media_management_asset_delete_stills
 */
class mediamosa_rest_call_delete_stills extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const STILL_ID = 'still_id';

  // ------------------------------------------------------------------ Public functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'Delete still only from this asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Owner of the stills.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The delete stills from this mediafile ID.',
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'Delete this specific still.',
        ),
      ),
    );

    return self::get_var_setup_default($a_var_setup);;
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Webservice enabled?
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $still_id = $this->get_param_value(self::STILL_ID);

    $mediafile_id = NULL;
    if ($this->isset_given_param(self::MEDIAFILE_ID)) {
      $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    }
    if (!$mediafile_id && $this->isset_given_param(self::STILL_ID)) {
      $mediafile = mediamosa_asset_mediafile::get($still_id);
      $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];
    }

    // Asset must exist.
    mediamosa_db::db_must_exists(
      mediamosa_asset_db::TABLE_NAME,
      array(
        mediamosa_asset_db::ID => $asset_id,
      )
    );

    if ($mediafile_id) {
      // Mediafiles must exist.
      mediamosa_db::db_must_exists(
        mediamosa_asset_mediafile_db::TABLE_NAME,
        array(
          mediamosa_asset_mediafile_db::ASSET_ID => $asset_id,
          mediamosa_asset_mediafile_db::ID => $mediafile_id,
        ),
        mediamosa_error::ERRORCODE_STILL_NOT_FOUND,
        array('@asset_id' => $asset_id)
      );
    }

    $a_row = mediamosa_db::db_query(
      'SELECT #app_id, #owner_id FROM {#mediamosa_asset} WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#app_id' => mediamosa_asset_db::APP_ID,
        '#owner_id' => mediamosa_asset_db::OWNER_ID,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    )->fetchAssoc();

    // Does the user has access?
    mediamosa_acl::owner_check(
      $app_id,
      $this->get_param_value(self::USER_ID),
      $a_row[mediamosa_asset_db::APP_ID],
      $a_row[mediamosa_asset_db::OWNER_ID]
    );

    // Default all ok.
    $o_mediamosa->set_result_okay();

    // Remove still.
    mediamosa_asset_mediafile_still::delete($asset_id, $mediafile_id, $still_id);
  }
}

/**
 * URI: /asset/$asset_id/still/default
 * Method: POST
 *
 * 1.x mediamosa_rest_call_set_still_default
 */
class mediamosa_rest_call_set_still_default extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const STILL_ID = 'still_id';

  // ------------------------------------------------------------------ Public functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'ID of asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Owner of the asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile of the asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STILL_ID,
          self::VAR_DESCRIPTION => 'Default this specific still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    return self::get_var_setup_default($a_var_setup);;
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Webservice enabled?
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $still_id = $this->get_param_value(self::STILL_ID);

    $mediafile_id = NULL;
    if ($this->isset_given_param(self::MEDIAFILE_ID)) {
      $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    }
    if (!$mediafile_id && $this->isset_given_param(self::STILL_ID)) {
      $mediafile = mediamosa_asset_mediafile::get($still_id);
      $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];
    }

    $user_id = $this->get_param_value(self::USER_ID);
    // Set default.
    mediamosa_asset_mediafile_still::set_still_default($app_id, $user_id, $asset_id, $mediafile_id, $still_id);

    // Default all ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /still/$still_id/watermark
 * Method: POST
 */
class mediamosa_rest_call_set_still_watermark extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const STILL_ID = 'still_id';
  const USER_ID = 'user_id';

  // Watermark.
  const WATERMARK_ID = 'watermark_id';
  const WATERMARK_DST_X = 'watermark_dst_x';
  const WATERMARK_DST_Y = 'watermark_dst_y';
  const WATERMARK_PCT = 'watermark_pct';
  const WATERMARK_V_ALIGN = 'watermark_v_align';
  const WATERMARK_H_ALIGN = 'watermark_h_align';

  const TAG = 'tag';
  const ORDER = 'order';
  const DEFAULT_ = 'default';

  // ------------------------------------------------------------------ Public functions.
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The still ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        // Watermark.
        self::WATERMARK_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The watermark ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
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
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Mediafile tag to set. An empty tag will clear possible set mediafile tag.',
        ),
      ),
    );

    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($app_ids);
    $user_id = $this->get_param_value(self::USER_ID);

    $still_id = $this->get_param_value(self::STILL_ID);
    $watermark_id = $this->get_param_value(self::WATERMARK_ID);
    $tag = $this->get_param_value(self::TAG);
    $order = $this->get_param_value(self::ORDER);
    $default = $this->get_param_value(self::DEFAULT_);

    // Get the still using.
    $still = mediamosa_asset_mediafile::must_exists($still_id, $app_id);

    // Check the watermark id.
    $watermark = mediamosa_asset_mediafile::must_exists($watermark_id, $app_id);

    $asset_id = $still[mediamosa_asset_mediafile_db::ASSET_ID];
    $owner_id = $still[mediamosa_asset_mediafile_db::OWNER_ID];
    $group_id = $still[mediamosa_asset_mediafile_db::GROUP_ID];

    // TODO: Needs later decision.
    if (FALSE) {
      // The new, watermarked still is inherit the basic values.
      $tag = $still[mediamosa_asset_mediafile_db::TAG];
      $order = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($still_id, mediamosa_asset_mediafile_metadata::STILL_ORDER);
      $default = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($still_id, mediamosa_asset_mediafile_metadata::STILL_DEFAULT);
    }

    // Check user quota.
    mediamosa_user::must_have_user_quota($app_id, $owner_id, $group_id);

    // Must be on.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_UPLOAD, array($app_id));

    // Does the user has access?
    mediamosa_acl::owner_check(
      $app_id,
      $user_id,
      $still[mediamosa_asset_mediafile_db::APP_ID],
      $still[mediamosa_asset_mediafile_db::OWNER_ID]
    );

    $still_parameters = array(
      // Watermark.
      self::WATERMARK_ID => $watermark_id,
      self::WATERMARK_DST_X => $this->get_param_value(self::WATERMARK_DST_X),
      self::WATERMARK_DST_Y => $this->get_param_value(self::WATERMARK_DST_Y),
      self::WATERMARK_PCT => $this->get_param_value(self::WATERMARK_PCT),
      self::WATERMARK_V_ALIGN => $this->get_param_value(self::WATERMARK_V_ALIGN),
      self::WATERMARK_H_ALIGN => $this->get_param_value(self::WATERMARK_H_ALIGN),
    );

    $create_new_still = TRUE;
    $still_id_new = mediamosa_asset_mediafile_still::create_watermark($still_id, $app_id, $still_parameters, $create_new_still);

    if (!$still_id_new) {
      // TODO: Better error code?
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STILL_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    $still_parameters = array(
      // TODO: New type?
      'still_type' => mediamosa_asset_mediafile_metadata::STILL_TYPE_PICTURE,
    );

    // Create still.
    mediamosa_asset_mediafile_still::create(
      $asset_id,
      $still_id_new,
      $app_id,
      $owner_id,
      $group_id,
      $order,
      $default,
      $still_parameters,
      NULL,
      $still[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE],
      $tag
    );

    // Add still_id to output.
    $mediamosa->add_item(array('still_id' => $still_id_new));

    // Set ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/still/order
 * Method: POST
 *
 * 1.x media_management_set_still_order
 */
class mediamosa_rest_call_set_still_order extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const STILL_ID = 'still_id';
  const ORDER = 'order';

  // ------------------------------------------------------------------ Public functions.
  public function get_var_setup() {
    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'ID of asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'Owner of the still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
        ),
        self::STILL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'Setting the order for this specific still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ORDER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The order for this still.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Webservice enabled?
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $still_id = $this->get_param_value(self::STILL_ID);

    $mediafile_id = NULL;
    if ($this->isset_given_param(self::MEDIAFILE_ID)) {
      $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    }
    if (!$mediafile_id && $this->isset_given_param(self::STILL_ID)) {
      $mediafile = mediamosa_asset_mediafile::get($still_id);
      $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE];
    }

    $user_id = $this->get_param_value(self::USER_ID);
    $order = $this->get_param_value(self::ORDER);

    // Set Still order.
    mediamosa_asset_mediafile_still::set_still_order($app_id, $user_id, $asset_id, $mediafile_id, $still_id, $order);

    // Default all ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /mediafile/$mediafile_id/still
 * Method: GET
 *
 * 1.x media_management_get_still_for_mediafile
 */
class mediamosa_rest_call_get_still_for_mediafile extends mediamosa_rest_call_asset_mediafile_play {
  // ------------------------------------------------------------------ Consts.
  // ------------------------------------------------------------------ Public functions.
  public function get_var_setup() {
    $a_var_setup = parent::get_var_setup();

    // Make not required, so we pass the check.
    $a_var_setup[self::VARS][self::ASSET_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;

    return $a_var_setup;
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Set the asset_id value.
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);

    // Now get the mediafile.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);

    // Set the asset_id.
    $this->set_param_value(mediamosa_rest_call_asset_mediafile_play::ASSET_ID, $mediafile[mediamosa_asset_mediafile_db::ASSET_ID]);

    // Now fix response on still generation.
    $this->set_param_value(mediamosa_rest_call_asset_mediafile_play::RESPONSE, mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL);

    // Now do play call.
    parent::do_call();
  }
}

/**
 * URI: /asset/$asset_id/still
 * Method: GET
 *
 * 1.x media_management_get_still
 */
class mediamosa_rest_call_get_still extends mediamosa_rest_call_asset_mediafile_play {
  // ------------------------------------------------------------------ Consts.
  // ------------------------------------------------------------------ Public functions.
  public function get_var_setup() {
    $var_setup = parent::get_var_setup();

    // Not required now.
    $var_setup[self::VARS][self::MEDIAFILE_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;

    $var_setup[self::VARS][self::RESPONSE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      self::VAR_DESCRIPTION => 'The still response type.',
      self::VAR_ALLOWED_VALUES => array(
        mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL,
      ),
      self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_play_proxy::RESPONSE_STILL
    );

    return $var_setup;
  }
}
