<?php
/**
 * @file
 * Global settings file.
 */

class mediamosa_settings {

  // ------------------------------------------------------------------ Consts.
  const SEARCH_INDEX_TYPE_ASSET_CREATE = 'asset_create';
  const SEARCH_INDEX_TYPE_ASSET_UPDATE = 'asset_update';
  const SEARCH_INDEX_TYPE_ASSET_NORMALIZE = 'asset_normalize';
  const SEARCH_INDEX_TYPE_MEDIAFILE_UPDATE = 'mediafile_update';
  const SEARCH_INDEX_TYPE_ASSET_COLLECTION = 'asset_collection';
  const SEARCH_INDEX_TYPE_METADATA = 'metadata';
  const SEARCH_INDEX_TYPE_FAV_USER = 'fav_user';
  const SEARCH_INDEX_TYPE_MASTER_SLAVE = 'master_slave';
  const SEARCH_INDEX_TYPE_ACL = 'acl';

  const MINIMUM_PHP_VERSION = '5.2.4';

  // Length of the unique ID generator, is always this length.
  // @deprecated: moved to mediamosa_sdk class, use that one instead.
  const UUID_LENGTH = 24;

  // URL to asset. (lib function mediamosa_lib::l_asset($asset_id, $text);)
  const URL_ASSET = 'admin/mediamosa/content/asset/@asset_id';

  // URL to collection. (lib function mediamosa_lib::l_collection($coll_id, $text);)
  const URL_COLLECTION = 'admin/mediamosa/content/collection/@coll_id';

  // Path to icon.
  const PATH_ICON = '/lib/misc/mediamosa-logo-16x16.png';

  // Path to logo.
  const PATH_LOGO = '/lib/misc/mediamosa-logo.png';


  // Atom fields.
  const ATOM_NAME = 'MediaMosa';
  const ATOM_GENERATOR = 'MediaMosa Atom API';
  const ATOM_GENERATOR_URI = 'http://mediamosa.org/';
  const ATOM_GENERATOR_VERSION = '1.0';
  const ATOM_ID = 'MediaMosa:feed:atom:search';
  const ATOM_SUBTITLE = 'Videos from MediaMosa';
  const ATOM_TITLE = 'MediaMosa Atom Feed';
  const ATOM_ENTRY_CATEGORY_MAX = 5;
  const ATOM_ENTRY_ID = 'MediaMosa:asset:';
  // RSS.
  const RSS_GENERATOR = 'MediaMosa RSS API';

  // Max Limits.
  const LIMIT_DEFAULT = 10;
  const LIMIT_DEFAULT_STATISTICS = 50;
  const LIMIT_DEFAULT_STATISTICS_NO_PAGER = 150;
  const LIMIT_DEFAULT_JOBLIST = 200;

  const LIMIT_MAX = 200;
  const LIMIT_MAX_IDS_ONLY = 5000;
  const LIMIT_MAX_JOBLIST = 200;

  // Max. supplements per asset.
  const MAX_SUPPLEMENT_PER_ASSET = 16;

  // Still Settings.

  // Maximum number of still per video file
  // See jobserver_job table mediafile_dest field size
  // Notice: One still length is around 32 + 5 byte (because of serialization).
  const STILL_MAXIMUM = 100;
  // Still names extension on the case of multiple images
  // See: lua/vpx_transcode file too, it has the same constant!
  // If you want to change, you have to do it on both places.
  const STILL_EXTENSION = '-%03d';

  // Maximum still file size in byte.
  const STILL_FILE_MAXIMUM = 1000000;

  // Scene still: The still is THIS number of frames after the detected still
  // change.
  const STILL_SCENE_AFTER = 5;
  // Scene still: Minimal distance between two stills in frames.
  const STILL_SCENE_DISTANCE = 25;

  // Time in seconds at which a still is made in a movie, if the movie is
  // shorter than 10 seconds, half the movie length is used.
  const STILL_DEFAULT_FRAME_TIME = 10;
  // Still default size 640x360.
  const STILL_DEFAULT_SIZE = '640x360';
  // Stills are generated with STILL_INTERVAL as interval in seconds.
  const STILL_INTERVAL = 1;
  // If you wish to space the interval (STILL_INTERVAL) every still being
  // created (STILLS_AMOUNT). 1 for equal spaced intervals.
  const STILL_INTERVAL_JITTER = 1;
  // Minimal amount of colors that need to be present in a still to be valid.
  const STILL_MIN_COLORS = 8;
  // Amount of extra stills to create to get the best still.
  const STILLS_AMOUNT = 4;
  // To check the stills the amount of colors is calculated, to improved speed
  // stills are resampled. Lower sizes is faster but less accurate.
  const STILL_RESAMPLE_WIDTH = 150;
  const STILL_RESAMPLE_HEIGHT = 150;

  // Media cache max time: 60*60*24*30 = 2592000 sec (30 days).
  const MEDIA_CACHE_SECOND = 2592000;

  // FFmpeg.

  const STILL_STRING = "' -s %s -padtop %d -padbottom %d -padleft %d -padright %d -vframes %d -an -deinterlace -y -ss %s -t %s -r %s -vcodec mjpeg -f image2'";
  const STILL_STRING_VFPAD = "' -s %s -vf 'pad=%d:%d:%d:%d:black' -vframes %d -an -deinterlace -y -ss %s -t %s -r %s -vcodec mjpeg -f image2'";

  const STILL_SCENE_STRING = 'ffmpeg -i %s -s %s -padtop %d -padbottom %d -padleft %d -padright %d -an -deinterlace -y -scene %s %s -r 1 -f image2 %s';
  const STILL_SCENE_STRING_VFPAD = 'ffmpeg -i %s -s %s -vf "pad=%d:%d:%d:%d:black" -an -deinterlace -y -scene %s %s -r 1 -f image2 %s';

  // Analyse settings.
  const ANALYSE_FILE_ALWAYS_HINT_MP4_OPTION = '--always_hint_mp4';
  const ANALYSE_FILE_ALWAYS_INSERT_MD_OPTION = '--always_insert_metadata';

  // Job timeout settings.
  const JOB_UPLOAD_TIMEOUT = 10800;
  const JOB_JOB_TIMEOUT = 10800;
  const JOB_TRANSCODE_TIMEOUT = 10800;
  // Default amount of transcoding slots per app.
  const JOB_MAX_SLOTS = 5;

  // Header item used in internal calls.
  const X_MEDIAMOSA_INTERNAL_PASS = 'X-MediaMosa-Internal-Pass';
  const X_MEDIAMOSA_VERSION = 'x-MediaMosa-Version';

  // Header item during rest call and simpletest. When running in simpletest we
  // can force to disable App on any interface. If not added to header, all
  // REST calls are considered on a REST (app) interface.
  const X_MEDIAMOSA_DISABLE_APP = 'X-MediaMosa-Disable-App';

  // Upload Settings.
  const UPLOAD_PUT_CHUNK_SIZE = 4096;
  const UPLOAD_PROGRESS_INTERVAL = 5;

  // Asset garbage collector.
  const ASSET_GARBAGE_COLLECTOR_CRON_INTERVAL = 86400;
  const ASSET_GARBAGE_COLLECTOR_ASSET_TIMEOUT = 1209600;

  // Cron timeout for ticket cleanup.
  const TICKET_CRON_INTERVAL = 600;

  // The number of items per page in admin listings, like client apps listing.
  const MAINTENANCE_ITEMS_PER_PAGE = 25;

  // The number of days for browsing the transcode length deviation.
  const MAINTENANCE_BROWSE_TRANSCODE_LENGTH_DEVIATION_DAYS = 14;
  // The deviation in percentage.
  const MAINTENANCE_BROWSE_TRANSCODE_LENGTH_DEVIATION_DEVIATION = 10;

  // The number of maximum tables in the database before we stop the status page
  // of running new simpletests. Will try to clean up if to many tables are
  // found.
  const STATUS_MAX_TABLES = 800;

  // History extention.
  const FTP_HISTORY_EXTENTION = 'history';

  // VUF extgention.
  const FTP_QUEUE_EXTENTION = 'vuf';

  // UI settings.
  // Weights of tab items. -10 left, +10 right.
  const UI_TAB_WEIGHT_LIST = -10;
  const UI_TAB_WEIGHT_VIEW = -10;
  const UI_TAB_WEIGHT_EDIT = 0;
  const UI_TAB_WEIGHT_ADD = -20;
  const UI_TAB_WEIGHT_DELETE = 1;
  const UI_TAB_WEIGHT_REVISIONS = 2;

  // Run at;
  const MEDIAMOSA_RUN = 'mediamosa_run';
  const MEDIAMOSA_RUN_ENABLED = 'mediamosa_run_enabled';
  const MEDIAMOSA_RUN_15_MINUTES  = 'mediamosa_run_15_minutes';
  const MEDIAMOSA_RUN_3_HOURS = 'mediamosa_run_3_hours';
  const MEDIAMOSA_RUN_DAILY = 'mediamosa_run_daily';
  const MEDIAMOSA_RUN_ALL = 'mediamosa_run_all';
  // Run interval in minutes.
  const MEDIAMOSA_RUN_INTERVAL_FAST = 30;
  const MEDIAMOSA_RUN_INTERVAL_NORMAL = 180;
  const MEDIAMOSA_RUN_INTERVAL_SLOW = 1440;

  // Default timezone for apps.
  const MEDIAMOSA_DEFAULT_TIMEZONE = 'Europe/Amsterdam';
  const MEDIAMOSA_DEFAULT_TIMEZONE_INTERNAL = 'UTC';

  // Status page check timeout.
  const MAINTENANCE_STATUS_CRON_TIME = 600;

  // Statistics log maximum process time.
  const STATISTICS_LOG_MAX_TIME = 3600;

  // Chunk size.
  const DOWNLOAD_CHUNK_SIZE = 1048576;

  // Client application settings.
  const APP_ID_MIN = 1;
  const APP_ID_MAX = 9999;

  // Openapi.

  // Name of the variable, that contains the app_id of open client for OpenAPI.
  const MEDIAMOSA_OPENAPI_OPEN_APP_ID = 'mediamosa_openapi_open_app_id';
  // Name of the variable, that define the default openapi response type.
  const MEDIAMOSA_OPENAPI_DEFAULT_RESPONSE_TYPE = 'mediamosa_openapi_default_response_type';
  // Openapi call uri prefix.
  const MEDIAMOSA_OPENAPI_CALL_URI_PREFIX = 'openapi';

  // Header.
  const MEDIAMOSA_OPENAPI_HEADER_HTTP_ACCEPT_TYPE = 'HTTP_ACCEPT_TYPE';
  const MEDIAMOSA_OPENAPI_HEADER_HTTP_ACCEPT = 'HTTP_ACCEPT';
  const MEDIAMOSA_OPENAPI_HEADER_ACCEPT_TYPE = 'ACCEPT_TYPE';

  // Image style.
  const IMAGE_STYLE_SEPARATOR = '_';

  // ------------------------------------------------------------------ Functions.
  /**
   * Returns the path where the lua scripts are.
   */
  public static function lua_base_get() {
    // LUA_BASE.
    $path = DRUPAL_ROOT . '/' . drupal_get_path('module', 'mediamosa');
    return $path . '/lib/lua/';
  }

  /**
   * Return the filename of the transcode  lua file.
   */
  public static function lua_transcode_script() {
    // FFMPEG_TRANSCODE_FILE.
    return self::lua_base_get() . 'vpx-transcode';
  }

  /**
   * Return the filename of the analyse lha file.
   */
  public static function lua_analyse_script() {
    // ANALYSE_FILE.
    return self::lua_base_get() . 'vpx-analyse';
  }

  /**
   * Returns the number of items per page in admin listings.
   */
  public static function maintenance_items_per_page() {
    return variable_get('mediamosa_maintenance_items_per_page', self::MAINTENANCE_ITEMS_PER_PAGE);
  }

  /**
   * Return url to asset.
   *
   * @param string $asset_id
   *   The ID of the asset.
   */
  public static function get_url_asset($asset_id) {
    return strtr(self::URL_ASSET, array('@asset_id' => $asset_id));
  }

  /**
   * Return url to collection.
   *
   * @param string $coll_id
   *   The ID of the collection.
   */
  public static function get_url_collection($coll_id) {
    return strtr(self::URL_COLLECTION, array('@coll_id' => $coll_id));
  }
}
