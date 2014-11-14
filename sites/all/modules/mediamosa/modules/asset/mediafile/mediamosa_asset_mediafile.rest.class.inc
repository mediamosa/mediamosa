<?php
/**
 * @file
 * The REST calls of the mediamosa_asset_mediafile module.
 */

/**
 * URI: /mediafile/count
 * Method: GET
 */
class mediamosa_rest_call_count_asset_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const GROUP_ID = 'group_id';
  const OWNER_ID = 'owner_id';

  // ------------------------------------------------------------------ Var Setup.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID to match.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_DEFAULT_VALUE => '',
        ),
        self::OWNER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_TRIM_VALUE => self::VAR_TRIM_VALUE_YES,
          self::VAR_DEFAULT_VALUE => '',
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $group_id = $this->get_param_value(self::GROUP_ID);
    $owner_id = $this->get_param_value(self::OWNER_ID);

    $mediamosa->set_result_okay();

    // Set the total count.
    $mediamosa->item_count_total = mediamosa_db::db_count_items(mediamosa_asset_mediafile_db::TABLE_NAME, $app_id, $owner_id, $group_id);
  }
}

/**
 * URI: /mediafile/$mediafile_id
 * Method: GET
 */
class mediamosa_rest_call_get_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const SHOW_STILLS = 'show_stills';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::SHOW_STILLS => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include the stills in the output.',
          self::VAR_DEFAULT_VALUE => 'TRUE'
        )
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);
    $show_stills = $this->get_param_value(self::SHOW_STILLS);

    // Must exists.
    mediamosa_asset_mediafile::must_exists_cached($mediafile_id);

    // Must have access.
    mediamosa_acl::read_single_object(mediamosa_acl::ACL_TYPE_MEDIAFILE, $mediafile_id, $app_ids);

    // Now enrich response with mediafile data.
    $items = mediamosa_asset_mediafile::enrich_response_mediafile(array($mediafile_id), $app_ids, FALSE, $show_stills);

    // Add to response.
    foreach ($items as $item) {
      $mediamosa->add_item($item);
    }

    // Done.
  }
}

/**
 * URI: /asset/$asset_id/mediafile
 * Method: GET
 */
class mediamosa_rest_call_get_asset_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const TAG = 'tag';
  const IS_STILL = 'is_still';
  const IS_OAI = 'is_oai';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to retrieve.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::IS_STILL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include the stills in the output.',
          self::VAR_DEFAULT_VALUE => 'FALSE'
        ),
        self::IS_OAI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Include extra information required for OAI output.',
          self::VAR_DEFAULT_VALUE => 'FALSE'
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Search on tag.',
          self::VAR_RANGE_END => mediamosa_asset_mediafile_db::TAG_LENGTH,
        )
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $asset_id = $this->get_param_value(self::ASSET_ID);
    $tag = $this->get_param_value(self::TAG);
    $is_still = $this->get_param_value(self::IS_STILL);
    $is_oai = $this->get_param_value(self::IS_OAI);

    // Asset must exist.
    $asset = mediamosa_asset::must_exists($asset_id);

    $mediafile_ids = mediamosa_asset_mediafile::mediafiles_search($asset_id, $tag, $is_still, ($asset['app_id'] == $app_id ? array() : $app_ids));

    // Create output.
    $items = mediamosa_asset_mediafile::enrich_response_mediafile($mediafile_ids, $app_ids, $is_oai);

    // Add to response.
    if (!empty($items['mediafile'])) {
      foreach ($items['mediafile'] as $item) {
        $mediamosa->add_item($item);
      }
    }
  }
}

/**
 * URI: /mediafile/$mediafile_id/delete
 * Method: GET
 */
class mediamosa_rest_call_delete_mediafiles extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const DELETE = 'delete';
  const NEW_ORIGINAL = 'new_original';
  const DELETE_CASCADE = 'cascade';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $var_setup = array(
      self::VARS => array(
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile to delete.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::DELETE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Put on cascade to delete its stills as well.',
          self::VAR_ALLOWED_VALUES => array('cascade', '')
        ),
        self::NEW_ORIGINAL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the new original mediafile.',
        ),
      )
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
    $delete = $this->get_param_value(self::DELETE);
    $user_id = $this->get_param_value(self::USER_ID);
    $new_original = $this->get_param_value(self::NEW_ORIGINAL);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Must exists.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Does the user have access?
    mediamosa_acl::owner_check(
      $app_id,
      $user_id,
      $mediafile[mediamosa_asset_mediafile_db::APP_ID],
      $mediafile[mediamosa_asset_mediafile_db::OWNER_ID]
    );

    // Check the new original.
    if ($new_original) {
      // Must exists.
      $mediafile_new_original = mediamosa_asset_mediafile::must_exists($new_original);

      // Does the user have access?
      mediamosa_acl::owner_check(
        $app_id,
        $user_id,
        $mediafile_new_original[mediamosa_asset_mediafile_db::APP_ID],
        $mediafile_new_original[mediamosa_asset_mediafile_db::OWNER_ID]
      );

      // Check whether the mediafile is original..
      if ($mediafile[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] != mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_IS_NOT_ORIGINAL, array('@mediafile_id' => $mediafile_id));
      }

      // Check whether the mediafile is parent of new original mediafile.
      if ($mediafile_new_original[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE] != $mediafile_id) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_MEDIAFILE_RELATION, array('@mediafile_id' => $mediafile_id, '@new_original' => $new_original));
      }
    }

    // Check if this mediafile is the only one under a sub-asset.
    $query = mediamosa_db::db_select(mediamosa_asset_db::TABLE_NAME, 'a');
    $query->fields('a', array(mediamosa_asset_db::PARENT_ID, mediamosa_asset_db::ID));
    $query->join(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf', strtr('a.#asset_id = mf.#asset_id', array('#asset_id' => mediamosa_asset_db::ID)));
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
    $a_asset = $query->execute()->fetchAssoc();

    // Count the number of mediafiles.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mf', array(mediamosa_asset_mediafile_db::ID));
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID, $a_asset[mediamosa_asset_db::ID]);
    $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
    $count = $query->countQuery()->execute()->fetchField();

    // Must be subasset or is this the only child?
    $delete_asset = (!is_null($a_asset['parent_id']) && $count == 1);

    if ($delete == self::DELETE_CASCADE) {
      // Deletes the stills.
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->fields('mf', array(mediamosa_asset_mediafile_db::ASSET_ID));
      $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
      $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
      $asset_id = $query->execute()->fetchField();

      mediamosa_asset_mediafile_still::delete($asset_id, $mediafile_id);
    }

    // Delete the mediafile.
    mediamosa_asset_mediafile::delete($mediafile_id);

    if ($delete_asset) {
      mediamosa_asset::delete($a_asset[mediamosa_asset_db::ID], TRUE, TRUE);
    }

    // Set the new original mediafile.
    if ($new_original) {
      mediamosa_asset_mediafile::set_new_original($mediafile_id, $new_original);
    }

    $mediamosa->set_result_okay();

    // update de timestamps van de asset
    mediamosa_asset::update_asset_timestamps($a_asset[mediamosa_asset_db::ID]);
  }
}

/**
 * URI: /asset/$asset_id/mediafile/create
 * Method: POST
 */
class mediamosa_rest_call_create_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const ASSET_ID = 'asset_id';
  const MEDIAFILE_ID = 'mediafile_id';
  const GROUP_ID = 'group_id';
  const USER_ID = 'user_id';
  const URI = 'uri';
  const IS_DOWNLOADABLE = 'is_downloadable';
  const SANNAS_MOUNT_POINT = 'sannas_mount_point';
  const IS_ORIGINAL_FILE = 'is_original_file';
  const MEDIAFILE_ID_SOURCE = 'mediafile_id_source';
  const FILENAME = 'filename';
  const TAG = 'tag';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => 'The ID of the parent asset.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The group ID.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::URI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URI,
          self::VAR_DESCRIPTION => 'The stream URI.',
        ),
        self::IS_DOWNLOADABLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mediafile can be downloaded. Only in combination with filename; must be FALSE when URI is used.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::IS_ORIGINAL_FILE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mark mediafile as original.',
          self::VAR_DEFAULT_VALUE => 'TRUE',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::MEDIAFILE_ID_SOURCE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The ID of the mediafile of the original.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'Filename of the mediafile, can not be combined with URI.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::SANNAS_MOUNT_POINT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The location path of the file. Only used in combination with filename.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'User specified tag.',
          self::VAR_RANGE_END => mediamosa_asset_mediafile_db::TAG_LENGTH,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $app_id = reset($app_ids);
    $is_app_admin = $this->get_param_value(self::IS_APP_ADMIN);
    $asset_id = $this->get_param_value(self::ASSET_ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Make sure we dont mix URI and Filename.
    if (!$this->empty_param(self::URI) && (!$this->empty_param(self::FILENAME) || $this->get_param_value(self::IS_DOWNLOADABLE))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MIX_OF_URI_AND_FILE);
    }

    // URI set? Unset the mount point.
    if ($this->isset_param(self::URI)) {
      $this->unset_param(self::SANNAS_MOUNT_POINT);
    }

    // Set mediafile ID to something if not present.
    if ($this->empty_param(self::MEDIAFILE_ID)) {
      $this->set_param_value(self::MEDIAFILE_ID, mediamosa_db::uuid($app_id));
    }

    // Must exists.
    $asset = mediamosa_asset::must_exists($asset_id);

    // Check owner of parent asset, need ownership.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    // Want to copy these fields to new mediafile.
    $params = array(
      self::GROUP_ID => mediamosa_asset_mediafile_db::GROUP_ID,
      self::IS_DOWNLOADABLE => mediamosa_asset_mediafile_db::IS_DOWNLOADABLE,
      self::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
      self::SANNAS_MOUNT_POINT => mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
      self::FILENAME => mediamosa_asset_mediafile_db::FILENAME,
      self::URI => mediamosa_asset_mediafile_db::URI,
      self::MEDIAFILE_ID_SOURCE => mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
      self::TAG => mediamosa_asset_mediafile_db::TAG,
    );

    // Fields to insert.
    $fields = array();

    // Now create the data to insert.
    foreach ($params as $param => $field_name) {
      if ($this->isset_param($param)) {
        $value = $this->get_param_value($param, TRUE);

        $fields[$field_name] = is_bool($value) ? ($value ? 'TRUE' : 'FALSE') : $value;
      }
    }

    // Get mediafile ID.
    $mediafile_id = $this->get_param_value(self::MEDIAFILE_ID);

    // Insert the mediafile.
    $mediafile_id = mediamosa_asset_mediafile::create($mediafile_id, $app_id, $asset_id, $user_id, $fields);

    // Ok add to output.
    $mediamosa->add_item(array('mediafile_id' => $mediafile_id));

    // Done.
  }
}

/**
 * URI: /mediafile/$mediafile_id
 * Method: POST
 */
class mediamosa_rest_call_update_mediafile extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const MEDIAFILE_ID = 'mediafile_id';
  const USER_ID = 'user_id';
  const IS_DOWNLOADABLE = 'is_downloadable';
  const MOUNT_POINT = 'sannas_mount_point';
  const IS_ORIGINAL_FILE = 'is_original_file';
  const FILENAME = 'filename';
  const TRANSCODE_INHERITS_ACL = 'transcode_inherits_acl';
  const MEDIAFILE_ID_SOURCE = 'mediafile_id_source';
  const TAG = 'tag';

  // ------------------------------------------------------------------ Public Functions.
  public function get_var_setup() {
    $var_setup = array();

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The owner ID to match.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::MEDIAFILE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_MEDIAFILE_ID,
          self::VAR_DESCRIPTION => 'The mediafile ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::FILENAME => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_FILENAME,
          self::VAR_DESCRIPTION => 'Change the filename of the file.',
        ),
        self::URI => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_URI,
          self::VAR_DESCRIPTION => 'The stream URI.',
        ),
        self::IS_DOWNLOADABLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mediafile can be downloaded.',
        ),
        self::TRANSCODE_INHERITS_ACL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Any transcodes will inherit ACL settings.',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'User specified tag.',
          self::VAR_RANGE_END => mediamosa_asset_mediafile_db::TAG_LENGTH,
        ),
        self::MOUNT_POINT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The location path of the file.',
          self::VAR_IS_INTERNAL_ONLY => self::VAR_IS_INTERNAL_ONLY_YES,
        ),
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $mediafile_id = $this->get_param_value(mediamosa_asset_mediafile_db::ID);
    $user_id = $this->get_param_value(self::USER_ID);

    // Must be enabled.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Fields allowed to update.
    $params = array(
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => array(),
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => array(FALSE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE, TRUE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE),
      mediamosa_asset_mediafile_db::FILENAME => array(),
      mediamosa_asset_mediafile_db::URI => array(),
      mediamosa_asset_mediafile_db::IS_DOWNLOADABLE => array(FALSE => mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_FALSE, TRUE => mediamosa_asset_mediafile_db::IS_DOWNLOADABLE_TRUE),
      mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL => array(FALSE => mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_FALSE, TRUE => mediamosa_asset_mediafile_db::TRANSCODE_INHERITS_ACL_TRUE),
      mediamosa_asset_mediafile_db::TAG => array(),
    );

    $fields = array();
    foreach ($params as $param => $values) {
      if (!$this->isset_given_param($param)) {
        // If its not supplied, skip it.
        continue;
      }

      // Get value.
      $value = $this->get_param_value($param);
      if (!empty($values)) {
        assert(isset($values[$value]));
        $value = $values[$value];
      }

      $fields[$param] = $value;
    }

    // Update the mediafile.
    mediamosa_asset_mediafile::update($app_id, $mediafile_id, $user_id, $fields);

    // Set all OK.
    $mediamosa->set_result_okay();
  }
}
