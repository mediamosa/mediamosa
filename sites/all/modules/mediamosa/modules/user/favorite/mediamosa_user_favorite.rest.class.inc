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
 * Favorite REST calls for user.
 */

/**
 * URI: user/$fav_user_id/favorites/create
 * user/$fav_user_id/favorites/add (deprecated)
 *
 * Method: POST
 *
 * 1.x: user_management_favorites_add
 *
 */
class mediamosa_rest_call_user_favorite_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'fav_user_id';
  const FAV_TYPE = 'fav_type';
  const FAV_ID = 'fav_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User name.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::FAV_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'Favorite type (asset or collection)',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_ALLOWED_VALUES => array(mediamosa_user_favorite_db::FAV_TYPE_ASSET, mediamosa_user_favorite_db::FAV_TYPE_COLLECTION),
        ),
        self::FAV_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'Favorite ID (asset or collection ID)',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $user_id = $this->get_param_value(self::USER_ID);
    $fav_type = $this->get_param_value(self::FAV_TYPE);
    $fav_id = $this->get_param_value(self::FAV_ID);

    // Create.
    mediamosa_user_favorite::create($app_id, $user_id, $fav_type, $fav_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: user/$fav_user_id/favorites/delete
 * user/$fav_user_id/favorites/detete [GET] (deprecated)
 *
 * Method: POST
 *
 * 1.x: user_management_favorites_delete
 *
 */
class mediamosa_rest_call_user_favorite_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'fav_user_id';
  const FAV_TYPE = 'fav_type';
  const FAV_ID = 'fav_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'User name.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::FAV_TYPE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'Favorite link type.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_ALLOWED_VALUES => array(mediamosa_user_favorite_db::FAV_TYPE_ASSET, mediamosa_user_favorite_db::FAV_TYPE_COLLECTION),
        ),
        self::FAV_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'Favorite ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $user_id = $this->get_param_value(self::USER_ID);
    $fav_type = $this->get_param_value(self::FAV_TYPE);
    $fav_id = $this->get_param_value(self::FAV_ID);

    // Create.
    mediamosa_user_favorite::delete($app_id, $user_id, $fav_type, $fav_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: user/$fav_user_id/favorites
 * (do not rename fav_user_id to user_id(!))
 *
 * Method: GET
 *
 * 1.x: user_management_favorites_list
 *
 */
class mediamosa_rest_call_user_favorite_search {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const APP_ID = 'app_id'; // because its not a normal REST call.
  const FAV_TYPE = 'fav_type';

  /**
   * get_object function is special. It can be used to supply the REST interface
   * with the correct class to create.
   */
  static public function get_object_rest_call($a_rest_call, $default_param_values = array()) {
    // Get the value.
    $fav_type = (isset($default_param_values[self::FAV_TYPE]) ? $default_param_values[self::FAV_TYPE] : mediamosa_rest_call::get_param_value_global(self::FAV_TYPE));

    // Based on type, we return the class.
    switch ($fav_type) {
      default:
      case mediamosa_user_favorite_db::FAV_TYPE_ASSET:
        return new mediamosa_rest_call_user_favorite_search_asset($a_rest_call);
      case mediamosa_user_favorite_db::FAV_TYPE_COLLECTION:
        return new mediamosa_rest_call_user_favorite_search_collection($a_rest_call);
    }

    // Invalid value.
    throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => self::FAV_TYPE, '@type' => mediamosa_sdk::TYPE_ALPHA_NUM));
  }
}

/**
 * Asset search with fav_user_id.
 */
class mediamosa_rest_call_user_favorite_search_asset extends mediamosa_rest_call_asset_search {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const FAV_TYPE = 'fav_type';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = parent::get_var_setup();

    // Add type.
    $a_var_setup[self::VARS][self::FAV_TYPE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
      self::VAR_DESCRIPTION => 'Favorite link type.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_ALLOWED_VALUES => array(mediamosa_user_favorite_db::FAV_TYPE_ASSET, mediamosa_user_favorite_db::FAV_TYPE_COLLECTION),
    );

    // fav_user_id is now required.
    $a_var_setup[self::VARS][self::FAV_USER_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;

    return $a_var_setup;
  }
}

/**
 * Collection search with fav_user_id.
 */
class mediamosa_rest_call_user_favorite_search_collection extends mediamosa_rest_call_collection_search {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const FAV_TYPE = 'fav_type';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = parent::get_var_setup();

    // Add type.
    $a_var_setup[self::VARS][self::FAV_TYPE] = array(
      self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
      self::VAR_DESCRIPTION => 'Favorite link type.',
      self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
      self::VAR_ALLOWED_VALUES => array(mediamosa_user_favorite_db::FAV_TYPE_ASSET, mediamosa_user_favorite_db::FAV_TYPE_COLLECTION),
    );

    // fav_user_id is now required.
    $a_var_setup[self::VARS][self::FAV_USER_ID][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;

    return $a_var_setup;
  }
}
