<?php
/**
 * @file
 * Contains the REST calls for the storage module.
 */

/**
 * URI: /storage/$mediafile_id/download
 * Method: POST
 */
class mediamosa_rest_call_storage_download extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';

  // ---------------------------------------------------------------- Functions.
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID of the MediaFile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::IS_APP_ADMIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_DESCRIPTION => mediamosa::t('NOT USED.'),
          self::VAR_DEFAULT_VALUE => 'false',
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Create the job.
    $job_id = mediamosa_job::create_job_media_download($app_id, $user_id, $is_app_admin, $mediafile_id);

    // Got here, then all is ok.
    $mediamosa->add_item(array('job_id' => $job_id));
  }
}

/**
 * URI: /storage/$mediafile_id/upload
 * Method: POST
 */
class mediamosa_rest_call_storage_upload extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const STORAGE_PROFILE_ID = 'storage_profile_id';

  // ---------------------------------------------------------------- Functions.
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID of the MediaFile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::IS_APP_ADMIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_DESCRIPTION => mediamosa::t('NOT USED.'),
          self::VAR_DEFAULT_VALUE => 'false',
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    // Create the job.
    $job_id = mediamosa_job::create_job_media_upload($app_id, $user_id, $is_app_admin, $mediafile_id);

    // Got here, then all is ok.
    $mediamosa->add_item(array('job_id' => $job_id));
  }
}

/**
 * URI: /storage/$mediafile_id/move
 * Method: POST
 */
class mediamosa_rest_call_storage_move extends mediamosa_rest_call {
  // ------------------------------------------------------------------- Consts.
  const USER_ID = 'user_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const STORAGE_PROFILE_ID = 'storage_profile_id';
  const PATH = 'path';

  // ---------------------------------------------------------------- Functions.
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID of the MediaFile.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::IS_APP_ADMIN => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_NO,
          self::VAR_DESCRIPTION => mediamosa::t('NOT USED.'),
          self::VAR_DEFAULT_VALUE => 'false',
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::STORAGE_PROFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_UINT,
          self::VAR_DESCRIPTION => 'The destination storage profile ID. You must provide ID 1 to move file to local default storage.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::PATH => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => "The path of the destination location. This might be for an S3 location the 'bucket' name.",
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $user_id = $this->get_param_value(self::USER_ID);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);

    $storage_profile_app_id = $this->get_param_value(self::STORAGE_PROFILE_ID);
    $path = $this->get_param_value(self::PATH);

    // Webservice must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_JOBS, $app_ids);

    // Make sure the mediafile exists, does not return the mediafile.
    mediamosa_asset_mediafile::must_exists_cached($mediafile_id, $app_id);

    // Create the job.
    $job_id = mediamosa_job::create_job_media_move($app_id, $user_id, $is_app_admin, $mediafile_id, $storage_profile_app_id, $path);

    // Got here, then all is ok.
    $mediamosa->add_item(array('job_id' => $job_id));
  }
}

/**
 * URI: /cron/storage/transition
 * Method: GET
 */
class mediamosa_rest_call_cron_storage_transition extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Methods.
  public function get_var_setup() {
    $var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Clean up.
    mediamosa_storage_transition::clean_up();

    // Set ok.
    $mediamosa->set_result_okay();
  }
}
