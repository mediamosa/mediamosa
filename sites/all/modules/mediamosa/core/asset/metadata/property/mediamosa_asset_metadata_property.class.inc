<?php
/**
 * @file
 * The function used for asset media metadata property.
 */

class mediamosa_asset_metadata_property {

  // ------------------------------------------------------------------ Static Function.
  public static function property_list($app_ids, $include_default_definitions, $name, $offset = 0, $limit = mediamosa_settings::LIMIT_MAX, $order_by = '', $order_direction = mediamosa_type::ORDER_DIRECTION_ASC) {

    // Make sure these are used.
    assert(in_array($order_direction, array(mediamosa_type::ORDER_DIRECTION_ASC, mediamosa_type::ORDER_DIRECTION_DESC)));

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
      $app_ids[] = 0;
    }

    // Must be from these app(s).
    $query->condition(mediamosa_asset_metadata_property_db::APP_ID, array_unique($app_ids), 'IN');
    if ($name != '') {
      $query->condition(mediamosa_asset_metadata_property_db::NAME, $name, '=');
    }

    // Range.
    $query->range($offset, $limit);

    if (!empty($order_by)) {
      $query->orderBy($order_by, $order_direction);
    }

    return $query->execute();
  }

  /**
   * Create metadata property for client app.
   *
   * @param integer $app_id
   *  Application ID.
   * @param string $name
   *  Name of the property.
   * @param string $type
   *  char, datetime, int
   * @param integer $min_occurences (reserved)
   * @param integer $max_occurences (reserved)
   * @param bool $is_hidden
   *
   * @return
   *  The new prop_id.
   */
  public static function property_create($app_id, $name, $type, $min_occurences = 0, $max_occurences = 0, $is_hidden = mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE) {

    // No longer accept array (3.0 change).
    assert(!is_array($app_id));

    // Get the propgroups.
    $property_groups = mediamosa_asset_metadata_property_group::property_group_row_get_with_appid($app_id);

    // If its not exists, then create.
    if (!$property_groups) {
      $propgroup_id = mediamosa_asset_metadata_property_group::property_group_create_with_appid($app_id);
    }
    else {
      $propgroup_id = $property_groups[mediamosa_asset_metadata_property_group_db::ID];
    }

    // Should be non-zero.
    assert($propgroup_id);

    // Make sure the property group does not exists yet (own app).
    if ($app_id && mediamosa_db::db_exists(
      mediamosa_asset_metadata_property_db::TABLE_NAME,
      array(
        mediamosa_asset_metadata_property_db::NAME => $name,
        mediamosa_asset_metadata_property_db::APP_ID => $app_id,
      )
    )) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_METADATA_DEFINITION_ALREADY_EXISTS);
    }

    // Make sure the property group does not exists yet (global app).
    if (mediamosa_db::db_exists(
      mediamosa_asset_metadata_property_db::TABLE_NAME,
      array(
        mediamosa_asset_metadata_property_db::NAME => $name,
        mediamosa_asset_metadata_property_db::APP_ID => 0,
      )
    )) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_METADATA_DEFINITION_ALREADY_EXISTS);
    }

    $fields = array(
      mediamosa_asset_metadata_property_db::NAME => $name,
      mediamosa_asset_metadata_property_db::TYPE => mediamosa_unicode::strtoupper($type),
      mediamosa_asset_metadata_property_db::PROPGROUP_ID => $propgroup_id,
      mediamosa_asset_metadata_property_db::APP_ID => $app_id,
      mediamosa_asset_metadata_property_db::MAX_OCCURENCES => $max_occurences,
      mediamosa_asset_metadata_property_db::MIN_OCCURENCES => $min_occurences,
      mediamosa_asset_metadata_property_db::IS_HIDDEN => $is_hidden,
    );

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    return mediamosa_db::db_insert(mediamosa_asset_metadata_property_db::TABLE_NAME)
      ->fields($fields)
      ->execute();
  }

  /**
   * Delete a metadata property.
   *
   * @param array/integer $mixed_app_ids
   * @param string $name
   * @param string $delete
   */
  public static function property_delete($app_id, $name, $delete) {

    // No longer accept array (3.0 change).
    assert(!is_array($app_id));

    $property_groups = mediamosa_asset_metadata_property_group::property_group_row_get_with_appid($app_id);
    if (!$property_groups) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_METADATA_DEFINITION);
    }

    // Get the propgroup_id
    $propgroup_id = $property_groups[mediamosa_asset_metadata_property_group_db::ID];
    assert($propgroup_id);

    // Get the property.
    $prop = self::property_get_row_with_name($name, $propgroup_id);
    if (!$prop) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_METADATA_DEFINITION);
    }

    // Get the prop ID.
    $prop_id = $prop[mediamosa_asset_metadata_property_db::ID];
    assert($prop_id);

    if ($delete != 'cascade') {
      if (mediamosa_db::db_exists(mediamosa_asset_metadata_db::TABLE_NAME, array(mediamosa_asset_metadata_db::PROP_ID => $prop_id))) {
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
  public static function property_get_row($prop_id) {
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
  public static function property_get_row_with_name($name, $propgroup_id) {
    return mediamosa_db::db_select(mediamosa_asset_metadata_property_db::TABLE_NAME, 'amp')
      ->fields('amp')
      ->condition(mediamosa_asset_metadata_property_db::PROPGROUP_ID, $propgroup_id, '=')
      ->condition(mediamosa_asset_metadata_property_db::NAME, $name, '=')
      ->execute()->fetchAssoc();
  }

  /**
   * Get definitions from the asset_property_definitions array
   *
   * @param array $metadata_property_groups
   * @param bool $get_all_aps
   *
   * @return array
   */
  public static function get_metadata_properties(array $metadata_property_groups = array(), $get_all_apps = FALSE) {

    // Caching.
    static $metadata_properties = array();

    // Check if its in our cache.
    $key = serialize($metadata_property_groups);
    if (isset($metadata_properties[$key])) {
      return $metadata_properties[$key];
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
    $query->condition(mediamosa_asset_metadata_property_group_db::NAME, $metadata_property_groups, 'IN');

    // Execute.
    $result = $query->execute();

    // Build output.
    $metadata_properties[$key] = array();
    foreach ($result as $row) {
      $metadata_properties[$key][$row['propdef_name']] = $row;
    }

    // Return our result.
    return $metadata_properties[$key];
  }

  /**
   * Get all metadata properties, plus our own based on the app_id
   *
   * @param mixed $mixed_app_id
   * @param array $metadata_property_groups
   *
   * @return array
   */
  public static function get_metadata_properties_full($mixed_app_id = FALSE, array $metadata_property_groups = array()) {

    // Get all metadata property groups.
    if (empty($metadata_property_groups)) {
      foreach (module_implements('mediamosa_metadata_info') as $module) {
        $metadata_property_groups = array_merge($metadata_property_groups, array_keys(module_invoke($module, 'mediamosa_metadata_info')));
      }
    }

    if ($mixed_app_id) {
      $app_ids = is_array($mixed_app_id) ? $mixed_app_id : array($mixed_app_id);

      foreach ($app_ids as $app_id) {
        $metadata_property_groups[] = sprintf('app_%d', $app_id);
      }
    }

    return mediamosa_asset_metadata_property::get_metadata_properties($metadata_property_groups);
  }

  /**
   * Get all metadata.
   */
  public static function get_all_metadata() {

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
