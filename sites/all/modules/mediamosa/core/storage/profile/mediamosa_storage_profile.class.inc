<?php
/**
 * @file
 * storage_profile module.
 */

class mediamosa_storage_profile {

  /**
   * Get storage profile.
   *
   * @param $id
   *   Profile to get.
   * @return
   *   Associative array with all profile attributes
   */
  public static function get($id) {
    $query = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->fields('sp')
      ->condition(mediamosa_storage_profile_db::ID, $id);

    return $query->execute()->fetchAssoc();
  }

  /**
   * Get the default storage profile.
   *
   * @param $app_id
   *   Application ID.
   * @return
   *   Associative array with all profile attributes
   */
  public static function get_default($app_id) {
    return mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->fields('sp')
      ->condition(mediamosa_storage_profile_db::APP_ID, array(0, $app_id), 'IN')
      ->condition(mediamosa_storage_profile_db::IS_DEFAULT_PROFILE, mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_TRUE)
      ->orderBy(mediamosa_storage_profile_db::APP_ID, 'DESC')
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get listing storage profiles for app.
   *
   * @param $app_id
   *   Application ID.
   * @return
   *   Array with storage profiles 
   */
  public static function get_all($app_id = NULL, array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp');
    
    if (count($fields)) {
      foreach ($fields as $key => $field) {
        $query->addField('sp', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields('sp');
    }

    if (isset($app_id)) {
      $query->condition(mediamosa_storage_profile_db::APP_ID, array(0, $app_id), 'IN');
    }
    
    return $query->execute();
  }

  /**
   * Reset the default storage profile
   *
   * @param $app_id
   *   Application ID
   */
  public static function reset_default($app_id) {
    mediamosa_db::db_update(mediamosa_storage_profile_db::TABLE_NAME)
      ->condition(mediamosa_storage_mapping_db::APP_ID, $app_id)
      ->fields(array(mediamosa_storage_profile_db::IS_DEFAULT_PROFILE => mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_FALSE))
      ->execute(); 
  }
}
