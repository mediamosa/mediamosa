<?php
/**
 * @file
 * Enables multi support for external search engines.
 */

class mediamosa_search {

  // ------------------------------------------------------------------- Consts.
  const SEARCH_SUPPORT_RELATED = 'related';
  const SEARCH_SUPPORT_FACET = 'facet';
  const SEARCH_SUPPORT_BOOST = 'boost';

  // ---------------------------------------------------------------- Functions.
  /**
   * Return with the selected search engine.
   */
  public static function get_search_engine() {
    $mediamosa_search_engine = &drupal_static(__FUNCTION__);

    if (empty($mediamosa_search_engine)) {
      $mediamosa_search_engines = module_invoke_all('mediamosa_search_engine_info');

      // Now make sure the selected also exists, else use default.
      $selected_mediamosa_search_engine = variable_get('mediamosa_search_engine', 'mediamosa_search');

      if (empty($mediamosa_search_engines[$selected_mediamosa_search_engine])) {
        $selected_mediamosa_search_engine = 'mediamosa_search';
      }

      // Select the search engine.
      $mediamosa_search_engine = $mediamosa_search_engines[$selected_mediamosa_search_engine];
      $mediamosa_search_engine['name'] = $selected_mediamosa_search_engine;

      // Add defaults.
      $mediamosa_search_engine += array(
        self::SEARCH_SUPPORT_RELATED => FALSE,
        self::SEARCH_SUPPORT_FACET => FALSE,
        self::SEARCH_SUPPORT_BOOST => FALSE,
      );
    }

    return $mediamosa_search_engine;
  }

  /**
   * Support any of the additional search options that MySQL does not support?
   *
   * @return bool
   *   Returns TRUE when any of the additional options are supported.
   */
  public static function support_additional() {
    $server = self::get_search_engine();
    return $server[self::SEARCH_SUPPORT_RELATED] || $server[self::SEARCH_SUPPORT_FACET];
  }

  /**
   * Does ths server support related?
   *
   * @return bool
   *   Returns TRUE
   */
  public static function support_related() {
    $server = self::get_search_engine();
    return $server[self::SEARCH_SUPPORT_RELATED];
  }

  /**
   * Does ths server support facted search?
   *
   * @return bool
   *   Returns TRUE
   */
  public static function support_facet() {
    $server = self::get_search_engine();
    return $server[self::SEARCH_SUPPORT_FACET];
  }

  /**
   * Verify the CQL syntax, used for CQL asset context set.
   *
   * @param string $cql
   *   The CQL string.
   * @param array $app_ids
   *   The application IDs.
   * @param string $error_text
   *   The error text, in case of error.
   *
   * @return bool
   *   Returns TRUE when no errors where found, FALSE otherwise.
   */
  public static function asset_verify($cql, $app_ids, &$error_text) {
    // Get the search engine.
    $mediamosa_search_engine = self::get_search_engine();

    // Get name.
    $name = $mediamosa_search_engine['name'];
    if ($name === 'mediamosa_search') {
      $name = 'mediamosa_asset_search';
    }

    mediamosa_exception::assert(method_exists($name, 'asset_verify'));

    $parameters = array(
      $cql,
      $app_ids,
      &$error_text,
    );
    return call_user_func_array($name . '::asset_verify', $parameters);
  }

  /**
   * Wrapper for asset_search function.
   */
  public static function asset($parameters) {

    // Get the search engine.
    $mediamosa_search_engine = self::get_search_engine();

    // Get name.
    $name = $mediamosa_search_engine['name'];

    // If the function does not exists, fail back to default.
    // Fixme: show deleted OAI will run using MySQL.
    if (!method_exists($name, 'asset_search') || $parameters['show_deleted']) {
      $name = 'mediamosa_search';
    }

    // Fill up default.
    $parameters += array(
      'cql' => NULL,
      'cql_store_stats' => FALSE, // Do not store stats about the search by default.
      'acl_user_id' => NULL,
      'acl_group_ids' => array(),
      'acl_domain' => NULL,
      'acl_realm' => NULL,

      'batch_id' => 0,
      'coll_id' => array(),

      'fav_user_id' => NULL,

      'published' => mediamosa_asset::METADATA_PUBLISHED_TRUE,
      'trashcan' => mediamosa_asset::METADATA_TRASHCAN_FALSE,

      'granted' => FALSE,
      'is_public_list' => FALSE,
      'is_app_admin' => FALSE,
      'hide_empty_assets' => FALSE,
      'show_deleted' => FALSE,
      'calculate_total_count' => TRUE,

      'limit' => 10,
      'offset' => 0,

      'related' => false,
      'related_count' => 10,
      'related_boost' => false,
    );
    drupal_alter('mediamosa_asset_search_parameters', $parameters);

    // Call function.
    try {
      $asset_search = call_user_func($name . '::asset_search', $parameters);
      $asset_search += array(
        'related' => array(),
        'keep_order_ids' => array(),
      );

      if (!empty($asset_search['keep_order_ids'])) {
        $asset_ids = $asset_search['asset_ids'];
        $asset_search['asset_ids'] = array_intersect($asset_search['keep_order_ids'], $asset_ids);
        // Merge the missing IDs at the end.
        if (count($asset_ids) != count($asset_search['asset_ids'])) {
          $asset_search['asset_ids'] = array_merge($asset_search['asset_ids'], array_diff($asset_search['asset_ids'], $asset_search['keep_order_ids']));
        }
      }

      return $asset_search;

    }
    catch (Exception $e) {
      // Do not support fallback for database when running tests.
      if (mediamosa::in_simpletest_sandbox()) {
        throw $e;
      }

      // If other database fails, then do MySQL default (unless it was MySQL).
      if ($name != 'mediamosa_search') {
        return self::asset_search($parameters);
      }
      else {
        throw $e;
      }
    }
  }

  /**
   * Wrapper for access_check_assets function.
   */
  public static function asset_access($parameters) {

    // Nothing to find?
    if (empty($parameters['asset_ids'])) {
      return array();
    }

    // Fill up default.
    $parameters += array(
      'is_app_admin' => FALSE,
    );

    // Get the search engine.
    $mediamosa_search_engine = self::get_search_engine();

    // Get name.
    $name = $mediamosa_search_engine['name'];

    // If the function does not exists, fail back to default.
    if (!method_exists($name, 'access_check_assets')) {
      $name = 'mediamosa_search';
    }

    // Call function.
    try {
      return call_user_func($name . '::access_check_assets', $parameters);
    }
    catch (Exception $e) {
      // Do not fall back on mysql in sandbox.
      if (mediamosa::in_simpletest_sandbox()) {
        throw $e;
      }

      // If other database fails, then do MySQL default (unless it was MySQL).
      if ($name !== 'mediamosa_search') {
        return self::access_check_assets($parameters);
      }
      else {
        throw $e;
      }
    }
  }

  /**
   * Search the asset_search function
   *
   * @param $parameters
   */
  public static function asset_search(array $parameters) {

    // Call our function.
    return mediamosa_asset_search::asset_search(
      $parameters['app_ids'],
      $parameters['cql'],
      $parameters['cql_store_stats'],
      $parameters['acl_user_id'],
      $parameters['acl_group_ids'],
      $parameters['acl_domain'],
      $parameters['acl_realm'],

      $parameters['batch_id'],
      $parameters['coll_id'],

      $parameters['fav_user_id'],

      $parameters['published'],
      $parameters['trashcan'],

      $parameters['granted'],
      $parameters['is_public_list'],
      $parameters['is_app_admin'],
      $parameters['hide_empty_assets'],
      $parameters['show_deleted'],
      $parameters['calculate_total_count'],

      $parameters['limit'],
      $parameters['offset']
    );
  }

  /**
   * Does access check on selected assets.
   *
   * @param array $parameters
   */
  public static function access_check_assets(array $parameters) {
    return mediamosa_acl::access_check_assets(
      $parameters['asset_ids'],
      $parameters['app_ids'],
      $parameters['acl_user_id'],
      $parameters['acl_group_ids'],
      $parameters['acl_domain'],
      $parameters['acl_realm'],
      $parameters['is_app_admin']
    );
  }

  /**
   * Reindex assets on external databases.
   *
   * @param array $asset_ids
   *   The assets to reindex.
   * @param string $search_index_type
   *   The type of indexing.
   *   @see mediamosa_settings::search_index_type_*
   */
  public static function asset_reindex(array $asset_ids, $search_index_type) {
    // Trigger the reindex hook for external search engines.
    try {
      module_invoke_all('mediamosa_asset_reindex', array('asset_ids' => $asset_ids), $search_index_type);
    }
    catch (Exception $e) {
      mediamosa_watchdog::log_asset(reset($asset_ids), "Reindex for external search engines failed during call of hook 'mediamosa_asset_reindex' for asset with ID '@asset_id'. Exception message; @message.", array('@asset_id' => reset($asset_ids), '@message' => $e->getMessage()));

      // Add to queue, so index is done later when search database is back.
      self::asset_queue($asset_ids);
    }
  }

  /**
   * Delete asset from external databases.
   *
   * @param array $asset_ids
   *   The Ids of assets to delete.
   */
  public static function asset_delete(array $asset_ids) {
    // Trigger the reindex hook for external search engines.
    try {
      module_invoke_all('mediamosa_asset_index_delete', array('asset_ids' => $asset_ids));
    }
    catch (Exception $e) {
      mediamosa_watchdog::log_asset(reset($asset_ids), "Index delete for external search engines failed during call of hook 'mediamosa_asset_reindex' for asset with ID '@asset_id'. Exception message; @message.", array('@asset_id' => reset($asset_ids), '@message' => $e->getMessage()));

      // Add to queue, so index is done later when search database is back.
      self::asset_queue($asset_ids);
    }
  }

  /**
   * Queue asset on external databases.
   *
   * @param array $asset_ids
   *   The IDs of the assets to queue.
   */
  public static function asset_queue(array $asset_ids) {
    // Trigger the reindex hook for external search engines.
    try {
      module_invoke_all('mediamosa_asset_queue', array('asset_ids' => $asset_ids));
    }
    catch (Exception $e) {
      mediamosa_watchdog::log_asset(reset($asset_ids), "Queue for external search engines failed during call of hook 'mediamosa_asset_reindex' for asset with ID '@asset_id'. Exception message; @message.", array('@asset_id' => reset($asset_ids), '@message' => $e->getMessage()));
    }
  }
}
