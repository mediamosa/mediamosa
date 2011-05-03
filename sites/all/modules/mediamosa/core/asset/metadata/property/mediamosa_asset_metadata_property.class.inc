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
  * The function used for asset media metadata property.
  */

class mediamosa_asset_metadata_property {

  // ------------------------------------------------------------------ Static Function.
    // 1.x name: media_management_metadata_definition_get (REST).
  static public function property_list($a_app_ids, $include_default_definitions, $name, $offset = 0, $limit = mediamosa_settings::LIMIT_MAX, $order_by = '', $order_direction = mediamosa_type::ORDER_DIRECTION_ASC) {

    // Build the select object.
    $query = mediamosa_db::db_select(mediamosa_asset_metadata_property_db::TABLE_NAME, 'amp');
    $query->join(
      mediamosa_asset_metadata_property_group_db::TABLE_NAME,
      'ampg',
      strtr('amp.#amp_propgroup_id = ampg.#ampg_propgroup_id',
        array(
          '#amp_propgroup_id' => mediamosa_asset_metadata_property_db::PROPGROUP_ID,
          '#ampg_propgroup_id' => mediamosa_asset_metadata_property_group_db::ID,
        )
      )
    );

    // The fields we need to get, amp.* and ampg.propgroup_name.
    $query->fields('amp');
    $query->fields('ampg', array(mediamosa_asset_metadata_property_group_db::NAME));

    if ($include_default_definitions) {
      $a_app_ids[] = 0;
    }

    // Must be from these app(s).
    $query->condition(mediamosa_rest_call::APP_ID, array_unique($a_app_ids), 'IN');
    if ($name != '') {
      $query->condition(mediamosa_asset_metadata_property_db::NAME, $name, '=');
    }

    // Range.
    $query->range($offset, $limit);

    if ($order_by != '') {
      $query->orderBy($order_by, $order_direction);
    }

    return $query->execute();
  }

  /**
   * Create property.
   *
   * 1.x: media_management_metadata_definition_create (REST).
   *
   * @param array/integer $mixed_app_ids
   *  Application ID.
   * @param string $name
   *  Name of the property.
   * @param string $type
   *  char, datetime, int
   * @param integer $min_occurences
   * @param integer $max_occurences
   * @param bool $is_hidden
   *
   * @return
   *  The new prop_id.
   */
  static public function property_create($mixed_app_ids, $name, $type, $min_occurences = 0, $max_occurences = 0, $is_hidden = mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE) {

    // First app_id is always the app of the eua.
    $app_id = is_array($mixed_app_ids) ? reset($mixed_app_ids) : $mixed_app_ids;

    // Get the propgroups.
    $a_propgroup = mediamosa_asset_metadata_property_group::property_group_row_get_with_appid($app_id);

    // If its not exists, then create.
    if (!$a_propgroup) {
      $propgroup_id = mediamosa_asset_metadata_property_group::property_group_create_with_appid($app_id);
    }
    else {
      $propgroup_id = $a_propgroup[mediamosa_asset_metadata_property_group_db::ID];
    }

    // Should be non-zero.
    assert($propgroup_id);

    // Make sure the probgroup does not exists yet (own app)
    if ($app_id && mediamosa_db::db_exists(
      mediamosa_asset_metadata_property_db::TABLE_NAME,
      array(
        mediamosa_asset_metadata_property_db::NAME => $name,
        mediamosa_asset_metadata_property_db::APP_ID => $app_id,
      )
    )) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_METADATA_DEFINITION_ALREADY_EXISTS);
    }

    // Make sure the probgroup does not exists yet (global app)
    if (mediamosa_db::db_exists(
      mediamosa_asset_metadata_property_db::TABLE_NAME,
      array(
        mediamosa_asset_metadata_property_db::NAME => $name,
        mediamosa_asset_metadata_property_db::APP_ID => 0,
      )
    )) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_METADATA_DEFINITION_ALREADY_EXISTS);
    }

    $a_fields = array(
      mediamosa_asset_metadata_property_db::NAME => $name,
      mediamosa_asset_metadata_property_db::TYPE => mediamosa_unicode::strtoupper($type),
      mediamosa_asset_metadata_property_db::PROPGROUP_ID => $propgroup_id,
      mediamosa_asset_metadata_property_db::APP_ID => $app_id,
      mediamosa_asset_metadata_property_db::MAX_OCCURENCES => $max_occurences,
      mediamosa_asset_metadata_property_db::MIN_OCCURENCES => $min_occurences,
      mediamosa_asset_metadata_property_db::IS_HIDDEN => $is_hidden,
    );

    // Add created value.
    $a_fields = mediamosa_db::db_insert_enrich($a_fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_asset_metadata_property_db::TABLE_NAME)
      ->fields($a_fields)
      ->execute();
  }

  /**
   * Delete a metadata property.
   *
   * @param array/integer $mixed_app_ids
   * @param string $name
   * @param string $delete
   */
  // 1.x name: media_management_metadata_definition_delete (REST).
  static public function property_delete($mixed_app_ids, $name, $delete) {

    // First app_id is always the app of the eua.
    $app_id = is_array($mixed_app_ids) ? reset($mixed_app_ids) : $mixed_app_ids;

    $a_propgroup = mediamosa_asset_metadata_property_group::property_group_row_get_with_appid($app_id);
    if (!$a_propgroup) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_METADATA_DEFINITION);
    }

    // Get the propgroup_id
    $propgroup_id = $a_propgroup[mediamosa_asset_metadata_property_group_db::ID];
    assert($propgroup_id);

    // Get the property.
    $a_prop = self::property_get_row_with_name($name, $propgroup_id);
    if (!$a_prop) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_METADATA_DEFINITION);
    }

    // Get the prop ID.
    $prop_id = $a_prop[mediamosa_asset_metadata_property_db::ID];
    assert($prop_id);

    if ($delete != 'cascade') {
      if (!mediamosa_db::db_exists(mediamosa_asset_metadata_db::TABLE_NAME, array(mediamosa_asset_metadata_db::ID => $prop_id))) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_METADATA_DEFINITION_NOT_EMPTY);
      }
    }

    // Remove all metadata linked to this property.
    mediamosa_db::db_delete(mediamosa_asset_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_metadata_db::PROP_ID, $prop_id, '=')
      ->execute();

    // Remove all property.
    mediamosa_db::db_delete(mediamosa_asset_metadata_property_db::TABLE_NAME)
      ->condition(mediamosa_asset_metadata_property_db::ID, $prop_id, '=')
      ->execute();
}

  /**
   * Get the asset metadata property from the database.
   *
   * @param integer $prop_id
   * @return array
   */
  static public function property_get_row($prop_id) {
    return mediamosa_db::db_select(mediamosa_asset_metadata_property_db::TABLE_NAME, 'amp')
      ->fields('amp')
      ->condition(mediamosa_asset_metadata_property_db::ID, $prop_id, '=')
      ->execute()->fetchAssoc();
  }

  /**
   * Get the asset metadata property from the database.
   *
   * @param string $name
   *  Name of the property.
   * @param integer $propgroup_id
   *  The ID of the propertygroup.
   * @return array
   */
  static public function property_get_row_with_name($name, $propgroup_id) {
    return mediamosa_db::db_select(mediamosa_asset_metadata_property_db::TABLE_NAME, 'amp')
      ->fields('amp')
      ->condition(mediamosa_asset_metadata_property_db::PROPGROUP_ID, $propgroup_id, '=')
      ->condition(mediamosa_asset_metadata_property_db::NAME, $name, '=')
      ->execute()->fetchAssoc();
  }

  /**
   * Get definitions from the asset_property_definitions array
   *
   * @param array $a_propgroup_name
   * @param bool $get_all_aps
   *
   * @return array
   */
  static public function get_metadata_properties(array $a_propgroup_name = array('dublin_core'), $get_all_apps = FALSE) {

    // Caching.
    static $a_metadata_properties = array();

    // Check if its in our cache.
    $key = serialize($a_propgroup_name);
    if (isset($a_metadata_properties[$key])) {
      return $a_metadata_properties[$key];
    }

    // Build the select query object.
    $query = mediamosa_db::db_select(mediamosa_asset_metadata_property_group_db::TABLE_NAME, 'ampg');

    // Add an join.
    $query->join(
      mediamosa_asset_metadata_property_db::TABLE_NAME,
      'amp',
      strtr(
        'ampg.#ampg_propgroup_id = amp.#amp_propgroup_id',
        array(
          '#ampg_propgroup_id' => mediamosa_asset_metadata_property_group_db::ID,
          '#amp_propgroup_id' => mediamosa_asset_metadata_property_db::PROPGROUP_ID,
        )
      )
    );

    // Fields to select.
    $query->addField('ampg', mediamosa_asset_metadata_property_group_db::ID);
    $query->addField('ampg', mediamosa_asset_metadata_property_group_db::NAME, 'propgroup_name');
    $query->addField('amp', mediamosa_asset_metadata_property_db::TYPE, 'propdef_type');
    $query->addField('amp', mediamosa_asset_metadata_property_db::ID, 'propdef_id');
    $query->addField('amp', mediamosa_asset_metadata_property_db::NAME, 'propdef_name');
    $query->addField('amp', mediamosa_asset_metadata_property_db::IS_HIDDEN, 'propdef_is_hidden');

    // Now where to search.
    $query->condition(mediamosa_asset_metadata_property_group_db::NAME, $a_propgroup_name, 'IN');

    // Execute.
    $result = $query->execute();

    // Build output.
    $a_metadata_properties[$key] = array();
    foreach ($result as $a_row) {
      $a_metadata_properties[$key][$a_row['propdef_name']] = $a_row;
    }

    // Return our result.
    return $a_metadata_properties[$key];
  }

  /**
   * Get all metadata properties, plus our own based on the app_id
   *
   * 1.x _media_management_get_metadata_definitions_full
   *
   * @param mixed $mixed_app_id
   * @param array $a_definitions_full
   *
   * @return array
   */
  static public function get_metadata_properties_full($mixed_app_id = FALSE, array $a_definitions_full = array('dublin_core', 'qualified_dublin_core', 'czp')) {
    if ($mixed_app_id) {
      $a_app_ids = is_array($mixed_app_id) ? $mixed_app_id : array($mixed_app_id);

      foreach ($a_app_ids as $app_id) {
        $a_definitions_full[] = sprintf('app_%d', $app_id);
      }
    }

    return mediamosa_asset_metadata_property::get_metadata_properties($a_definitions_full);
  }

  /**
   * Get all metadata.
   */
  static public function get_all_metadata() {

    // Caching.
    static $all_metadata_properties = array();

    // Check if its in our cache.
    if (!empty($all_metadata_properties)) {
      return $all_metadata_properties;
    }

    // Build the select query object.
    $query = mediamosa_db::db_select(mediamosa_asset_metadata_property_group_db::TABLE_NAME, 'ampg');

    // Add an join.
    $query->join(
      mediamosa_asset_metadata_property_db::TABLE_NAME,
      'amp',
      strtr(
        'ampg.#ampg_propgroup_id = amp.#amp_propgroup_id',
        array(
          '#ampg_propgroup_id' => mediamosa_asset_metadata_property_group_db::ID,
          '#amp_propgroup_id' => mediamosa_asset_metadata_property_db::PROPGROUP_ID,
        )
      )
    );

    // Fields to select.
    $query->addField('ampg', mediamosa_asset_metadata_property_group_db::ID);
    $query->addField('ampg', mediamosa_asset_metadata_property_group_db::NAME, 'propgroup_name');
    $query->addField('amp', mediamosa_asset_metadata_property_db::TYPE, 'propdef_type');
    $query->addField('amp', mediamosa_asset_metadata_property_db::ID, 'propdef_id');
    $query->addField('amp', mediamosa_asset_metadata_property_db::NAME, 'propdef_name');
    $query->addField('amp', mediamosa_asset_metadata_property_db::IS_HIDDEN, 'propdef_is_hidden');

    // Order by.
    $query->orderBy('propdef_name', 'ASC');

    // Execute.
    $result = $query->execute();

    // Build output.
    $all_metadata_properties = array();
    foreach ($result as $row) {
      $all_metadata_properties[$row['propdef_name']] = $row;
    }

    // Return our result.
    return $all_metadata_properties;
  }
}
