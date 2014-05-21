<?php
/**
 * @file
 * Metadata functions.
 */
class mediamosa_asset_mediafile_metadata_property {
  /**
   * Get the property id.
   * @return int
   */
  public static function get_property_id_cache($prop_name, $type, $reset = FALSE) {
    static $prop_ids = NULL;

    if (is_null($prop_ids) || $reset) {
      $prop_ids = array();

      $result = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'p')
        ->fields('p', array(
          mediamosa_asset_mediafile_metadata_property_db::ID,
          mediamosa_asset_mediafile_metadata_property_db::NAME,
          mediamosa_asset_mediafile_metadata_property_db::TYPE,
        ))
        ->execute();

      foreach ($result as $row) {
        $prop_ids
          [$row[mediamosa_asset_mediafile_metadata_property_db::NAME]]
          [$row[mediamosa_asset_mediafile_metadata_property_db::TYPE]] =
          $row[mediamosa_asset_mediafile_metadata_property_db::ID];
      }
    }

    return (!empty($prop_ids[$prop_name][$type]) ? $prop_ids[$prop_name][$type] : NULL);
  }

  /**
   * Get the property id. Save the property, if needed.
   *
   * Warning: Not master-master save.
   *
   * @return int
   */
  public static function get_property_id($prop_name, $prop_type) {
    // Get the property id, if exists.
    $prop_id = self::get_property_id_cache($prop_name, $prop_type);

    if (is_null($prop_id) ) {
      $prop_id = self::get_property_id_cache($prop_name, $prop_type, TRUE);
    }

    if (is_null($prop_id) ) {
      if (!in_array(
        $prop_type,
        array(
          mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME,
          mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
          mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
        ))
      ) {
        // Shouldn't be here.
        throw new mediamosa_exception_program_error(strtr('Invalid mediafile metadata type (@prop_type).', array('@prop_type' => $prop_type)));
      }

      // Save the property id.
      $fields = array(
        'prop_name' => $prop_name,
        'type' => $prop_type,
      );

      // Add created/changed.
      $fields = mediamosa_db::db_insert_enrich($fields);

      $prop_id = mediamosa_db::db_insert(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME)
        ->fields($fields)
        ->execute();

      // Get the property id. It must exists.
      $prop_id = self::get_property_id_cache($prop_name, $prop_type, TRUE);
    }

    return $prop_id;
  }

  /**
   * Get the property id - CHAR.
   */
  public static function get_property_id_char($prop_name) {
    return self::get_property_id($prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);
  }

  /**
   * Get the property id - INT.
   */
  public static function get_property_id_int($prop_name) {
    return self::get_property_id($prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_INT);
  }

  /**
   * Get the property id - DATETIME.
   */
  public static function get_property_id_datetime($prop_name) {
    return self::get_property_id($prop_name, mediamosa_asset_mediafile_metadata_property_db::TYPE_DATETIME);
  }

  /**
   * Get the property.
   *
   * FIXME: caching here is not used (see usage of get_property()).
   *
   * @param string $prop_id
   *   The ID of the property to get.
   * @param bool $reset
   *   Reload static data.
   *
   * @return array
   *   Returns single array(prop_name, type)
   */
  public static function get_property($prop_id, $reset = FALSE) {
    static $props = NULL;

    if (is_null($props) || $reset) {
      $props = mediamosa_db::db_select(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'p')
        ->fields('p', array(
          mediamosa_asset_mediafile_metadata_property_db::ID,
          mediamosa_asset_mediafile_metadata_property_db::NAME,
          mediamosa_asset_mediafile_metadata_property_db::TYPE,
        ))
        ->execute()
        ->fetchAllAssoc(mediamosa_asset_mediafile_metadata_property_db::ID);
    }

    return (!empty($props[$prop_id]) ? $props[$prop_id] : NULL);
  }
}
