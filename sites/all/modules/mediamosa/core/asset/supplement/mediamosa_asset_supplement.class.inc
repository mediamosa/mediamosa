<?php
/**
 * @file
 * Functions of asset supplement.
 */

class mediamosa_asset_supplement {

  // ------------------------------------------------------------------- Consts.
  // The maximum size of the supplement in Kb.
  const DEFAULT_ASSET_SUPPLEMENT_MAX_SIZE = 256;

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the asset supplement.
   *
   * @param string $supplement_id
   */
  public static function get($supplement_id) {

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
  public static function get_all_with_assetid($asset_id) {

    $query = mediamosa_db::db_select(mediamosa_asset_supplement_db::TABLE_NAME, 'aas');
    $query->fields('aas');
    $query->condition('aas.' . mediamosa_asset_supplement_db::ASSET_ID, $asset_id);
    return $query->execute();
  }

  /**
   * Create the supplement.
   *
   * @param int $app_id
   *  The application ID.
   * @param string $asset_id
   *  The asset ID on which to create the supplement on.
   * @param string $user_id
   *  The user ID must be owner of the asset to add supplement.
   */
  public static function create($app_id, $asset_id, $user_id) {

    // Asset must exist for supplement to be inserted.
    $asset = mediamosa_asset::must_exists($asset_id);

    // Check if maximum supplments is reached.
    $rows = mediamosa_db::db_count_rows(mediamosa_asset_supplement_db::TABLE_NAME, array(mediamosa_asset_supplement_db::ASSET_ID => $asset_id));

    if ($rows >= mediamosa_settings::MAX_SUPPLEMENT_PER_ASSET) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SUPPLEMENT_LIMIT_REACHED, array('@max' => mediamosa_settings::MAX_SUPPLEMENT_PER_ASSET));
    }

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID]);

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
   * @param int $app_id
   *  The application ID.
   * @param string $user_id
   *  The user ID, must be owner of parent asset.
   * @param string $supplement_id
   *  The supplement ID to update.
   * @param string $oai_prefix
   * @param string $oai_namespaceuri
   * @param string $oai_schemalocation
   * @param string $supplement_value
   */
  public static function update($app_id, $user_id, $supplement_id, $oai_prefix, $oai_namespaceuri, $oai_schemalocation, $supplement_value) {

    // Supplement must exist.
    $supplement = mediamosa_db::db_must_exists(mediamosa_asset_supplement_db::TABLE_NAME, array(mediamosa_asset_supplement_db::ID => $supplement_id));

    // Get the asset.
    $asset = mediamosa_asset::must_exists($supplement[mediamosa_asset_supplement_db::ASSET_ID]);

    // Must be owner.
    mediamosa_acl::owner_check($app_id, $user_id, $asset[mediamosa_asset_db::APP_ID], $asset[mediamosa_asset_db::OWNER_ID]);

    $fields = array();
    if (!is_null($oai_prefix)) {
      $fields[mediamosa_asset_supplement_db::OAI_PREFIX] = $oai_prefix;
    }
    if (!is_null($oai_namespaceuri)) {
      $fields[mediamosa_asset_supplement_db::OAI_NAMESPACEURI] = $oai_namespaceuri;
    }
    if (!is_null($oai_schemalocation)) {
      $fields[mediamosa_asset_supplement_db::OAI_SCHEMALOCATION] = $oai_schemalocation;
    }
    if (!is_null($supplement)) {
      // Is file upload.
      if (is_array($supplement_value) && isset($supplement_value['tmp_name'])) {
        $supplement_value = file_get_contents($supplement_value['tmp_name']);
      }

      if (!is_string($supplement_value)) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_VALIDATE_FAILED, array('@param' => 'supplement', '@type' => mediamosa_sdk::TYPE_STRING));
      }

      $fields[mediamosa_asset_supplement_db::SUPPLEMENT] = $supplement_value;
    }

    // Must update something.
    if (!count($fields)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_NO_CHANGES);
    }

    // Add changed value.
    $fields = mediamosa_db::db_update_enrich($fields);

    // Update set fields.
    mediamosa_db::db_update(mediamosa_asset_supplement_db::TABLE_NAME)
      ->fields($fields)
      ->condition(mediamosa_asset_supplement_db::ID, $supplement_id)
      ->execute();
  }

  /**
   * Delete the supplement.
   *
   * @param int $app_id
   *  The application ID.
   * @param string $user_id
   *  Must be owner of the asset to delete supplement.
   * @param string $supplement_id
   *  The supplement ID.
   */
  public static function delete($app_id, $user_id, $supplement_id) {

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
