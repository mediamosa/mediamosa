<?php
/**
 * @file
 * REST calls of asset supplement.
 */

/**
 * URI: /asset/$asset_id/supplement
 * Method: GET
 *
 * Get the asset supplement.
 */
class mediamosa_rest_call_asset_supplement_get extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';

  // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
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

    // Asset ID.
    $asset_id = $this->get_param_value(self::ASSET_ID);

    // Asset must exist.
    mediamosa_asset::must_exists($asset_id);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // FIXME: No APP check, can get any supplement, just by suppling an asset id...

    // Get the supplements.
    $supplements = mediamosa_asset_supplement::get_all_with_assetid($asset_id);

    // Add to response.
    foreach ($supplements as $supplement) {
      $supplement['supplement_base64'] = base64_encode($supplement[mediamosa_asset_supplement_db::SUPPLEMENT]);
      unset($supplement[mediamosa_asset_supplement_db::SUPPLEMENT]);

      // Convert times.
      $supplement[mediamosa_asset_supplement_db::CHANGED] = mediamosa::utcdate2appdate($supplement[mediamosa_asset_supplement_db::CHANGED]);
      $supplement[mediamosa_asset_supplement_db::CREATED] = mediamosa::utcdate2appdate($supplement[mediamosa_asset_supplement_db::CREATED]);

      $mediamosa->add_item($supplement);
    }
  }
}

/**
 * URI: /asset/$asset_id/supplement/create
 * Method: POST
 *
 * Create the asset supplement.
 */
class mediamosa_rest_call_asset_supplement_create extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------------ Get Var Setup.
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
          self::VAR_DESCRIPTION => 'The user ID.',
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

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Create the supplement.
    $supplement_id = mediamosa_asset_supplement::create($app_id, $asset_id, $user_id);

    // Add id to response.
    $mediamosa->add_item(array('supplement_id' => $supplement_id));
  }
}

/**
 * URI: /asset/$asset_id/supplement/$supplement_id/update
 *      /supplement/update (FIXME: this (internal) URL should never would have worked under 1.x?)
 *      /supplement/$supplement_id/update (deprecated)
 * Method: POST
 *
 * Update the asset supplement.
 */
class mediamosa_rest_call_asset_supplement_update extends mediamosa_rest_call {
  // ------------------------------------------------=------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const SUPPLEMENT_ID = 'supplement_id';
  const USER_ID = 'user_id';
  const OAI_PREFIX = 'oai_prefix';
  const OAI_NAMESPACEURI = 'oai_namespaceuri';
  const OAI_SCHEMALOCATION = 'oai_schemalocation';
  const SUPPLEMENT = 'supplement';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::SUPPLEMENT_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_SUPPLEMENT_ID,
          self::VAR_DESCRIPTION => 'The supplement ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::SUPPLEMENT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The supplement.',
          self::VAR_RANGE_END => variable_get('mediamosa_asset_supplement_max_size_kb', mediamosa_asset_supplement::DEFAULT_ASSET_SUPPLEMENT_MAX_SIZE) * 1024, // 64k default max length.
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::OAI_PREFIX => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The OAI prefix.',
          self::VAR_RANGE_END => mediamosa_asset_supplement_db::OAI_PREFIX_LENGTH
        ),
        self::OAI_NAMESPACEURI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The OAI namespace URI.',
          self::VAR_RANGE_END => mediamosa_asset_supplement_db::OAI_NAMESPACEURI_LENGTH
        ),
        self::OAI_SCHEMALOCATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The OAI schema location.',
          self::VAR_RANGE_END => mediamosa_asset_supplement_db::OAI_SCHEMALOCATION_LENGTH
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ----------------------------------------------- Override Process Rest Args.
  protected function process_rest_args(array $var_setup) {

    // Run parent so we can get the values.
    parent::process_rest_args($var_setup);

    // Either all are not set or all are set.
    $combined = array(self::OAI_NAMESPACEURI, self::OAI_PREFIX, self::OAI_SCHEMALOCATION);

    // If one of these are set, then make them all required.
    $oai_set = FALSE;
    foreach ($combined as $param) {
      if ($this->isset_given_param($param)) {
        $oai_set = TRUE;
        break;
      }
    }

    // One or more set? Then all required; make them all required.
    if ($oai_set) {
      foreach ($combined as $param) {
        $var_setup[self::VARS][$param][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
      }

      // When providing OAI stuff, then the supplement is a XML.
      $var_setup[self::VARS][self::SUPPLEMENT][self::VAR_TYPE] = mediamosa_sdk::TYPE_XML_OAI;
    }
    else {
      // No OAI set? Then Supplement is required.
      $var_setup[self::VARS][self::SUPPLEMENT][self::VAR_IS_REQUIRED] = self::VAR_IS_REQUIRED_YES;
    }

    // Run parent again.
    return parent::process_rest_args($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Get args.
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $supplement_id = $this->get_param_value(self::SUPPLEMENT_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $oai_prefix = $this->get_param_value(self::OAI_PREFIX);
    $oai_namespaceuri = $this->get_param_value(self::OAI_NAMESPACEURI);
    $oai_schemalocation = $this->get_param_value(self::OAI_SCHEMALOCATION);
    $supplement = $this->get_param_value(self::SUPPLEMENT);

    // Must exists.
    mediamosa_asset::must_exists($asset_id);

    // Update the supplement.
    mediamosa_asset_supplement::update($app_id, $user_id, $supplement_id, $oai_prefix, $oai_namespaceuri, $oai_schemalocation, $supplement);

    // Set ok.
    $mediamosa->set_result_okay();
  }
}

/**
 * URI: /asset/$asset_id/supplement/$supplement_id/delete
 *      /supplement/delete (FIXME: this (internal) URL should never would have worked under 1.x?)
 *      /supplement/$supplement_id/delete (deprecated)
 * Method: POST
 *
 * Update the asset supplement.
 */
class mediamosa_rest_call_asset_supplement_delete extends mediamosa_rest_call {
  // ---------------------------------------------------------=--------- Consts.
  // Rest vars;
  const SUPPLEMENT_ID = 'supplement_id';
  const ASSET_ID = 'asset_id';
  const USER_ID = 'user_id';

  // ------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::SUPPLEMENT_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_SUPPLEMENT_ID,
          self::VAR_DESCRIPTION => 'The supplement ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The asset ID.',
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

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Get args.
    $supplement_id = $this->get_param_value(self::SUPPLEMENT_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Delete it.
    mediamosa_asset_supplement::delete($app_id, $user_id, $supplement_id);

    // Set ok.
    $mediamosa->set_result_okay();
  }
}
