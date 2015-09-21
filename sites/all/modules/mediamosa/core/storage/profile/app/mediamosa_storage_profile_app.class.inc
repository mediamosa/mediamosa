<?php
/**
 * @file
 * MediaMosa storage profile application.
 *
 * The storage profile application is the link between the client application
 * and the storage profile. It also stores specific data for this client
 * application.
 */

class mediamosa_storage_profile_app {
  // ---------------------------------------------------------------- Functions.
  /**
   * Get storage profile app.
   *
   * @param int $profile_app_id
   *   Storage profile app ID. This is the ID of the profile; its origin is
   *   table mediamosa_storage_profile->storage_profile_id.
   *
   * @return array
   *   The storage profile app or FALSE.
   */
  public static function get($profile_app_id) {
    $query = mediamosa_db::db_select(mediamosa_storage_profile_app_db::TABLE_NAME, 'sp')
      ->fields('sp')
      ->condition(mediamosa_storage_profile_app_db::ID, $profile_app_id);

    return $query->execute()->fetchAssoc();
  }

  /**
   * Get storage profile app by building it.
   *
   * @param int $app_id
   *   Storage profile app ID.
   *
   * @return array
   *   The storage profile link with app array.
   */
  protected static function get_default($app_id) {
    // Get default storage profile.
    $storage_profile = mediamosa_storage_profile::get_default($app_id);
    if ($storage_profile === NULL) {
      $storage_profile = array(
        mediamosa_storage_profile_db::ID => 0,
      );
    }

    // Create dummy.
    return array(
      mediamosa_storage_profile_app_db::APP_ID => $app_id,
      mediamosa_storage_profile_app_db::ID => 0,
      mediamosa_storage_profile_app_db::PATH => 'global',
      mediamosa_storage_profile_app_db::PATH_STILLS => 'global',
      mediamosa_storage_profile_app_db::PROFILE_ID => $storage_profile[mediamosa_storage_profile_db::ID],
      mediamosa_storage_profile_app_db::PROFILE_ID_STILLS => $storage_profile[mediamosa_storage_profile_db::ID],
    );
  }

  /**
   * Get the storage profile ID used by this application.
   *
   * @param int $app_id
   *   The application ID.
   *
   * @return integer
   *   The storage profile ID.
   */
  public static function get_app_storage_profile($app_id) {
    $storage_profile_apps = &drupal_static(__FUNCTION__, array());

    // Get storage_profile ID, if not already.
    if (!isset($storage_profile_apps[$app_id])) {
      // Get the storage profile app row.
      $storage_profile_apps[$app_id] = mediamosa_db::db_select(mediamosa_storage_profile_app_db::TABLE_NAME, 'spa')
        ->fields('spa')
        ->condition(mediamosa_storage_profile_app_db::APP_ID, $app_id)
        ->orderBy(mediamosa_storage_profile_app_db::ID, 'DESC')
        ->range(0, 1)
        ->execute()
        ->fetchAssoc();

      // If we didn't find it, then get default, now even there is no link for
      // this app.
      if (!$storage_profile_apps[$app_id]) {
        // Get default storage profile for this app.
        $storage_profile_apps[$app_id] = mediamosa_storage_profile_app::get_default($app_id);
      }
    }

    return $storage_profile_apps[$app_id];
  }


  /**
   * Get the storage profile app for specific app.
   *
   * @param int $app_id
   *   Application ID.
   *
   * @return array
   *   The storage profile app or FALSE.
   */
  public static function get_for_app($app_id) {
    return mediamosa_db::db_select(mediamosa_storage_profile_app_db::TABLE_NAME, 'spa')
      ->fields('spa')
      ->condition(mediamosa_storage_profile_app_db::APP_ID, $app_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the human readable default storage profile app.
   *
   * @param int $app_id
   *   Application ID.
   * @param bool $for_stills
   *   Specify TRUE to get setting for stills.
   *
   * @return string
   *   The human readable text.
   */
  public static function get_view_for_app($app_id, $for_stills = FALSE) {
    // Get the link between profile and app.
    $profile_app = self::get_for_app($app_id);

    if ($for_stills) {
      // Get the profile name.
      $profile = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
        ->condition(mediamosa_storage_profile_db::ID, $profile_app[mediamosa_storage_profile_app_db::PROFILE_ID_STILLS])
        ->fields('sp', array(mediamosa_storage_profile_db::PROFILE))
        ->execute()
        ->fetchField();

      // Return human readable text.
      return $profile . ' - ' . $profile_app[mediamosa_storage_profile_app_db::PATH_STILLS];
    }

    // Get the profile name.
    $profile = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->condition(mediamosa_storage_profile_db::ID, $profile_app[mediamosa_storage_profile_app_db::PROFILE_ID])
      ->fields('sp', array(mediamosa_storage_profile_db::PROFILE))
      ->execute()
      ->fetchField();

    // Return human readable text.
    return $profile . ' - ' . $profile_app[mediamosa_storage_profile_app_db::PATH];
  }

  /**
   * Get the default storage mapping as uri.
   *
   * @param $app_id
   *   Application ID.
   */
  public static function get_default_uri($app_id) {
    $storage_profile_app = self::get_for_app($app_id);
    return $storage_profile_app[mediamosa_storage_profile_app_db::PROFILE_ID] . '/' . $storage_profile_app[mediamosa_storage_profile_app_db::PATH];
  }

  /**
   * Get the default storage mapping as uri.
   *
   * @param $app_id
   *   Application ID.
   */
  public static function get_default_uri_stills($app_id) {
    $storage_profile_app = self::get_for_app($app_id);
    return $storage_profile_app[mediamosa_storage_profile_app_db::PROFILE_ID_STILLS] . '/' . $storage_profile_app[mediamosa_storage_profile_app_db::PATH_STILLS];
  }

  /**
   * Update default storage profile app from uri.
   *
   * @todo: function name is not logical.
   *
   * @param $app_id
   *   Application ID
   * @param $uri
   *   Storage uri
   */
  public static function update_default_uri($app_id, $uri, $uri_stills) {
    list($profile_id, $path) = explode('/', $uri, 2);
    list($profile_id_stills, $path_stills) = explode('/', $uri_stills, 2);
    self::update_default($app_id, $profile_id, $path, $profile_id_stills, $path_stills);
  }

  /**
   * Update default storage profile app.
   *
   * @param int $app_id
   *   Application ID.
   * @param int $profile_id
   *   Storage profile ID
   * @param string $path
   *   Storage path.
   * @param int $profile_id_stills
   *   Storage profile ID stills.
   * @param string $path_stills
   *   Storage path stills.
   */
  public static function update_default($app_id, $profile_id, $path, $profile_id_stills, $path_stills) {
    // Is not set during unit test creation of app.
    if (empty($profile_id) || empty($profile_id_stills)) {
      return;
    }

    $profile_app = self::get_for_app($app_id);
    if ($profile_app) {
      self::update($profile_app[mediamosa_storage_profile_app_db::ID], $profile_id, $path, $profile_id_stills, $path_stills);
    }
    else {
      self::create($app_id, $profile_id, $path, $profile_id_stills, $path_stills);
    }
  }

  /**
   * Insert storage profile app.
   *
   * @param int $app_id
   *   Application ID.
   * @param int $profile_id
   *   Storage profile ID
   * @param string $path
   *   Storage path.
   * @param int $profile_id_stills
   *   Storage profile ID stills.
   * @param string $path_stills
   *   Storage path stills.
   *
   * @return integer
   *   ID of the mapping
   */
  public static function create($app_id, $profile_id, $path, $profile_id_stills, $path_stills) {
    $fields = array(
      mediamosa_storage_profile_app_db::APP_ID => $app_id,
      mediamosa_storage_profile_app_db::PROFILE_ID => $profile_id,
      mediamosa_storage_profile_app_db::PATH => $path,
      mediamosa_storage_profile_app_db::PROFILE_ID_STILLS => $profile_id_stills,
      mediamosa_storage_profile_app_db::PATH_STILLS => $path_stills,
    );
    $fields = mediamosa_db::db_insert_enrich($fields);

    return mediamosa_db::db_insert(mediamosa_storage_profile_app_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Update storage profile app.
   *
   * @param int $profile_app_id
   *   Storage profile app ID.
   * @param int $profile_id
   *   Storage profile ID.
   * @param string $path
   *   Storage path.
   */
  public static function update($profile_app_id, $profile_id, $path, $profile_id_stills, $path_stills) {
    $fields = array(
      mediamosa_storage_profile_app_db::PROFILE_ID => $profile_id,
      mediamosa_storage_profile_app_db::PATH => $path,
      mediamosa_storage_profile_app_db::PROFILE_ID_STILLS => $profile_id_stills,
      mediamosa_storage_profile_app_db::PATH_STILLS => $path_stills,
    );
    $fields = mediamosa_db::db_update_enrich($fields);

    mediamosa_db::db_update(mediamosa_storage_profile_app_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_storage_profile_app_db::ID, $profile_app_id)
      ->execute();
  }

  /**
   * Delete storage profile app.
   *
   * @param $maid
   *   Storage mapping ID
   */
  public static function delete($profile_app_id) {
    mediamosa_db::db_delete(mediamosa_storage_profile_app_db::TABLE_NAME)
      ->condition(mediamosa_storage_profile_app_db::ID, $profile_app_id)
      ->execute();
  }

  /**
   * Delete all storage profile apps for given application.
   *
   * @param int $app_id
   *   Application ID
   */
  public static function delete_for_app($app_id) {
    mediamosa_db::db_delete(mediamosa_storage_profile_app_db::TABLE_NAME)
      ->condition(mediamosa_storage_profile_app_db::APP_ID, $app_id)
      ->execute();
  }
}
