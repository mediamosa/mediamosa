<?php
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
   * @param int $property_group_id (unique)
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
   *   The name of the group.
   *
   * @return array
   *   The found propery group or FALSE.
   */
  public static function property_group_row_get_with_name($property_group_name) {
    static $property_groups = array();

    if (!isset($property_groups[$property_group_name])) {
      $property_groups[$property_group_name] = db_select(mediamosa_asset_metadata_property_group_db::TABLE_NAME, 'ampg')
        ->fields('ampg')
        ->condition(mediamosa_asset_metadata_property_group_db::NAME, $property_group_name)
        ->execute()
        ->fetchAssoc();
    }

    return $property_groups[$property_group_name];
  }

  /**
   * Get a property group by app_id.
   *
   * @param int $app_id
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
   * @param int $app_id
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
   * @return bool
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
   * @param int $property_group_id
   *   The parent property group ID.
   * @param string $property_name
   *   The name of the property.
   * @param string $type
   *   The type of the propert, CHAR, DATETIME or INT.
   * @param array $options
   *  An array of options;
   *   - min_occurences: minimum of occurences as data (reserved).
   *   - max_occurences: maximum of occurences as data (reserved).
   *   - is_hidden: hide metadata when generating output.
   *   - is_protected: metadata can not be removed.
   */
  public static function property_create($property_group_id, $property_name, $type, array $options = array()) {
    // Default options.
    $options += array(
      'min_occurences' => 0,
      'max_occurences' => 0,
      'is_hidden' => mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE,
      'is_protected' => mediamosa_asset_metadata_property_db::IS_PROTECTED_FALSE,
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
      mediamosa_asset_metadata_property_db::IS_PROTECTED => $options['is_protected'],
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

  /**
   * Returns the short version of the metadata group name.
   *
   * @param string $context_full
   *   The group name to convert.
   */
  public static function property_group_get_context($context_full) {

    $metadata_property_groups = &drupal_static('mediamosa_metadata_property_groups');

    if (empty($metadata_property_groups)) {
      $metadata_property_groups = module_invoke_all('mediamosa_metadata_info');
    }

    foreach ($metadata_property_groups as $metadata_property_group) {
      if ($metadata_property_group['context_full'] === $context_full) {
        return $metadata_property_group['context'];
      }
    }

    // Not found? Return default.
    return $context_full;
  }

  /**
   * Returns the full version of the metadata group name.
   *
   * @param string $context
   *   The group name to convert.
   */
  public static function property_group_get_context_full($context) {

    $metadata_property_groups = &drupal_static('mediamosa_metadata_property_groups');
    if (empty($metadata_property_groups)) {
      $metadata_property_groups = module_invoke_all('mediamosa_metadata_info');
    }

    foreach ($metadata_property_groups as $metadata_property_group) {
      if ($metadata_property_group['context'] === $context) {
        return $metadata_property_group['context_full'];
      }
    }

    // Not found? Return default.
    return $context;
  }
}
