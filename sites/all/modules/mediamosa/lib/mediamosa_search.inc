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
 * Enables multi support for external search engines.
 */


class mediamosa_search {
  /**
   * Return with the selected search engine.
   */
  static public function get_search_engine() {
    $mediamosa_search_engine = &drupal_static(__FUNCTION__);

    if (empty($mediamosa_search_engine)) {
      $mediamosa_search_engines = module_invoke_all('mediamosa_search_engine');

      // Now make sure the selected also exists, else use default.
      $selected_mediamosa_search_engine = variable_get('mediamosa_search_engine', 'mediamosa_search');

      if (empty($mediamosa_search_engines[$selected_mediamosa_search_engine])) {
        $selected_mediamosa_search_engine = 'mediamosa_search';
      }

      // Select the search engine.
      $mediamosa_search_engine = $mediamosa_search_engines[$selected_mediamosa_search_engine];
      $mediamosa_search_engine['name'] = $selected_mediamosa_search_engine;
    }

    return $mediamosa_search_engine;
  }

  /**
   * Wrapper for asset_search function.
   */
  static public function asset($parameters) {

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
      'aut_user_id' => NULL,
      'aut_group_ids' => array(),
      'aut_domain' => NULL,
      'aut_realm' => NULL,

      'batch_id' => 0,
      'coll_id' => array(),

      'fav_user_id' => NULL,

      'granted' => FALSE,
      'is_public_list' => FALSE,
      'is_app_admin' => FALSE,
      'hide_empty_assets' => FALSE,
      'show_deleted' => FALSE,
      'calculate_total_count' => TRUE,

      'limit' => 10,
      'offset' => 0,
    );

    // Call function.
    try {
      return call_user_func($name . '::asset_search', $parameters);
    }
    catch (Exception $e) {
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
  static public function asset_access($parameters) {

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
      // If other database fails, then do MySQL default (unless it was MySQL).
      if ($name != 'mediamosa_search') {
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
  static public function asset_search(array $parameters) {

    // Call our function.
    return mediamosa_asset_search::asset_search(
      $parameters['app_ids'],
      $parameters['cql'],
      $parameters['aut_user_id'],
      $parameters['aut_group_ids'],
      $parameters['aut_domain'],
      $parameters['aut_realm'],

      $parameters['batch_id'],
      $parameters['coll_id'],

      $parameters['fav_user_id'],

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
  static public function access_check_assets(array $parameters) {
    return mediamosa_aut::access_check_assets(
      $parameters['asset_ids'],
      $parameters['app_ids'],
      $parameters['aut_user_id'],
      $parameters['aut_group_ids'],
      $parameters['aut_domain'],
      $parameters['aut_realm'],
      $parameters['is_app_admin']
    );
  }

  /**
   * Reindex assets on external databases.
   *
   * @param $parameters
   */
  static public function asset_reindex(array $asset_ids, $search_index_type) {
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
   * @param $parameters
   */
  static public function asset_delete(array $asset_ids) {
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
   * @param $parameters
   */
  static public function asset_queue(array $asset_ids) {
    // Trigger the reindex hook for external search engines.
    try {
      module_invoke_all('mediamosa_asset_queue', array('asset_ids' => $asset_ids));
    }
    catch (Exception $e) {
      mediamosa_watchdog::log_asset(reset($asset_ids), "Queue for external search engines failed during call of hook 'mediamosa_asset_reindex' for asset with ID '@asset_id'. Exception message; @message.", array('@asset_id' => reset($asset_ids), '@message' => $e->getMessage()));
    }
  }
}
