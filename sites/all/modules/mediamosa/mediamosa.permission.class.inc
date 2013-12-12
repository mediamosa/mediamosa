<?php
/**
 * @file
 * Contains all roles for Drupal / admin.
 */

/**
 * The MediaMosa permission work from right to left. Having a part of the role
 * name, will grant you access splitted on the '|';
 *
 * mediamosa|subrole will match mediamosa.
 */
class mediamosa_permission {

  // Name of the function to use for access ('access callback').
  const CALLBACK_USER_ACCESS = '_mediamosa_user_access';

  // Check if user has access to MediaMosa (enables access to front page, even
  // when role has no permissions, but user IS a EGA.
  const CALLBACK_USER_ACCESS_MEDIAMOSA = '_mediamosa_user_access_mediamosa';

  // Super user.
  const ADMIN_MEDIAMOSA = 'mediamosa';

  // Status page.
  const ACCESS_STATUS = 'mediamosa|access|status';
  const ACCESS_STATUS_REFRESH = 'mediamosa|access|status|refresh';
  const ACCESS_STATUS_COMMON = 'mediamosa|access|status|common';
  const ACCESS_STATUS_SERVERS = 'mediamosa|access|status|servers';
  const ACCESS_STATUS_JOB = 'mediamosa|access|status|job';

  // Content page.
  const ACCESS_CONTENT = 'mediamosa|access|content';
  const ACCESS_CONTENT_ASSETS = 'mediamosa|access|content|assets';
  const ACCESS_CONTENT_COLLECTIONS = 'mediamosa|access|content|collections';
  const ACCESS_CONTENT_REVISIONS = 'mediamosa|access|content|revisions';
  const ACCESS_CONTENT_ERRORCODES = 'mediamosa|access|content|errorcodes';

  // Tools page.
  const ACCESS_TOOLS = 'mediamosa|access|tools';
  const ACCESS_TOOLS_RESTCALLS = 'mediamosa|access|tools|restcalls';
  const ACCESS_TOOLS_LOG = 'mediamosa|access|tools|log';
  const ACCESS_TOOLS_TESTING = 'mediamosa|access|tools|testing';
  const ACCESS_TOOLS_TESTING_SETTINGS = 'mediamosa|access|tools|testing|settings';
  const ACCESS_TOOLS_BATCH = 'mediamosa|access|tools|batch';
  const ACCESS_TOOLS_FILE_INTEGRITY = 'mediamosa|access|tools|file_integrity';
  const ACCESS_CONTENT_JOB_FAILURE = 'mediamosa|access|tools|job_failure';
  const ACCESS_TOOLS_TRANSCODE_LENGTH_DEVIATION = 'mediamosa|access|tools|transcode_length_deviation';
  const ACCESS_TOOLS_UPLOAD = 'mediamosa|access|tools|upload';
  const ACCESS_TOOLS_SUPPLIER = 'mediamosa|access|tools|supplier';

  // Start right.
  const ACCESS_START_FILE_INTEGRITY_START = 'mediamosa|start|file_integrity';

  // Edit options browse page.
  const ACCESS_EDIT_ERRORCODES = 'mediamosa|update|errorcodes';

  // Config.
  const ACCESS_CONFIG = 'mediamosa|access|config';
  const ACCESS_CONFIG_SERVERS = 'mediamosa|access|config|servers';
  const ACCESS_CONFIG_VIEWERS = 'mediamosa|access|config|viewers';
  const UPDATE_CONFIG_VIEWERS = 'mediamosa|update|config|viewers';
  const ACCESS_CONFIG_APPS = 'mediamosa|access|config|apps';
  const ACCESS_CONFIG_GLOBAL = 'mediamosa|access|config|global';
  const ACCESS_CONFIG_DEVELOPMENT = 'mediamosa|access|config|development';
  const ACCESS_CONFIG_TOOLPARAMS = 'mediamosa|access|config|toolparams';
  const ACCESS_CONFIG_TOOLMAPPING = 'mediamosa|access|config|toolmapping';
  const ACCESS_CONFIG_TRANSCODE_PROFILES = 'mediamosa|access|config|transcode_profiles';
  const ACCESS_CONFIG_STORAGE_PROFILES = 'mediamosa|access|config|storage_profiles';
  const ACCESS_CONFIG_WEBSERVICES = 'mediamosa|access|config|webservices';
  const UPDATE_CONFIG_WEBSERVICES = 'mediamosa|update|config|webservices';

  // hmmm.
  const ACCESS_DRUPAL = 'mediamosa|drupal page';

  // Statistics.
  const ACCESS_STATS = 'mediamosa|access|stats';
  const ACCESS_STATS_DISKSPACE_USED = 'mediamosa|access|stats|diskspace used';
  const ACCESS_STATS_LATEST_COLLECTIONS = 'mediamosa|access|stats|latest collection';
  const ACCESS_STATS_LATEST_MEDIAFILES = 'mediamosa|access|stats|latest mediafile';
  const ACCESS_STATS_SEARCH_WORDS = 'mediamosa|access|stats|search words';
  const ACCESS_STATS_STREAM_REQUEST = 'mediamosa|access|stats|stream request';

  // ---------------------------------------------------------------- Functions.
  /**
   * Test for ownership asset through client application.
   *
   * @param string $asset_id
   *   The asset ID.
   *
   * @return boolean
   *   Returns TRUE for ownership client application.
   */
  public static function is_app_owner_asset($asset_id) {
    $assets = &drupal_static(__CLASS__ . '::' . __FUNCTION__, array());

    if (empty($assets[$asset_id])) {
      $assets[$asset_id] = mediamosa_asset::get($asset_id);
    }

    if (empty($assets[$asset_id])) {
      return FALSE;
    }

    // Get app_id.
    $app_id = $assets[$asset_id]['app_id'];

    // Has app access?
    return _mediamosa_user_access(mediamosa_permission::ADMIN_MEDIAMOSA . '|app|' . $app_id) || user_access(mediamosa_permission::ADMIN_MEDIAMOSA);
  }
}
