<?php
/**
 * @file
 * main storage class.
 */

abstract class mediamosa_storage {

  // ------------------------------------------------------------------- Consts.
  const STORAGE_FOLDER_TICKET = 'ticket';
  
  // Ticket locations.
  const PLAY_TICKET_LOCATION = 'links'; // -> media
  const DOWNLOAD_TICKET_LOCATION = 'download_links'; // -> media
  
  // Expiration settings, in seconds.
  // @todo: currently all ticket types have same expiration because of design.
  
  // Symbolic link based.
  const TICKET_EXPIRATION_PLAY = 600; // 10 minutes
  const TICKET_EXPIRATION_DOWNLOAD = 600; // 10 minutes
  const TICKET_EXPIRATION_STILL = 600; // 10 minutes
  
  // Not symbolic link.
  const TICKET_EXPIRATION_UPLOAD = 86400; // 24 hours
  
  // ---------------------------------------------------------------- Static functions.
  /**
   * Is the storage file system path specified?
   *
   * @return
   *  Returns TRUE when storage file system path has been specified.
   */
  public static function is_storage_setup() {
    return mediamosa_io::realpath('mediamosa-storage://') != '';
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
   * Return the storage scheme uri.
   *
   * @return
   *   The storage scheme uri.
   */
  public static function get_uri() {
    return 'mediamosa-storage://';
  }

  /**
   * Return the simpletest storage scheme uri.
   *
   * @return
   *   The simpletest storage scheme uri.
   */
  public static function get_uri_simpletest() {
    return 'mediamosa-storage-simpletest://';
  }

  /**
   * Return the data folder.
   *
   * @return
   *   The path for 'data'.
   */
  public static function get_path_data() {
    return 'data/';
  }

  /**
   * Get the data location path.
   */
  public static function get_uri_data() {
    return 'mediamosa-storage://data/';
  }

  /**
   * Get the data location path translated.
   */
  public static function get_realpath_data() {
    return mediamosa_io::realpath(self::get_uri_data());
  }

  /**
   * Return the still file name location.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile data location.
   *
   * @return
   *   The location of the
   *   mediamosa-storage://data/[letter]/[mediafile_id].
   */
  public static function get_uri_data_file($mediafile_id, $mount_point = '') {
    $mount_point = empty($mount_point) ? self::get_realpath() : $mount_point;
    return $mount_point . 'data/' . self::map_uuid_to_path($mediafile_id);
  }

  /**
   * Return the still file name location.
   *
   * @param string $mediafile_id
   *   The ID of the mediafile data location.
   *
   * @return
   *   The location of the
   *   mediamosa-storage://data/[letter]/[mediafile_id].
   */
  public static function get_realpath_data_file($mediafile_id, $mount_point = '') {
    return mediamosa_io::realpath(self::get_uri_data_file($mediafile_id, $mount_point));
  }

  /**
   * Return the tmp transcode folder.
   *
   * @note: only used in .test
   *
   * @return
   *   The location of the temporary transcode folder; data/transcode.
   */
  public static function get_uri_data_transcode() {
    return 'mediamosa-storage://data/transcode/';
  }

  /**
   * Return the translated tmp transcode folder.
   *
   * @return
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
    return 'mediamosa-storage://data/transcode/' . $filename;
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
   * Returns the full path and filename of the status file of
   * given filename.
   *
   * @param $filename
   *   The status file.
   *
   * @return
   *   The uri to the status file.
   */
  public static function get_uri_status_file($filename) {
    return 'mediamosa-storage://data/transcode/' . $filename . '.status';
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
    return mediamosa_io::realpath('mediamosa-storage://data/transcode/' . $filename . '.status');
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

  /**
   * Return the uri for the play ticket.
   *
   * @return
   *   The ticket play uri.
   */
  public static function get_uri_media_play($app_id = -1, $mediafile_id = '') {
    return self::get_uri_media($app_id, $mediafile_id);
  }

  /**
   * Return the uri for the play ticket.
   *
   * @return
   *   The ticket play uri.
   */
  public static function get_realpath_media_play($app_id = -1, $mediafile_id = '') {
    return mediamosa_io::realpath(self::get_uri_media($app_id, $mediafile_id));
  }

  /**
   * Return the path of the media directory.
   *
   * @param $app_id
   *   The application ID (optional).
   * @param $ticket_id
   *   The ticket ID (optional, required when app_id is provided).
   *
   * @return
   *   The path.
   */
  public static function get_uri_media($app_id = -1, $mediafile_id = '') {
    return 'mediamosa-storage://media/' . ($app_id > 0 ? $app_id . '/' . (empty($mediafile_id) ? '' : $mediafile_id[0] . '/') : '');
  }

  /**
   * Return the real path of the media directory.
   *
   * @param $app_id
   *   The application ID (optional).
   * @param $ticket_id
   *   The ticket ID (optional, required when app_id is provided).
   *
   * @return string
   *   The path.
   */
  public static function get_realpath_media($app_id = -1, $mediafile_id = '') {
    return mediamosa_io::realpath(self::get_uri_media($app_id, $mediafile_id));
  }

  /**
   * Return the uri for the download ticket location.
   *
   * @param integer $app_id
   *   (optional) The client application.
   * @param string $mediafile_id
   *   (optional) The mediafile ID.
   *
   * @return string
   *   The location of the download symlink path.
   */
  public static function get_uri_media_download_ticket($app_id = -1, $mediafile_id = '') {
    return self::get_uri_media($app_id, $mediafile_id);
  }

  /**
   * Return the realpath uri for the download ticket location.
   *
   * @param integer $app_id
   *   (optional) The client application.
   * @param string $mediafile_id
   *   (optional) The mediafile ID.
   *
   * @return string
   *   The location of the download symlink path.
   */
  public static function get_realpath_media_download_ticket($app_id = -1, $mediafile_id = '') {
    return self::get_realpath_media($app_id, $mediafile_id);
  }

  /**
   * Return the full path and filename of the media.
   *
   * @param $app_id
   *   The application ID.
   * @param $mediafile_id
   *   The mediafile ID.
   *
   * @return
   *   The path.
   */
  public static function get_uri_media_file($app_id, $mediafile_id) {
    return self::get_uri_media($app_id, $mediafile_id) . $mediafile_id;
  }

  /**
   * Return the path of the media directory.
   *
   * @param $app_id
   *   The application ID (optional).
   * @param $ticket_id
   *   The ticket ID (optional, required when app_id is provided).
   *
   * @return
   *   The path.
   */
  public static function get_uri_media_ticket($app_id = -1, $mediafile_id = '') {
    return 'mediamosa-storage://media/ticket/' . ($app_id > 0 ? $app_id . '/' . (empty($mediafile_id) ? '' : $mediafile_id[0] . '/') : '');
  }

  /**
   * Return the real path of the media directory.
   *
   * @param $app_id
   *   The application ID (optional).
   * @param $ticket_id
   *   The ticket ID (optional, required when app_id is provided).
   *
   * @return
   *   The path.
   */
  public static function get_realpath_media_ticket($app_id = -1, $mediafile_id = '') {
    return mediamosa_io::realpath(self::get_uri_media_ticket($app_id, $mediafile_id));
  }

  /**
   * Return the full path and filename of the media.
   *
   * @param $app_id
   *   The application ID.
   * @param $ticket_id
   *   The ticket ID.
   *
   * @return
   *   The uri to the ticket file.
   */
  public static function get_uri_media_ticket_file($app_id, $ticket_id) {
    return self::get_uri_media_ticket($app_id, $ticket_id) . $ticket_id;
  }

  /**
   * Return file location of asx file.
   *
   * The ASX file is stored in still directory, true, must be downloadable.
   *
   * @param $app_id
   *   The app ID
   * @param $ticket
   *   The ticket ID.
   *
   * @return
   *  path to ticket location
   */
  public static function get_uri_asx_file($app_id, $ticket) {
    return self::get_uri_media_file($app_id, $ticket) . '.asx';
  }

  /**
   * Create the permanent uri.
   *
   * @param $app_id
   *   The client application that is managing the object_id.
   * @param $object_id
   *   The object_id is either the ticket ID or the ID of the object.
   */
  public static function objectid_to_path($app_id, $object_id) {
    return $app_id . '/' . self::map_uuid_to_path($object_id);
  }

  /**
   * Create the (temporary) ticket based uri.
   *
   * @param $app_id
   *   The client application that is managing the object_id.
   * @param $object_id
   *   The object_id is either the ticket ID or the ID of the object.
   */
  public static function objectid_to_ticket_path($app_id, $object_id) {
    return self::STORAGE_FOLDER_TICKET . '/' . $app_id . '/' . self::map_uuid_to_path($object_id);
  }

  /**
   * Map the object ID to store file into location and filename.
   *
   * Format: $object_id[0]/$object_id
   *
   * @param $object_id
   *   The object_id is either the ticket ID or the ID of the object.
   */
  public static function map_uuid_to_path($object_id) {
    return $object_id[0] . '/' . $object_id;
  }

  /**
   * Get the Mediafile mountpath path.
   *
   * @param array $mediafile
   *   The mediafile array or simular, must contain at least 'mediafile_id'.
   *
   * @return
   *   The URI to the mediafile. Might return normal path in case of mediafile
   *   has own mountpoint.
   */
  public static function get_uri_mediafile(array $mediafile) {
    // Get the mount point either from mediafile or our default setting.
    $mount_point = empty($mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT]) ? 'mediamosa-storage://' : $mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT];

    return $mount_point . 'data/' . self::map_uuid_to_path($mediafile[mediamosa_asset_mediafile_db::ID]);
  }

  /**
   * Get the Mediafile translated mountpath path.
   *
   * @param array $mediafile
   *   The mediafile array or simular, must contain at least 'mediafile_id'.
   *
   * @return
   *   The translated path to the mediafile.
   */
  public static function get_realpath_mediafile(array $mediafile) {
    return mediamosa_io::realpath(self::get_uri_mediafile($mediafile));
  }

  /**
   * Get the URI Mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile_id.
   * @param boolean $precreate_dir
   *   Create directory when needed.
   *
   * @return
   *   The URI to the mediafile.
   */
  public static function get_uri_mediafile_id($mediafile_id, $precreate_dir = FALSE) {
    $file = 'mediamosa-storage://data/' . self::map_uuid_to_path($mediafile_id);

    // Pre-create the directory.
    if ($precreate_dir) {
      mediamosa_io::mkdir(mediamosa_io::dirname($file));
    }

    return $file;
  }

  /**
   * Get the URI Mediafile.
   *
   * @param string $mediafile_id
   *   The mediafile_id.
   *
   * @return
   *   The URI to the mediafile.
   */
  public static function get_realpath_mediafile_id($mediafile_id, $precreate_dir = FALSE) {
    return mediamosa_io::realpath(self::get_uri_mediafile_id($mediafile_id, $precreate_dir));
  }

  /**
   * Get the uri
   */
  public static function get_uri_ftp() {
    return 'mediamosa-storage://ftp/';
  }

  /**
   * Get the SAN/NAS history path.
   */
  public static function get_uri_ftp_history() {
    return 'mediamosa-storage://ftp/history/';
  }

  /**
   * Get the current job path.
   */
  public static function get_uri_ftp_history_job() {
    return 'mediamosa-storage://ftp/history/current.txt';
  }

  /**
   * Returns the contents for the .htaccess file under [mountpoint]/media.
   *
   * @return
   *   The contents for the .htaccess file for [mount_pount]/media/.htaccess.
   */
  public static function file_get_htaccess_contents() {
    $subdir = trim(url(), '/');
    $ht_access = file_get_contents(drupal_get_path('module', 'mediamosa') . '/lib/misc/media_htaccess_contents.txt');
    return strtr($ht_access, array('{SUBDIR}' => $subdir == '' ? '' : '/' . $subdir));
  }

  /**
   * Creates an .htaccess file in the given directory using the given contents.
   *
   * @param $directory
   *   The directory.
   * @param $contents
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
    if (file_exists($htaccess_path)) {
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
   * Adds an '/' at the end and makes sure that only one is added.
   *
   * @param string $uri
   *   The uri to trim.
   */
  public static function trim_uri($uri) {
    return drupal_substr($uri, -3) != '://' ? rtrim($uri, '/') . '/' : $uri;
  }
  
  /**
   * Create storage instance based on a storage profile
   * 
   * @param $profile
   *   Profile object, array or id
   * 
   * @return mediamosa_storage
   *   MediaMosa storage object
   *   
   * @throws mediamosa_exception_error
   *   If the storage class does not exist (mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND)
   *   or is invalid (mediamosa_error::ERRORCODE_STORAGE_CLASS_INVALID)
   */
  public static function create_instance_for_profile($profile) {
    if (is_object($profile)) {
      $profile = get_object_vars($profile);
    }
    if (is_numeric($profile)) {
      $profile = mediamosa_storage_profile::get($profile);
    }
    $class = 'mediamosa_storage_' . $profile[mediamosa_storage_profile_db::TYPE];

    if (!class_exists($class)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND, array('@class' => $class));
    }
    
    if (!in_array('mediamosa_storage', class_parents($class))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CLASS_INVALID, array('@class' => $class));
    }
    
    return new $class($profile[mediamosa_storage_profile_db::NID]);
  }
  
  /**
   * Create storage instance based on a storage mapping
   * 
   * @param $mapping
   *   Mapping array or id
   * 
   * @return mediamosa_storage
   *   MediaMosa storage object
   *   
   * @throws mediamosa_exception_error
   *   If the storage class does not exist (mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND)
   *   or is invalid (mediamosa_error::ERRORCODE_STORAGE_CLASS_INVALID)
   */
  public static function create_instance_for_mapping($mapping) {
    if (!is_numeric($mapping)) {
      $mapping = $mapping[mediamosa_storage_mapping_db::ID];
    }
    
    return self::create_instance_for_uri(mediamosa_storage_mapping::get_uri($mapping));
  }
  
  /**
   * Create storage instance based on a storage uri
   * 
   * @param $uri
   *   Storage uri
   *   
   * @return mediamosa_storage
   *   MediaMosa storage object
   *   
   * @throws mediamosa_exception_error
   *   If the storage class does not exist (mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND)
   *   or is invalid (mediamosa_error::ERRORCODE_STORAGE_CLASS_INVALID)
   */
  public static function create_instance_for_uri($uri) {
    $uri = parse_url($uri);
    $class = 'mediamosa_storage_' . $uri['scheme'];
    $path = $uri['path'];
    
    $profile = null;
    if (is_numeric($uri['host'])) {
      $profile = mediamosa_storage_profile::get($uri['host']);
      $profile = $profile[mediamosa_storage_profile_db::NID];
    }
    
    if (!class_exists($class)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND, array('@class' => $class));
    }
    
    if (!in_array('mediamosa_storage', class_parents($class))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CLASS_INVALID, array('@class' => $class));
    }
    
    $instance = new $class($profile); 
    $instance->set_uri($uri);
    
    return $instance;
  }
  
  /**
   * Create storage instance based on a mediafile
   * 
   * @param $mediafile
   *   Mediafile id
   * 
   * @return mediamosa_storage
   *   MediaMosa storage object
   *   
   * @throws mediamosa_exception_error
   *   If the storage class does not exist (mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND)
   *   or is invalid (mediamosa_error::ERRORCODE_STORAGE_CLASS_INVALID)
   */
  public static function create_instance_for_mediafile($mediafile_id) {
    $mediafile = mediamosa_asset_mediafile::get($mediafile_id);

    if (!mediafile) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $mediafile_id));
    }
    
    $uri = $mediafile[mediamosa_asset_mediafile_db::SANNAS_MOUNT_POINT];
    
    // Fallback to standard mediamosa storage if no SANNAS mountpoint is provided
    if (!$uri) {
      $uri = mediamosa_storage_standard::get_uri($mediafile[mediamosa_asset_mediafile_db::FILENAME]);
    }
    
    $uri = parse_url($uri);
    $class = 'mediamosa_storage_' . $uri['scheme'];
    $path = $uri['path'];
    
    if (!$profile && is_numeric($uri['host'])) {
      $profile = $uri['host'];
    }
    
    if (is_object($profile)) {
      $profile = get_object_vars($profile);
    }
    
    if (!class_exists($class)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND, array('@class' => $class));
    }
    
    if (!in_array('mediamosa_storage', class_parents($class))) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CLASS_INVALID, array('@class' => $class));
    }
    
    $instance = new $class($profile); 
    $instance->set_mediafile($uri);
    
    return $instance;
  }
  
  // ---------------------------------------------------------------- Class vars.
  protected $profile;
  protected $mediafile;
  protected $uri;
  
  // ---------------------------------------------------------------- Class methods.
  /**
   * Main constructor
   * 
   * @param $profile
   *   Storage profile node or nid
   * 
   * @param $uri
   *   Optional uri to the storage object
   */
  private function __construct($profile) {
    if (is_numeric($profile)) {
      $profile_id = $profile;
      $profile = node_load($profile_id);
      if (!$profile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_PROFILE_NOT_FOUND, array('@profile_id' => $profile_id));
      }
    }
    $this->profile = $profile;
  }
  
  /**
   * Set mediafile
   * 
   * @param $mediafile
   *   Mediafile id or array
   */
  public function set_mediafile($mediafile) {
    if (is_numeric($mediafile)) {
      $mediafile_id = $mediafile; 
      $mediafile = mediamosa_asset_mediafile::get($mediafile_id);
      if (!mediafile) {
        throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $mediafile_id));
      }
    }
    
    $this->mediafile = $mediafile;
    //TODO: set $this->uri based on $this->mediafile
  }
  
  /**
   * Set uri
   * 
   * @param $uri
   */
  public function set_uri($uri) {
    $this->uri = $uri;
    //TODO: set $this-> mediafile based on $this->uri if possible
  }
  
  /**
   * Return the storage options available for the storage type
   * 
   * @param $profile
   *   Storage profile object
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
   * 
   * @param $group_id
   *   Group ID
   * 
   * @return
   *   Array with upload parameters
   */
  public abstract function get_upload_parameters($app_id, $user_id, $group_id);
  
}

/**
 * Our MediaMosa storage stream wrapper class.
 *
 * Under storage;
 * /data contains all physical files.
 * /media contains symbolic links to the /data files.
 *
 * mediamosa-storage://
 *
 * Provides support for all files and symlinks used by MediaMosa using the
 * Drupal file interface.
 *
 * @see mediamosa_storage::get_uri()
 */
class mediamosa_storage_stream_wrapper extends DrupalLocalStreamWrapper {

  // ------------------------------------------------- Stream wrapper functions.
  /**
   * Implements abstract public function getDirectoryPath()
   */
  public function getDirectoryPath() {
    return self::get_storage_path();
  }

  /**
   * Overrides getExternalUrl().
   *
   * Return the HTML URI of a public file.
   */
  public function getExternalUrl() {
    // Unsupported for now.
    return '';

    //$path = str_replace('\\', '/', $this->getTarget());
    //return self::getDirectoryPath() . '/' . drupal_encode_path($path);
  }

  /**
   * Support for stat().
   *
   * Override:
   * Fixing problem with Simpletests where notice of @stat is not suppressed.
   *
   * @param $uri
   *   A string containing the URI to get information about.
   * @param $flags
   *   A bit mask of STREAM_URL_STAT_LINK and STREAM_URL_STAT_QUIET.
   *
   * @return
   *   An array with file status, or FALSE in case of an error - see fstat()
   *   for a description of this array.
   *
   * @see http://php.net/manual/en/streamwrapper.url-stat.php
   */
  public function url_stat($uri, $flags) {
    $this->uri = $uri;
    $path = $this->getLocalPath();

    // Prevent the @stat(), which will give notice in Simpletest, even when
    // its suppressed by @.
    if (!file_exists($path)) {
      return FALSE;
    }

    // Suppress warnings if requested or if the file or directory does not
    // exist. This is consistent with PHP's plain filesystem stream wrapper.
    if ($flags & STREAM_URL_STAT_QUIET) {
      return @stat($path);
    }

    return stat($path);
  }

  // ---------------------------------------------- MediaMosa related functions.
  /**
   * Return the moint point path based on OS. Will return path to sandbox when
   * in simpletest.
   *
   * @return
   *   The path to the storage path.
   */
  public static function get_storage_path() {
    if (mediamosa::in_simpletest_sandbox()) {
      return mediamosa_storage_simpletest_stream_wrapper::get_storage_path();
    }

    return mediamosa_storage::trim_uri(mediamosa::is_windows_platform() ? self::get_storage_path_windows() : self::get_storage_path_linux(), '/') . '/';
  }

  /**
   * Get the root of the storage setting for linux.
   *
   * @return
   *   The path of the storage root.
   */
  public static function get_storage_path_linux() {
    return mediamosa::variable_get('mediamosa_current_mount_point', '/srv/mediamosa');
  }

  /**
   * Get the root of the storage setting for windows.
   *
   * @return
   *   The path of the storage root.
   */
  public static function get_storage_path_windows() {
    return mediamosa::variable_get('mediamosa_current_mount_point_windows', '');
  }
}

/**
 * Our MediaMosa storage simpletest stream wrapper class. Use this wrapper in
 * case you need to use the simpletest path during normal operations.
 *
 * mediamosa-storage-simpletest://
 *
 * @see mediamosa_storage::get_uri_simpletest()
 */
class mediamosa_storage_simpletest_stream_wrapper extends mediamosa_storage_stream_wrapper {

  /**
   * Return the moint point path based on OS. Will return path to sandbox when
   * in simpletest.
   *
   * @param $in_simpletest_sandbox
   *   Specify TRUE if you want the path used during simpletest.
   *
   * @return
   *   The path to the storage path.
   */
  public static function get_storage_path() {
    return mediamosa_storage::trim_uri(mediamosa::is_windows_platform() ? self::get_storage_path_windows() : self::get_storage_path_linux()) . '/media/simpletest/';
  }
}