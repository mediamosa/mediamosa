<?php
/**
 * @file
 * The asset metadata module.
 */

class mediamosa_asset_metadata {
  // ------------------------------------------------------------------ Consts.
  const MATCH_TYPE_CONTAINS = 'contains';
  const MATCH_TYPE_EXACT = 'exact';
  const MATCH_TYPE_BEGIN = 'begin';
  const MATCH_TYPE_BOOLEAN = 'boolean';
  const MATCH_TYPE_PERIOD = 'period';
  const MATCH_TYPE_PERIOD_FROM = 'period_from';
  const MATCH_TYPE_PERIOD_TO = 'period_to';
  const MATCH_TYPE_RANGE = 'range';
  const MATCH_TYPE_RANGE_FROM = 'range_from';
  const MATCH_TYPE_RANGE_TO = 'range_to';

  // ---------------------------------------------------------------- Functions.
  /**
   * Return a list of property values and their count per given property.
   *
   * @param int $app_id
   *   The application ID.
   * @param int $prop_id
   *   The property ID.
   * @param int $offset
   *   The offset in the result set.
   * @param int $limit
   *   Maximum number of rows in result.
   */
  public static function tagcount($app_id, $prop_id, $offset, $limit) {
    return mediamosa_db::db_query_range(
      'SELECT COUNT(#val_char) AS tagcount, #val_char FROM {#mediamosa_asset} AS a
       INNER JOIN {#mediamosa_asset_metadata} AS ap ON a.#asset_id = ap.#asset_id
       WHERE ap.#prop_id = :prop_id AND a.#app_id = :app_id
       GROUP BY #val_char
       ORDER BY tagcount DESC',
      $offset,
      $limit,
      array(
        '#val_char' => mediamosa_asset_metadata_db::VAL_CHAR,
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_db::ID,
        '#prop_id' => mediamosa_asset_metadata_db::PROP_ID,
        ':prop_id' => $prop_id,
        '#app_id' => mediamosa_asset_db::APP_ID,
        ':app_id' => $app_id,
      )
    );
  }

  /**
   * Create metadata for asset.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param array $metadata_definitions_full
   *   The allowed metadata properties.
   * @param array $params
   *   The values for metadata array(name => value)
   * @param string $action
   *   The actions;
   *     - 'append': Append new metadata to existing metadata.
   *     - 'replace': Remove all metadata (of given definitions) and replace
   *                  with new.
   *     - 'update': Remove all metadata from single property and insert new.
   * @param bool $reindex
   *   Reindex the asset metadata.
   */
  public static function metadata_create($asset_id, array $metadata_definitions_full, array $params, $action, $reindex = TRUE) {
    $values_set = array();

    // Transaction in.
    $transaction = mediamosa_db::db_transaction();

    try {
      $prop_count = array();
      if ($action === 'append') {
        $prop_count = mediamosa_db::db_query('SELECT prop_id, COUNT(*) AS total FROM {mediamosa_asset_metadata} WHERE asset_id = :asset_id GROUP BY prop_id', array(':asset_id' => $asset_id))->fetchAllKeyed();
      }

      foreach ($metadata_definitions_full as $name => $definition) {
        $do_delete = FALSE;

        // Lets not use the alias.
        $prop_id = $definition['propdef_id'];

        // Based on action, set it.
        switch ($action) {
          case 'replace':
            if ($definition['propdef_is_protected'] !== mediamosa_asset_metadata_property_db::IS_PROTECTED_TRUE || ($definition['propdef_is_protected'] === mediamosa_asset_metadata_property_db::IS_PROTECTED_TRUE && isset($params[$name]))) {
              $do_delete = TRUE;
              break;
            }

          case 'append':
            break;

          case 'update':
            if (isset($params[$name])) {
              $do_delete = TRUE;
            }
            break;
        }

        if ($do_delete) {
          mediamosa_db::db_query(
            'DELETE FROM {#mediamosa_asset_metadata} WHERE #asset_id = :asset_id AND #prop_id = :prop_id',
            array(
              '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
              '#asset_id' => mediamosa_asset_metadata_db::ASSET_ID,
              ':asset_id' => $asset_id,
              '#prop_id' => mediamosa_asset_metadata_db::PROP_ID,
              ':prop_id' => $prop_id
            )
          );
          $prop_count[$prop_id] = 0;
        }

        if (!isset($params[$name])) {
          continue;
        }

        // Get the value.
        $values = $params[$name];

        if (!empty($definition['propdef_min_occurrences']) || !empty($definition['propdef_max_occurrences'])) {
          $value_count = count($values) + (isset($prop_count[$prop_id]) ? $prop_count[$prop_id] : 0);

          if (!empty($definition['propdef_min_occurrences']) && $value_count < $definition['propdef_min_occurrences']) {
            throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_METADATA_MIN_OCCURRENCE_FAILURE, array('@name' => $name, '@times' => $definition['propdef_min_occurrences']));
          }
          if (!empty($definition['propdef_min_occurrences']) && $value_count > $definition['propdef_max_occurrences']) {
            throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_METADATA_MAX_OCCURRENCE_FAILURE, array('@name' => $name, '@times' => $definition['propdef_max_occurrences']));
          }
        }

        foreach ($values as $value) {
          // We do not store empty or null strings.
          if (trim($value) === '' || $value === null) {
            continue;
          }

          $query = mediamosa_db::db_insert(mediamosa_asset_metadata_db::TABLE_NAME);
          $fields = array(
            mediamosa_asset_metadata_db::ASSET_ID => $asset_id,
            mediamosa_asset_metadata_db::PROP_ID => $prop_id,
          );

          // Add created value.
          $fields = mediamosa_db::db_insert_enrich($fields);

          switch ($definition['propdef_type']) {
            case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
              $fields[mediamosa_asset_metadata_db::VAL_DATETIME] = $value;
              break;

            case mediamosa_asset_metadata_property_db::TYPE_INT:
              $fields[mediamosa_asset_metadata_db::VAL_INT] = $value;
              break;

            case mediamosa_asset_metadata_property_db::TYPE_CHAR:
              $fields[mediamosa_asset_metadata_db::VAL_CHAR] = $value;
              $fields[mediamosa_asset_metadata_db::VAL_CHAR_LFT] = mediamosa_unicode::substr($value, 0, mediamosa_asset_metadata_db::VAL_CHAR_LFT_LENGTH);
              $fields[mediamosa_asset_metadata_db::VAL_CHAR_RGHT] = mediamosa_unicode::strrev(mediamosa_unicode::substr($value, 0, mediamosa_asset_metadata_db::VAL_CHAR_RGHT_LENGTH));
              break;

            default:
              assert(0);
              throw new mediamosa_exception_program_error('Unknown metadata type');
          }

          // Insert it.
          $query->fields($fields)->execute();

          $values_set[$name][] = $value;
        }
      }

      // Update de timestamps van de asset.
      mediamosa_asset::update_asset_timestamps($asset_id);
    }
    catch (Exception $e) {
      $transaction->rollback();
      throw $e;
    }

    // Set the empty asset.
    mediamosa_asset::update_asset_info_is_empty_asset($asset_id, FALSE);

    if ($reindex) {
      mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_METADATA);
    }

    // Return whats been changed.
    return $values_set;
  }

  /**
   * Collect the metadata of the asset.
   *
   * In array ->
   * [groupname][propertyname] = array(
   *   'values' => array(
   *     value,
   *     value,
   *   ),
   *   'type' => type,
   * ).
   *
   * @param string $asset_id
   *   The asset to get metadata from.
   * @param string $propgroup_name
   *   The name of the property group.
   * @param string $prop_name
   *   The property name.
   *
   * @return array
   *   The metadata of the asset.
   */
  public static function metadata_get($asset_id, $propgroup_name = NULL, $prop_name = NULL) {
    $sql = 'SELECT * FROM {#mediamosa_asset_metadata} AS am
       JOIN {#mediamosa_asset_metadata_property} AS amp ON amp.#prop_id = am.#prop_id
       JOIN {#mediamosa_asset_metadata_property_group} AS ampg ON ampg.#propgroup_id = amp.#propgroup_id
       WHERE am.#asset_id = :asset_id';
    $params = array(
      '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
      '#mediamosa_asset_metadata_property' => mediamosa_asset_metadata_property_db::TABLE_NAME,
      '#prop_id' => mediamosa_asset_metadata_property_db::ID,
      '#mediamosa_asset_metadata_property_group' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
      '#propgroup_id' => mediamosa_asset_metadata_property_group_db::ID,
      '#asset_id' => mediamosa_asset_metadata_db::ASSET_ID,
      ':asset_id' => $asset_id,
    );
    if ($propgroup_name) {
      $sql .= ' AND ampg.propgroup_name = :propgroup_name';
      $params[':propgroup_name'] = $propgroup_name;
    }
    if ($prop_name) {
      $sql .= ' AND amp.prop_name = :prop_name';
      $params[':prop_name'] = $prop_name;
    }
    $result = mediamosa_db::db_query($sql, $params);

    $result_metadata = array();
    foreach ($result as $metadata) {
      // Choose the value.
      $value = NULL;
      switch ($metadata[mediamosa_asset_metadata_property_db::TYPE]) {
        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
          $value = $metadata[mediamosa_asset_metadata_db::VAL_CHAR];
          break;

        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          $value = $metadata[mediamosa_asset_metadata_db::VAL_DATETIME];
          break;

        case mediamosa_asset_metadata_property_db::TYPE_INT:
          $value = $metadata[mediamosa_asset_metadata_db::VAL_INT];
          break;
      }

      // Collect the value.
      $result_metadata[$metadata[mediamosa_asset_metadata_property_group_db::NAME]][$metadata[mediamosa_asset_metadata_property_db::NAME]]['type'] = $metadata[mediamosa_asset_metadata_property_db::TYPE];
      $result_metadata[$metadata[mediamosa_asset_metadata_property_group_db::NAME]][$metadata[mediamosa_asset_metadata_property_db::NAME]]['values'][] = $value;
    }

    return $result_metadata;
  }

  /**
   * Collect the metadata of the asset of a single property.
   *
   * @param string $asset_id
   *   The asset id.
   * @param int $prop_id
   *   The property id.
   *
   * @return array
   *   The metadata property values.
   */
  public static function metadata_prop_get($asset_id, $prop_id) {
    $result = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_asset_metadata} AS am
       JOIN {#mediamosa_asset_metadata_property} AS amp ON amp.#prop_id = am.#prop_id
       WHERE am.#asset_id = :asset_id AND am.prop_id = :prop_id',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_metadata_db::ASSET_ID,
        '#prop_id' => mediamosa_asset_metadata_property_db::ID,
        ':asset_id' => $asset_id,
        ':prop_id' => $prop_id,
      )
    );

    $value = array();
    foreach ($result as $metadata) {
      switch ($metadata[mediamosa_asset_metadata_property_db::TYPE]) {
        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
          $value[] = $metadata[mediamosa_asset_metadata_db::VAL_CHAR];
          break;

        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          $value[] = $metadata[mediamosa_asset_metadata_db::VAL_DATETIME];
          break;

        case mediamosa_asset_metadata_property_db::TYPE_INT:
          $value[] = $metadata[mediamosa_asset_metadata_db::VAL_INT];
          break;
      }
    }
    return $value;
  }

  /**
   * Get metadata of the asset of a single property with a specific value.
   *
   * @param string $asset_id
   *   The asset id.
   * @param int $prop_id
   *   The property id.
   * @param int $value
   *   value.
   *
   * @return array
   *   The metadata property values.
   */
  public static function metadata_prop_value_get($asset_id, $prop_id, $value) {
    $result = mediamosa_db::db_query(
      'SELECT * FROM {#mediamosa_asset_metadata} AS am
       JOIN {#mediamosa_asset_metadata_property} AS amp ON amp.#prop_id = am.#prop_id
       WHERE am.#asset_id = :asset_id AND am.prop_id = :prop_id and am.val_char = :value',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#asset_id' => mediamosa_asset_metadata_db::ASSET_ID,
        '#prop_id' => mediamosa_asset_metadata_property_db::ID,
        ':asset_id' => $asset_id,
        ':prop_id' => $prop_id,
        ':value' => $value,
      )
    );

    $value = array();
    foreach ($result as $metadata) {
      switch ($metadata[mediamosa_asset_metadata_property_db::TYPE]) {
        case mediamosa_asset_metadata_property_db::TYPE_CHAR:
          $value[] = $metadata[mediamosa_asset_metadata_db::VAL_CHAR];
          break;

        case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
          $value[] = $metadata[mediamosa_asset_metadata_db::VAL_DATETIME];
          break;

        case mediamosa_asset_metadata_property_db::TYPE_INT:
          $value[] = $metadata[mediamosa_asset_metadata_db::VAL_INT];
          break;
      }
    }
    return $value;
  }


  /**
   * Deletes all metadata from a asset.
   *
   * @param string $asset_id
   *   The asset id of the asset to delete all metadata from.
   */
  public static function metadata_asset_delete($asset_id) {
    db_delete(mediamosa_asset_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_metadata_db::ASSET_ID, $asset_id)
      ->execute();
  }

  /**
   * Deletes all metadata from an asset of 1 property.
   *
   * @param string $asset_id
   *   asset id.
   * @param string $propgroup_name
   *   property group_name.
   * @param string $prop_name
   *   property name.
   */
  public static function metadata_asset_delete_prop($asset_id, $propgroup_name, $prop_name) {
    $prop_id = mediamosa_db::db_query(
      'SELECT prop_id FROM {#mediamosa_asset_metadata_property} AS amp
       JOIN {#mediamosa_asset_metadata_property_group} AS ampg ON ampg.propgroup_id = amp.propgroup_id
       WHERE ampg.propgroup_name = :propgroup_name AND amp.prop_name = :prop_name',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property' => mediamosa_asset_metadata_property_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property_group' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        ':propgroup_name' => $propgroup_name,
        ':prop_name' => $prop_name,
      )
    )->fetchField();

    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_asset_metadata} WHERE prop_id = :prop_id and asset_id = :asset_id',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        ':prop_id' => $prop_id,
        ':asset_id' => $asset_id,
      )
    );
  }

  /**
   * Deletes all properties with given prop_name and val_char value.
   */
  public static function metadata_asset_delete_prop_value($propgroup_name, $prop_name, $value) {
    $prop_id = mediamosa_db::db_query(
      'SELECT prop_id FROM {#mediamosa_asset_metadata_property} AS amp
       JOIN {#mediamosa_asset_metadata_property_group} AS ampg ON ampg.propgroup_id = amp.propgroup_id
       WHERE ampg.propgroup_name = :propgroup_name AND amp.prop_name = :prop_name',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property' => mediamosa_asset_metadata_property_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property_group' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        ':propgroup_name' => $propgroup_name,
        ':prop_name' => $prop_name,
      )
    )->fetchField();

    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_asset_metadata} WHERE prop_id = :prop_id and val_char = :value',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        ':prop_id' => $prop_id,
        ':value' => $value,
      )
    );
  }

  /**
   * Delete properties triplet: prop_name and val_char value of a single asset.
   */
  public static function metadata_asset_delete_asset_prop_value($asset_id, $propgroup_name, $prop_name, $value) {
    $prop_id = mediamosa_db::db_query(
      'SELECT prop_id FROM {#mediamosa_asset_metadata_property} AS amp
       JOIN {#mediamosa_asset_metadata_property_group} AS ampg ON ampg.propgroup_id = amp.propgroup_id
       WHERE ampg.propgroup_name = :propgroup_name AND amp.prop_name = :prop_name',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property' => mediamosa_asset_metadata_property_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property_group' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        ':propgroup_name' => $propgroup_name,
        ':prop_name' => $prop_name,
      )
    )->fetchField();

    mediamosa_db::db_query(
      'DELETE FROM {#mediamosa_asset_metadata} WHERE asset_id = :asset_id and prop_id = :prop_id and val_char = :value',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        ':prop_id' => $prop_id,
        ':value' => $value,
        ':asset_id' => $asset_id,
      )
    );
  }

  /**
   * Copy the metadata of the asset to another asset.
   *
   * @param string $from_asset_id
   *   The source asset id.
   * @param string $to_asset_id
   *   The destination asset id.
   * @param bool $clear_metadata
   *   (default: TRUE) Clear metadata on destination before copying. Some
   *   properties must be removed when copying from other asset as some
   *   properties can not exists more than once on the asset (published,
   *   trashcan).
   */
  public static function metadata_copy($from_asset_id, $to_asset_id, $clear_metadata = TRUE) {
    if ($clear_metadata) {
      static::metadata_asset_delete($to_asset_id);
    }

    mediamosa_db::db_query('INSERT INTO {#mediamosa_asset_metadata} ' .
      '(asset_id, prop_id, val_char, val_char_lft, val_char_rght, val_datetime, val_int, created, changed) ' .
      'SELECT :to_asset_id, prop_id, val_char, val_char_lft, val_char_rght, val_datetime, val_int, created, changed ' .
      'FROM {#mediamosa_asset_metadata} WHERE asset_id = :from_asset_id',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        ':to_asset_id' => $to_asset_id,
        ':from_asset_id' => $from_asset_id,
      )
    );
  }

  /**
   * Search for a val_char value.
   *
   * @param string $propgroup_name
   *   property groupname.
   * @param string $prop_name
   *   property name.
   * @param string $value
   *   val_char value.
   */
  public static function metadata_search_val_char($propgroup_name, $prop_name, $value) {
    $result = mediamosa_db::db_query(
      'SELECT asset_id FROM {#mediamosa_asset_metadata} AS am
       JOIN {#mediamosa_asset_metadata_property} AS amp ON amp.prop_id = am.prop_id
       JOIN {#mediamosa_asset_metadata_property_group} AS ampg ON ampg.propgroup_id = amp.propgroup_id
       WHERE ampg.propgroup_name = :propgroup_name AND amp.prop_name = :prop_name and am.val_char = :value',
      array(
        '#mediamosa_asset_metadata' => mediamosa_asset_metadata_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property' => mediamosa_asset_metadata_property_db::TABLE_NAME,
        '#mediamosa_asset_metadata_property_group' => mediamosa_asset_metadata_property_group_db::TABLE_NAME,
        ':propgroup_name' => $propgroup_name,
        ':prop_name' => $prop_name,
        ':value' => $value,
      )
    )->fetchAssoc();
    return $result;
  }

  /**
   * Create or replace metadata specific for asset set.
   *
   * @param string $asset_id
   *   The ID of the asset.
   * @param array $params
   *   The name => key value pairs.
   * @param bool $reindex
   *   Reindex the asset metadata.
   *
   * @return array
   *   The set values.
   */
  public static function metadata_asset_create($asset_id, $params, $reindex = TRUE) {
    return static::metadata_create(
      $asset_id,
      mediamosa_asset_metadata_property::get_metadata_properties_full(FALSE, array('asset')),
      $params,
      'update',
      $reindex
    );
  }
}
