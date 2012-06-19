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
  * The database class of the asset metadata property group module.
  */

class mediamosa_asset_metadata_property_group {

  // ------------------------------------------------------------------ Consts.
  const APP_PREFIX = 'app_';

  // ------------------------------------------------------------------ Functions (public).
  /**
   * Get a property group.
   *
   * @param integer $property_group_id (unique)
   * @return array
   */
  public static function property_group_row_get($property_group_id) {
    return mediamosa_db::db_query(
      'SELECT * FROM {#table_name} WHERE #propgroup_id = :propgroup_id',
      array(
        '#table_name' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        '#propgroup_id' => mediamosa_asset_metadata_property_group_db::ID,
        ':propgroup_id' => $property_group_id,
      )
    )->fetchAssoc();
  }

  /**
   * Get a property group by name.
   *
   * @param string $property_group_name
   * @return array
   */
  public static function property_group_row_get_with_name($property_group_name) {
    return mediamosa_db::db_query(
      'SELECT * FROM {#table_name} WHERE #propgroup_name = :propgroup_name',
      array(
        '#table_name' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        '#propgroup_name' => mediamosa_asset_metadata_property_group_db::NAME,
        ':propgroup_name' => $property_group_name,
      )
    )->fetchAssoc();
  }

  /**
   * Get a property group by app_id.
   *
   * @param integer $app_id
   * @return array
   */
  public static function property_group_row_get_with_appid($app_id) {
    return mediamosa_db::db_query(
      'SELECT * FROM {#table_name} WHERE #propgroup_id = :propgroup_id',
      array(
        '#table_name' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        '#propgroup_id' => mediamosa_asset_metadata_property_group_db::NAME,
        ':propgroup_id' => self::APP_PREFIX . (int) $app_id,
      )
    )->fetchAssoc();
  }

  /**
   * Create a property group.
   *
   * @param string $name
   *  The name of the property group.
   * @return integer
   *  The new ID of the object.
   */
  public static function property_group_create($name) {
    $fields = array(
      mediamosa_asset_metadata_property_group_db::NAME => $name,
    );

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_asset_metadata_property_group_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Create a property group for application.
   *
   * @param integer $app_id
   *  The name of the property group.
   *
   * @return integer
   *  The new ID of the object.
   */
  public static function property_group_create_with_appid($app_id) {
    return mediamosa_asset_metadata_property_group::property_group_create(self::APP_PREFIX . (int) $app_id);
  }

  /**
   * Returns of property group already exists.
   *
   * @param string $property_group_name
   *
   * @return boolean
   *  TRUE / FALSE
   */
  public static function property_group_installed($property_group_name) {
    return mediamosa_db::db_exists(
      mediamosa_asset_metadata_property_group_db::TABLE_NAME,
      array(mediamosa_asset_metadata_property_group_db::NAME => $property_group_name)
    );
  }

  /**
   * Create global property.
   *
   * @param integer $property_group_id
   * @param string $property_name
   * @param string $type
   * @param array $options
   *  An array of options;
   *   - min_occurences: minimum of occurences as data (reserved).
   *   - max_occurences: maximum of occurences as data (reserved).
   *   - is_hidden: hide metadata when generating output.
   *
   */
  public static function property_create($property_group_id, $property_name, $type, array $options = array()) {
    // Default options.
    $options += array(
      'min_occurences' => 0,
      'max_occurences' => 0,
      'is_hidden' => mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE,
      'app_id' => 0,
    );

    $fields = array(
      mediamosa_asset_metadata_property_db::NAME => $property_name,
      mediamosa_asset_metadata_property_db::TYPE => mediamosa_unicode::strtoupper($type),
      mediamosa_asset_metadata_property_db::PROPGROUP_ID => $property_group_id,
      mediamosa_asset_metadata_property_db::APP_ID => $options['app_id'],
      mediamosa_asset_metadata_property_db::MAX_OCCURENCES => $options['max_occurences'],
      mediamosa_asset_metadata_property_db::MIN_OCCURENCES => $options['min_occurences'],
      mediamosa_asset_metadata_property_db::IS_HIDDEN => $options['is_hidden'],
    );

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_asset_metadata_property_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Delete the metadata group.
   *
   * Warning; will delete all metadata of the group.
   *
   * @param string $property_name
   */
  public static function property_group_delete($property_name) {

    // Get the group.
    $property_group = self::property_group_row_get_with_name($property_name);
    if (empty($property_group)) {
      return;
    }

    // Get the group id.
    $property_group_id = $property_group[mediamosa_asset_metadata_property_group_db::ID];
    assert($property_group_id);

    // Get all prop_ids.
    $prop_ids = mediamosa_db::db_select(mediamosa_asset_metadata_property_db::TABLE_NAME, 'amp')
      ->fields('amp', array(mediamosa_asset_metadata_property_db::PROPGROUP_ID))
      ->condition(mediamosa_asset_metadata_property_db::PROPGROUP_ID, $property_group_id)
      ->execute()->fetchCol();

    if (!empty($prop_ids)) {
      // Delete all metadata of this property group.
      mediamosa_db::db_delete(mediamosa_asset_metadata_db::TABLE_NAME)
        ->condition(mediamosa_asset_metadata_db::PROP_ID, $prop_ids, 'IN')
        ->execute();
    }

    // Delete all properties.
    mediamosa_db::db_delete(mediamosa_asset_metadata_property_db::TABLE_NAME)
      ->condition(mediamosa_asset_metadata_property_db::PROPGROUP_ID, $property_group_id)
      ->execute();

    // Delete the property group.
    mediamosa_db::db_delete(mediamosa_asset_metadata_property_group_db::TABLE_NAME)
      ->condition(mediamosa_asset_metadata_property_group_db::ID, $property_group_id)
      ->execute();
  }
}
