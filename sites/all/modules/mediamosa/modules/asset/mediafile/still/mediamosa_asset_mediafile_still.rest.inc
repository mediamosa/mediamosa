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

    $a_app_ids = $this->get_param_value(self::APP_ID);
    $app_id = reset($a_app_ids);

    // Webservice enabled?
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

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
    mediamosa_aut::owner_check(
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

    $a_app_ids = $this->get_param_value(self::APP_ID);
    $app_id = reset($a_app_ids);

    // Webservice enabled?
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

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

    return self::get_var_setup_default($a_var_setup);;
  }

  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value(self::APP_ID);
    $app_id = reset($a_app_ids);

    // Webservice enabled?
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

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

    $a_app_ids = $this->get_param_value(self::APP_ID);
    $app_id = reset($a_app_ids);

    // Set the asset_id value.
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);

    // Now get the mediafile.
    $a_mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id, $app_id);

    // Set the asset_id.
    $this->set_param_value(mediamosa_rest_call_asset_mediafile_play::ASSET_ID, $a_mediafile[mediamosa_asset_mediafile_db::ASSET_ID]);

    // Now fix response on still generation.
    $this->set_param_value(mediamosa_rest_call_asset_mediafile_play::RESPONSE, mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL);

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
    $a_var_setup = parent::get_var_setup();

    // Not required now.
    $a_var_setup[self::VARS][self::MEDIAFILE_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_NO;

    $a_var_setup[self::VARS][self::RESPONSE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
      self::VAR_DESCRIPTION => 'The still response type.',
      self::VAR_ALLOWED_VALUES => array(
        mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL,
      ),
      self::VAR_DEFAULT_VALUE => mediamosa_asset_mediafile_play_proxy::RESPONSE_TYPE_STILL
    );

    return $a_var_setup;
  }
}
