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
  * Functions of asset supplement.
  */

class mediamosa_asset_supplement {

  // The maximum size of the supplement in Kb.
  const DEFAULT_ASSET_SUPPLEMENT_MAX_SIZE = 256;

  /**
   * Get the asset supplement.
   *
   * @param string $supplement_id
   */
  static public function get($supplement_id) {

    $query = mediamosa_db::db_select(mediamosa_asset_supplement_db::TABLE_NAME, 'aas');
    $query->fields('aas');
    $query->condition('aas.' . mediamosa_asset_supplement_db::ID, $supplement_id);
    return $query->execute()->fetchAssoc();
  }

  /**
   * Get the asset supplement.
   *
   * @param string $asset_id
   */
  static public function get_all_with_assetid($asset_id) {

    $query = mediamosa_db::db_select(mediamosa_asset_supplement_db::TABLE_NAME, 'aas');
    $query->fields('aas');
    $query->condition('aas.' . mediamosa_asset_supplement_db::ASSET_ID, $asset_id);
    return $query->execute();
  }

  /**
   * Create the supplement.
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $asset_id
   *  The asset ID on which to create the supplement on.
   * @param string $user_id
   *  The user ID must be owner of the asset to add supplement.
   */
  static public function create($app_id, $asset_id, $user_id) {

    // Asset must exist to be deleted.
    mediamosa_db::db_must_exists(mediamosa_asset_db::TABLE_NAME, array(mediamosa_asset_db::ID => $asset_id));

    // Check if maximum supplments is reached.
    $rows = mediamosa_db::db_count_rows(mediamosa_asset_supplement_db::TABLE_NAME, array(mediamosa_asset_supplement_db::ASSET_ID => $asset_id));

    if ($rows >= mediamosa_settings::MAX_SUPPLEMENT_PER_ASSET) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SUPPLEMENT_LIMIT_REACHED, array('@max' => mediamosa_settings::MAX_SUPPLEMENT_PER_ASSET));
    }

    // Get the asset.
    $a_asset = mediamosa_asset::get($asset_id);

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID]);

    // Create a new ID.
    $supplement_id = mediamosa_db::uuid($app_id);

    // Create new supplement.
    $fields = array(
      mediamosa_asset_supplement_db::ASSET_ID => $asset_id,
      mediamosa_asset_supplement_db::ID => $supplement_id,
    );

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_asset_supplement_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Return the new ID.
    return $supplement_id;
  }

  /**
   * Update a asset supplement.
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $user_id
   *  The user ID, must be owner of parent asset.
   * @param string $supplement_id
   *  The supplement ID to update.
   * @param string $oai_prefix
   * @param string $oai_namespaceuri
   * @param string $oai_schemalocation
   * @param string $supplement
   */
  static public function update($app_id, $user_id, $supplement_id, $oai_prefix, $oai_namespaceuri, $oai_schemalocation, $supplement) {

    // Supplement must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_supplement_db::TABLE_NAME, array(mediamosa_asset_supplement_db::ID => $supplement_id));

    // Get the supplement.
    $a_supplement = mediamosa_asset_supplement::get($supplement_id);

    // Get the asset.
    $a_asset = mediamosa_asset::get($a_supplement[mediamosa_asset_supplement_db::ASSET_ID]);

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID]);

    $a_fields = array();
    if (!is_null($oai_prefix)) {
      $a_fields[mediamosa_asset_supplement_db::OAI_PREFIX] = $oai_prefix;
    }
    if (!is_null($oai_namespaceuri)) {
      $a_fields[mediamosa_asset_supplement_db::OAI_NAMESPACEURI] = $oai_namespaceuri;
    }
    if (!is_null($oai_schemalocation)) {
      $a_fields[mediamosa_asset_supplement_db::OAI_SCHEMALOCATION] = $oai_schemalocation;
    }
    if (!is_null($supplement)) {
      $a_fields[mediamosa_asset_supplement_db::SUPPLEMENT] = $supplement;
    }

    // Must update something.
    if (!count($a_fields)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_CHANGES);
    }

    // Add changed value.
    $a_fields = mediamosa_db::db_update_enrich($a_fields);

    // Update set fields.
    mediamosa_db::db_update(mediamosa_asset_supplement_db::TABLE_NAME)
      ->fields($a_fields)
      ->condition(mediamosa_asset_supplement_db::ID, $supplement_id)
      ->execute();
  }

  /**
   * Delete the supplement.
   *
   * @param integer $app_id
   *  The application ID.
   * @param string $user_id
   *  Must be owner of the asset to delete supplement.
   * @param string $supplement_id
   *  The supplement ID.
   */
  static public function delete($app_id, $user_id, $supplement_id) {

    // Supplement must exist.
    mediamosa_db::db_must_exists(mediamosa_asset_supplement_db::TABLE_NAME, array(mediamosa_asset_supplement_db::ID => $supplement_id));

    // Get the supplement.
    $a_supplement = mediamosa_asset_supplement::get($supplement_id);

    // Get the asset.
    $a_asset = mediamosa_asset::get($a_supplement[mediamosa_asset_supplement_db::ASSET_ID]);

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $a_asset[mediamosa_asset_db::APP_ID], $a_asset[mediamosa_asset_db::OWNER_ID]);

    // Delete it.
    mediamosa_db::db_delete(mediamosa_asset_supplement_db::TABLE_NAME)
      ->condition(mediamosa_asset_supplement_db::ID, $supplement_id)
      ->execute();
  }
}
