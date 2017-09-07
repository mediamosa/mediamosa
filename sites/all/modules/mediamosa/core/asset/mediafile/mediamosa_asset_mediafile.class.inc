<?php
/**
 * @file
 * Common function class for mediamosa_asset_mediafile
 */

class mediamosa_asset_mediafile {

  // ------------------------------------------------------------------ Consts.
  const RESPONSE_TRUE = 'TRUE';
  const RESPONSE_FALSE = 'FALSE';

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the mediafile.
   *
   * Provide the app_id when you need to be sure that the mediafile is from
   * specific application.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param int $app_id
   *   (optional) The client application ID.
   * @param array $fields
   *   (optional) The columns to get from database.
   * @param bool $is_still
   *   (optional) Load only when still or when not still type.
   *
   * @return array|boolean
   *   The mediafile or FALSE.
   */
  public static function get($mediafile_id, $app_id = NULL, array $fields = array(), $is_still = NULL) {
    $query =
      mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
        ->fields('mf', $fields)
        ->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);

    if (isset($app_id)) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::APP_ID, $app_id);
    }

    if (isset($is_still)) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, $is_still);
    }

    return $query->execute()->fetchAssoc();
  }

  /**
   * Get the filename from the mediafile.
   *
   * @param array $mediafile_id
   *   The mediafile array.
   * @param string $filename
   *   The filename to use as default.
   *
   * @return string|FALSE
   *   The filename or FALSE.
   */
  public static function get_filename(array $mediafile) {
    // See if we can find the filename.
    return $mediafile[mediamosa_asset_mediafile_db::FILENAME] != '' ? $mediafile[mediamosa_asset_mediafile_db::FILENAME] : $mediafile[mediamosa_asset_mediafile_db::ID];
  }

  /**
   * Find the first parent mediafile that has an filename.
   *
   * @param string $still_id
   *   The still ID.
   * @param bool $parent
   *   (default: FALSE) You must ignore this setting, as its used internally.
   *
   * @return string
   *   The filename.
   */
  public static function get_parent_filename($mediafile_id, $parent = FALSE) {
    // Get mediafile data.
    $mediafile = self::get($mediafile_id, NULL, array(mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, mediamosa_asset_mediafile_db::FILENAME));

    // Has a filename.
    if ($mediafile[mediamosa_asset_mediafile_db::FILENAME] != '') {
      return $mediafile[mediamosa_asset_mediafile_db::FILENAME];
    }

    // Has parent?
    if ($mediafile[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE] != '') {
      $filename = self::get_parent_filename($mediafile[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE], TRUE);

      // Return filename or fall back on ID as filename.
      return empty($filename) ? $mediafile_id : $filename;
    }

    return $parent ? FALSE : $mediafile_id;
  }

  /**
   * Create a filename with valid extension.
   *
   * @param string $destination_uri
   *   Location of the file.
   * @param string $base_filename
   *   The filename without extension. Can be in some case the mediafile ID.
   *
   * @return string
   *   The created filename.
   */
  public static function create_filename($destination_uri, $base_filename) {
    // Get mimetype.
    $mimetype = mediamosa_mimetype::get_mime_type($destination_uri);

    // Get extension.
    $ext = self::get_file_extension_by_mimetype($mimetype);

    // Return the filename.
    return $base_filename . '.' . $ext;
  }

  /**
   * Get the asset_id of the mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   */
  public static function get_asset_id($mediafile_id) {
    $mediafile = self::get($mediafile_id, NULL, array(mediamosa_asset_mediafile_db::ASSET_ID));
    return empty($mediafile[mediamosa_asset_mediafile_db::ASSET_ID]) ? FALSE : $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];
  }

  /**
   * Get all mediafiles of given asset_id.
   *
   * Warning; will also get stills, which are also mediafiles, use
   * exclude_stills to exclude (off by default).
   *
   * @param string $asset_id
   *   The asset ID to get mediafiles from.
   * @param array $fields
   *   Selection of fields to get, keep empty to get all fields.
   * @param array $options
   *   An associative array containing;
   *   - get_originals_only (default: FALSE)
   *     Get only the original mediafiles.
   *   - exclude_stills (default: FALSE)
   *     Do not include stills.
   *   - get_all_children (default: FALSE)
   *     Include any children, in case of asset parenting (future, used).
   *     Provide in case asset parenting is enabled and you need all mediafiles,
   *     e.g. in case of ACL rules changes.
   *   - return_resource (default: FALSE)
   *     Returns an DB resource instead of array with mediafile ID as key. This
   *     will prevent memory problems with larger results.
   *
   * @param string $asset_id
   *   Asset id of request.
   * @param array $fields
   *   Database fields to return.
   * @param array $options
   *   Options for query.
   *
   * @return array
   *   An associative array where key is the mediafile ID.
   */
  public static function get_by_asset_id($asset_id, $fields = array(), $options = array()) {

    $options += array(
      'get_originals_only' => FALSE,
      'exclude_stills' => FALSE,
      'mime_type' => FALSE,
      'filename' => FALSE,
      'limit' => FALSE,
      'orderby' => FALSE,
      'get_all_children' => FALSE,
      'return_resource' => FALSE,
    );

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->fields('mf', $fields)
      ->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID, $asset_id);

    // Only originals.
    if ($options['get_originals_only']) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE);
    }

    // Skip stills.
    if ($options['exclude_stills']) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);
    }

    // Mime Type query.
    if ($options['mime_type']) {
      $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', sprintf('mfm.mediafile_id = mf.mediafile_id '));
      $query->leftJoin(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'mfmp', sprintf('mfmp.prop_id = mfm.prop_id AND mfmp.%s = %d', mediamosa_asset_mediafile_metadata_property_db::NAME, 'mime-type'));
      $query->condition('mfm.val_char', $options['mime_type']);
    }

    // Container query.
    if (isset($options['container_type'])) {
      $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', sprintf('mfm.mediafile_id = mf.mediafile_id '));
      $query->leftJoin(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'mfmp', sprintf('mfmp.prop_id = mfm.prop_id AND mfmp.%s = %d', mediamosa_asset_mediafile_metadata_property_db::NAME, 'container_type'));
      $query->condition('mfm.val_char', $options['container_type']);
    }

    // Video codec query.
    if (isset($options['video_codec'])) {
      $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', sprintf('mfm.mediafile_id = mf.mediafile_id '));
      $query->leftJoin(mediamosa_asset_mediafile_metadata_property_db::TABLE_NAME, 'mfmp', sprintf('mfmp.prop_id = mfm.prop_id AND mfmp.%s = %d', mediamosa_asset_mediafile_metadata_property_db::NAME, 'video_codec'));
      $query->condition('mfm.val_char', $options['video_codec']);
    }

    // Transcode profile query.
    if (isset($options['transcode_profile_id'])) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID, $options['transcode_profile_id']);
    }

    // Transcode tool.
    if (isset($options['tool'])) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::TOOL, $options['tool']);
    }

    // Tag query.
    if (isset($options['tag'])) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::TAG, $options['tag']);
    }

    // Filename (like, % is supported).
    if ($options['filename']) {
      $query->condition('mf.' . mediamosa_asset_mediafile_db::FILENAME, $options['filename'], 'LIKE');
    }

    if ($options['limit'] && is_numeric($options['limit'])) {
      $query->range(0, $options['limit']);
    }

    // Order by.
    if ($options['orderby']) {
      $query->orderBy('mf.' . $options['orderby'], 'ASC');
    }

    // Return resource.
    if ($options['return_resource']) {
      return $query->execute();
    }

    return $query->execute()->fetchAllAssoc(mediamosa_asset_mediafile_db::ID);
  }

  /**
   * Return a matching mediafile based on the asset_id and a profile_id.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param int $profile_id
   *   The transcode profile ID.
   * @param string $original_mediafile_id
   *   The ID of the original mediafile.
   *
   * @return string|FALSE
   *   The matched mediafile ID or FALSE when not found.
   */
  public static function get_mediafile_id_by_profile($asset_id, $profile_id, $original_mediafile_id = NULL) {

    // No original mediafile?
    if (!isset($original_mediafile_id)) {
      return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
        ->fields('mf', array(mediamosa_asset_mediafile_db::ID))
        ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
        ->condition(mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID, $profile_id)
        ->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE)
        ->range(0, 1)
        ->execute()
        ->fetchField();
    }

    // Get the specific transcoded mediafile with provided original mediafile
    // and its profile id.
    return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->fields('mf', array(mediamosa_asset_mediafile_db::ID))
      ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition(mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID, $profile_id)
      ->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE)
      ->condition(mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $original_mediafile_id)
      ->range(0, 1)
      ->execute()
      ->fetchField();
  }

  /**
   * Return a matching mediafile based on the asset_id and a tag.
   *
   * @param string $asset_id
   *   The asset ID.
   * @param string $tag
   *   The tag to search.
   *
   * @return
   *   The matched mediafile ID or FALSE.
   */
  public static function get_mediafile_id_by_tag($asset_id, $tag) {
    return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->fields('mf', array(mediamosa_asset_mediafile_db::ID))
      ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition(mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE)
      ->condition(mediamosa_asset_mediafile_db::TAG, $tag)
      ->range(0, 1)
      ->execute()
      ->fetchField();
  }


  /**
   * Returns the parent asset id of mediafile.
   *
   * FIXME: parent_ids no longer exists.
   *
   * @param string $mediafile_id
   */
  public static function get_my_parent_assetid($mediafile_id) {
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->fields('mf', array(mediamosa_asset_mediafile_db::ASSET_ID));
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_id);
    return $query->execute()->fetchField();
  }

  /**
   * Returns the size of mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile ID to get size from.
   *
   * @return string
   *   Size of the mediafile, or FALSE.
   */
  public static function get_size($mediafile_id) {
    $width = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile_id, 'width');
    if (!$width) {
      return FALSE;
    }

    $height = mediamosa_asset_mediafile_metadata::get_mediafile_metadata_int($mediafile_id, 'height');
    if (!$height) {
      return FALSE;
    }

    return $width . 'x' . $height;
  }

  /**
   * Will test if mediafile exits, will throw when fail.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param int $app_id
   *   (optional) The application ID.
   *
   * @return array
   *   The found mediafile.
   */
  public static function must_exists($mediafile_id, $app_id = NULL) {
    if (isset($app_id)) {
      return mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::APP_ID => $app_id, mediamosa_asset_mediafile_db::ID => $mediafile_id));
    }

    return mediamosa_db::db_must_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));
  }

  /**
   * Will test if mediafile exits, will throw when fail.
   *
   * Does not return the mediafile but boolean.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param int $app_id
   *   (optional) The application ID.
   *
   * @return bool
   *   Returns TRUE when mediafile exists.
   */
  public static function must_exists_cached($mediafile_id, $app_id = 0) {
    // Store when we tested.
    $mediafile_exists = &drupal_static(__FUNCTION__, array());
    if (isset($mediafile_exists[$mediafile_id . '/' . $app_id])) {
      return TRUE;
    }

    // If the array gets to large, then pop the last one.
    if (count($mediafile_exists) >= 1000) {
      array_pop($mediafile_exists);
    }

    if ($app_id) {
      $mediafile_exists[$mediafile_id . '/' . $app_id] = mediamosa_db::db_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id, mediamosa_asset_mediafile_db::APP_ID => $app_id));
    }
    else {
      $mediafile_exists[$mediafile_id . '/' . $app_id] = mediamosa_db::db_exists(mediamosa_asset_mediafile_db::TABLE_NAME, array(mediamosa_asset_mediafile_db::ID => $mediafile_id));
    }

    // Check if we got something back.
    if (!$mediafile_exists[$mediafile_id . '/' . $app_id]) {
       throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $mediafile_id));
    }

    return TRUE;
  }

  /**
   * Remove one or more mediafiles.
   *
   * @param array|string $mixed_mediafile_id
   *   The mediafile ID(s) of the mediafile(s) to remove. Provide an array to
   *   delete multiple.
   */
  public static function delete($mixed_mediafile_id) {
    $mediafile_ids = is_array($mixed_mediafile_id) ? $mixed_mediafile_id : (empty($mixed_mediafile_id) ? array() : array($mixed_mediafile_id));

    // Must have something.
    if (empty($mediafile_ids)) {
      return;
    }

    // Delete mediafile metadata first.
    mediamosa_db::db_delete(mediamosa_asset_mediafile_metadata_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_metadata_db::MEDIAFILE_ID, $mediafile_ids, 'IN')
      ->execute();

    // Delete from SAN/NAS.
    $result = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
      ->fields(
        'mf',
        array(
          mediamosa_asset_mediafile_db::APP_ID,
          mediamosa_asset_mediafile_db::ASSET_ID,
          mediamosa_asset_mediafile_db::ID,
          mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT,
          mediamosa_asset_mediafile_db::IS_STILL,
          mediamosa_asset_mediafile_db::FILENAME,
          mediamosa_asset_mediafile_db::FILE_EXTENSION,
        )
      )
      ->condition(mediamosa_asset_mediafile_db::ID, $mediafile_ids, 'IN')
      ->execute();

    // Collect the asset ids for normalization.
    $normalize_asset_ids = array();

    // Now delete the file of each found mediafile.
    foreach ($result as $mediafile) {
      // Collect asset id.
      $normalize_asset_ids[$mediafile[mediamosa_asset_mediafile_db::ASSET_ID]] = $mediafile[mediamosa_asset_mediafile_db::ASSET_ID];

      // Remove perm. link.
      mediamosa_media::remove_public_link($mediafile[mediamosa_asset_mediafile_db::APP_ID], $mediafile[mediamosa_asset_mediafile_db::ID]);

      // The delete mediafile from transition.
      mediamosa_storage_transition::delete_mediafile($mediafile[mediamosa_asset_mediafile_db::ID]);

      // Unlink mediafile.
      mediamosa_io::unlink(mediamosa_storage::get_uri_mediafile($mediafile));
    }

    // Delete mediafile from DB.
    mediamosa_db::db_delete(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_db::ID, $mediafile_ids, 'IN')
      ->execute();

    // Update the asset information.
    foreach ($normalize_asset_ids as $asset_id) {
      mediamosa_asset::update_asset_info($asset_id);
    }
  }

  /**
   * Delete unofficial mediafiles.
   *
   * @param string $asset_id
   */
  public static function delete_transcoded_unoriginal_mediafiles($asset_id, $mediafile_id) {
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->addField('mf', mediamosa_asset_mediafile_db::ID);
    $query->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id);
    $query->condition(mediamosa_asset_mediafile_db::ID, $mediafile_id);
    $query->condition(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE);
    $result = $query->execute()->fetchCol();

    // Delete mediafiles.
    if (!empty($result)) {
      self::delete($result);
    }
  }

  /**
   * Enrich the response with mediafile data.
   *
   * @param array $mediafile_ids
   *   The mediafiles to include in output.S
   * @param array $app_ids
   *   Either array or integer application ID.
   * @param bool $is_oai
   *   Include extended EGA Play Urls.
   * @param bool $show_stills
   *   Include the stills.
   * @param bool $old_output
   *   Output as pre-2.0.
   */
  public static function enrich_response_mediafile(array $mediafile_ids, array $app_ids, $is_oai, $show_stills = TRUE, $old_output = FALSE) {

    // Can be empty or null.
    if (empty($mediafile_ids) || is_null(reset($mediafile_ids))) {
      return;
    }

    // Get main app.
    $app_id = reset($app_ids);

    // Prefix for metadata in array.
    $prefix = 'mediafile_metadata_';

    // Get the columns from the schema.
    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
    $query->leftJoin(mediamosa_asset_db::TABLE_NAME, 'a', 'mf.asset_id = a.asset_id');
    $query->condition('mf.' . mediamosa_asset_mediafile_db::ID, $mediafile_ids, 'IN');
    $query->fields('mf');

    // Execute query.
    $result = $query->execute();

    // Add to item.
    $items = array();
    foreach ($result as $mediafile) {
      $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::ID];

      // UTC conversion to app dates.
      $mediafile[mediamosa_asset_mediafile_db::CREATED] = mediamosa::utcdate2appdate($mediafile[mediamosa_asset_mediafile_db::CREATED]);
      $mediafile[mediamosa_asset_mediafile_db::CHANGED] = mediamosa::utcdate2appdate($mediafile[mediamosa_asset_mediafile_db::CHANGED]);

      $items[$mediafile_id] = $mediafile;

      // Get metadata.
      $metadata = mediamosa_asset_mediafile_metadata::get_all_mediafile_metadata($mediafile_id);

      // @todo: move these into the tool hook.
      $mediafile_metadata_properties = array(
        mediamosa_asset_mediafile_metadata::VIDEO_CODEC,
        mediamosa_asset_mediafile_metadata::COLORSPACE,
        mediamosa_asset_mediafile_metadata::WIDTH,
        mediamosa_asset_mediafile_metadata::HEIGHT,
        mediamosa_asset_mediafile_metadata::FPS,
        mediamosa_asset_mediafile_metadata::AUDIO_CODEC,
        mediamosa_asset_mediafile_metadata::SAMPLE_RATE,
        mediamosa_asset_mediafile_metadata::CHANNELS,
        mediamosa_asset_mediafile_metadata::FILE_DURATION,
        mediamosa_asset_mediafile_metadata::CONTAINER_TYPE,
        mediamosa_asset_mediafile_metadata::BITRATE,
        mediamosa_asset_mediafile_metadata::BPP,
        mediamosa_asset_mediafile_metadata::FILESIZE,
        mediamosa_asset_mediafile_metadata::MD5,
        mediamosa_asset_mediafile_metadata::MIME_TYPE,
        mediamosa_asset_mediafile_metadata::IS_HINTED,
        mediamosa_asset_mediafile_metadata::IS_INSERTED_MD,
      );

      // Allow tool modules to add extra properties.
      drupal_alter('mediafile_metadata_properties', $mediafile_metadata_properties);

      // Add them.
      foreach ($mediafile_metadata_properties as $mediafile_metadata_property) {
        if (isset($metadata[$mediafile_metadata_property])) {
          $items[$mediafile_id][$prefix . $mediafile_metadata_property] = $metadata[$mediafile_metadata_property];
        }
        else {
          $items[$mediafile_id][$prefix . $mediafile_metadata_property] = '';
        }
      }
    }

    // Now we know which mediafiles where found.
    $mediafile_ids_found = array_keys($items);

    // Lets include the still, if we need them.
    if (!empty($mediafile_ids_found) && $show_stills) {
      $prop_id = mediamosa_asset_mediafile_metadata_property::get_property_id_int(mediamosa_asset_mediafile_metadata::STILL_ORDER);

      // Get the stills.
      $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf');
      $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', sprintf('mfm.mediafile_id = mf.mediafile_id AND mfm.%s = %d', mediamosa_asset_mediafile_metadata_db::PROP_ID, $prop_id));
      $result = $query
        ->fields('mf')
        ->condition('mf.' . mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $mediafile_ids_found, 'IN')
        ->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_TRUE)
        ->orderBy('mfm.' . mediamosa_asset_mediafile_metadata_db::VAL_INT, 'ASC')
        ->execute();

      $i = 0;
      foreach ($result as $still) {
        try {
          // Create response.
          $response = mediamosa_media::do_response_still($app_id, $still[mediamosa_asset_mediafile_db::OWNER_ID], FALSE, $still);
          $still['still_ticket'] = $response['output'];

          // Add still to mediafile.
          assert(!empty($still[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE])); // Make sure its here.

          $mediafile_id = $still[mediamosa_asset_mediafile_db::ID];
          $still = array_merge($still, mediamosa_asset_mediafile_metadata::get_all_mediafile_metadata($mediafile_id));

          // Unset duration.
          unset($still[mediamosa_asset_mediafile_metadata::FILE_DURATION]);

          $items[$still[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]]['still'][$i++] = $still;
        }
        catch (mediamosa_exception $e) {
          // Ignore.
          mediamosa_debug::log_asset($mediafile[mediamosa_asset_mediafile_db::ASSET_ID], "Caught and ignored; \n" . $e->getMessage());
        }
      }
    }

    // Get the app.
    $a_app = mediamosa_app::get_by_appid($app_id);

    $patterns = array(
      '/{asset_id}/i',
      '/{mediafile_id}/i',
      '/{mediafile_filename}/i',
    );

    // Result array.
    $items_result = array();

    $i = 1;

    // Now add to result array.
    // Must be in the same order as $mediafile_ids.
    foreach ($mediafile_ids as $mediafile_id) {
      if (!isset($items[$mediafile_id])) { // In case the mediafile was not found.
        continue;
      }

      // Set empty array for metadata.
      $items[$mediafile_id]['metadata'] = array();

      // Test for downloadable.
      $is_downloadable = mediamosa_lib::boolstr2bool($items[$mediafile_id][mediamosa_asset_mediafile_db::IS_DOWNLOADABLE]);

      // Download URI replacements.
      $replacements = array(
        $items[$mediafile_id][mediamosa_asset_db::ID],
        $items[$mediafile_id][mediamosa_asset_mediafile_db::ID],
        $items[$mediafile_id][mediamosa_asset_mediafile_db::FILENAME],
      );

      // Default empty.
      $items[$mediafile_id]['ega_download_url'] = '';

      // If downloadable and application has its download URL set, then include it in our output.
      if ($is_downloadable && !empty($a_app[mediamosa_app_db::DOWNLOAD_URL])) {

        // Set EGA download url.
        $items[$mediafile_id]['ega_download_url'] = preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::DOWNLOAD_URL]);
      }

      // Default empty.
      $items[$mediafile_id]['ega_stream_url'] = [];

      // Stream URL, only if streamable.
      if (isset($items[$mediafile_id][$prefix . mediamosa_asset_mediafile_metadata::CONTAINER_TYPE]) && mediamosa_server::is_streamable($items[$mediafile_id][$prefix . mediamosa_asset_mediafile_metadata::CONTAINER_TYPE])) {
        $items[$mediafile_id]['ega_play_url'] = preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::PLAY_PROXY_URL]);

        // Output extra?
        if ($is_oai) {
          // Stream URL(s) for OAI.
          $mediafile_app_id = $items[$mediafile_id][mediamosa_asset_mediafile_db::APP_ID];

          // Check if mediafile APP ID is in the requested app_ids.
          if (in_array($mediafile_app_id, $app_ids)) {
            $items[$mediafile_id]['ega_stream_url'][] = !empty($a_app[mediamosa_app_db::STREAM_URL]) ? preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::STREAM_URL]) : '';
          }

          // Get masters of this mediafile.
          $app_id_masters = mediamosa_acl_app_master_slave::master_get($mediafile_app_id, mediamosa_acl::ACL_TYPE_MEDIAFILE, $items[$mediafile_id][mediamosa_asset_mediafile_db::ID]);

          foreach ($app_id_masters as $app_id_master) {
            if (in_array($app_id_master, $app_ids)) {
              // Get the master app.
              $app_tmp = mediamosa_app::get_by_appid($app_id_master);

              // Set the ega_stream_url of the master.
              $items[$mediafile_id]['ega_stream_url'][] = preg_replace($patterns, $replacements, $app_tmp[mediamosa_app_db::STREAM_URL]);
            }
          }
        }
        else {
          $items[$mediafile_id]['ega_stream_url'][] = !empty($a_app[mediamosa_app_db::STREAM_URL]) ? preg_replace($patterns, $replacements, $a_app[mediamosa_app_db::STREAM_URL]) : '';
        }
      }

      // Remove the prefix from the metadata and move into the metadata array of the asset.
      foreach (array_keys($items[$mediafile_id]) as $key) {
        if (mediamosa_unicode::strpos($key, $prefix) === 0) { // is it an metadata tag?
          $new_key = mediamosa_unicode::substr($key, mediamosa_unicode::strlen($prefix));
          $items[$mediafile_id]['metadata'][$new_key] = $items[$mediafile_id][$key];
          unset($items[$mediafile_id][$key]);
        }
      }

      // Response object/plain/metafile available.
      // When file is missing these will be non-existing, make them empty.
      $items[$mediafile_id]['metadata']['container_type'] = isset($items[$mediafile_id]['metadata']['container_type']) ? $items[$mediafile_id]['metadata']['container_type'] : '';
      $items[$mediafile_id]['metadata']['video_codec'] = isset($items[$mediafile_id]['metadata']['video_codec']) ? $items[$mediafile_id]['metadata']['video_codec'] : '';

      $response = mediamosa_server::get_streaming_info($items[$mediafile_id]['metadata']['container_type'], $items[$mediafile_id]['metadata']['video_codec'], $items[$mediafile_id][mediamosa_asset_mediafile_db::URI]);
      // Object.
      $items[$mediafile_id]['response_object_available'] = ($response['response_object_available'] ? self::RESPONSE_TRUE : self::RESPONSE_FALSE);
      // Plain.
      $items[$mediafile_id]['response_plain_available'] = ($response['response_plain_available'] ? self::RESPONSE_TRUE : self::RESPONSE_FALSE);
      // Metafile.
      $items[$mediafile_id]['response_metafile_available'] = ($response['response_metafile_available'] ? self::RESPONSE_TRUE : self::RESPONSE_FALSE);

      // Add to result.
      if ($old_output) {
        $items_result['mediafile_' . $i++] = $items[$mediafile_id];
      }
      else {
        // Add to result.
        $items_result['mediafile']['#' . serialize(array('id' => $i++))] = $items[$mediafile_id];
      }
    }

    return $items_result;
   }

  /**
   * Search the mediafiles of the asset.
   *
   * @param string $asset_id
   * @param string $tag
   * @param bool $is_still
   * @param array $app_ids
   */
  public static function mediafiles_search($asset_id, $tag = NULL, $is_still = FALSE, array $app_ids = array()) {

    // Asset must exist. In trashcan is allowed.
    mediamosa_asset::must_exists($asset_id, NULL, TRUE);

    // FIXME write me to normal db_query.
    $a_query = array();
    $a_query[mediamosa_db_query::A_SELECT_EXPR][] = "mf.mediafile_id";
    $a_query[mediamosa_db_query::A_FROM][] = "{mediamosa_asset} AS a";

    $a_query[mediamosa_db_query::ALLOW_DISTINCT] = FALSE;

    // No left join here, only include assets that have mediafiles
    $a_query[mediamosa_db_query::A_JOIN]["mediafile"][] = "JOIN {mediamosa_asset_mediafile} AS mf ON a.asset_id = mf.asset_id";

    if (!empty($app_ids)) {
      // Filter mediafiles according to the master-slave aut.
      $a_query[mediamosa_db_query::A_JOIN]["mediafile"][] = sprintf("
        JOIN {mediamosa_acl_app_master_slave} AS ms ON ms.acl_object_type = 'MEDIAFILE'
          AND ms.acl_object_id = mf.mediafile_id
          AND ms.app_id_master IN (%s)", implode(',', $app_ids));
    }

    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("a.asset_id = '%s'", mediamosa_db::escape_string($asset_id));
    $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['asset'][] = sprintf("mf.is_still = '%s'", $is_still ? mediamosa_asset_mediafile_db::IS_STILL_TRUE :  mediamosa_asset_mediafile_db::IS_STILL_FALSE);

    if ($tag) {
      $a_query[mediamosa_db_query::A_WHERE][mediamosa_db_query::WHERE_AND]['tag'] = sprintf("mf.tag = '%s'", mediamosa_db::escape_string($tag));
    }

    // Create the query
    $query = mediamosa_db_query::query_select($a_query);

    // Do the query and return found mediafiles as an array with mediafile_ids.
    return mediamosa_db::db_query($query)->fetchCol();
  }

  /**
   * Get all transcoded mediafiles of given asset ID.
   *
   * @param $asset_id
   */
  public static function get_all_transcoded($asset_id) {
    // FIXME: just wondering if it must be asset_root_id here.
    return mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->condition(mediamosa_asset_mediafile_db::ASSET_ID, $asset_id)
      ->condition(mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE)
      ->isNotNull(mediamosa_asset_mediafile_db::TOOL)
      ->isNotNull(mediamosa_asset_mediafile_db::COMMAND)
      ->isNotNull(mediamosa_asset_mediafile_db::FILE_EXTENSION)
      ->execute();
  }

  /**
   * Returns TRUE/FALSE if the mediafile is a original.
   *
   * @param array $mediafile
   *   The mediafile to check.
   *
   * @return bool
   *   Returns TRUE when original, FALSE otherwise.
   */
  public static function is_original(array $mediafile) {
    return ($mediafile[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] === mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE);
  }

  /**
   * Returns TRUE/FALSE if the mediafile is a still.
   *
   * @param array $mediafile
   *   The mediafile to check.
   *
   * @return bool
   *   Returns TRUE when still, FALSE otherwise.
   */
  public static function is_still(array $mediafile) {
    return (isset($mediafile[mediamosa_asset_mediafile_db::IS_STILL]) && $mediafile[mediamosa_asset_mediafile_db::IS_STILL] === mediamosa_asset_mediafile_db::IS_STILL_TRUE);
  }

  /**
   * Returns TRUE/FALSE if the mediafile is external.
   *
   * @param array $mediafile
   *   The mediafile to check.
   *
   * @return bool
   *   Returns TRUE when external, FALSE otherwise.
   */
  public static function is_external(array $mediafile) {
    return ($mediafile[mediamosa_asset_mediafile_db::URI] !== NULL && $mediafile[mediamosa_asset_mediafile_db::URI] !== '');
  }

  /**
   * Update the mediafile.
   *
   * @param int $app_id
   *   The client application ID.
   * @param string $mediafile_id
   *   The MediaFile ID to update.
   * @param string $user_id
   *   The owner of the mediafile.
   * @param array $fields
   *   The mediafile fields to save.
   * @param array $metadatas
   *   Data to store in technical metadata. Not all is allowed.
   *   Format: (consists of array of arrays)
   *   - 'value': the value to store.
   *   - 'prop_name:' the name of the property.
   *   - 'type': the type to store.
   */
  public static function update($app_id, $mediafile_id, $user_id, array $fields, array $metadatas = array()) {
    // Cant mix input of uri/filename/is_downloadable.
    if (isset($fields[mediamosa_asset_mediafile_db::URI]) && (isset($fields[mediamosa_asset_mediafile_db::FILENAME]) || isset($fields[mediamosa_asset_mediafile_db::IS_DOWNLOADABLE]))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MIX_OF_URI_AND_FILE);
    }

    // Mediafile must exists.
    $mediafile = mediamosa_asset_mediafile::must_exists($mediafile_id);

    // Does the user has access?
    mediamosa_acl::owner_check(
      $app_id,
      $user_id,
      $mediafile[mediamosa_asset_mediafile_db::APP_ID],
      $mediafile[mediamosa_asset_mediafile_db::OWNER_ID]
    );

    // Can't change the type.
    if (!empty($mediafile[mediamosa_asset_mediafile_db::URI]) && ( isset($fields[mediamosa_asset_mediafile_db::FILENAME]) || isset($fields[mediamosa_asset_mediafile_db::IS_DOWNLOADABLE]) ) || (!empty($mediafile[mediamosa_asset_mediafile_db::FILENAME]) && isset($fields[mediamosa_asset_mediafile_db::URI])) ) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_CHANGE_URI_AND_FILE);
    }

    // When URI is set, we need to set the mount point to external uri.
    if (!empty($fields[mediamosa_asset_mediafile_db::URI])) {
      $fields[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT] = mediamosa_storage_uri::SCHEME . '://';
    }

    // Update the timestamps of the asset
    mediamosa_asset::update_asset_timestamps($mediafile[mediamosa_asset_mediafile_db::ASSET_ID]);

    // Add changed field.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Build the query.
    mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_mediafile_db::ID, $mediafile_id)
      ->execute();

    mediamosa_asset_mediafile_metadata::create_mediafile_multiple_metadata($mediafile_id, $metadatas);

    // Set the external.
    mediamosa_asset::update_asset_info_is_external($mediafile[mediamosa_asset_mediafile_db::ASSET_ID], FALSE);

    // Set the empty asset.
    mediamosa_asset::update_asset_info_is_empty_asset($mediafile[mediamosa_asset_mediafile_db::ASSET_ID], FALSE);

    // Reindex the asset.
    mediamosa_asset::mediamosa_asset_reindex(array($mediafile[mediamosa_asset_mediafile_db::ASSET_ID]), mediamosa_settings::SEARCH_INDEX_TYPE_MEDIAFILE_UPDATE);
  }

  /**
   * Update the mediafile_id.
   *
   * @param string mediafile_id
   * @param mediafile_dest
   */
  public static function update_mediafile_id($mediafile_id_source, $mediafile_id_dest) {
    // Build the query.
    mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields(array(
        mediamosa_asset_mediafile_db::ID => $mediafile_id_dest,
      ))
      ->condition(mediamosa_asset_mediafile_db::ID, $mediafile_id_source)
      ->execute();
  }

  /**
   * Set new original mediafile.
   *
   * @param mediafile_id
   *   This is the mediafile_id of old original mediafile.
   * @param new_original
   *   This is the mediafile_id of new original mediafile.
   */
  public static function set_new_original($mediafile_id, $new_original) {
    // All mediafile_id_source in the asset are changed to new original mediafile id now.
    $fields = array(
      mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => $new_original,
    );
    // Build the query.
    mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE, $mediafile_id)
      ->execute();

    // The mediafile_id_source of new original mediafile is NULL.
    // And is_original_file = TRUE.
    $fields = array(
      mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => NULL,
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE,
    );
    // Build the query.
    mediamosa_db::db_update(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_mediafile_db::ID, $new_original)
      ->execute();
  }

  /**
   * Create the mediafile.
   *
   * @param string $mediafile_id
   *   Provide the new mediafile_id.
   * @param int $app_id
   *   The client application ID.
   * @param string $asset_id
   *   Parent asset.
   * @param string $user_id
   *   The new owner.
   * @param array $fields
   *   Optional fields. See mediamosa_asset_mediafile_db.
   * @param array $metadatas
   *   Data to store in technical metadata. Not all is allowed.
   *   Format: (consists of array of arrays)
   *   - 'value': the value to store.
   *   - 'prop_name:' the name of the property.
   *   - 'type': the type to store.
   */
  public static function create($mediafile_id, $app_id, $asset_id, $user_id, array $fields = array(), array $metadatas = array()) {

    $fields += array(
      mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE,
      mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => NULL,
    );

    // Is original with mediafile_id_source combo.
    assert(
      (
        isset($fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]) && $fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE] !== ''
        &&
        $fields[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_FALSE
      )
      ||
      (
        (!isset($fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]) || $fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE] === '')
        &&
        $fields[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE
      )
      ||
      (
        (!isset($fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE]) || $fields[mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE] === '')
        &&
        $fields[mediamosa_asset_mediafile_db::URI] !== ''
      )
    );

    $fields[mediamosa_asset_mediafile_db::ID] = $mediafile_id;
    $fields[mediamosa_asset_mediafile_db::APP_ID] = $app_id;
    $fields[mediamosa_asset_mediafile_db::ASSET_ID] = $asset_id;
    $fields[mediamosa_asset_mediafile_db::OWNER_ID] = $user_id;

    // When URI is set, we need to set the mount point to external uri.
    if (!empty($fields[mediamosa_asset_mediafile_db::URI])) {
      $fields[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT] = mediamosa_storage_uri::SCHEME . '://';
    }

    // Enrich for created/changed.
    $fields = mediamosa_db::db_insert_enrich($fields);

    // Insert it.
    mediamosa_db::db_insert(mediamosa_asset_mediafile_db::TABLE_NAME)
      ->fields($fields)
      ->execute();

    // Set the external.
    mediamosa_asset::update_asset_info_is_external($asset_id);

    // Set the empty asset.
    mediamosa_asset::update_asset_info_is_empty_asset($asset_id);

    if (self::is_still($fields)) {
      $metadatas[] = array(
        'value' => 0,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_ORDER,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_INT,
      );
      $metadatas[] = array(
        'value' => mediamosa_asset_mediafile_metadata::STILL_TYPE_NONE,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_TYPE,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      );
      $metadatas[] = array(
        'value' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT_FALSE,
        'prop_name' => mediamosa_asset_mediafile_metadata::STILL_DEFAULT,
        'type' => mediamosa_asset_mediafile_metadata_property_db::TYPE_CHAR,
      );
    }

    mediamosa_asset_mediafile_metadata::create_mediafile_multiple_metadata($mediafile_id, $metadatas);

    // Return the mediafile ID.
    return $mediafile_id;
  }

  /**
   * Reindex the asset.
   *
   * Triggers hook(s) for reindexing external databases.
   *
   * @param array $mediafile_ids
   *   Array of mediafile ids for assets to reindex.
   * @param string $search_index_type
   *   @see mediamosa_settings::search_index_type_*
   */
  public static function mediamosa_asset_reindex(array $mediafile_ids, $search_index_type) {

    $asset_ids = array();
    foreach ($mediafile_ids as $mediafile_id) {
      $asset_id = self::get_asset_id($mediafile_id);
      if (!empty($asset_id)) {
        $asset_ids[] = $asset_id;
      }
    }

    if (!empty($asset_ids)) {
      mediamosa_asset::mediamosa_asset_reindex($asset_ids, $search_index_type);
    }
  }

  /**
   * Returns when mediafile has no ACL rules or when parent asset has no ACL or
   * when asset is not set on unappropriate (FALSE)
   *
   * Will return TRUE when its protected, but you have access.
   *
   * @param $mediafile_id
   *   The ID of mediafile to check.
   * @param bool $is_app_admin
   *   The app admin flag.
   *
   * @throws mediamosa_exception_error_mediafile_not_found()
   * @throws mediamosa_exception_error_asset_not_found()
   * @throws mediamosa_exception_error_access_denied()
   * @throws mediamosa_exception_error_is_inappropriate()
   */
  public static function is_mediafile_protected($mediafile_id, $is_app_admin = FALSE) {
    // Get the mediafile_id, asset_id.
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id);
    if (!$mediafile) {
      throw new mediamosa_exception_error_mediafile_not_found(array('@mediafile_id' => $mediafile_id));
    }

    // Call our extended version.
    return self::is_mediafile_protected_ext($mediafile, $is_app_admin);
  }

  /**
   * Returns when mediafile has no ACL rules or when parent asset has no ACL or
   * when asset is not set on unappropriate (FALSE)
   *
   * Will return TRUE when its protected, but you have access.
   *
   * @param array $mediafile
   *   The mediafile to check.
   * @param bool $is_app_admin
   *   The app admin flag.
   *
   * @throws mediamosa_exception_error_mediafile_not_found()
   * @throws mediamosa_exception_error_asset_not_found()
   * @throws mediamosa_exception_error_access_denied()
   * @throws mediamosa_exception_error_is_inappropriate()
   */
  public static function is_mediafile_protected_ext(array $mediafile, $is_app_admin = FALSE) {

    // Conditions;
    // Mediafile may not be protected.
    // Asset may not be protected.
    // Asset may not be set to unappropriate.

    // Forbidden when protected.
    if ($mediafile[mediamosa_asset_mediafile_db::IS_PROTECTED] != mediamosa_asset_mediafile_db::IS_PROTECTED_FALSE) {
      throw new mediamosa_exception_error_access_denied(array('@reason' => 'mediafile is protected'));
    }

    // Get the asset.
    $asset = mediamosa_asset::get($mediafile[mediamosa_asset_mediafile_db::ASSET_ID]);
    if (!$asset) {
      throw new mediamosa_exception_error_asset_not_found(array('@asset' => $mediafile[mediamosa_asset_mediafile_db::ASSET_ID])); // not found?
    }

    // Forbidden when protected. We dont have enough information to do a media play call here.
    if ($asset[mediamosa_asset_db::IS_PROTECTED] != mediamosa_asset_db::IS_PROTECTED_FALSE) {
      throw new mediamosa_exception_error_access_denied(array('@reason' => 'asset is protected'));
    }

    // Check asset is_unappropriate flag, access only when $is_app_admin == TRUE.
    if ($asset[mediamosa_asset_db::IS_INAPPROPRIATE] != mediamosa_asset_db::IS_INAPPROPRIATE_FALSE && !$is_app_admin) {
      throw new mediamosa_exception_error_is_inappropriate();
    }

    if ($asset[mediamosa_asset_db::IS_INAPPROPRIATE] != mediamosa_asset_db::IS_INAPPROPRIATE_FALSE && $is_app_admin) {
      // Is protected, but you have access. (not fatal).
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Return the file extension for output. Does not require to be the real
   * extension of the filename, but should always reflect the file type.
   *
   * @param array $mediafile
   *   The mediafile.
   * @param string $default
   *  The default value for extension when not available.
   *
   * @return string
   *   The file extension (e.g. 'jpeg') without the dot or FALSE ($default).
   */
  public static function get_file_extension(array $mediafile, $default = FALSE) {

    // Get container type.
    $container_type = self::get_container_type($mediafile[mediamosa_asset_mediafile_db::ID]);

    // Now based on certain extensions, find the extension.
    $container2ext = array (
      mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_FLV => 'flv',
      mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP4 => 'mp4',
      mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MP3 => 'mp3',
      mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_OGG => 'ogg',
    //  mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WMV => 'asf',
      mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_WEBM => 'webm',
      mediamosa_asset_mediafile_metadata::CONTAINER_TYPE_MATROSKA_WEBM => 'webm',
    );

    // Try to match against the container type.
    $file_ext = isset($container2ext[$container_type]) ? $container2ext[$container_type] : '';

    // No extension yet? Try to get one based on mimetype.
    if (empty($file_ext)) {
      $file_ext = $mediafile[mediamosa_asset_mediafile_db::FILE_EXTENSION];
    }

    // Some must return specific extensions.
    $hotfixes = array(
      'asx' => 'asf',
    );

    // Return hotfix when needed.
    if (isset($hotfixes[$file_ext])) {
      return $hotfixes[$file_ext];
    }

    return empty($file_ext) ? $default : $file_ext;
  }

  /**
   * Return the file extension for output. Does not require to be the real
   * extension of the filename, but should always reflect the file type.
   *
   * @param string $mime_type
   *   The mime_type.
   *
   * @return string
   *   The file extension (e.g. 'jpeg') without the dot.
   */
  public static function get_file_extension_by_mimetype($mime_type) {
    // Get mime-type, and match it to a file-ext.
    return mediamosa_mimetype::mimetype2extension($mime_type);
  }

  /**
   * Return the mime type of the mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   *
   * @return string
   *   The mime-type or NULL.
   */
  public static function get_mime_type($mediafile_id) {
    // Get the mime type (content-type) from the mediafile metadata.
    return mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::MIME_TYPE);
  }

  /**
   * Returns the md5 of mediafile, if analyzed.
   *
   * @param string $mediafile_id
   *   The mediafile ID to get md5 from.
   *
   * @return string
   *   MD5 of the mediafile, or FALSE.
   */
  public static function get_md5($mediafile_id) {
    return mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::MD5);
  }

  /**
   * Return the file extension for output. Does not require to be the real
   * extension of the filename, but should always reflect the file type.
   *
   * @param string $mediafile_id
   *   The mediafile ID.
   *
   * @return string
   *   The container type like 'mov;mp4;m4a;3gp;3g2;mj2' or NULL.
   */
  public static function get_container_type($mediafile_id) {
    // Get the container type from the mediafile metadata.
    return mediamosa_asset_mediafile_metadata::get_mediafile_metadata_char($mediafile_id, mediamosa_asset_mediafile_metadata::CONTAINER_TYPE);
  }

  /**
   * Copies all mediafiles from 1 asset to another.
   *
   * @param string $from_asset_id
   *   Copy from the asset.
   * @param string $to_asset_id
   *   Copy to the asset.
   * @param bool $symlink
   *   Create symlink (unused, reserved).
   */
  public static function copy($from_asset_id, $to_asset_id, $symlink = TRUE) {

    $mf_mapping = array();
    $mfs = self::get_by_asset_id($from_asset_id, array('mediafile_id', 'app_id'), array());
    foreach ($mfs as $mf) {

      // Copy a mediafile, first the main record, then the metadata, then the
      // file itself (can be symlinked).

      // Copy mediamosa_asset_mediafile.
      // @todo: mediafile_id_source.
      $mediafile_id_new = mediamosa_db::uuid($mf['app_id']);
      mediamosa_db::db_query("INSERT INTO {#mediamosa_asset_mediafile} " .
        "(mediafile_id, asset_id, mediafile_id_source, tag, app_id, owner_id, group_id, is_original_file, is_downloadable, is_streamable, is_still, " .
        " filename, uri, sannas_mount_point, transcode_profile_id, tool, command, file_extension, is_protected, created, changed, transcode_inherits_acl) " .
        "SELECT :mediafile_id, :asset_id, mediafile_id_source, tag, app_id, owner_id, group_id, is_original_file, is_downloadable, is_streamable, is_still, " .
        " filename, uri, sannas_mount_point, transcode_profile_id, tool, command, file_extension, is_protected, created, changed, transcode_inherits_acl " .
        "FROM {#mediamosa_asset_mediafile} WHERE mediafile_id = :from_mediafile_id",
        array(
          '#mediamosa_asset_mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
          ':mediafile_id' => $mediafile_id_new,
          ':asset_id' => $to_asset_id,
          ':from_mediafile_id' => $mf['mediafile_id'],
        )
      );
      $mf_mapping[$mf['mediafile_id']] = $mediafile_id_new;

      // @todo: acl rights.

      // Copy mediamosa_asset_mediafile_metadata.
      mediamosa_db::db_query("INSERT INTO {#mediamosa_asset_mediafile_metadata} " .
        "(mediafile_id, prop_id, val_char, val_datetime, val_int) " .
        "SELECT :mediafile_id, prop_id, val_char, val_datetime, val_int " .
        "FROM {#mediamosa_asset_mediafile_metadata} WHERE mediafile_id = :from_mediafile_id",
        array(
          '#mediamosa_asset_mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
          ':from_mediafile_id' => $mf['mediafile_id'],
          ':mediafile_id' => $mediafile_id_new,
        )
      );

      // Create a symlink. @todo: or filecopy.
      mediamosa_io::symlink(
        mediamosa_io::realpath(mediamosa_storage::create_storage_uri($mf['app_id'], $mf['mediafile_id'])),
        mediamosa_io::realpath(mediamosa_storage::create_storage_uri($mf['app_id'], $mediafile_id_new))
      );
    }
  }

  /**
   * Get an empty mediafile when given an asset id.
   */
  public static function get_empty_mf_by_asset_id($asset_id) {

    $query = mediamosa_db::db_select(mediamosa_asset_mediafile_db::TABLE_NAME, 'mf')
           ->fields('mf', array('mediafile_id'))
           ->condition('mf.' . mediamosa_asset_mediafile_db::ASSET_ID, $asset_id);

    // Only originals.
    $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE, mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE_TRUE);

    // Skip stills.
    $query->condition('mf.' . mediamosa_asset_mediafile_db::IS_STILL, mediamosa_asset_mediafile_db::IS_STILL_FALSE);

    // No Metadata.
    $query->leftJoin(mediamosa_asset_mediafile_metadata_db::TABLE_NAME, 'mfm', 'mfm.mediafile_id = mf.mediafile_id');
    $query->condition('mfm.val_char', NULL);

    // Transcode profile query.
    $query->condition('mf.' . mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID, NULL);

    // Transcode tool.
    $query->condition('mf.' . mediamosa_asset_mediafile_db::TOOL, NULL);

    // Tag query.
    $query->condition('mf.' . mediamosa_asset_mediafile_db::TAG, NULL);

    // Filename.
    $query->condition('mf.' . mediamosa_asset_mediafile_db::FILENAME, NULL);

    $query->range(0, 1);

    return $query->execute()->fetchField();
  }
}
