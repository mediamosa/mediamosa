<?php
/**
 * @file
 * The helper functions of asset module.
 */

class mediamosa_asset {
  // ------------------------------------------------------------------- Consts.
  const METADATA_PUBLISHED = 'published';
  const METADATA_PUBLISHED_TRUE = 'TRUE';
  const METADATA_PUBLISHED_FALSE = 'FALSE';
  const METADATA_TRASHCAN = 'trashcan';
  const METADATA_TRASHCAN_TRUE = 'TRUE';
  const METADATA_TRASHCAN_FALSE = 'FALSE';

  // ---------------------------------------------------------------- Functions.
  /**
   * Preprocess the asset for output.
   *
   * @param array $asset
   *   The asset to preprocess.
   * @param array $metadata_definitions_fulls
   *   The parameters to clear.
   *
   * @return array
   *   The processed asset.
   */
  public static function process_asset_output(array $asset, array $metadata_definitions_fulls) {
    // Clear out these.
    foreach ($metadata_definitions_fulls as $key => $value) {
      $asset[$value['propgroup_name']][$key] = NULL;
    }

    // Unset these values, to skip out of output.
    unset(
      $asset['parent_id'],
      $asset['asset_property_value'],
      $asset['asset_property_name'],
      $asset['asset_property_group_name'],
      $asset['asset_property_type'],
      $asset['asset_property_val_char'],
      $asset['asset_property_val_int'],
      $asset['asset_property_val_datetime'],
      $asset['asset_property_is_hidden']
    );

    // Convert dates to local timezone.
    if (isset($asset[mediamosa_asset_db::VIDEOTIMESTAMP])) {
      $asset[mediamosa_asset_db::VIDEOTIMESTAMP] = mediamosa::utcdate2appdate($asset[mediamosa_asset_db::VIDEOTIMESTAMP]);
    }
    if (isset($asset[mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED])) {
      $asset[mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED] = mediamosa::utcdate2appdate($asset[mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED]);
    }

    return $asset;
  }

  /**
   * Get the asset.
   *
   * @param string $asset_id
   *   The ID of the asset.
   * @param int $app_id
   *   (optional) The application ID of the asset to verify asset is indeed from
   *   supplied app.
   * @param array $fields
   *   (optional) List of fields to select. Keep empty for all.
   *
   * @return array|FALSE
   *   The found asset or FALSE.
   */
  public static function get($asset_id, $app_id = NULL, array $fields = array()) {

    $query = mediamosa_db::db_select(mediamosa_asset_db::TABLE_NAME, 'a')
      ->fields('a', $fields)
      ->condition('a.' . mediamosa_asset_db::ID, $asset_id);
    if (!empty($app_id)) {
      $query->condition('a.' . mediamosa_asset_db::APP_ID, $app_id);
    }
    return $query->execute()->fetchAssoc();
  }

  /**
   * Get multiple assets within an array, with key
   * as asset_id.
   *
   * @param array $asset_ids
   *   The IDs of the assets to get.
   * @param array $fields
   *   The fields to retrieve.
   *
   * @return array
   *   An associative array:
   *     - The key is asset ID with asset as data.
   */
  public static function getAllAssoc(array $asset_ids, array $fields = array()) {
    // Return the assets.
    return mediamosa_db::db_select(mediamosa_asset_db::TABLE_NAME, 'a')
      ->fields('a', $fields)
      ->condition('a.' . mediamosa_asset_db::ID, $asset_ids, 'IN')
      ->execute()
      ->fetchAllAssoc(mediamosa_asset_db::ID, PDO::FETCH_ASSOC);
  }

  /**
   * Delete jobs attached to asset.
   *
   * @param string $asset_id
   *   The asset ID of the jobs to remove.
   *
   * @return bool
   *   Will return FALSE when no jobs where deleted.
   */
  public static function delete_jobs($asset_id) {
    return mediamosa_job::delete_job_by_asset($asset_id);
  }

  /**
   * Remove asset from database.
   *
   * @param string $asset_id
   *   The asset to delete.
   * @param bool $cascade
   *   Cascade mode will allow deletion of mediafiles. If FALSE and mediafiles
   *   (not counting stills) are present, then deletion will fail.
   * @param bool|null $trashcan
   *   Supply FALSE for removing all data from database and disk. TRUE will keep
   *   all related data and will give the asset.trashcan flag the value TRUE.
   *   NULL value will retrieve the app trashcan setting and use it.
   */
  public static function delete($asset_id, $cascade = TRUE, $trashcan = FALSE) {

    // Null value will retrieve the setting from app.
    if (is_null($trashcan)) {
      $trashcan = mediamosa_app_db::TRASHCAN_ASSET_DEFAULT_FALSE;

      $asset = mediamosa_asset::get();
      if (!empty($asset)) {
        $app = mediamosa_app::get_by_appid($asset['app_id']);
        mediamosa_exception::assert($app, 'Unable to find app with ID ' . $asset['app_id']);
        if (!empty($app)) {
          $trashcan = $app[mediamosa_app_db::TRASHCAN_ASSET_DEFAULT] === mediamosa_app_db::TRASHCAN_ASSET_DEFAULT_TRUE;
        }
      }
    }

    // When trashcan, we ignore cascade.
    if ($trashcan) {
      static::trashcan($asset_id);
      return;
    }

    // Start transaction.
    $transaction = mediamosa_db::db_transaction();

    try {
      // Find all my asset children.
      // @todo: parent_id no longer used (is always NULL).
      $asset_ids = mediamosa_db::db_query(
        'SELECT #asset_id FROM {#mediamosa_asset} WHERE #parent_id = :asset_id',
        array(
          '#asset_id' => mediamosa_asset_db::ID,
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#parent_id' => mediamosa_asset_db::PARENT_ID,
          ':asset_id' => $asset_id,
        )
      )->fetchCol();

      // Add me (last).
      $asset_ids[] = $asset_id;

      // Find all mediafiles (except stills, they dont count for cascade mode.).
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->addField('mf', mediamosa_asset_mediafile_db::ID);
      $query->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_ids, 'IN');
      $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
      $mediafile_ids = $query->execute()->fetchCol();

      // Any mediafiles left and casade off?
      if (count($mediafile_ids) && !$cascade) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_EMPTY, array('@mediafile_count' => count($mediafile_ids)));
      }

      // Remove all jobs.
      foreach ($asset_ids as $asset_id) {
        $result = FALSE;

        try {
          $result = mediamosa_asset::delete_jobs($asset_id);
        }
        catch (Exception $e) {
        }

        if (!$result) {
          throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_JOBS_COULD_NOT_BE_STOPPED);
        }
      }

      // Check the temporary wmv files
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->addField('mf', mediamosa_asset_mediafile_db::ID);
      $query->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_ids, 'IN');
      $query->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
      $query->condition(mediamosa_asset_mediafile_db::ID, '%.avi', 'LIKE');
      $mediafile_ids = $query->execute()->fetchCol();
      if (count($mediafile_ids)) {
        // Delete the temporary wmv files
        mediamosa_asset_mediafile::delete($mediafile_ids);
      }
      unset($mediafile_ids);

      // Remove all metadata of the asset.
      $query = mediamosa_db::db_delete(mediamosa_asset_metadata_db::TABLE_NAME);
      $query->condition(mediamosa_asset_metadata_db::ASSET_ID, $asset_id);
      $query->execute();

      // Find all mediafiles.
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->addField('mf', mediamosa_asset_mediafile_db::ID);
      $query->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id);
      $mediafile_ids = $query->execute()->fetchCol();

      // Delete the mediafiles.
      if (count($mediafile_ids)) {
        mediamosa_asset_mediafile::delete($mediafile_ids);
      }

      // @TODO: parent_id is no longer used.
      // Find my children.
      $result = mediamosa_db::db_query(
        "SELECT asset_id FROM {#mediamosa_asset} WHERE parent_id = :asset_id",
        array(
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          ':asset_id' => $asset_id
        )
      );

      // Foreach my children and call myself again.
      foreach ($result as $child_asset_id) {
        mediamosa_asset::delete($child_asset_id, $cascade, $trashcan);
      }

      // Now we removed our children, lets remove us.
      $app_id = mediamosa_db::db_query(
        "SELECT #app_id FROM {#mediamosa_asset} WHERE #asset_id = :asset_id",
        array(
          '#app_id' => mediamosa_asset_db::APP_ID,
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_db::ID,
          ':asset_id' => $asset_id
        )
      )->fetchField();

      // Find my collections.
      $result = mediamosa_db::db_query(
        "SELECT #coll_id FROM {#mediamosa_collection} WHERE #asset_id = :asset_id",
        array(
          '#coll_id' => mediamosa_asset_collection_db::COLL_ID,
          '#mediamosa_collection' => mediamosa_asset_collection_db::TABLE_NAME,
          '#asset_id' => mediamosa_asset_collection_db::ASSET_ID,
          ':asset_id' => $asset_id
        )
      )->fetchCol();

      // Foreach my collection.
      $is_coll = FALSE;
      foreach ($result as $collection_id) {
        $is_coll = TRUE;
        // Create entry in the mediamosa_asset_delete table.
        mediamosa_db::db_query(
          "REPLACE LOW_PRIORITY {#mediamosa_asset_delete} (#asset_id, #app_id, #videotimestampmodified, #coll_id) VALUES (:asset_id, :app_id, :now, :coll_id)",
          array(
            '#mediamosa_asset_delete' => mediamosa_asset_delete_db::TABLE_NAME,
            '#asset_id' => mediamosa_asset_delete_db::ID,
            '#app_id' => mediamosa_asset_delete_db::APP_ID,
            '#videotimestampmodified' => mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED,
            '#coll_id' => mediamosa_asset_delete_db::COLL_ID,
            ':asset_id' => $asset_id,
            ':app_id' => $app_id,
            ':coll_id' => $collection_id,
            ':now' => mediamosa_datetime::utc_current_timestamp_now(),
          )
        );
      }
      if (!$is_coll) {
        // Create entry in the mediamosa_asset_delete table.
        mediamosa_db::db_query(
          "REPLACE LOW_PRIORITY {#mediamosa_asset_delete} (#asset_id, #app_id, #videotimestampmodified, #coll_id) VALUES (:asset_id, :app_id, :now, '')",
          array(
            '#mediamosa_asset_delete' => mediamosa_asset_delete_db::TABLE_NAME,
            '#asset_id' => mediamosa_asset_delete_db::ID,
            '#app_id' => mediamosa_asset_delete_db::APP_ID,
            '#videotimestampmodified' => mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED,
            '#coll_id' => mediamosa_asset_delete_db::COLL_ID,
            ':asset_id' => $asset_id,
            ':app_id' => $app_id,
            ':now' => mediamosa_datetime::utc_current_timestamp_now(),
          )
        );
      }

      // Remove the asset from each collection.
      mediamosa_asset_collection::delete($asset_id);

      // Now delete the asset.
      $query = mediamosa_db::db_delete(mediamosa_asset_db::TABLE_NAME);
      $query->condition(mediamosa_asset_db::ID, $asset_id);
      $query->execute();

      // Call delete index on external database.
      mediamosa_asset::mediamosa_asset_delete(array($asset_id));
    }
    catch (Exception $e) {
      $transaction->rollback();
      throw $e;
    }
  }

  /**
   * Trashcan the asset.
   *
   * Only assets that where put in the trashcan can be restored. Expects the
   * asset to exist.
   *
   * @param string $asset_id
   *   The ID of the asset to put into trashcan.
   */
  protected static function trashcan($asset_id) {
    mediamosa_asset_metadata::metadata_asset_create($asset_id, array(self::METADATA_TRASHCAN => array(self::METADATA_TRASHCAN_TRUE)));
  }

  /**
   * Undelete the asset.
   *
   * Only asset that where not permanent deleted can be restored. Expects the
   * asset to exist.
   *
   * @param string $asset_id
   *   The ID of the asset to restore.
   */
  public static function restore($asset_id) {
    mediamosa_asset_metadata::metadata_asset_create($asset_id, array(self::METADATA_TRASHCAN => array(self::METADATA_TRASHCAN_FALSE)));
  }

  /**
   * Update the normalized fields in asset table.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile of which the asset needs to normalize.
   */
  public static function update_asset_info_with_mediafileid($mediafile_id) {
    $asset_id = mediamosa_asset_mediafile::get_my_parent_assetid($mediafile_id);
    if ($asset_id) {
      self::update_asset_info($asset_id);
    }
  }

  /**
   * Update the normalized fields in asset table.
   *
   * @param string $asset_id
   */
  public static function update_asset_info($asset_id) {

    // Find the length of the first mediafile.
    $mediafile_info = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->fields('mf', array(mediamosa_asset_mediafile_db::ID, mediamosa_asset_mediafile_db::FILENAME))
      ->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition('mf.' . mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE)
      ->orderBy('mf.changed')
      ->range(0, 1)
      ->execute()->fetchAssoc();

    $mediafile_id = (empty($mediafile_info[mediamosa_asset_mediafile_db::ID]) ? NULL : $mediafile_info[mediamosa_asset_mediafile_db::ID]);
    if (!empty($mediafile_id)) {
      $file_duration = mediamosa_asset_mediafile_metadata::get_mediafile_metadata($mediafile_id, mediamosa_asset_mediafile_metadata::FILE_DURATION, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);
      $container_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata($mediafile_id, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);
      $mime_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata($mediafile_id, mediamosa_asset_mediafile_metadata::MIME_TYPE, mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR);

      $fields = array();
      $fields[mediamosa_asset_db::MEDIAFILE_DURATION] = $file_duration;
      $fields[mediamosa_asset_db::MEDIAFILE_CONTAINER_TYPE] = $container_type;
      $fields[mediamosa_asset_db::MEDIAFILE_MIME_TYPE] = $mime_type;
      $fields[mediamosa_asset_db::ASSET_TYPE] = mediamosa_mimetype::get_type_part($mime_type);

      // Normalize the original filename.
      $fields[mediamosa_asset_db::MEDIAFILE_FILENAME] = mediamosa_asset_mediafile::get_filename($mediafile_info);

      db_update(mediamosa_asset_db::TABLE_NAME)
        ->fields($fields)
        ->condition(mediamosa_asset_db::ID, $asset_id)
        ->execute();
    }

    // Normalize the original filename. If we didn't find an original mediafile,
    // then empty the filename
    $fields[mediamosa_asset_db::MEDIAFILE_FILENAME] = empty($mediafile_info) ? '' : mediamosa_asset_mediafile::get_filename($mediafile_info);

    db_update(mediamosa_asset_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_db::ID, $asset_id)
      ->execute();

    // Set the external.
    mediamosa_asset::update_asset_info_is_external($asset_id, FALSE);

    // Set the empty asset.
    mediamosa_asset::update_asset_info_is_empty_asset($asset_id, FALSE);

    // Reindex the asset.
    mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_NORMALIZE);
  }

  /**
   * Update the normalized field in asset for is_empty_asset flag.
   *
   * @param string $asset_id
   *   The asset ID to update empty state.
   * @param bool $do_reindex
   *   Trigger re-index on external databases. This will only be triggered if
   *   the empty_asset flag is changed.
   */
  public static function update_asset_info_is_empty_asset($asset_id, $do_reindex = TRUE) {
    $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::FILESIZE);
    $mediafile = mediamosa_db::db_query_range(
      "SELECT m.#uri, m.#filename FROM {#mediamosa_asset_mediafile} AS m
       LEFT JOIN {#mediamosa_asset_mediafile_metadata} AS mm USING (#mediafile_id)
       WHERE m.#is_original_file = :is_original_file_true AND m.#asset_id = :asset_id AND
             (m.#uri IS NOT NULL OR m.#filename IS NOT NULL OR (mm.#prop_id = :prop_id AND mm.#val_int > 0))",
      0,
      1,
      array(
        '#uri' => mediamosa_asset_mediafile_db::URI,
        '#filename' => mediamosa_asset_mediafile_db::FILENAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        '#is_original_file' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
        ':is_original_file_true' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
        '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
        ':asset_id' => $asset_id,
        '#uri' => mediamosa_asset_mediafile_db::URI,
        '#filename' => mediamosa_asset_mediafile_db::FILENAME,
        '#prop_id' => mediamosa_asset_mediafile_metadata_db::PROP_ID,
        ':prop_id' => $prop_id,
        '#val_int' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
      )
    )->fetchAssoc();

    $query = db_select(mediamosa_asset_metadata_db::TABLE_NAME, 'am');
    $query->addJoin('INNER', mediamosa_asset_metadata_property_db::TABLE_NAME, 'amp', 'amp.prop_id = am.prop_id');
    $query->addJoin('INNER', mediamosa_asset_metadata_property_group_db::TABLE_NAME, 'ampg', 'ampg.propgroup_id = amp.propgroup_id');
    $query->condition('am.' . mediamosa_asset_metadata_db::ASSET_ID, $asset_id);
    $query->condition('ampg.' . mediamosa_asset_metadata_property_group_db::NAME, 'asset', '!=');
    $has_metadata = $query->countQuery()->execute()->fetchField() > 0;

    // A asset is empty if it has no files with content and no metadata.
    $is_empty_asset = (!$has_metadata && !$mediafile);

    // Is empty, then see if provider_id or reference_id is set.
    if ($is_empty_asset) {
      $asset = db_select(mediamosa_asset_db::TABLE_NAME, 'a')
        ->fields('a', array(mediamosa_asset_db::PROVIDER_ID, mediamosa_asset_db::REFERENCE_ID))
        ->condition('a.' . mediamosa_asset_db::ID, $asset_id)
        ->execute()
        ->fetchAssoc();

      // Can only be empty now if provider ID and reference ID are both empty.
      $is_empty_asset =
        ($asset[mediamosa_asset_db::PROVIDER_ID] === '' || $asset[mediamosa_asset_db::PROVIDER_ID] === NULL)
        &&
        ($asset[mediamosa_asset_db::REFERENCE_ID] === '' || $asset[mediamosa_asset_db::REFERENCE_ID] === NULL);
    }


    // Set is_empty_asset.
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #is_empty_asset = :is_empty_asset WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#is_empty_asset' => mediamosa_asset_db::IS_EMPTY_ASSET,
        ':is_empty_asset' => $is_empty_asset ? mediamosa_asset_db::IS_EMPTY_ASSET_TRUE : mediamosa_asset_db::IS_EMPTY_ASSET_FALSE,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
      )
    );

    // Set is_empty_asset in asset_relation.
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset_collection} SET #is_empty_asset = :is_empty_asset WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset_collection' => mediamosa_asset_collection_db::TABLE_NAME,
        '#is_empty_asset' => mediamosa_asset_collection_db::IS_EMPTY_ASSET,
        ':is_empty_asset' => $is_empty_asset ? mediamosa_asset_collection_db::IS_EMPTY_ASSET_TRUE : mediamosa_asset_collection_db::IS_EMPTY_ASSET_FALSE,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
      )
    );

    if ($do_reindex) {
      // Reindex the asset.
      mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_NORMALIZE);
    }
  }

  /**
   * Update the normalized field in asset for is_external flag.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param bool $do_reindex
   *   Reindex the asset after update.
   */
  public static function update_asset_info_is_external($asset_id, $do_reindex = TRUE) {

    $mediafile = mediamosa_db::db_query_range(
      'SELECT #uri, #filename FROM {#mediamosa_asset_mediafile} WHERE #is_original_file = :TRUE AND #asset_id = :asset_id',
      0, // From.
      1, // Count.
      array(
        '#uri' => mediamosa_asset_mediafile_db::URI,
        '#filename' => mediamosa_asset_mediafile_db::FILENAME,
        '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#is_original_file' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
        ':TRUE' => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
        '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
        ':asset_id' => $asset_id,
      )
    )->fetchAssoc();

    if ($mediafile) {
      $uri = $mediafile[mediamosa_asset_mediafile_db::URI];

      // Set is_external.
      mediamosa_db::db_query(
        "UPDATE {#mediamosa_asset} SET #is_external = :is_external WHERE #asset_id = :asset_id",
        array(
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#is_external' => mediamosa_asset_db::IS_EXTERNAL,
          ':is_external' => trim($uri) != '' ? mediamosa_asset_db::IS_EXTERNAL_TRUE : mediamosa_asset_db::IS_EXTERNAL_FALSE,
          '#asset_id' => mediamosa_asset_db::ID,
          ':asset_id' => $asset_id,
        )
      );
    }

    if ($do_reindex) {
      // Reindex the asset.
      mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_NORMALIZE);
    }
  }

  /**
   * Collect the asset information.
   *
   * @param array $asset_ids
   *   Array asset_ids.
   * @param array $app_ids
   *   Application app ids.
   * @param string $acl_user_id
   *   The acl_user ID.
   * @param bool $granted
   *   Show or hide accessable assets.
   * @param array $acl_group_ids
   *   Group ID authentication.
   * @param string $acl_domain
   *   Domain authentication.
   * @param string $acl_realm
   *   Realm authentication.
   * @param bool $is_app_admin
   *   Used for including unappropriate assets.
   * @param bool $show_stills
   *   Include stills.
   * @param bool $show_collections
   *   Include collection information.
   * @param bool $add_has_streamable_mediafiles
   *   TRUE when asset contains streamable mediafiles.
   * @param bool $view_hidden_metadata
   *    Show hidden metadata.
   */
  public static function asset_collect(
    array $asset_ids,
    array $app_ids,
    $acl_user_id,
    $granted,
    array $acl_group_ids,
    $acl_domain,
    $acl_realm,
    $is_app_admin = FALSE,
    $show_stills = TRUE,
    $show_collections = FALSE,
    $add_has_streamable_mediafiles = FALSE,
    $view_hidden_metadata = FALSE,
    $options = array()
  ) {
    // Something to collect?
    if (empty($asset_ids)) {
      return array();
    }

    $prop_ids = array();
    $metadata_definitions_fulls = mediamosa_asset_metadata_property::get_metadata_properties_full($app_ids);
    foreach ($metadata_definitions_fulls as $metadata_definitions_full) {
      $prop_ids[] = $metadata_definitions_full['propdef_id'];
    }

    $s_ids = implode("','", $asset_ids);
    $query = array();
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'a.*';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'ap.val_char AS asset_property_val_char';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'ap.val_datetime AS asset_property_val_datetime';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'ap.val_int AS asset_property_val_int';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'apd.prop_name AS asset_property_name';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'apd.type AS asset_property_type';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'apg.propgroup_name AS asset_property_group_name';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'apd.is_hidden AS asset_property_is_hidden';

    // Main select on asset table.
    $query[mediamosa_db_query::A_FROM][] = '{mediamosa_asset} AS a';

    // Joins.
    $query[mediamosa_db_query::A_JOIN]['ap'] = 'LEFT JOIN {mediamosa_asset_metadata} AS ap ON ap.asset_id = a.asset_id';
    $query[mediamosa_db_query::A_JOIN]['apd'] = 'LEFT JOIN {mediamosa_asset_metadata_property} AS apd ON ap.prop_id = apd.prop_id';
    $query[mediamosa_db_query::A_JOIN]['apg'] = 'LEFT JOIN {mediamosa_asset_metadata_property_group} AS apg ON apd.propgroup_id = apg.propgroup_id';

    if (count($prop_ids)) {
      $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['ap'][mediamosa_db_query::WHERE_OR]['prop_id'][] = sprintf('apd.prop_id IN(%s)', implode(',', $prop_ids));
    }

    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['ap'][mediamosa_db_query::WHERE_OR]['prop_id'][] = "apd.prop_id IS NULL"; // in case it doesn't have properties
    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['a'][] = sprintf("a.asset_id IN ('%s')", $s_ids);

    $query[mediamosa_db_query::A_ORDER_BY][] = 'ap.prop_id ASC';

    // Build the query
    $query_str = mediamosa_db_query::query_select($query);

    // Do the query.
    $result = mediamosa_db::db_query($query_str);

    $items = array();
    foreach ($result as $row) {
      if (!empty($row['still_id']) && $row['still_default'] != 'FALSE') {
        $items['stills'][$row['asset_id']] = $row['still_id'];
      }

      unset(
        $row['created'],
        $row['changed'],
        $row['still_id']
      );

      // 2.x changes to database need to be aliased to 1.x name giving (for
      // now).
      $aliases = array(
        mediamosa_asset_db::IS_INAPPROPRIATE => 'is_unappropiate',
      );

      // Aliases.
      foreach ($aliases as $from => $to) {
        if (isset($row[$from])) {
          $row[$to] = $row[$from];
        }
      }

      $items['assets'][] = $row;
    }

    // Find deleted.
    $result = mediamosa_db::db_select(mediamosa_asset_delete_db::TABLE_NAME, 'a')
      ->fields('a')
      ->condition(mediamosa_asset_delete_db::ID, $asset_ids, 'IN')
      ->execute();

    foreach ($result as $row) {
      $row['status'] = 'deleted';
      $items['assets'][] = $row;
    }

    $result = mediamosa_app::get_by_appids($app_ids,
      array(
        'id' => mediamosa_app_db::APP_ID,
        mediamosa_app_db::PLAY_PROXY_URL,
        mediamosa_app_db::VIEW_ASSET_URL,
        mediamosa_app_db::PREVIEW_PROFILE_ID,
        mediamosa_app_db::STILL_URL
      )
    );

    foreach ($result as $app) {
      $items['app_info'][$app['id']] = $app;
    }

    // Get all favorites.
    $items['user_favorites'] = array();
    if ($acl_user_id != '') {
      $result = mediamosa_db::db_select('mediamosa_user_favorite', 'uf')
        ->fields('uf', array(mediamosa_user_favorite_db::FAV_ID))
        ->condition(mediamosa_user_favorite_db::NAME, $acl_user_id)
        ->condition(mediamosa_user_favorite_db::APP_ID, $app_ids, 'IN')
        ->condition(mediamosa_user_favorite_db::FAV_TYPE, mediamosa_user_favorite_db::FAV_TYPE_ASSET)
        ->condition(mediamosa_user_favorite_db::FAV_ID, $asset_ids)
        ->execute();

      foreach ($result as $row) {
        $items['user_favorites'][] = $row[mediamosa_user_favorite_db::FAV_ID];
      }
    }

    // Get all preview mediafiles.
    $query = array();
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'm.mediafile_id';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'm.filename AS mediafile_filename';
    $query[mediamosa_db_query::A_SELECT_EXPR][] = 'IFNULL(a.parent_id, a.asset_id) AS asset_id';
    $query[mediamosa_db_query::A_FROM][] = '{mediamosa_asset_mediafile} AS m';
    $query[mediamosa_db_query::A_JOIN]['asset'] = 'JOIN {mediamosa_asset} AS a USING(asset_id)';
    if (isset($items['app_info'])) {
      $keys = array_keys($items['app_info']);
      foreach ($keys as $key) {
        if (isset($items['app_info'][$key])) {
          $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['profile'][mediamosa_db_query::WHERE_OR][] = sprintf("(m.transcode_profile_id = %d AND m.app_id = %d)", $items['app_info'][$key]['preview_profile_id'], $key);
        }
      }
    }

    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND][] = sprintf("a.app_id IN(%s)", implode(",", $app_ids));
    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][mediamosa_db_query::WHERE_OR][] = sprintf("a.asset_id IN('%s')", $s_ids);
    $query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][mediamosa_db_query::WHERE_OR][] = sprintf("a.parent_id IN('%s')", $s_ids);

    $query_str = mediamosa_db_query::query_select($query);
    $result = mediamosa_db::db_query($query_str);
    foreach ($result as $row) {
      $items['mediafile_preview'][$row['asset_id']] = $row;
    }

    // If granted is TRUE, we check access, but allow assets in our result even when we dont have access.
    // If FALSE, we expect search only to supply the assets where we have access on.
    $items['granted'] = array(); // must exist, else granted flag is not included on the non-granted

    if ($granted) {
      $asset_ids_access = mediamosa_search::asset_access(array(
        'asset_ids' => $asset_ids,
        'app_ids' => $app_ids,
        'acl_user_id' => $acl_user_id,
        'acl_group_ids' => $acl_group_ids,
        'acl_domain' => $acl_domain,
        'acl_realm' => $acl_realm,
        'is_app_admin' => $is_app_admin,
      ));

      foreach ($asset_ids_access as $asset_id) {
        $items['granted'][$asset_id] = $asset_id;
      }
    }
    else {
      // Expect we have access on all...
      foreach ($items['assets'] as $row) {
        $items['granted'][$row['asset_id']] = $row['asset_id'];
      }
    }

    return mediamosa_asset::enrich_response_asset(
      $items,
      $asset_ids,
      $app_ids,
      $acl_user_id,
      $is_app_admin,
      $show_stills,
      $show_collections,
      $add_has_streamable_mediafiles,
      $view_hidden_metadata,
      $options
    );
  }

  /**
   * Collect related asset data.
   *
   * @param int $app_id
   *   The client application ID.
   * @param bool $is_app_admin
   *   Is app admin, for super user access.
   * @param array $items
   *   The items array to populate.
   * @param array $related
   *   The related result.
   * @param array $options
   *   Associative array with options;
   *   - 'show_stills': (false) Include still.
   *   - 'show_viewed_asset': (false) Include the view counts of the related
   *     assets.
   */
  public static function asset_collect_related($app_id, $is_app_admin, &$items, $related, $options = array()) {
    $viewed = array();
    if (!empty($options['show_viewed_asset'])) {
      $asset_ids = array();
      foreach ($items as $item) {
        if (!isset($related[$item['asset_id']])) {
          continue;
        }

        $asset_ids = array_merge($asset_ids, array_keys($related[$item['asset_id']]['assets']));
      }
      $asset_ids = array_unique($asset_ids);
      if (!empty($asset_ids)) {
        $viewed = mediamosa_db::db_select(mediamosa_asset_db::TABLE_NAME, 'a')
          ->fields('a', array(mediamosa_asset_db::ID, mediamosa_asset_db::VIEWED))
          ->condition(mediamosa_asset_db::ID, $asset_ids)
          ->execute()
          ->fetchAllKeyed(0, 1);
      }
    }

    foreach ($items as &$item) {
      $item['related'] = array('count_total' => 0, 'count' => 0, 'score_max' => 0, 'assets' => array());
      if (!isset($related[$item['asset_id']])) {
        continue;
      }

      $item['related']['count_total'] = $related[$item['asset_id']]['count_total'];
      $item['related']['count'] = $related[$item['asset_id']]['count'];
      $item['related']['score_max'] = $related[$item['asset_id']]['score_max'];

      foreach ($related[$item['asset_id']]['assets'] as $asset_id => $asset) {
        $asset['still_url'] = empty($options['show_stills']) ? '' : mediamosa_media::do_response_still_default($app_id, $asset_id, $is_app_admin);

        if (!empty($options['show_viewed_asset'])) {
          $asset['viewed'] = isset($viewed[$asset_id]) ? $viewed[$asset_id] : 0;
        }
        $item['related']['assets']['asset']['#' . serialize(array('asset_id' => $asset_id))] = $asset;
      }
    }
  }

  /**
   * Convert the facet data into output ready data.
   */
  public static function asset_collect_solr_facet($solr_facet) {

    foreach ($solr_facet['solr_facet_fields'] as &$data) {
      $converted = array();
      foreach ($data as $name => $value) {
        $converted['field'][] =  array('name' => $name, 'value' => $value);
      }
      $data = array('fields' => $converted);
    }

    foreach ($solr_facet['solr_facet_dates'] as &$data) {
      $converted = array();
      foreach ($data as $name => $value) {
        $converted['field'][] =  array('name' => $name, 'value' => $value);
      }
      $data = array('fields' => $converted);
    }

    return $solr_facet;
  }


  /**
   * Enrich the response object with asset information.
   *
   * @param array $items
   * @param array $asset_ids
   * @param array $app_ids
   * @param string $acl_user_id
   * @param bool $show_stills
   * @param bool $show_collections
   *  Include collections (was hack on _GET)
   * @param $add_has_streamable_mediafiles,
   * @param $view_hidden_metadata
   *   Show hidden metadata,
   */
  public static function enrich_response_asset(
    array $items,
    array $asset_ids,
    array $app_ids,
    $acl_user_id,
    $is_app_admin,
    $show_stills = TRUE,
    $show_collections = FALSE,
    $add_has_streamable_mediafiles = FALSE,
    $view_hidden_metadata = FALSE,
    $options = array()
  ) {

    $collections = array();

    $items_pre_sort = array();

    // Need the first one.
    $app_id = reset($app_ids);

    // Get the metadata definitions.
    $metadata_definitions_fulls = mediamosa_asset_metadata_property::get_metadata_properties_full($app_ids);
    // Hidden metadata.
    if (!$view_hidden_metadata) {
      foreach ($metadata_definitions_fulls as $metadata_definition_name => $metadata_definitions_full) {
        if ($metadata_definitions_full['propdef_is_hidden'] == 'TRUE') {
          unset($metadata_definitions_fulls[$metadata_definition_name]);
        }
      }
    }

    // Need to get collections?
    if ($show_collections) {
      // Get all collection information in one go.
      $asset_ids = array();
      foreach ($items['assets'] as $asset) {
        $asset_ids[$asset['asset_id']] = $asset['asset_id'];
      }

      $result = mediamosa_asset_collection::get_by_asset_ids(
        $asset_ids,
        array(
          mediamosa_asset_collection_db::ASSET_ID => 'ac.' . mediamosa_asset_collection_db::ASSET_ID,
          mediamosa_collection_db::ID => 'c.' . mediamosa_collection_db::ID,
          mediamosa_collection_db::TITLE => 'c.' . mediamosa_collection_db::TITLE
        )
      );

      // Now store it in the cache.
      foreach ($result as $collection) {
        $collections[$collection[mediamosa_asset_collection_db::ASSET_ID]][$collection[mediamosa_asset_collection_db::COLL_ID]] = $collection;
      }
    }

    // Has streamable mediafiles.
    if ($add_has_streamable_mediafiles) {
      $result = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
        ->fields(
          'mf',
          array(
            mediamosa_asset_mediafile_db::ASSET_ID,
            mediamosa_asset_mediafile_db::URI,
            mediamosa_asset_mediafile_db::ID,
          )
        )
        ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_ids, 'IN')
        ->execute();

      $mf = array();
      foreach ($result as $row) {
        // It is FALSE in the beginning.
        if (!isset($mf[$row[mediamosa_asset_mediafile_db::ASSET_ID]])) {
          $mf[$row[mediamosa_asset_mediafile_db::ASSET_ID]] = FALSE;
        }
        // Get the info.
        $mediafile_id = $row[mediamosa_asset_mediafile_db::ID];
        $container_type = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE);
        $video_codec = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::VIDEO_CODEC);
        $response = mediamosa_server::get_streaming_info($container_type, $video_codec, $row[mediamosa_asset_mediafile_db::URI]);

        // Set the mediafile has_streamable_mediafiles value.
        $mf[$row[mediamosa_asset_mediafile_db::ASSET_ID]] = ($mf[$row[mediamosa_asset_mediafile_db::ASSET_ID]] || $response['response_object_available'] == 'TRUE');
      }
    }

    foreach ($items['assets'] as $asset) {
      // Now only stuff that may be set once per asset.
      if (!isset($items_pre_sort[$asset[mediamosa_asset_db::ID]])) {
        $items_pre_sort[$asset[mediamosa_asset_db::ID]] = mediamosa_asset::process_asset_output($asset, $metadata_definitions_fulls);

        if (isset($items['app_info'][$asset['app_id']]) && isset($items['app_info'][$asset['app_id']]['play_proxy_url']) && isset($items['mediafile_preview'][$asset[mediamosa_asset_db::ID]])) {
          $patterns = array(
            '/{asset_id}/i',
            '/{preview_profile_id}/i',
            '/{mediafile_id}/i',
            '/{mediafile_filename}/i',
          );

          $replacements = array(
            $asset[mediamosa_asset_db::ID],
            $items['app_info'][$asset['app_id']]['preview_profile_id'],
            $items['mediafile_preview'][$asset[mediamosa_asset_db::ID]]['mediafile_id'],
            $items['mediafile_preview'][$asset[mediamosa_asset_db::ID]]['mediafile_filename'],
          );

          $items_pre_sort[$asset[mediamosa_asset_db::ID]]['ega_play_url'] = isset($items['app_info'][$asset[mediamosa_asset_db::APP_ID]]) ? preg_replace($patterns, $replacements, $items['app_info'][$asset[mediamosa_asset_db::APP_ID]]['play_proxy_url']) : '';
        }
        else {
          $result = mediamosa_db::db_query('
            SELECT DISTINCT ms.#app_master_id
            FROM {#mediafile} m
            INNER JOIN {#acl_app_master_slave} ms ON m.#mediafile_id = ms.#acl_object_id AND ms.#acl_object_type = :type_mediafile AND ms.#app_slave_id = :app_slave_id
            WHERE m.#asset_id = :asset_id
          ', array(
            '#mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
            '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
            '#asset_id' => mediamosa_asset_mediafile_db::ASSET_ID,
            '#acl_app_master_slave' => mediamosa_acl_app_master_slave_db::TABLE_NAME,
            '#app_master_id' => mediamosa_acl_app_master_slave_db::APP_ID_MASTER,
            '#acl_object_id' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_ID,
            '#acl_object_type' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE,
            ':type_mediafile' => mediamosa_acl_app_master_slave_db::ACL_OBJECT_TYPE_MEDIAFILE,
            '#app_slave_id' => mediamosa_acl_app_master_slave_db::APP_ID_SLAVE,
            ':app_slave_id' => $asset[mediamosa_asset_db::APP_ID],
            ':asset_id' => $asset[mediamosa_asset_db::ID],
          ));
          $master_app_ids = array();
          foreach ($result as $record) {
            $master_app_ids[] = $record[mediamosa_acl_app_master_slave_db::APP_ID_MASTER];
          }

          foreach ($app_ids as $app_id_2) {
            if (isset($items['app_info'][$app_id_2]) && $items['app_info'][$app_id_2]['view_asset_url']) {
              if ($app_id_2 == $asset[mediamosa_asset_db::APP_ID] || in_array($app_id_2, $master_app_ids)) {
                $items_pre_sort[$asset[mediamosa_asset_db::ID]]['ega_view_url'][] = preg_replace('/{asset_id}/i', $asset[mediamosa_asset_db::ID], $items['app_info'][$app_id_2]['view_asset_url']);
              }
            }
          }
        }

        // Include the user favorites (if any).
        $items_pre_sort[$asset[mediamosa_asset_db::ID]]['is_favorite'] = isset($items['user_favorites']) && array_search($asset[mediamosa_asset_db::ID], $items['user_favorites']) !== FALSE ? 'TRUE' : 'FALSE';

        // Insert granted.
        $items_pre_sort[$asset[mediamosa_asset_db::ID]]['granted'] = isset($items['granted']) && array_search($asset[mediamosa_asset_db::ID], $items['granted']) !== FALSE ? 'TRUE' : 'FALSE';

        // has_streamable_mediafiles.
        if ($add_has_streamable_mediafiles) {
          $items_pre_sort[$asset[mediamosa_asset_db::ID]]['has_streamable_mediafiles'] = (isset($mf[$asset[mediamosa_asset_db::ID]]) && $mf[$asset[mediamosa_asset_db::ID]] ? 'TRUE' : 'FALSE');
        }

        // Empty by default.
        $items_pre_sort[$asset[mediamosa_asset_db::ID]]['ega_still_url'] = '';

        if ($show_stills) {
          // Can happen more than once, don't do it again when vpx_still_url is already set.
          if (!isset($items_pre_sort[$asset[mediamosa_asset_db::ID]]['vpx_still_url'])) {

            // Empty by default (so previous isset will work when empty).
            $items_pre_sort[$asset[mediamosa_asset_db::ID]]['vpx_still_url'] = '';
            $items_pre_sort[$asset[mediamosa_asset_db::ID]]['ega_still_url'] = '';

            // Process still.
            try {
              $still = array();

              // Get the still_id.
              if (empty($items['stills'][$asset[mediamosa_asset_db::ID]])) {
                $still = mediamosa_asset_mediafile_still::find_default($asset[mediamosa_asset_db::ID]);
                if ($still) {
                  $items['stills'][$asset[mediamosa_asset_db::ID]] = $still[mediamosa_asset_mediafile_db::ID];
                }
              }
              else {
                // Get the still ID.
                $still_id = $items['stills'][$asset[mediamosa_asset_db::ID]];

                // Get the still.
                $still = mediamosa_asset_mediafile::get($still_id);
              }

              if ($still && $app_id) {
                // Create still response.
                $response = mediamosa_media::do_response_still($app_id, $acl_user_id, $is_app_admin, $still);
                $items_pre_sort[$asset[mediamosa_asset_db::ID]]['vpx_still_url'] = $response['output'];
                $items_pre_sort[$asset[mediamosa_asset_db::ID]]['ega_still_url'] = (isset($items['app_info'][$asset['app_id']]) ? str_replace('{asset_id}', $asset[mediamosa_asset_db::ID], $items['app_info'][$asset['app_id']]['still_url']) : '');
              }
            }
            catch (mediamosa_exception $e) {
              // Ignore.
              mediamosa_debug::log_asset($asset[mediamosa_asset_db::ID], "Caught and ignored; \n" . $e->getMessage());
            }
          }
        }
        else {
          // Make sure its empty by default.
          $items_pre_sort[$asset[mediamosa_asset_db::ID]]['vpx_still_url'] = '';
        }
      }

      // Now stuff that can be set more than once (f.e. metadata).
      if (!empty($asset['asset_property_group_name']) && ($asset['asset_property_is_hidden'] == 'FALSE' || $view_hidden_metadata)) {
        // Choose the value.
        switch ($asset['asset_property_type']) {
          case mediamosa_asset_metadata_property_db::TYPE_CHAR:
            $items_pre_sort[$asset[mediamosa_asset_db::ID]][$asset['asset_property_group_name']][$asset['asset_property_name']][] = $asset['asset_property_val_char'];
            break;

          case mediamosa_asset_metadata_property_db::TYPE_DATETIME:
            $items_pre_sort[$asset[mediamosa_asset_db::ID]][$asset['asset_property_group_name']][$asset['asset_property_name']][] = mediamosa::utcdate2appdate($asset['asset_property_val_datetime']);
            break;

          case mediamosa_asset_metadata_property_db::TYPE_INT:
            $items_pre_sort[$asset[mediamosa_asset_db::ID]][$asset['asset_property_group_name']][$asset['asset_property_name']][] = $asset['asset_property_val_int'];
            break;
        }
      }

      // Add collection data.
      if ($show_collections && isset($collections[$asset[mediamosa_asset_db::ID]])) {
        // Show in old format? Yes, if the app client < 3.0.
        $mediamosa_version = mediamosa::get_environment_setting(mediamosa::ENV_MEDIAMOSA_VERSION);
        $is_version_30 = $mediamosa_version[mediamosa_version::MAJOR] >= 3;

        if ($is_version_30) {
          $items_pre_sort[$asset[mediamosa_asset_db::ID]]['collections'] = array();
        }
        foreach ($collections[$asset[mediamosa_asset_db::ID]] as $coll_id => $collection) {
          if ($is_version_30) {
            $items_pre_sort[$asset[mediamosa_asset_db::ID]]['collections']['collection']['#' . serialize(array('id' => $coll_id,))][mediamosa_collection_db::TITLE] = $collection[mediamosa_collection_db::TITLE];
          }
          else {
            // Old version.
            $items_pre_sort[$asset[mediamosa_asset_db::ID]]['collection_' . $coll_id] = $collection[mediamosa_collection_db::TITLE];
          }
        }
      }
      // Allow other modules to alter output.
      $options += array(
        'app_id' => $app_id,
        'acl_user_id' => $acl_user_id,
        'is_app_admin' => $is_app_admin,
      );
      drupal_alter('mediamosa_asset_enrich_response', $items_pre_sort[$asset[mediamosa_asset_db::ID]], $options);
    }

    $items = array();
    foreach ($asset_ids as $asset_id) {
      $items[] = $items_pre_sort[$asset_id];
    }

    return $items;
  }

  /**
   * Enrich the response object with has_streamable_mediafiles information.
   *
   * @param array $mediafiles
   *   The mediafiles in output.
   *
   * @return bool
   *   Returns TRUE when input contains streamable mediafiles, FALSE otherwise.
   */
  public static function enrich_response_has_streamable_mediafiles($mediafiles) {
    $has_streamable_mediafiles = FALSE;

    // Empty? Then no streamable media.
    if (empty($mediafiles)) {
      return FALSE;
    }

    foreach ($mediafiles as $mediafile) {
      // Check for serialize attributes.
      if (substr(key($mediafile), 0, 1) == '#') {
        foreach ($mediafile as $mediafile2) {
          $has_streamable_mediafiles = ($has_streamable_mediafiles || $mediafile2['response_object_available'] == 'TRUE');
        }
      }
      else {
        $has_streamable_mediafiles = ($has_streamable_mediafiles || $mediafile['response_object_available'] == 'TRUE');
      }
    }

    // Return boolean.
    return $has_streamable_mediafiles;
  }

  /**
   * Update the dates for asset.
   *
   * @param string $asset_id
   *   The asset ID.
   */
  public static function update_asset_timestamps($asset_id) {
    // Get the NOW date.
    $now = mediamosa_datetime::utc_current_timestamp_now();

    // Update videotimestamp.
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #videotimestamp = :now WHERE #asset_id = :asset_id AND #videotimestamp IS NULL',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#videotimestamp' => mediamosa_asset_db::VIDEOTIMESTAMP,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
        ':now' => $now,
      )
    );

    // Update videotimestampmodified.
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #videotimestampmodified = :now WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#videotimestampmodified' => mediamosa_asset_db::VIDEOTIMESTAMPMODIFIED,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id,
        ':now' => $now,
      )
    );
  }

  /**
   * Increase viewed.
   *
   * @param $asset_id
   */
  public static function asset_viewed($asset_id) {
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #viewed = #viewed + 1 WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#viewed' => mediamosa_asset_db::VIEWED,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    );

    // Reindex the asset.
    mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_METADATA);
  }

  /**
   * Increase played.
   *
   * @param string $asset_id
   *   The asset ID that played.
   */
  public static function asset_played($asset_id) {
    mediamosa_db::db_query(
      'UPDATE {#mediamosa_asset} SET #played = #played + 1 WHERE #asset_id = :asset_id',
      array(
        '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
        '#played' => mediamosa_asset_db::PLAYED,
        '#asset_id' => mediamosa_asset_db::ID,
        ':asset_id' => $asset_id
      )
    );

    // Reindex the asset.
    mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_METADATA);
  }

  /**
   * Test access for inappropriate.
   *
   * @param array $app_ids
   *   The applications IDs.
   * @param string $asset_id
   *   The asset ID.
   * @param string $user_id
   *   The current user ID in case of ownership of asset.
   * @param bool $is_app_admin
   *   Is app admin for overruling the inappropriate status.
   */
  public static function is_inappropriate(array $app_ids, $asset_id, $user_id, $is_app_admin = FALSE, $play_call = FALSE) {

    $asset = self::get($asset_id);
    if (!$asset) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    // If its not unapp. then leave.
    if ($asset[mediamosa_asset_db::IS_INAPPROPRIATE] != mediamosa_asset_db::IS_INAPPROPRIATE_TRUE) {
      return;
    }

    // If play call, then even if we are owner, then we can only play it when
    // is_app_admin is TRUE.
    if ($play_call && !$is_app_admin) {
      throw new mediamosa_exception_error_is_inappropriate();
    }

    // Must be owner or admin.
    try {
      mediamosa_acl::owner_check(reset($app_ids), $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);
    }
    catch (mediamosa_exception_error_access_denied $e) {
      assert($e);
      throw new mediamosa_exception_error_is_inappropriate();
    }
  }

  /**
   * Create asset.
   *
   * @param int $app_id
   *   The Application ID.
   * @param string $owner_id
   *   The owner ID.
   * @param string $group_id
   *   The group ID.
   * @param string $reference_id
   *   The reference ID.
   * @param string $provider_id
   *   The provider ID
   * @param bool $published
   *   The asset is published.
   * @param bool $trashcan
   *   The asset is by default in trashcan.
   * @param bool $asset_id
   *   If given, use this id to create new asset.
   *
   * @param string
   *   The new asset ID of the created asset.
   */
  public static function create($app_id, $owner_id, $group_id = NULL, $reference_id = NULL, $provider_id = NULL, $published = TRUE, $trashcan = FALSE, $asset_id = NULL) {

    if ($asset_id == NULL) {
      $asset_id = mediamosa_db::uuid($app_id);
    }

    $fields = array(
      mediamosa_asset_db::ID => $asset_id,
      mediamosa_asset_db::APP_ID => $app_id,
      mediamosa_asset_db::OWNER_ID => $owner_id,
    );

    if ($group_id) {
      $fields[mediamosa_asset_db::GROUP_ID] = $group_id;
    }
    if ($reference_id) {
      $fields[mediamosa_asset_db::REFERENCE_ID] = $reference_id;
    }
    if ($provider_id) {
      $fields[mediamosa_asset_db::PROVIDER_ID] = $provider_id;
    }

    // Add created value.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Transaction in.
    $transaction = mediamosa_db::db_transaction();
    try {
      // Insert it.
      mediamosa_db::db_insert(mediamosa_asset_db::TABLE_NAME)
        ->fields($fields)
        ->execute();

      // Publish the asset.
      if ($published) {
        mediamosa_asset::publish($asset_id, FALSE);
      }
      else {
        mediamosa_asset::unpublish($asset_id, FALSE);
      }

      if ($trashcan) {
        mediamosa_asset::trashcan($asset_id);
      }
      else {
        mediamosa_asset::restore($asset_id);
      }

      // Reindex the asset.
      mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_CREATE);
    }
    catch (Exception $e) {
      $transaction->rollback();
      throw $e;
    }

    // previously generated above.
    return $asset_id;
  }

  /**
   * Update a asset.
   *
   * @param string $app_id
   *   The application ID.
   * @param string $user_id
   *   The user id of current user.
   * @param bool $is_app_admin
   *   Is application admin.
   * @param string $asset_id
   *   The asset ID.
   * @param string $owner_id
   *   The new owner ID.
   * @param string $group_id
   *   The new group ID.
   * @param string $play_restriction_start
   *   Start range restriction.
   * @param string $play_restriction_end
   *   End range restriction.
   * @param bool $isprivate
   *   Is private flag.
   * @param bool $is_unappropriate
   *   Is unappropriate flag.
   * @param string $reference_id
   *   Eternal id to reference this asset to.
   * @param string $provider_id
   *   Id to identify provider.
   * @param string $asset_type
   *   Type of this asset. Normally this is determined on the basis of the
   *   original mediafile but can be overruled.
   */
  public static function update($app_id, $user_id, $is_app_admin, $asset_id, $owner_id = NULL, $group_id = NULL, $play_restriction_start = NULL, $play_restriction_end = NULL, $isprivate = NULL, $is_unappropriate = NULL, $reference_id = NULL, $provider_id = NULL, $asset_type = NULL) {

    // Asset must exist.
    $asset = mediamosa_asset::must_exists($asset_id);

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID], $is_app_admin);

    // If not null, then check if this is the ega admin...
    if (!is_null($is_unappropriate)) {
      mediamosa_acl::app_admin_check($app_id, $asset[mediamosa_asset_db::APP_ID], $is_app_admin);
    }

    // Check if the start and end dates are not switched (start > end).
    if (!is_null($play_restriction_start) && $play_restriction_start !== '' && !is_null($play_restriction_end) && $play_restriction_end !== '') {
      if (mediamosa_datetime::date2unix($play_restriction_start) > mediamosa_datetime::date2unix($play_restriction_end)) {
        // Swap start with end.
        list($play_restriction_end, $play_restriction_start) = array($play_restriction_start, $play_restriction_end);
      }
    }

    $fields = array();
    if (!is_null($play_restriction_start)) {
      $fields[mediamosa_asset_db::PLAY_RESTRICTION_START] = $play_restriction_start === '' ? NULL : $play_restriction_start;
    }
    if (!is_null($play_restriction_end)) {
      $fields[mediamosa_asset_db::PLAY_RESTRICTION_END] = $play_restriction_end  === '' ? NULL : $play_restriction_end;
    }
    if (!is_null($isprivate)) {
      $fields[mediamosa_asset_db::ISPRIVATE] = $isprivate ? mediamosa_asset_db::ISPRIVATE_TRUE : mediamosa_asset_db::ISPRIVATE_FALSE;
    }
    if (!is_null($is_unappropriate)) {
      $fields[mediamosa_asset_db::IS_INAPPROPRIATE] = $is_unappropriate ? mediamosa_asset_db::IS_INAPPROPRIATE_TRUE : mediamosa_asset_db::IS_INAPPROPRIATE_FALSE;
    }

    if (isset($reference_id)) {
      $fields[mediamosa_asset_db::REFERENCE_ID] = $reference_id;
    }
    if (isset($provider_id)) {
      $fields[mediamosa_asset_db::PROVIDER_ID] = $provider_id;
    }
    if (isset($asset_type)) {
      $fields[mediamosa_asset_db::ASSET_TYPE] = drupal_substr($asset_type, 0, mediamosa_asset_db::ASSET_TYPE_LENGTH);
    }

    if ($is_app_admin) {
      if (!is_null($owner_id)) {
        $fields[mediamosa_asset_db::OWNER_ID] = $owner_id;
      }
      if (!is_null($group_id)) {
        $fields[mediamosa_asset_db::GROUP_ID] = $group_id;
      }
    }

    // No changes, just return.
    if (!count($fields)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_CHANGES);
    }

    // Add changed value.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Update the asset.
    mediamosa_db::db_update(mediamosa_asset_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_db::ID, $asset_id)
      ->execute();

    // When is_inappropriate is updated, we remove all perm. links to all
    // mediafiles.
    if (!is_null($is_unappropriate)) {
      // Remove all perm. links from all mediafiles of this asset.
      mediamosa_media::remove_public_link_asset($asset_id);
    }

    // Change the owner_id / group_id of mediafiles and jobs
    if ($is_app_admin) {
      // Collect the data
      $fields = array();
      if (!is_null($owner_id)) {
        $fields[mediamosa_asset_mediafile_db::OWNER_ID] = $owner_id;
      }
      if (!is_null($group_id)) {
        $fields[mediamosa_asset_mediafile_db::GROUP_ID] = $group_id;
      }
      if (count($fields)) {
        // Change the mediafile table
        mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
          ->fields($fields)
          ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
          ->execute();

        if (!is_null($owner_id)) {
          // Change the job table
          $fields = array();
          $fields[mediamosa_job_db::OWNER_ID] = $owner_id;
          mediamosa_db::db_update(mediamosa_job_db::TABLE_NAME)
            ->fields($fields)
            ->condition(mediamosa_job_db::ASSET_ID, $asset_id)
            ->execute();
        }
      }
    }

    if (isset($reference_id) || isset($provider_id)) {
      static::update_asset_info_is_empty_asset($asset_id, FALSE);
    }

    // Reindex the asset.
    mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_ASSET_UPDATE);
  }

  /**
   * Clone an asset.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param bool $symlink
   *   Create symlink (unused, reserved).
   */
  public static function copy($asset_id, $symlink = TRUE) {

    // @todo: may not be found in search results at any time.

    $asset = self::get($asset_id);

    // Create a new asset, but no reindex yet.
    $new_asset_id = self::create($asset['app_id'], $asset['owner_id'], $asset['group_id'], $asset['reference_id'], $asset['provider_id'], NULL, FALSE);

    // Remove the metadata from destination, copy metadata from source to dest.
    mediamosa_asset_metadata::metadata_copy($asset_id, $new_asset_id, TRUE);

    // Copy mediafiles.
    mediamosa_asset_mediafile::copy($asset_id, $new_asset_id, $symlink);

    // Do more stuff: collections, hooks.

    return $new_asset_id;
  }

  /**
   * The asset must exist.
   *
   * @param string $asset_id
   *   The asset to check.
   * @param int $app_id
   *   (optional) Application ID.
   * @param bool $include_trashcan
   *   Allow exists test on assets in the trashcan, using trashcan metadata flag.
   *
   * @return array|FALSE
   *   Returns the asset row or throws exception.
   */
  public static function must_exists($asset_id, $app_id = NULL, $include_trashcan = FALSE) {
    $query = db_select(mediamosa_asset_db::TABLE_NAME, 'a');
    $query->fields('a');
    $query->condition('a.' . mediamosa_asset_db::ID, $asset_id);

    if (isset($app_id)) {
      $query->condition('a.' . mediamosa_asset_db::APP_ID, $app_id);
    }

    // Exclude trashcan.
    if (!$include_trashcan) {
      $query->join(mediamosa_asset_metadata_property_group_db::TABLE_NAME, 'ampg', 'ampg.' . mediamosa_asset_metadata_property_group_db::NAME . '= :name', array(':name' => mediamosa_metadata_asset::METADATA_PROPERTY_GROUP_NAME));
      $query->join(mediamosa_asset_metadata_property_db::TABLE_NAME, 'amp', 'amp.' . mediamosa_asset_metadata_property_db::PROPGROUP_ID . '= ampg.'. mediamosa_asset_metadata_property_group_db::ID);
      $query->leftJoin(mediamosa_asset_metadata_db::TABLE_NAME, 'am', 'am.' . mediamosa_asset_metadata_db::ASSET_ID . '= a.' . mediamosa_asset_db::ID . ' AND am.' . mediamosa_asset_metadata_db::PROP_ID . '= amp.' . mediamosa_asset_metadata_property_db::ID);
      $query->condition('amp.' . mediamosa_asset_metadata_property_db::NAME, mediamosa_metadata_asset::TRASHCAN);
      // Is null is tested because the asset could have lost all his metadata,
      // e.g. metadata is deleted before the asset is deleted.
      $query->condition(
        db_or()
          ->condition('am.' . mediamosa_asset_metadata_db::VAL_CHAR, 'FALSE')
          ->isNull('am.' . mediamosa_asset_metadata_db::VAL_CHAR)
      );
    }
    $asset = $query->range(0, 1)->execute()->fetchAssoc();
    if (!$asset) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $asset_id));
    }

    return $asset;
  }

  /**
   * Clean up assets that are marked empty asset.
   *
   * @param bool $lifetime_hours
   *   The number of minutes that an empty asset can exist.
   * @param bool $return_status
   *   (FALSE) Do nothing, but return number of garbage assets.
   *
   * @return array
   *   The result.
   */
  public static function garbage_cleanup($lifetime_hours = null, $return_status = FALSE) {

    $lock_name = __CLASS__ . '::' . __FUNCTION__;

    $result = array(
      'cleaned' => 0,
      'failed' => 0,
      'failed_ids' => array(),
    );

    // Lock for an hour.
    if (lock_acquire($lock_name, 3600 - 1)) {
      $lifetime_hours = is_null($lifetime_hours) ? variable_get('mediamosa_asset_garbage_lifetime', mediamosa_settings::ASSET_GARBAGE_LIFETIME) : $lifetime_hours;

      $assets = mediamosa_db::db_query(
        "SELECT t.#asset_id FROM {#mediamosa_asset} AS t WHERE
          t.#is_empty_asset = '#is_empty_asset_true' AND
          UTC_TIMESTAMP() > DATE_ADD(t.#created, INTERVAL #lifetime_hours HOUR)
          ORDER BY t.created ASC",
        array(
          '#lifetime_hours' => $lifetime_hours,
          '#asset_id' => mediamosa_asset_db::ID,
          '#is_empty_asset' => mediamosa_asset_db::IS_EMPTY_ASSET,
          '#is_empty_asset_true' => mediamosa_asset_db::IS_EMPTY_ASSET_TRUE,
          '#mediamosa_asset' => mediamosa_asset_db::TABLE_NAME,
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          '#mediamosa_asset_metadata_property' => mediamosa_asset_metadata_property_db::TABLE_NAME,
          '#id' => mediamosa_asset_metadata_property_db::ID,
          '#is_hidden' => mediamosa_asset_metadata_property_db::IS_HIDDEN,
          '#is_hidden_false' => mediamosa_asset_metadata_property_db::IS_HIDDEN_FALSE,
          '#prop_id' => mediamosa_asset_metadata_db::PROP_ID,
          '#created' => mediamosa_asset_db::CREATED,
        )
      );

      if (!$return_status) {
        foreach ($assets as $asset) {
          try {
            mediamosa_asset::delete($asset["asset_id"], TRUE, FALSE);
            $result['cleaned']++;
          }
          catch (Exception $e) {
            $result['failed_ids'][] = $assets['asset_id'];
          }
        }
        if ($result['cleaned']) {
          mediamosa_watchdog::log('Garbage cleanup: @cleaned assets deleted.', array('@cleaned' => $result['cleaned']), WATCHDOG_INFO, 'mediamosa_asset');
        }
        else {
          mediamosa_watchdog::log('Garbage cleanup: no assets to clean.', array(), WATCHDOG_INFO, 'mediamosa_asset');
        }
        if (count($result['failed_ids']) > 0) {
          mediamosa_watchdog::log("Garbage cleanup failed for assets with IDs: '@assets'.", array('@assets' => implode("', '", $result['failed_ids'])), WATCHDOG_WARNING, 'mediamosa_asset');
        }
      }
      else {
        $result['cleaned'] = $assets->rowCount();
      }

      lock_release($lock_name);
    }
    else {
      mediamosa_watchdog::log('Garbage cleanup: unable to get exclusive lock.', array(), WATCHDOG_INFO, 'mediamosa_asset');
    }

    $result['failed'] = count($result['failed_ids']);

    return $result;
  }

  /**
   * Retrieve all data for asset needed to index for external database.
   *
   * @param $asset_id
   *   The asset ID of the asset.
   *
   * @return
   *  Returns FALSE if asset does not exists.
   */
  public static function get_asset_for_index($asset_id) {

    // Get asset.
    $result['asset'] = self::get($asset_id);

    // Found asset?
    if (empty($result['asset'])) {
      return FALSE;
    }

    // Get metadata.
    $result['asset_metadata'] = mediamosa_asset_metadata::metadata_get($asset_id);

    // Get possible batch links.
    $result['batch_ids'] = mediamosa_ftp_batch_asset::get_batch_relation($asset_id);

    // Get mediafiles and metadata.
    $result['mediamosa_asset_mediafile_metadata'] = mediamosa_asset_mediafile_metadata::get_all_mediafile_metadata_for_asset($asset_id);

    // Get possible collection links.
    $result['collections'] = mediamosa_asset_collection::get_collections($asset_id);

    // Get all fav. of this asset.
    $result['user_favorites'] = mediamosa_user_favorite::get_fav_of_asset($asset_id);

    // Get mediafiles.
    $result['mediafiles'] = mediamosa_asset_mediafile::get_by_asset_id($asset_id);

    // Get acl rules.
    $result['acl'] = mediamosa_acl_object::get_for_asset($asset_id);

    return $result;
  }

  /**
   * Reindex the asset.
   *
   * Triggers hook(s) for reindexing external databases.
   *
   * @param array $asset_ids
   *   Asset ID.
   * @param string $search_index_type
   *   @see mediamosa_settings::search_index_type_*
   */
  public static function mediamosa_asset_reindex(array $asset_ids, $search_index_type) {
    mediamosa_search::asset_reindex($asset_ids, $search_index_type);
  }

  /**
   * Remove asset from external databases.
   *
   * @param array $asset_ids
   *   The assets to remove.
   */
  public static function mediamosa_asset_delete(array $asset_ids) {
    mediamosa_search::asset_delete($asset_ids);
  }

  /**
   * Get listing of the deleted assets.
   *
   * @param int $app_id
   *   The client application.
   * @param int $offset
   *   The record offset.
   * @param int $limit
   *   THe maximum offset.
   * @param string $direction
   *   Either ASC or DESC.
   *
   * @return array
   *   The result.
   */
  public static function get_deleted($app_id, $fields = array(), $offset = 0, $limit = 10, $order_by = mediamosa_asset_delete_db::VIDEOTIMESTAMPMODIFIED, $direction = mediamosa_db::ORDER_DIRECTION_DESC) {
    $assets = mediamosa_db::db_select(mediamosa_asset_delete_db::TABLE_NAME, 'ad')
      ->fields('ad', $fields)
      ->condition('app_id', $app_id)
      ->orderBy($order_by, $direction)
      ->range($offset, $limit)
      ->groupBy(mediamosa_asset_delete_db::ID)
      ->execute();

    $asset_deleted = array();
    foreach ($assets as $asset) {
      unset($asset['coll_id']);
      $asset['coll_ids'] = array();
      $asset_deleted[$asset[mediamosa_asset_delete_db::ID]] = $asset;
    }

    if (!empty($asset_deleted)) {
      $collections = mediamosa_db::db_select(mediamosa_asset_delete_db::TABLE_NAME, 'ad')
        ->fields('ad', array(mediamosa_asset_delete_db::ID, mediamosa_asset_delete_db::COLL_ID))
        ->condition(mediamosa_asset_delete_db::ID, array_keys($asset_deleted))
        ->condition(db_and()->isNotNull(mediamosa_asset_delete_db::COLL_ID)->condition(mediamosa_asset_delete_db::COLL_ID, '', '!='))
        ->orderBy(mediamosa_asset_delete_db::COLL_ID, 'ASC')
        ->execute();

      foreach ($collections as $collection) {
        $asset_deleted[$collection[mediamosa_asset_delete_db::ID]]['coll_ids'][$collection[mediamosa_asset_delete_db::COLL_ID]] = array($collection[mediamosa_asset_delete_db::COLL_ID]);
      }
    }

    $item_count_total = mediamosa_db::db_count_rows(mediamosa_asset_delete_db::TABLE_NAME, array('app_id' => $app_id));
    return array('asset_deleted' => $asset_deleted, 'item_count_total' => $item_count_total);
  }

  /**
   * Publish the asset.
   *
   * @param string $asset_id
   *   The asset to publish.
   * @param bool $reindex
   *   Reindex the asset metadata.
   */
  public static function publish($asset_id, $reindex = TRUE) {
    mediamosa_asset_metadata::metadata_asset_create($asset_id, array(self::METADATA_PUBLISHED => array(self::METADATA_PUBLISHED_TRUE)));

    if ($reindex) {
      mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_METADATA);
    }
  }

  /**
   * Unpublish the asset.
   *
   * @param string $asset_id
   *   The asset to unpublish.
   * @param bool $reindex
   *   Reindex the asset metadata.
   */
  public static function unpublish($asset_id, $reindex = TRUE) {
    mediamosa_asset_metadata::metadata_asset_create($asset_id, array(self::METADATA_PUBLISHED => array(self::METADATA_PUBLISHED_FALSE)));

    if ($reindex) {
      mediamosa_asset::mediamosa_asset_reindex(array($asset_id), mediamosa_settings::SEARCH_INDEX_TYPE_METADATA);
    }
  }
}
