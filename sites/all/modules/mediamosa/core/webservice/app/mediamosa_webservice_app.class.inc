<?php
 /**
  * @file
  * Webservices management per app.
  */
class mediamosa_webservice_app {
  // ------------------------------------------------------------------ Consts.
  const HANDLE_MEDIA_MANAGEMENT = 'media_management';
  const HANDLE_BATCH_UPLOAD = 'batch_upload';
  const HANDLE_JOBS = 'jobs';
  const HANDLE_MEDIA_UPLOAD = 'media_upload';
  const HANDLE_TRANSCODING = 'transcoding';

  // ---------------------------------------------------------------- Functions.
  /**
   * Check active status of an webservice.
   *
   * @param string $handle
   *   Webservice handle.
   * @param int $app_id
   *   Application ID.
   *
   * @return bool
   *   TRUE when webservice is allowed, FALSE otherwise.
   */
  public static function webservice_is_active($handle, array $app_ids = array()) {
    if (!empty($app_ids)) {
      $result = mediamosa_db::db_query(
        'SELECT wa.#wa_status AS wa_status, w.#w_status AS w_status
         FROM {#mediamosa_webservice_app} AS wa
         JOIN {#mediamosa_webservice} AS w ON w.#handle = wa.#webservice_handle
         JOIN {#mediamosa_app} AS ma ON ma.#app_id = wa.#app_id
         WHERE wa.#webservice_handle = :handle AND ma.#app_id IN (:app_id)',
        array(
          '#wa_status' => mediamosa_webservice_app_db::STATUS,
          '#w_status' => mediamosa_webservice_db::STATUS,
          '#mediamosa_webservice_app' => mediamosa_webservice_app_db::TABLE_NAME,
          '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
          '#handle' => mediamosa_webservice_db::HANDLE,
          '#webservice_handle' => mediamosa_webservice_app_db::WEBSERVICE_HANDLE,
          '#mediamosa_app' => mediamosa_app_db::TABLE_NAME,
          '#app_id' => mediamosa_app_db::APP_ID,
          ':handle' => $handle,
          ':app_id' => $app_ids
        )
      )->fetchAssoc();

      if (!$result) {
        return TRUE; // Not found then ON.
      }

      // Return only false when webservice is specified as OFF.
      return ($result['wa_status'] == mediamosa_webservice_app_db::STATUS_TRUE && $result['w_status'] == mediamosa_webservice_db::STATUS_TRUE) ? TRUE : FALSE;
    }

    $result = mediamosa_db::db_query(
      'SELECT #status FROM {#mediamosa_webservice} WHERE #handle = :handle',
      array(
        '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
        '#handle' => mediamosa_webservice_db::HANDLE,
        ':handle' => $handle,
        '#status' => mediamosa_webservice_db::STATUS,
      )
    )->fetchField();

    if (!$result) {
      return TRUE; // Not found then ON.
    }

    // If specified ON, return TRUE.
    return $result == mediamosa_webservice_db::STATUS_TRUE ? TRUE : FALSE;
  }

  /**
   * Same as webservice_is_active, except will throw an exception.
   *
   * @param string $handle
   *  Webservice handle.
   * @param int $app_id
   *  Application ID.
   */
  public static function webservice_must_be_active($handle, array $app_ids = array()) {
    if (!self::webservice_is_active($handle, $app_ids)) {
      throw new mediamosa_exception_error_webservice_disabled();
    }
  }

  /**
   * Enable webservice for app.
   *
   * @param int $app_id
   * @param string $handle
   */
  public static function webservice_status($app_id, $handle, $status) {

    // get all possible handles.
    $handles = mediamosa_webservice::get_all_handles();

    // Handle must exists.
    if (!in_array($handle, $handles)) {
      mediamosa_watchdog::log("Try to insert handle '!handle' that does not exists as webservice.", array('!handle' => $handle));
      return; // ignore.
    }

    // Get current.
    $current = mediamosa_db::db_select(mediamosa_webservice_app_db::TABLE_NAME, 'wa')
      ->fields('wa')
      ->condition(mediamosa_webservice_app_db::APP_ID, $app_id)
      ->condition(mediamosa_webservice_app_db::WEBSERVICE_HANDLE, $handle)
      ->execute()
      ->fetchAssoc();


    // FIXME: Not master-master save.
    if ($current) {
      mediamosa_db::db_update(mediamosa_webservice_app_db::TABLE_NAME)
        ->fields(array(
          mediamosa_webservice_app_db::STATUS => $status ? mediamosa_webservice_app_db::STATUS_TRUE : mediamosa_webservice_app_db::STATUS_FALSE,
        ))
        ->condition(mediamosa_webservice_app_db::APP_ID, $app_id)
        ->condition(mediamosa_webservice_app_db::WEBSERVICE_HANDLE, $handle)
        ->execute();
    }
    else {
      // Enable.
      mediamosa_db::db_insert(mediamosa_webservice_app_db::TABLE_NAME)
        ->fields(array(
          mediamosa_webservice_app_db::APP_ID => $app_id,
          mediamosa_webservice_app_db::STATUS => $status ? mediamosa_webservice_app_db::STATUS_TRUE : mediamosa_webservice_app_db::STATUS_FALSE,
          mediamosa_webservice_app_db::WEBSERVICE_HANDLE => $handle,
        ))
        ->execute();
    }
  }

  /**
   * Return enabled/disabled statussem of webservices of each app.
   */
  public static function webservice_get_all_status($app_ids = array()) {
    $query = mediamosa_db::db_select(mediamosa_webservice_app_db::TABLE_NAME, 'w')
      ->fields('w');

    // If we provide app_ids, then only retrieve those.
    if (!empty($app_ids)) {
      $query->condition(mediamosa_webservice_app_db::APP_ID, $app_ids, 'IN');
    }

    $result = $query->execute();

    $a_webservice_apps = array();
    foreach ($result as $a_webservice_app) {
      $a_webservice_apps[$a_webservice_app[mediamosa_webservice_app_db::APP_ID]][$a_webservice_app[mediamosa_webservice_app_db::WEBSERVICE_HANDLE]] = ($a_webservice_app[mediamosa_webservice_app_db::STATUS] == mediamosa_webservice_app_db::STATUS_TRUE);
    }

    return $a_webservice_apps;
  }

  /**
   * Return percentage used diskspace of each app.
   */
  public static function diskspace_used_get_all_status($app_ids = array()) {

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'm');
    $query->join(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mm', 'm.mediafile_id = mm.mediafile_id');
    $query->join(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'mmp', 'mm.prop_id = mmp.prop_id');
    $query->fields('mm', array('val_int'));
    $query->fields('m', array('app_id'));
    $query->condition('mmp.prop_name', 'filesize');
    $query->groupBy('app_id');
    $query->addExpression('sum(val_int)', 'total');

    // If we provide app_ids, then only retrieve those.
    if (!empty($app_ids)) {
      $query->condition(mediamosa_asset_mediafile_db::APP_ID, $app_ids, 'IN');
    }

    $result = $query->execute();

    $a_quotas = array();
    foreach ($result as $diskspace) {
      $a_quotas[$diskspace[mediamosa_asset_mediafile_db::APP_ID]] = $diskspace['total'];
    }
    return $a_quotas;
  }

  /**
   * Remove all handles from this app_id. Provide 0 for all globals.
   * @param int $app_id
   */
  public static function remove_all($app_id) {
    mediamosa_db::db_delete(mediamosa_webservice_db::TABLE_NAME)
      ->condition(mediamosa_webservice_app_db::APP_ID, $app_id)
      ->execute();
  }
}
