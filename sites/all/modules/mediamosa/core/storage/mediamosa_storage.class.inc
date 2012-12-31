<?php
/**
 * @file
 * main storage class.
 */

abstract class mediamosa_storage {
  // ------------------------------------------------------------------- Consts.
  const STORAGE_FOLDER_TICKET = 'ticket';

  // --------------------------------------------------------------------- Vars.
  /**
   * The storage profile array.
   *
   * @var mediamosa_storage
   */
  protected $storage_profile;

  /**
   * The applications linked to profile for mediafiles.
   *
   * @var mediamosa_storage_app
   */
  protected $storage_profile_apps;

  /**
   * The applications linked to profile for stills.
   *
   * @var mediamosa_storage_app
   */
  protected $storage_profile_apps_stills;

  // ----------------------------------------------------- Functions (abstract).
  /**
   * Get the default scheme (without the '://').
   *
   * @param boolean $is_still
   *   Is mount point for still?
   *
   * @return string
   *   The uri of the moint point to store.
   */
  public abstract static function get_scheme($is_still = FALSE);

  /**
   * Return the storage options available for the storage type
   *
   * @return
   *   Array of available storage options
   */
  public abstract function get_storage_options();

  /**
   * Return alle required upload parameters for the storage type
   * which allow a frontend application to build the correct upload form
   *
   * $param $app_id
   *   Application ID.
   *
   * @param $user_id
   *   User ID
   * @param $group_id
   *   Group ID
   *
   * @return
   *   Array with upload parameters
   */
  public abstract function get_upload_parameters($app_id, $user_id, $group_id);

  // -------------------------------------------------------------- Constructor.
  /**
   * Main constructor
   *
   * @param array $storage_profile
   *   Storage profile.
   */
  public function __construct(array $storage_profile) {
    $this->storage_profile = $storage_profile;

    // Get all storage profiles apps.
    $this->load_all_storage_profile_apps();
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Return with uri of the profile mount point.
   *
   * Unless local mount point, the uri must include $storage_profile::ID and
   * $storage_profile_app::PATH or $storage_profile_app::PATH_STILLS.
   *
   * @param integer $storage_profile_id
   *   The storage profile ID.
   * @param string $path
   *   The storage path.
   * @param boolean $is_still
   *   Is mount point for still?
   *
   * @return string
   *   The uri of the storage profile.
   */
  public function build_mount_point_uri($storage_profile_id, $path, $is_still = FALSE) {
    return self::trim_uri(static::get_scheme($is_still) . '://' . $storage_profile_id) . $path;
  }

  /**
   * Is storage object local or external?
   *
   * @return boolean
   *   Return TRUE when all data is stored on local storage.
   */
  public function is_local() {
    return TRUE;
  }

  /**
   * Load all linked storage_profile_apps.
   */
  protected function load_all_storage_profile_apps() {
    // Build the query.
    $storage_profile_apps = mediamosa_db::db_select(mediamosa_storage_profile_app_db::TABLE_NAME, 'spa')
      ->condition(db_or()
        ->condition(mediamosa_storage_profile_app_db::PROFILE_ID, $this->storage_profile[mediamosa_storage_profile_db::ID])
        ->condition(mediamosa_storage_profile_app_db::PROFILE_ID_STILLS, $this->storage_profile[mediamosa_storage_profile_db::ID])
      )
      ->fields('spa')
      ->execute();

    // Storage profiles for mediafiles.
    $this->storage_profiles_apps = array();

    // Storage profiles for stills.
    $this->storage_profiles_apps_stills = array();

    // Store them.
    foreach ($storage_profile_apps as $storage_profile_app) {
      if ($storage_profile_app[mediamosa_storage_profile_app_db::PROFILE_ID] == $this->storage_profile[mediamosa_storage_profile_db::ID]) {
        $this->storage_profile_apps[$storage_profile_app[mediamosa_storage_profile_app_db::APP_ID]] = $storage_profile_app;
      }

      if ($storage_profile_app[mediamosa_storage_profile_app_db::PROFILE_ID_STILLS] == $this->storage_profile[mediamosa_storage_profile_db::ID]) {
        $this->storage_profiles_apps_stills[$storage_profile_app[mediamosa_storage_profile_app_db::APP_ID]] = $storage_profile_app;
      }
    }
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Is the storage file system path specified?
   *
   * @return boolean
   *   Returns TRUE when storage file system path has been specified.
   */
  public static function is_storage_setup() {
    return mediamosa_io::realpath('mediamosa-storage://') != '';
  }

  /**
   * Return the storage scheme uri.
   *
   * @return string
   *   The storage scheme uri.
   */
  public static function get_uri() {
    return 'mediamosa-storage://';
  }

  /**
   * Return the translated real path of the storage.
   *
   * @return string
   *   The storage real path.
   */
  public static function get_realpath() {
    return mediamosa_io::realpath('mediamosa-storage://');
  }

  /**
   * Return the simpletest storage scheme uri during simpletest.
   *
   * @return string
   *   The simpletest storage scheme uri.
   */
  public static function get_uri_simpletest() {
    return 'mediamosa-storage-simpletest://';
  }

  // ------------------------------------------------------- Storage mountpoint.
  /**
   * Get the mount point uri based on the storage profile and path.
   *
   * Both these values are found in the mediamosa_storage_profile_app table.
   *
   * @param array $storage_profile_id
   *   The storage profile ID.
   * @param string $path
   *   The (prefix) path on the uri.
   * @param boolean $is_still
   *   TRUE when mediafile is still.
   *
   * @return string
   *   The local moint point uri.
   */
  public static function get_mount_point_uri($storage_profile_id, $path, $is_still = FALSE) {
    // Need parent storage profile.
    $storage_profile = mediamosa_storage_profile::get($storage_profile_id);

    // Get uri.
    return self::get_mount_point_uri_by_storage_profile($storage_profile, $path, $is_still);
  }

  /**
   * Get the mount point uri based on the storage profile and path.
   *
   * Both these values are found in the mediamosa_storage_profile_app table.
   *
   * @param array $storage_profile_app
   *   The storage profile app (link between app and storage profile).
   * @param boolean $is_still
   *   TRUE when mediafile is still.
   *
   * @return string
   *   The local moint point uri.
   */
  public static function get_mount_point_uri_by_storage_profile_app(array $storage_profile_app, $is_still = FALSE) {
    // Get right path.
    $path = $is_still ? $storage_profile_app[mediamosa_storage_profile_app_db::PATH_STILLS] : $storage_profile_app[mediamosa_storage_profile_app_db::PATH];

    return self::get_mount_point_uri($storage_profile_app[mediamosa_storage_profile_app_db::PROFILE_ID], $path, $is_still);
  }

  /**
   * Get the mount point uri based on the storage profile app array.
   *
   * @param array $storage_profile
   *   The storage profile.
   * @param string $path
   *   The (prefix) path on the uri.
   * @param boolean $is_still
   *   TRUE when mediafile is still.
   *
   * @return string
   *   The local moint point uri.
   */
  public static function get_mount_point_uri_by_storage_profile(array $storage_profile, $path, $is_still = FALSE) {
    // Create the storage instance.
    $mediamosa_storage = mediamosa_storage_profile::create_storage_instance_by_profile($storage_profile);

    // Build the local mount point.
    return $mediamosa_storage->build_mount_point_uri($storage_profile[mediamosa_storage_profile_db::ID], $path, $is_still);
  }

  // -------------------------------------------------------------- Storage URI.
  /**
   * Create the uri of the file where the mediafile will be stored.
   *
   * The storage uri can be local or external (e.g. S3). This function can not
   * be used to get the current URI location of the mediafile (use
   * mediamosa_storage::get_mediafile_uri() instead).
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param boolean $is_still
   *   TRUE when mediafile is still.
   *
   * @return string
   *   The storage destination uri.
   */
  public static function create_storage_uri($app_id, $mediafile_id, $is_still = FALSE) {
    // Get the storage profile app based on the application ID.
    $storage_profile_app = mediamosa_storage_profile_app::get_app_storage_profile($app_id);

    // Build up the mount point uri.
    $mount_point_uri = self::get_mount_point_uri_by_storage_profile_app($storage_profile_app, $is_still);

    // Return the complete uri.
    return self::trim_uri($mount_point_uri, '') . mediamosa_io_streamwrapper::create_uri_mediafile_id_path($mediafile_id);
  }

  // -------------------------------------------------------------------- Local.
  /**
   * Create the local mount point uri.
   *
   * This function provides the mountpoint of the first uri of the new created
   * mediafiles on local file system. Is stored in the sannas_mount_point field.
   *
   * @todo: need to return local moint point (always).
   *
   * @param integer $app_id
   *   The application ID.
   * @param boolean $is_still
   *   TRUE when mediafile is still.
   *
   * @return string
   *   The local moint point uri.
   */
  public static function create_local_mount_point_uri($app_id, $is_still = FALSE) {
    // Get the storage profile app based on the application ID.
    $storage_profile_app = mediamosa_storage_profile_app::get_app_storage_profile($app_id);

    // Build up the mount point uri.
    return self::get_mount_point_uri_by_storage_profile_app($storage_profile_app, $is_still);
  }

  /**
   * Create the complete file path for new mediafiles.
   *
   * The path and filename returned are intended for creation of the new file.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $mediafile_id
   *   The mediafile ID.
   * @param boolean $is_still
   *   TRUE when mediafile is still.
   *
   * @return string
   *   The local mediafile uri.
   */
  public static function create_local_mediafile_uri($app_id, $mediafile_id, $is_still = FALSE) {
    return self::trim_uri(self::create_local_mount_point_uri($app_id, $is_still), '') . mediamosa_io_streamwrapper::create_uri_mediafile_id_path($mediafile_id);
  }

  /**
   * Return the local path location of the mediafile.
   *
   * Need to return the path without $mediafile_id[0] . '/' . $mediafile_id.
   * Also used by simpletests.
   *
   * @param array|string $mediafile
   *   Either the mediafile ID or the mediafile array.
   *
   * @return string
   *   The uri without $mediafile_id[0] . '/' . $mediafile_id
   */
  public static function get_uri_mediafile_path($mediafile) {
    // Get the mediafile uri.
    $source_uri = self::get_uri_mediafile($mediafile);

    // Build up the stream wrapper.
    $source_streamwrapper = mediamosa_io::require_stream_wrapper_instance_by_uri($source_uri);

    return $source_streamwrapper->get_uri_mount_point();
  }

  /**
   * Return the local path location of the mediafile.
   *
   * Used by LUA scripts:
   * Need to return the path without $mediafile_id[0] . '/' . $mediafile_id.
   * Also used by simpletests.
   *
   * @param array|string $mediafile
   *   Either the mediafile ID or the mediafile array.
   *
   * @return string
   *   The path without $mediafile_id[0] . '/' . $mediafile_id.
   */
  public static function get_local_mediafile_path($mediafile) {
    // Get the mediafile uri.
    $source_uri = self::get_uri_mediafile($mediafile);

    // Build up the stream wrapper.
    $source_streamwrapper = mediamosa_io::require_stream_wrapper_instance_by_uri($source_uri);

    return $source_streamwrapper->get_realpath_mount_point();
  }

  // -------------------------------------------------- /data related functions.
  /**
   * Return the translated real path of the storage.
   *
   * @return string
   *   The storage real path.
   */
  public static function get_realpath_data() {
    return mediamosa_io::realpath('mediamosa-storage-data://');
  }

  /**
   * Return the mediafile uri.
   *
   * @param array|string $mediafile
   *   Either the mediafile ID or the mediafile array.
   *
   * @return string
   *   The location of the data location.
   *
   * @see get_realpath_mediafile()
   */
  public static function get_uri_mediafile($mediafile) {
    // MediaFile ID?
    if (is_string($mediafile)) {
      // @todo: not cached.
      $mediafile = mediamosa_asset_mediafile::must_exists($mediafile);
    }

    // If no mount point, then assign one.
    if (empty($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT])) {
      $mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT] = self::create_local_mount_point_uri($mediafile[mediamosa_asset_mediafile_db::APP_ID], $mediafile[mediamosa_asset_mediafile_db::IS_STILL] == mediamosa_asset_mediafile_db::IS_STILL_TRUE);
    }

    return self::trim_uri($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]) . mediamosa_io_streamwrapper::create_uri_mediafile_id_path($mediafile[mediamosa_asset_mediafile_db::ID]);
  }

  /**
   * Return the mediafile location.
   *
   * Will throw exception when file is not local or does not exist.
   *
   * @param array|string $mediafile
   *   Either the mediafile ID or the mediafile array.
   *
   * @return string
   *   The location of the the data location.
   */
  public static function get_realpath_mediafile($mediafile) {
    return mediamosa_io::realpath(self::get_uri_mediafile($mediafile));
  }

  /**
   * Check if physical file is found.
   *
   * @param string $mediafile_id
   *   The mediafile to check.
   */
  public static function file_must_exist($mediafile_id) {
    // Check if still file exists.
    $file = self::get_realpath_mediafile($mediafile_id);
    if (!mediamosa_io::file_exists($file)) {
      throw new mediamosa_exception_error_file_not_found(array('@filename' => $file));
    }
  }

  // -------------------------------------------- /transition related functions.
  /**
   * Return the transition folder.
   *
   * @return string
   *   The location of the transition folder; data/transition.
   *
   * @see get_realpath_transition()
   */
  protected static function get_uri_transition() {
    return 'mediamosa-storage-transition://';
  }

  /**
   * Return the translated tmp transcode folder.
   *
   * @return string
   *   The location of the transition folder; data/transition.
   */
  public static function get_realpath_transition() {
    return mediamosa_io::realpath(self::get_uri_transition());
  }

  // --------------------------------------------- /temporary related functions.
  /**
   * Return the tmp transcode folder.
   *
   * @return string
   *   The location of the temporary transcode folder; data/transcode.
   *
   * @see get_realpath_data_transcode()
   */
  public static function get_uri_data_transcode() {
    return 'mediamosa-storage-temporary://';
  }

  /**
   * Return the translated tmp transcode folder.
   *
   * @return string
   *   The location of the temporary transcode folder; data/transcode.
   */
  public static function get_realpath_data_transcode() {
    return mediamosa_io::realpath(self::get_uri_data_transcode());
  }

  /**
   * Returns the full path and filename of the transcode file of
   * given filename.
   *
   * @param string $filename
   *   The filename to use.
   *
   * @return string
   *   The location of the trancode folder with filename.
   */
  public static function get_uri_data_transcode_file($filename) {
    return self::trim_uri(self::get_uri_data_transcode()) . $filename;
  }

  /**
   * Returns the translated full path and filename of the transcode file of
   * given filename.
   *
   * @param string $filename
   *   The filename to use.
   *
   * @return string
   *   The location of the trancode folder with filename.
   */
  public static function get_realpath_data_transcode_file($filename) {
    return mediamosa_io::realpath(self::get_uri_data_transcode_file($filename));
  }

  /**
   * Returns the full path and filename of the status file of given filename.
   *
   * @param string $filename
   *   The status file.
   *
   * @return
   *   The uri to the status file.
   */
  public static function get_uri_status_file($filename) {
    return self::get_uri_data_transcode() . rawurlencode($filename) . '.status';
  }

  /**
   * Returns the full path and filename of the status file of
   * given filename.
   *
   * @param $filename
   *   The status file.
   *
   * @return
   *   The uri to the status file.
   */
  public static function get_realpath_status_file($filename) {
    return mediamosa_io::realpath(self::get_uri_status_file($filename));
  }

  /**
   * Returns the full path and filename of the scene file of given job.
   *
   * @param $job_id
   *   The ID of the job.
   *
   * @return
   *   The URI to the scene file.
   */
  public static function get_uri_scene_file($job_id) {
    return 'mediamosa-storage://data/transcode/' . $job_id . '_scene.txt';
  }

  // ------------------------------------------------- /media related functions.
  /**
   * Return the uri of the media directory.
   *
   * @return string
   *   The uri.
   */
  protected static function get_uri_media() {
    return 'mediamosa-storage-media://';
  }

  /**
   * Return the path of the media directory.
   *
   * @return string
   *   The path.
   */
  public static function get_realpath_media() {
    return mediamosa_io::realpath(self::get_uri_media());
  }

  /**
   * Return the uri of the media directory.
   *
   * @return string
   *   The uri.
   */
  public static function get_uri_media_ticket() {
    return 'mediamosa-storage-media://ticket';
  }

  /**
   * Return the path of the media directory.
   *
   * @return string
   *   The path.
   */
  public static function get_realpath_media_ticket() {
    return mediamosa_io::realpath(self::get_uri_media_ticket());
  }

  /**
   * Return the path, usable for URLs. Not for URI usage.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $mediafile_id
   *   (optional) The mediafile ID.
   * @param string $filename
   *   (optional) The filename of the file.
   *
   * @return string
   *   The path.
   */
  public static function get_path_media_permanent_file($app_id, $mediafile_id = '', $filename = '') {
    return $app_id . '/' . (!empty($mediafile_id) ? $mediafile_id[0] . '/' . $mediafile_id . '/' . $filename : '');
  }

  /**
   * Return the realpath of a permanent symlink in the /media directory.
   *
   * The path returned here is an symbolic link to the actual mediafile. The
   * link returned is permanent and is, unlike ticket links, not cleaned up
   * after an amount of time. The permanent link can only exist when the
   * mediafile is not protected by ACL.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $mediafile_id
   *   (optional) The mediafile ID.
   * @param string $filename
   *   (optional) The filename of the file.
   *
   * @return string
   *   The uri.
   */
  public static function get_realpath_media_permanent_file($app_id, $mediafile_id = '', $filename = '') {
    // Return the realpath.
    return self::get_realpath_media() . '/' . self::get_path_media_permanent_file($app_id, $mediafile_id, $filename);
  }

  /**
   * Return the path usable for URLs. Not for URIs.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $ticket_id
   *   The mediafile ID.
   * @param string $filename
   *   The filename of the file.
   * @param string $ticket_type.
   *   (optional) The type of the ticket is used to identify the type of the
   *   link. Is optional in case when $filename already contains the type.
   *
   * @return string
   *   The path.
   */
  public static function get_path_media_ticket_file($app_id, $ticket_id, $filename, $ticket_type = '') {
    // Return the path.
    return 'ticket/' . (empty($ticket_type) ? '' : (strtolower($ticket_type) . '/')) . $app_id . '/' . $ticket_id[0] . '/' . $ticket_id . '/' . $filename;
  }

  /**
   * Return the uri of a temporary symlink in the /media directory.
   *
   * The path returned here is an symbolic link to the actual mediafile. The
   * link returned is temporary and is cleaned up after an amount of time.
   * Ticket based symlinks are used when the file is protected by ACL and access
   * is granted to the user. The ticket ID is random string that only the
   * authenticated party knows.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $ticket_id
   *   The mediafile ID.
   * @param string $filename
   *   The filename of the file.
   * @param string $ticket_type.
   *   (optional) The type of the ticket is used to identify the type of the
   *   link. Is optional in case when $filename already contains the type.
   *
   * @return string
   *   The uri.
   */
  public static function get_uri_media_ticket_file($app_id, $ticket_id, $filename, $ticket_type = '') {
    // Return the uri.
    return self::get_uri_media() . self::get_path_media_ticket_file($app_id, $ticket_id, $filename, $ticket_type);
  }

  /**
   * Return the realpath of a temporary symlink in the /media directory.
   *
   * The path returned here is an symbolic link to the actual mediafile. The
   * link returned is temporary and is cleaned up after an amount of time.
   * Ticket based symlinks are used when the file is protected by ACL and access
   * is granted to the user. The ticket ID is random string that only the
   * authenticated party knows.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $ticket_id
   *   The mediafile ID.
   * @param string $filename
   *   The filename of the file.
   * @param string $ticket_type.
   *   (optional) The type of the ticket is used to identify the type of the
   *   link. Is optional in case when $filename already contains the type.
   *
   * @return string
   *   The uri.
   */
  public static function get_realpath_media_ticket_file($app_id, $ticket_id, $filename, $ticket_type = '') {
    // Return the uri.
    return mediamosa_io::realpath(self::get_uri_media_ticket_file($app_id, $ticket_id, $filename, $ticket_type));
  }

  // ---------------------------------------------------- /media/object related.
  /**
   * Return uri for object files.
   *
   * This location is used for storage and downloading of extra object files,
   * f.e. asx text/html files.
   *
   * @param integer $app_id
   *   (optional) The application ID.
   * @param string $filename
   *   The filename (optional but required when app_id is provided). This can be
   *   f.e. the ticket ID or mediafile ID with file extension.
   *
   * @return
   *   The path to ticket location
   */
  protected static function get_uri_object_file($app_id, $ticket_id, $filename) {
    return 'mediamosa-storage-media://object/' . $app_id . '/' . mediamosa_io_streamwrapper::create_uri_mediafile_id_path($ticket_id) . '/' . $filename;
  }

  /**
   * Return file location for object files.
   *
   * This location is used for storage and downloading of extra object files,
   * f.e. asx text/html files.
   *
   * @param integer $app_id
   *   (optional) The application ID.
   * @param string $filename
   *   The filename (optional but required when app_id is provided). This can be
   *   f.e. the ticket ID or mediafile ID with file extension.
   *
   * @return
   *   The path to ticket location
   */
  public static function get_realpath_object_file($app_id, $ticket_id, $filename) {
    return mediamosa_io::realpath(self::get_uri_object_file($app_id, $ticket_id, $filename));
  }

  /**
   * Return the path to object file (e.g. asx) usable for URLs, not for URIs.
   *
   * @param integer $app_id
   *   The application ID.
   * @param string $ticket_id
   *   The mediafile ID.
   * @param string $filename
   *   The filename of the file.
   *
   * @return string
   *   The path.
   */
  public static function get_path_media_object_file($app_id, $ticket_id, $filename) {
    // Return the path.
    return 'object/' . $app_id . '/' . $ticket_id[0] . '/' . $ticket_id . '/' . $filename;
  }

  // --------------------------------------------------- /ftp related functions.
  /**
   * Get the ftp uri.
   *
   * @return string
   *   The FTP location in uri.
   */
  public static function get_realpath_ftp() {
    return mediamosa_io::realpath('mediamosa-storage://ftp/');
  }

  /**
   * Get the SAN/NAS history path.
   *
   * @return string
   *   The FTP history location in uri.
   */
  public static function get_realpath_ftp_history() {
    return mediamosa_io::realpath('mediamosa-storage://ftp/history/');
  }

  /**
   * Get the history file location uri.
   *
   * @return string
   *   The history location uri.
   */
  public static function get_realpath_ftp_history_job() {
    return mediamosa_io::realpath('mediamosa-storage://ftp/history/current.txt');
  }

  /**
   * Returns the contents for the .htaccess file under [mountpoint]/media.
   *
   * @return string
   *   The contents for the .htaccess file for [mount_pount]/media/.htaccess.
   */
  public static function file_get_media_htaccess_contents() {
    $subdir = trim(url(), '/');
    $ht_access = file_get_contents(drupal_get_path('module', 'mediamosa') . '/lib/misc/media_htaccess_contents.txt');
    return strtr($ht_access, array('{SUBDIR}' => $subdir == '' ? '' : '/' . $subdir));
  }

  /**
   * Creates an .htaccess file in the given directory using the given contents.
   *
   * @param string $directory
   *   The destination directory.
   * @param string $contents
   *   The contents to write.
   *
   * @return boolean
   *   Returns either TRUE (success) or FALSE (failure).
   */
  public static function file_create_htaccess($directory, $contents) {
    if (file_uri_scheme($directory)) {
      $directory = file_stream_wrapper_uri_normalize($directory);
    }
    else {
      $directory = rtrim($directory, '/\\');
    }
    $htaccess_path =  $directory . '/.htaccess';

    // If file already exists, then bail out.
    if (mediamosa_io::file_exists($htaccess_path)) {
      return TRUE;
    }

    // Write the .htaccess file.
    if (file_put_contents($htaccess_path, $contents)) {
      drupal_chmod($htaccess_path, 0444);
      return TRUE;
    }

    // Make log entry about error.
    $variables = array('%directory' => $directory, '@path' => $htaccess_path, '!htaccess' => '<br />' . nl2br(check_plain($contents)));
    mediamosa_watchdog::log("Warning: Couldn't write .htaccess file in location @path. Please create a .htaccess file in your %directory directory which contains the following lines: <code>!htaccess</code>", $variables, WATCHDOG_ERROR);
    return FALSE;
  }

  /**
   * Trims the uri correctly without messing up schema.
   *
   * @param string $uri
   *   The uri to trim.
   * @param string $end
   *   By default will add '/' at the end.
   *
   * @return string
   *   The trimed uri.
   */
  public static function trim_uri($uri, $add = '/') {
    return drupal_substr($uri, -3) != '://' ? rtrim($uri, '/') . $add : $uri;
  }

  /**
   * Update the mountpoint to the new location.
   *
   * @param array $mediafile
   *
   * @param string $sannas_mount_point
   */
  public static function mediafile_update_mointpoint($mediafile, $sannas_mount_point) {
    $fields = array(
      mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT => $sannas_mount_point,
    );

    $app_id = $mediafile[mediamosa_asset_mediafile_db::APP_ID];
    $mediafile_id = $mediafile[mediamosa_asset_mediafile_db::ID];
    $user_id = $mediafile[mediamosa_asset_mediafile_db::OWNER_ID];

    // Update the mediafile.
    mediamosa_asset_mediafile::update($app_id, $mediafile_id, $user_id, $fields);
  }

  /**
   * Copy mediafile to any location.
   *
   * @param string $source_uri
   *   The uri of the source file.
   * @param string $destination_uri
   *   The uri of the target file. Must include the filename.
   *
   * @throws mediamosa_exception_error
   */
  public static function mediafile_copy($source_uri, $destination_uri) {
    // This could happen.
    if ($source_uri == $destination_uri) {
      mediamosa_watchdog::log('Not coping mediafile; source and target are the same.');
      return;
    }
// @todo: remove from release.
    mediamosa_debug::log_export('debug in ' . __FUNCTION__ . '() on line ' . __LINE__ . ' in ' . __FILE__);
    mediamosa_debug::log_export($source_uri);
// @todo: remove from release.
// @todo: remove from release.
    mediamosa_debug::log_export('debug in ' . __FUNCTION__ . '() on line ' . __LINE__ . ' in ' . __FILE__);
    mediamosa_debug::log_export($destination_uri);
// @todo: remove from release.

    // Find the stream wrapper.
    $mediamosa_io_streamwrapper_source = mediamosa_io::require_stream_wrapper_instance_by_uri($source_uri);

    // Find the stream wrapper destination.
    $mediamosa_io_streamwrapper_destination = mediamosa_io::require_stream_wrapper_instance_by_uri($destination_uri);

    // Now copy.
    $mediamosa_io_streamwrapper_destination->mediafile_copy($mediamosa_io_streamwrapper_source);

    // Copied, return.
  }

  /**
   * Move mediafile from one storage location to another.
   *
   * Moving a mediafile from its current location to another comes in 4 flavors;
   * 1. Moving from local to local storage (move file).
   * 2. Moving from local to external storage (upload file).
   * 3. Moving from external to local storage (download file).
   * 4. Moving from external to external storage (move file).
   *
   * @param array $mediafile
   *   The mediafile to move.
   * @param integer $storage_profile_id
   *   The destination location.
   * @param string $path
   *   The destination path.
   *
   * @throws mediamosa_exception_program_error
   * @throws mediamosa_exception_error
   */
  public static function mediafile_move(array $mediafile, $storage_profile_id, $path) {

    // Source uri.
    $source_uri = mediamosa_storage::get_uri_mediafile($mediafile);

    // Get the storage profile.
    $storage_profile = mediamosa_storage_profile::must_exists($storage_profile_id);

    // Is source mediafile a still?
    $is_still = mediamosa_asset_mediafile::is_still($mediafile);

    // Build the destination uri.
    $destination_sannas_mount_point = mediamosa_storage::get_mount_point_uri_by_storage_profile($storage_profile, $path, $is_still);
    $destination_uri = $destination_sannas_mount_point . mediamosa_io_streamwrapper::create_uri_mediafile_id_path($mediafile[mediamosa_asset_mediafile_db::ID]);

    // Get the destination stream wrapper.
    $mediamosa_io_streamwrapper_destination = mediamosa_io::require_stream_wrapper_instance_by_uri($destination_uri);

    // Copy file to destination.
    self::mediafile_copy($source_uri, $destination_uri);

    // Done, update the new location.
    self::mediafile_update_mointpoint($mediafile, $mediamosa_io_streamwrapper_destination->get_uri_mount_point());

    // Store md5 metadata.
    mediamosa_asset_mediafile_metadata::create_mediafile_metadata_char($mediafile[mediamosa_asset_mediafile_db::ID], $mediamosa_io_streamwrapper_destination->getMD5(), mediamosa_asset_mediafile_metadata::MD5);

    // Remove old file. If the file is in use somewhere else, it will fail but
    // is not fatal.
    mediamosa_io::unlink($source_uri);
  }
}
