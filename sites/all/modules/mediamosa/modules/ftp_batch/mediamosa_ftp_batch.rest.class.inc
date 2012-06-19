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
  * The ftp batch module.
  *
  * @TODO: move the code to core, lets not leave them into rest calls.
  */

/**
 * URI: /ftp_batch
 * Method: GET
 *
 * 1.x: vpx_ftp_batch_list
 *
 */
class mediamosa_rest_call_ftp_batch_list extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = $this->get_var_setup_range($a_var_setup);

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $limit = $this->get_param_value_limit();
    $offset = $this->get_param_value_offset();

    // Get the list.
    $result = mediamosa_ftp_batch::search($app_id, $offset, $limit);

    foreach ($result as $row) {
      // UTC -> App timezone.
      $row[mediamosa_ftp_batch_db::STARTED] = mediamosa::utcdate2appdate($row[mediamosa_ftp_batch_db::STARTED]);
      $row[mediamosa_ftp_batch_db::FINISHED] = mediamosa::utcdate2appdate($row[mediamosa_ftp_batch_db::FINISHED]);

      $o_mediamosa->add_item($row);
    }

    $o_rest_reponse->item_count_total = $result->found_rows;
  }
}

/**
 * URI: /ftp_batch/$batch_id
 * Method: GET
 *
 * 1.x: vpx_ftp_batch_get
 *
 */
class mediamosa_rest_call_ftp_batch_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const BATCH_ID = 'batch_id';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::BATCH_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the batch.',
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

    $batch_id = $this->get_param_value(self::BATCH_ID);

    // Batch must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_batch_db::TABLE_NAME, array(mediamosa_ftp_batch_db::APP_ID => $app_id, mediamosa_ftp_batch_db::ID => $batch_id));

    // Get the batch.
    $row = mediamosa_ftp_batch::get($batch_id);

    // UTC -> App timezone.
    $row[mediamosa_ftp_batch_db::STARTED] = mediamosa::utcdate2appdate($row[mediamosa_ftp_batch_db::STARTED]);
    $row[mediamosa_ftp_batch_db::FINISHED] = mediamosa::utcdate2appdate($row[mediamosa_ftp_batch_db::FINISHED]);

    // Return the item.
    $o_mediamosa->add_item($row);
  }
}

/**
 * URI: /ftp_batch/create
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_ftp_batch_create
 *
 */
class mediamosa_rest_call_ftp_batch_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
          self::VAR_DEFAULT_VALUE => '',
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

    $owner_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);

    // Create the batch.
    $batch_id = mediamosa_ftp_batch::create($app_id, $owner_id, $group_id);

    // Add ID to result.
    $o_mediamosa->add_item(array(
      'batch_id' => $batch_id,
      'result' => mediamosa_response::SUCCESS,
      'result_id' => mediamosa_error::ERRORCODE_OKAY,
      'result_description' => '',
    ));
  }
}

/**
 * URI: /ftp_batch/$batch_id
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_ftp_batch_update
 *
 */
class mediamosa_rest_call_ftp_batch_update extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const BATCH_ID = 'batch_id';
  const VUF = 'vuf';
  const STARTED = 'started';
  const FINISHED = 'finished';
  const EMAIL_ADDRESS = 'email_address';
  const EMAIL_CONTENTS = 'email_contents';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
          self::VAR_DEFAULT_VALUE => '',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::BATCH_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the batch.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::VUF => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'VUF Value.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::STARTED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME,
          self::VAR_DESCRIPTION => 'Start time.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FINISHED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME,
          self::VAR_DESCRIPTION => 'Finished time.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::EMAIL_ADDRESS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Email address.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::EMAIL_CONTENTS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Email contents.',
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

    $owner_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $batch_id = $this->get_param_value(self::BATCH_ID);

    $vuf = $this->get_param_value(self::VUF);

    $started = $this->get_param_value(self::STARTED);
    $finished = $this->get_param_value(self::FINISHED);
    $email_address = $this->get_param_value(self::EMAIL_ADDRESS);
    $email_contents = $this->get_param_value(self::EMAIL_CONTENTS);

    // Batch must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_batch_db::TABLE_NAME, array(mediamosa_ftp_batch_db::APP_ID => $app_id, mediamosa_ftp_batch_db::ID => $batch_id));

    // Update FTP batch.
    mediamosa_ftp_batch::update($app_id, $batch_id, $owner_id, $group_id, $vuf, $started, $finished, $email_address, $email_contents);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /ftp_batch/$batch_id/delete
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_ftp_batch_delete
 *
 */
class mediamosa_rest_call_ftp_batch_delete extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const BATCH_ID = 'batch_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::BATCH_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the batch.',
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

    $batch_id = $this->get_param_value(self::BATCH_ID);

    // Batch must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_batch_db::TABLE_NAME, array(mediamosa_ftp_batch_db::APP_ID => $app_id, mediamosa_ftp_batch_db::ID => $batch_id));

    // Delete the batch.
    mediamosa_ftp_batch::delete($batch_id);

    // Set Ok.
    $o_mediamosa->set_result_okay();
  }
}

/**
 * URI: /ftp_batch/$batch_id/assets
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_ftp_batch_assets_get
 *
 */
class mediamosa_rest_call_ftp_batch_assets_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const BATCH_ID = 'batch_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::BATCH_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the batch.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    $a_var_setup = $this->get_var_setup_range($a_var_setup);

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    $a_app_ids = $this->get_param_value_app();
    $app_id = reset($a_app_ids);

    $batch_id = $this->get_param_value(self::BATCH_ID);

    $limit = $this->get_param_value_limit();
    $offset = $this->get_param_value_offset();

    // Batch must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_batch_db::TABLE_NAME, array(mediamosa_ftp_batch_db::APP_ID => $app_id, mediamosa_ftp_batch_db::ID => $batch_id));

    // Get the assets links.
    $result = mediamosa_ftp_batch_asset::get_asset_relations($app_id, $batch_id, $offset, $limit);

    // Add to response.
    foreach ($result as $row) {
      $row[mediamosa_ftp_batch_db::STARTED] = mediamosa::utcdate2appdate($row[mediamosa_ftp_batch_db::STARTED]);
      $row[mediamosa_ftp_batch_db::FINISHED] = mediamosa::utcdate2appdate($row[mediamosa_ftp_batch_db::FINISHED]);

      $o_mediamosa->add_item($row);
    }

    $o_rest_reponse->item_count_total = $result->found_rows;
  }
}


/**
 * URI: /ftp_batch/$batch_id/assets/add
 * Method: POST
 *
 * Internal only.
 *
 * 1.x: vpx_ftp_batch_assets_add
 *
 */
class mediamosa_rest_call_ftp_batch_assets_add extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const BATCH_ID = 'batch_id';
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const ASSET_ID = 'asset_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::BATCH_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => 'The ID of the batch.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
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

    $batch_id = $this->get_param_value(self::BATCH_ID);

    $owner_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $a_asset_ids = $this->get_param_value(self::ASSET_ID);

    // Batch must exist.
    mediamosa_db::db_must_exists(mediamosa_ftp_batch_db::TABLE_NAME, array(mediamosa_ftp_batch_db::APP_ID => $app_id, mediamosa_ftp_batch_db::ID => $batch_id));

    // Add asset relation.
    mediamosa_ftp_batch_asset::add_asset_relation($app_id, $a_asset_ids, $batch_id);

    // Set OK.
    $o_mediamosa->set_result_okay();
  }
}
