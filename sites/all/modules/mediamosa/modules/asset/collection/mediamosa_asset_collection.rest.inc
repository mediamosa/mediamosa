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
  * Asset <-> Collection REST calls.
  */

/**
 * URI: /collection/$coll_id/asset_relation
 * Method: POST
 *
 * Create a link between collection and asset.
 *
 * 1.x media_management_create_collection_relation
 */
class mediamosa_rest_call_asset_collection_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const COLL_ID = 'coll_id';
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::COLL_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_COLLECTION_ID,
          self::VAR_DESCRIPTION => 'The collection ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID to add to the collection. Multiple assets may be given here with asset_id[].',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID, must be owner of the collection.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES
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

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    // App admin.
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Get params.
    $coll_id = $this->get_param_value(self::COLL_ID);
    $a_asset_ids = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

    // Collection must exist.
    mediamosa_db::db_must_exists(mediamosa_collection_db::TABLE_NAME, array(mediamosa_collection_db::ID => $coll_id));

    foreach ($a_asset_ids as $asset_id) {
      try {
        // Create the link.
        mediamosa_asset_collection::create($app_id, $user_id, $is_app_admin, $asset_id, $coll_id);

        $o_mediamosa->add_item(
          array(
            'coll_id' => $coll_id,
            'asset_id' => $asset_id,
            'result' => mediamosa_response::SUCCESS,
            'result_id' => mediamosa_error::ERRORCODE_OKAY,
            'result_description' => mediamosa_error::error_code_find_description(mediamosa_error::ERRORCODE_OKAY)
          )
        );
      }
      catch (mediamosa_exception_error $e) {
        $o_mediamosa->add_item(
          array(
            'coll_id' => $coll_id,
            'asset_id' => $asset_id,
            'result' => mediamosa_response::ERROR,
            'result_id' => $e->getCode(),
            'result_description' => $e->getMessage()
          )
        );
      }
    }
  }
}

/**
 * URI: /asset/$asset_id/collection_relation/delete
 * Method: POST
 *
 * Delete all links between given asset and his collections.
 *
 * Internal only.
 *
 * 1.x: media_management_delete_asset_from_all_collections
 */
class mediamosa_rest_call_asset_collection_delete_asset_from_all_collections extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES
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

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    // App admin.
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Get params.
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $a_app_ids);

    // FIXME: no output on what was deleted (or not).

    // Delete all links.
    mediamosa_asset_collection::delete_asset_from_all_collections($app_id, $user_id, $is_app_admin, $asset_id);

    // All done.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/collection/count (1.1.5)
 *      /asset/$asset_id/collections/count (1.1.0)
 *      /asset/$asset_id/count_collections (1.1.0) (deprecated)
 *
 * Method: POST
 *
 * Count the number of collections where given arrat is in.
 *
 * 1.x: media_management_asset_count_collections
 */
class mediamosa_rest_call_asset_collection_count extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';

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
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();

    // Get the 1st app.
    $app_id = reset($a_app_ids);

    // App admin.
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Get params.
    $asset_id = $this->get_param_value(self::ASSET_ID);

    // Get the count.
    $o_mediamosa->item_count_total =  mediamosa_asset_collection::count_collections($asset_id);

    // All done.
    $o_mediamosa->set_result_okay();
  }
}

