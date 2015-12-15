<?php
/**
 * @file
 * API documentation MediaMosa.
 *
 * @todo: doc api hooks;
 *
 * hook_mediamosa_configuration_collect
 * hook_mediamosa_configuration_validate
 * hook_mediamosa_configuration_submit
 * hook_mediamosa_storage_info
 * hook_mediamosa_tool_can_analyse
 * hook_mediamosa_tool_can_generate_still
 * hook_mediamosa_tool_info
 * hook_mediamosa_tool_mapping
 * hook_mediamosa_tool_param_checking
 * hook_mediamosa_tool_analyse
 * hook_mediamosa_asset_reindex
 * hook_mediamosa_asset_index_delete
 * hook_mediamosa_asset_queue
 * hook_mediamosa_register_rest_call
 * hook_mediamosa_register_rest_call_doc
 * hook_mediamosa_simpletest_clean_environment
 * hook_mediamosa_status_collect
 * hook_mediamosa_status_collect_realtime
 * hook_mediamosa_response_get
 * hook_mediamosa_pass_call_drupal (needs rename).
 * hook_mediamosa_rest_call_var_setup
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Return information about an metadata set.
 *
 * The metadata info hook describes the metadata set. The hook allows
 * registration of an metadata set to be used in MediaMosa.
 *
 * @return array
 *   An associative array, where main key is the main ID of the metadata set.
 *   - 'title'
 *     The title of the mediadata set.
 *   - 'description'
 *     The description of the metadata set.
 *   - 'context'
 *     A unigue ID, used as context ID: only alphanum only.
 *   - 'context_full'
 *     A unique long ID, alphum / underscore only.
 */
function hook_mediamosa_metadata_info() {
  return array(
    'dublin_core' => array(
      'title' => t('Dublin Core'),
      'description' => t('MediaMosa support for the metadata format Dublin Core. See !link for more information.', array('!link' => url('http://dublincore.org/'))),
      'context' => 'dc',
      'context_full' => 'dublin_core',
    ),
  );
}

/**
 * Return information about an search engine.
 *
 * Note:
 * Hook been renamed from 'mediamosa_search_engine' to
 * 'mediamosa_search_engine_info'.
 *
 * @return array
 *   An associative array, where main key is the main ID of the search engine.
 *   - 'title'
 *     The title of the search engine.
 *   - 'description'
 *     The description of the search engine.
 */
function hook_mediamosa_search_engine_info() {

  // Return the information about the default search engine.
  return array(
    'mediamosa_search' => array(
      'title' => t('MediaMosa default search'),
      'description' => t('The default search engine for searching in MediaMosa. This search engine is always available.'),
    ),
  );
}

/**
 * Process ACL changes to an object.
 *
 * When hook is called, the ACL right change has already been processed and the
 * ACL change was allowed.
 *
 * @param array $object
 *   The object that had the ACL change. This can be a asset,  mediafile or
 *   collection.
 * @param string $op
 *   The operation performed;
 *   - 'clear'
 *     All rights where cleared from the object.
 *   - 'add'
 *     Some rights where added.
 *   - 'replace'
 *     All rights where cleared and replaced.
 * @param string $acl_type
 *   The type of ACL, see mediamosa_acl::ACL_TYPE_*.
 * @param int $app_id
 *   The application ID that called the ACL change.
 * @param string $user_id
 *   The owner of the object.
 * @param bool $is_app_admin
 *   Is application admin.
 * @param array $acl_data
 *   An associative array containing the data for the 'set' operation. Is empty
 *   when called with operator 'clear'.
 *   - 'app_id_slaves'
 *     (array) Array of slaves set on app_id.
 *   - 'acl_user_ids' => $acl_user_ids,
 *     (array) Array of ACL user IDs.
 *   - 'acl_group_ids'
 *     (array) Array of ACL group user IDs.
 *   - 'acl_domains'
 *     (array) Array of ACL domains.
 *   - 'acl_realms'
 *     (array) Array of ACL realms.
 *   - 'rights'
 *     (array) Array of ACL rights (reserved). Is right default is
 *      mediamosa_acl::RIGHT_ACCESS.
 */
function hook_mediamosa_acl(array $object, $op, $acl_type, $app_id, $user_id, $is_app_admin, array $acl_data) {
}

/**
 * Authorize the client application.
 *
 * @return bool
 *   Return TRUE when authorized, or FALSE otherwise.
 */
function hook_mediamosa_app_authorized() {

  // Simple example to authrize the client application when Drupal user is
  // logged in.
  if (user_is_logged_in()) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Change the version shown.
 *
 * @param array $version
 *   The version array;
 *   - 'major': The major version.
 *   - 'minor': The minor part of version.
 *   - 'release': The release part of version.
 *   - 'build': The current build.
 *   - 'info': The info string. This field is optional and can be used to
 *     indicate special platform or version.
 */
function hook_mediamosa_version_alter(&$version) {
  $version['info'] = 'MediaPlatform 66';
}

/**
 * Define extra mediafile properties for use in tool modules.
 *
 * @return array
 *   An associative array, where main key is the property.
 *   - 'title': (required)
 *     The descriptive title of the property. Title will go through t()
 *     function, do not use t() here.
 *   - 'index': (optional, default: FALSE)
 *     Indicate that field can be searched. Field will be prefixed with 'mfmd_'
 *     for use. In CQL f.e. 'mfmd_example == "^foo^"' or
 *     'mfmd.mfmd_example == "^foo^"' will return all assets that have
 *     mediafiles containing 'foo' in 'example' mediafile metadata field.
 *     MySQL does not support this feature, only external search engines f.e.
 *     Apache Solr.
 *   - 'show': (optional, default: TRUE)
 *     Indicate that this value can be displayed. You can use the hook
 *     hook_mediafile_metadata_propertie_value for displaying the data.
 *   - 'type': (optional, default
 *     mediamosa_asset_mediafile_metadata_db::VAL_CHAR).
 *     Type of value, always defaults to string (val_char).
 *     @see mediamosa_asset_mediafile_metadata_db::VAL_*
 *
 * @see hook_mediafile_metadata_propertie_value()
 */
function hook_mediafile_metadata_properties() {
  // Example property.
  return array(
    // Display metadata value for 'example', will not be indexed for search.
    'example' => array(
      'title' => 'Example property',
      'index' => TRUE,
      'type' => mediamosa_asset_mediafile_metadata_db::VAL_INT,
    ),
  );
}

/**
 * Hook for returning the mediafile metadata value for display.
 *
 * This value is used for indexing and display. Do not HTML encode. Metadata
 * values are optional and can be null or ''. Returning a '' value will display
 * the value. On hook_mediafile_metadata_properties() the property must have
 * 'show' as TRUE (default), else hook will be not be called. The value returned
 *  will not pass through t().
 *
 * For retrieving 'example' field value:
 * hook_mediafile_metadata_property_show_example()
 *
 * @param array $mediafile_metadata
 *   The array with metadata values, $name => $value.
 *
 * @return string
 *   The value of the field.
 */
function hook_mediafile_metadata_property_show_NAME($mediafile_metadata) {
  if (isset($mediafile_metadata['example']) && $mediafile_metadata['example'] !== '') {
    return $mediafile_metadata['example'];
  }
}

/**
 * @} End of "addtogroup hooks".
 */
