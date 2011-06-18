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
 * Contains all roles for Drupal / admin.
 */

/**
 * The MediaMosa permission work from right to left. Having a part of the role
 * name, will grant you access splitted on the '|';
 *
 * mediamosa|subrole will match mediamosa.
 *
 *
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
  const ACCESS_STATUS_REFRESH = 'mediamosa|access|status|refresh'; // Able to refresh the status page.
  const ACCESS_STATUS_COMMON = 'mediamosa|access|status|common';
  const ACCESS_STATUS_SERVERS = 'mediamosa|access|status|servers';
  const ACCESS_STATUS_JOB = 'mediamosa|access|status|job';

  // Browse page.
  const ACCESS_BROWSE = 'mediamosa|access|browse';
  const ACCESS_BROWSE_ASSETS = 'mediamosa|access|browse|assets';
  const ACCESS_BROWSE_COLLECTIONS = 'mediamosa|access|browse|collections';
  const ACCESS_BROWSE_RESTCALLS = 'mediamosa|access|browse|restcalls';
  const ACCESS_BROWSE_REVISIONS = 'mediamosa|access|browse|revisions';
  const ACCESS_BROWSE_LOG = 'mediamosa|access|browse|log';
  const ACCESS_BROWSE_ERRORCODES = 'mediamosa|access|browse|errorcodes';
  const ACCESS_BROWSE_TESTING = 'mediamosa|access|browse|testing';
  const ACCESS_BROWSE_BATCH = 'mediamosa|access|browse|batch';
  const ACCESS_BROWSE_FILE_INTEGRITY = 'mediamosa|access|browse|file_integrity';

  // Start right.
  const ACCESS_START_FILE_INTEGRITY_START = 'mediamosa|start|file_integrity';

  // Edit options browse page.
  const ACCESS_EDIT_ERRORCODES = 'mediamosa|update|errorcodes';

  // Config.
  const ACCESS_CONFIG = 'mediamosa|access|config';
  const ACCESS_CONFIG_SERVERS = 'mediamosa|access|config|servers';
  const ACCESS_CONFIG_APPS = 'mediamosa|access|config|apps';
  const ACCESS_CONFIG_GLOBAL = 'mediamosa|access|config|global';
  const ACCESS_CONFIG_DEVELOPMENT = 'mediamosa|access|config|development';
  const ACCESS_CONFIG_TOOLPARAMS = 'mediamosa|access|config|toolparams';
  const ACCESS_CONFIG_TOOLMAPPING = 'mediamosa|access|config|toolmapping';
  const ACCESS_CONFIG_TRANSCODE_PROFILES = 'mediamosa|access|config|transcode_profiles';
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
  static public function is_app_owner_asset($asset_id) {
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
