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
  * functions for the webservice module.
  */

class mediamosa_webservice {

  /**
   * Returns all in use (webservice) handles.
   */
  static public function get_all_handles() {
    return mediamosa_db::db_select(mediamosa_webservice_db::TABLE_NAME, 'w')
      ->fields('w', array(mediamosa_webservice_db::HANDLE))
      ->execute()
      ->fetchCol();
  }

  /**
   * Return with the webservice row.
   *
   * @param integer $webservice_id
   */
  static public function get($webservice_id) {
    return mediamosa_db::db_select(mediamosa_webservice_db::TABLE_NAME, 'w')
      ->fields('w')
      ->condition(mediamosa_webservice_db::ID, $webservice_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get all webservice.
   *
   * @param integer $app_id
   */
  static public function get_all($app_id = 0) {

    if (empty($app_id)) {
      return mediamosa_db::db_query(
        'SELECT * FROM {#mediamosa_webservice} AS w ORDER BY w.#handle ASC',
        array(
          '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
          '#handle' => mediamosa_webservice_db::HANDLE,
        )
      );
    }

    // Do the query.
    return mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_webservice} AS w
       JOIN {#mediamosa_webservice_app} AS wa ON wa.#webservice_handle = w.#handle
       WHERE wa.#app_id = :app_id
       ORDER BY w.#handle ASC',
      array(
        '#mediamosa_webservice' => mediamosa_webservice_db::TABLE_NAME,
        '#mediamosa_webservice_app' => mediamosa_webservice_app_db::TABLE_NAME,
        '#handle' => mediamosa_webservice_db::HANDLE,
        '#webservice_handle' => mediamosa_webservice_app_db::WEBSERVICE_HANDLE,
        '#app_id' => mediamosa_webservice_app_db::APP_ID,
        ':app_id' => $app_id,
      )
    );
  }
}
