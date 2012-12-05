<?php
/**
 * @file
 * storage_mapping module.
 */

class mediamosa_storage_mapping {

  /**
   * Get storage mapping.
   *
   * @param $id
   *   Storage mapping ID.
   */
  public static function get($id) {
    $query = mediamosa_db::db_select(mediamosa_storage_mapping_db::TABLE_NAME, 'sp')
      ->fields('sp')
      ->condition(mediamosa_storage_mapping_db::ID, $id);

    return $query->execute()->fetchAssoc();
  }

  /**
   * Get storage mapping as uri.
   *
   * @param $id
   *   Storage mapping ID.
   */
  public static function get_uri($id) {
    $mapping = self::get($id);
    $type = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->condition(mediamosa_storage_profile_db::ID, $mapping[mediamosa_storage_mapping_db::PROFILE_ID])
      ->addField('sp', mediamosa_storage_profile_db::TYPE)
      ->execute()
      ->fetchField();
    return $type . '://' . $mapping[mediamosa_storage_mapping_db::PROFILE_ID] . '/' . $mapping[mediamosa_storage_mapping_db::PATH];
  }

  /**
   * Get the default storage mapping.
   *
   * @param $app_id
   *   Application ID.
   */
  public static function get_default($app_id) {
    return mediamosa_db::db_select(mediamosa_storage_mapping_db::TABLE_NAME, 'sm')
      ->fields('sm')
      ->condition(mediamosa_storage_mapping_db::APP_ID, $app_id)
      ->condition(mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING, mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_TRUE)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the default storage mapping as uri.
   *
   * @param $app_id
   *   Application ID.
   */
  public static function get_default_uri($app_id) {
    $mapping = self::get_default($app_id);
    $type = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->condition(mediamosa_storage_profile_db::ID, $mapping[mediamosa_storage_mapping_db::PROFILE_ID])
      ->fields('sp', array(mediamosa_storage_profile_db::TYPE))
      ->execute()
      ->fetchField();
    return $type . '://' . $mapping[mediamosa_storage_mapping_db::PROFILE_ID] . '/' . $mapping[mediamosa_storage_mapping_db::PATH];
  }
  
  /**
   * Get the human readable default storage mapping.
   *
   * @param $app_id
   *   Application ID.
   */
  public static function get_default_view($app_id) {
    $mapping = self::get_default($app_id);
    $profile = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->condition(mediamosa_storage_profile_db::ID, $mapping[mediamosa_storage_mapping_db::PROFILE_ID])
      ->fields('sp', array(mediamosa_storage_profile_db::PROFILE))
      ->execute()
      ->fetchField();
    return $profile . ' - ' . $mapping[mediamosa_storage_mapping_db::PATH];
  }

  /**
   * Get the default storage object for the given application.
   *
   * @param $app_id
   *   Application ID.
   * 
   * @return mediamosa_storage
   *   Storage object
   */
  public static function get_default_object($app_id) {
    $mapping = self::get_default($app_id);
    $profile = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->condition(mediamosa_storage_profile_db::ID, $mapping[mediamosa_storage_mapping_db::PROFILE_ID])
      ->fields('sp')
      ->execute()
      ->fetchAssoc();
    $class = 'mediamosa_storage_' . $profile[mediamosa_storage_profile_db::TYPE];
    $object = new $class($profile, $mapping);
  }
  
  /**
   * Get listing storage mappings for app.
   *
   * @param $app_id
   *   Application ID.
   */
  public static function get_all($app_id = NULL, array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_storage_mapping_db::TABLE_NAME, 'sm');
    
    if (count($fields)) {
      foreach ($fields as $key => $field) {
        $query->addField('sm', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields('sm');
    }

    if (isset($app_id)) {
      $query->condition(mediamosa_storage_mapping_db::APP_ID, $app_id);
    }
    
    return $query->execute();
  }

  /**
   * Reset the default storage mapping
   *
   * @param $app_id
   *   Application ID
   */
  public static function reset_default($app_id) {
    mediamosa_db::db_update(mediamosa_storage_mapping_db::TABLE_NAME)
      ->condition(mediamosa_storage_mapping_db::APP_ID, $app_id)
      ->fields(array(mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING => mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE))
      ->execute();
  }
  
  /**
   * Update default storage mapping from uri
   * 
   * @param $app_id
   *   Application ID
   * @param $uri
   *   Storage uri
   */
  public static function update_default_uri($app_id, $uri) {
    $uri = explode('/', $uri, 4);
    self::update_default($app_id, $uri[2], $uri[3]);
  }
  
  /**
   * Update default storage mapping
   * 
   * @param $app_id
   *   Application ID
   * @param $profile_id
   *   Storage profile ID
   * @param $path
   *   Storage path
   */
  public static function update_default($app_id, $profile_id, $path) {
    $mapping = self::get_default($app_id);
    if ($mapping) {
      self::update($mapping[mediamosa_storage_mapping_db::ID], $profile_id, $path, TRUE);
    }
    else {
      self::create($app_id, $profile_id, $path, TRUE);
    }
  }
  
  /**
   * Insert storage mapping
   * 
   * @param $app_id
   *   Application ID
   * @param $profile_id
   *   Storage profile ID
   * @param $path
   *   Storage path
   * @param $default
   *   TRUE if this mapping is the default mapping for the application
   *   
   * @return 
   *   ID of the mapping
   */
  public static function create($app_id, $profile_id, $path, $default = FALSE) {
    if ($default) {
      self::reset_default($app_id);
    }
    $fields = array(
      mediamosa_storage_mapping_db::APP_ID => $app_id,
      mediamosa_storage_mapping_db::PROFILE_ID => $profile_id,
      mediamosa_storage_mapping_db::PATH => $path,
      mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING => ($default? mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_TRUE : mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE),
    );
    $fields = mediamosa_db::db_insert_enrich($fields);
    return mediamosa_db::db_insert(mediamosa_storage_mapping_db::TABLE_NAME)->fields($fields)->execute();
  }

  /**
   * Update storage mapping
   * 
   * @param $id
   *   Storage mapping ID
   * @param $profile_id
   *   Storage profile ID
   * @param $path
   *   Storage path
   * @param $default
   *   TRUE if this mapping is the default mapping for the application
   */
  public static function update($id, $profile_id, $path, $default = FALSE) {
    if ($default) {
      self::reset_default($app_id);
    }
    $fields = array(
      mediamosa_storage_mapping_db::PROFILE_ID => $profile_id,
      mediamosa_storage_mapping_db::PATH => $path,
      mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING => ($default? mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_TRUE : mediamosa_storage_mapping_db::IS_DEFAULT_MAPPING_FALSE),
    );
    $fields = mediamosa_db::db_update_enrich($fields);
    mediamosa_db::db_update(mediamosa_storage_mapping_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_storage_mapping_db::ID, $id)
      ->execute();
  }
  
  /**
   * Delete storage mapping
   * 
   * @param $id
   *   Storage mapping ID
   */
  public static function delete($id) {
    mediamosa_db::db_delete(mediamosa_storage_mapping_db::TABLE_NAME)
      ->condition(mediamosa_storage_mapping_db::ID, $id)
      ->execute();
  }

  /**
   * Delete all storage mappings for given application
   * 
   * @param $app_id
   *   Application ID
   */
  public static function delete_all($app_id) {
    mediamosa_db::db_delete(mediamosa_storage_mapping_db::TABLE_NAME)
      ->condition(mediamosa_storage_mapping_db::APP_ID, $app_id)
      ->execute();
  }
}
