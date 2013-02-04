<?php
/**
 * @file
 * Helper class for MediaMosa profile.
 */

if (file_exists(DRUPAL_ROOT . '/sites/all/modules/mediamosa/mediamosa.version.class.inc')) {
  include_once DRUPAL_ROOT . '/sites/all/modules/mediamosa/mediamosa.version.class.inc';
}

/**
 * Contains helper functions.
 */
class mediamosa_profile {

  // ---------------------------------------------------------------- Functions.
  /**
   * Retrieve the version.
   *
   * @return string
   *   Return the version of MediaMosa found in this installation.
   */
  public static function get_version_mediamosa() {
    // Try to include our version file.
    if (class_exists('mediamosa_version')) {
      return mediamosa_version::get_current_version_str(TRUE);
    }

    // Unable to include should not be fatal.
  }

  /**
   * Build title during install.
   *
   * @return string
   *   The title to use during installation.
   */
  public static function get_title() {
    return st('Installing MediaMosa @version', array('@version' => self::get_version_mediamosa()));
  }

  /**
   * Retrieve the server domain and path.
   *
   * @return string
   *   Return the server domain and path.
   */
  public static function get_server_name() {
    // Call url() so we can take the host and path.
    $url = url('', array('absolute' => TRUE));

    // Remove the scheme.
    $pos = strpos($url, '://');

    // Return the rest of the host + path.
    return trim(drupal_substr($url, $pos + 3), '/');
  }

  /**
   * Retrieve the server domain and path.
   *
   * @return string
   *   Return the server domain and path.
   */
  public static function get_host() {
    // Call url() so we can take the host and path.
    $url = url('', array('absolute' => TRUE));

    // Parse it.
    $parse_url = parse_url($url);

    // Return the host.
    return $parse_url['host'];
  }

  /**
   * Trims the uri correctly without messing up schema.
   *
   * Adds an '/' at the end and makes sure that only one is added.
   *
   * @param string $uri
   *   The uri to trim.
   * @param string $add
   *   The char to add.
   *
   * @return string
   *   The trimed uri.
   */
  public static function trim_uri($uri, $add = '/') {
    return drupal_substr($uri, -3) != '://' ? rtrim($uri, '/') . $add : $uri;
  }

  /**
   * Test if given command is installed / found.
   *
   * Also returns its default stdio / strerr output in $exec_output
   *
   * @param string $command
   *   Command to test.
   * @param array $exec_output
   *   Gets the output of stdio/stderr
   * @param array $allowed_ret_values
   *   List of return values that indicate command is installed. Most programs
   *   will use exit(0) as indication that it was run. Some use exit(1) or
   *   higher as exit value.
   *
   * @return boolean
   *   TRUE when commond is installed, FALSE otherwise.
   */
  static public function command_installed($command, array &$exec_output, array $allowed_ret_values = array(0)) {
    $exec_output = array();
    $ret_val = 0;
    exec($command . ' 2>/dev/null', $exec_output, $ret_val);

    // If ret_val is ok, then check if $exec_output is empty.
    if (in_array($ret_val, $allowed_ret_values)) {
      if (empty($exec_output)) {
        // Maybe stderr gave something back.
        exec($command . ' 2>&1', $exec_output);
      }

      return TRUE;
    }

    return FALSE;
  }

  /**
   * Requirements php modules.
   *
   * @return array
   *   Returns an associative array;
   *   - 'errors':
   *     The number of errors encountered.
   *   - 'requirements':
   *     Array of requirements used for display.
   */
  public static function requirements_php_modules() {
    // Required modules.
    $required_extensions = array('gd', 'curl', 'SimpleXML');

    // Check which database driver its using.
    if (db_driver() == 'pgsql') {
      // Default MySQL.
      $required_extensions[] = 'pgsql';
    }
    else {
      // Default MySQL.
      $required_extensions[] = 'mysql';
      $required_extensions[] = 'mysqli';
    }

    // Build the requirements.
    $requirements = array();
    $errors = 0;

    // Get the loaded extensions which PHP has loaded.
    $loaded_extensions = get_loaded_extensions();
    foreach ($required_extensions as $extension) {
      // Extension available?
      $missing = !in_array($extension, $loaded_extensions);

      $requirements[$extension] = array(
        'title' => '<b>' . st('PHP module @extension:', array('@extension' => $extension)) . '</b>',
        'value' => !$missing ? st('Installed') : st('PHP module @extension is not installed.', array('@extension' => $extension)) ,
        'severity' => !$missing ? REQUIREMENT_OK : REQUIREMENT_ERROR,
      );

      if ($missing) {
        $errors++;
      }
    }

    // Done.
    return array('errors' => $errors, 'requirements' => $requirements);
  }

  /**
   * Requirements installed programs.
   *
   * @return array
   *   Returns an associative array;
   *   - 'errors':
   *     The number of errors encountered.
   *   - 'requirements':
   *     Array of requirements used for display.
   */
  public static function requirements_installed_programs() {

    // FFmpeg check.
    $exec_output = array();
    $ffmpeg_installed = self::command_installed('ffmpeg -version', $exec_output);

    $requirements = array();
    $requirements['ffmpeg'] = array(
      'title' => '<b>FFmpeg:</b>',
      'value' => $ffmpeg_installed ? 'Installed' : 'FFmpeg is not installed or inaccessable for PHP.' ,
      'severity' => $ffmpeg_installed ? REQUIREMENT_OK : REQUIREMENT_ERROR,
      'description' => $ffmpeg_installed ? 'Found ' . reset($exec_output) : st('Install !ffmpeg.', array('!ffmpeg' => l('FFmpeg', 'http://www.ffmpeg.org/', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE)))),
    );

    // Lua.
    $exec_output = array();
    $lua_installed = self::command_installed('lua -v', $exec_output);

    $requirements['lua'] = array(
      'title' => '<b>LUA 5.1:</b>',
      'value' => $lua_installed ? 'Installed' : 'LUA is not installed.' ,
      'severity' => $lua_installed ? REQUIREMENT_OK : REQUIREMENT_ERROR,
      'description' => $lua_installed ? st('Found @found', array('@found' => reset($exec_output))) : st('Install LUA 5.1. You can find more information how to install LUA !here', array('!here' => l('here', 'http://mediamosa.org/forum/viewtopic.php', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE, 'query' => array('f'=> '13', 't' => '175', 'start' => '10'), 'fragment' => 'p687')))),
    );

    // LUA test.
    $exec_output = array();
    exec('lua ' . escapeshellcmd(DRUPAL_ROOT) . '/profiles/mediamosa_profile/lua/lua_test', $exec_output);
    $lua_run = !empty($exec_output) && reset($exec_output) == 'lua works';
    if (!$lua_run) {
      $text = array(st('Unable to execute LUA script.'));
      if ($lua_installed) {
        $text[] = st("LUA was detected, but was not able to execute our test script. Please check if execute flags are our test scripts located at location '!scripts'", array('!scripts' => DRUPAL_ROOT . '/profiles/mediamosa_profile/lua/'));
      }
    }

    $requirements['lpeg'] = array(
      'title' => '<b>LUA 5.1 execution test:</b>',
      'value' => $lua_run ? st('Passed') : st('Failed.') ,
      'severity' => $lua_run ? REQUIREMENT_OK : REQUIREMENT_ERROR,
      'description' => $lua_run ? '' : implode(' ', $text),
    );

    // Lpeg.
    $exec_output = array();
    exec('lua ' . escapeshellcmd(DRUPAL_ROOT) . '/profiles/mediamosa_profile/lua/lua_lpeg_test', $exec_output);
    $lua_lpeg_found = !empty($exec_output) && reset($exec_output) == 'lpeg works';
    if (!$lua_lpeg_found) {
      $text = array(st('Install LHA 5.1 Lpeg extension. You can find more information how to install Lpeg !here', array('!here' => l('here', 'http://mediamosa.org/forum/viewtopic.php', array('attributes' => array('target' => '_blank'), 'absolute' => TRUE, 'external' => TRUE, 'query' => array('f'=> '13', 't' => '175', 'start' => '10'), 'fragment' => 'p687')))));
      if ($lua_installed) {
        $text[] = st("LUA was detected, but was not able to execute our test script. Please check if execute flags are our test scripts located at location '!scripts'", array('!scripts' => DRUPAL_ROOT . '/profiles/mediamosa_profile/lua/'));
      }
    }

    // Lua Lpeg.
    $requirements['app_lua_lpeg'] = array(
      'title' => '<b>LUA 5.1 Lpeg execution test:</b>',
      'value' => $lua_run ? st('Passed') : st('Failed.') ,
      'severity' => $lua_lpeg_found ? REQUIREMENT_OK : REQUIREMENT_ERROR,
      'description' => $lua_lpeg_found ? '' : implode(' ', $text),
    );

    $errors = 0;
    foreach ($requirements as $requirement) {
      if ($requirement['severity'] == REQUIREMENT_ERROR || $requirement['severity'] == REQUIREMENT_WARNING) {
        $errors++;
      }
    }

    // Done
    return array('errors' => $errors, 'requirements' => $requirements);
  }

  /**
   * Checking the PHP Settings.
   *
   * Only possible warnings for now.
   *
   * @return array
   *   Returns an associative array;
   *   - 'errors':
   *     The number of errors encountered.
   *   - 'requirements':
   *     Array of requirements used for display.
   */
  public static function requirements_php_settings() {

    $requirements = array();

    $php_upload_max_filesize = ini_get('upload_max_filesize');
    $too_low = (substr($php_upload_max_filesize, 0, -1) < 100) && (substr($php_upload_max_filesize, -1) != 'M' || substr($php_upload_max_filesize, -1) != 'G');
    $requirements['upload_max_filesize'] = array(
      'title' => '<b>' . st('upload_max_filesize:') . '</b>',
      'value' => $php_upload_max_filesize,
      'severity' => !$too_low ? REQUIREMENT_OK : REQUIREMENT_WARNING,
      'description' => !$too_low ? '' : st('Warning: upload_max_filesize should be at least 100M.'),
    );

    $php_memory_limit = ini_get('memory_limit');
    $too_low = (substr($php_memory_limit, 0, -1) < 128) && (substr($php_memory_limit, -1) != 'M' || substr($php_memory_limit, -1) != 'G');
    $requirements['memory_limit'] = array(
      'title' => '<b>' . st('memory_limit:') . '</b>',
      'value' => $php_memory_limit,
      'severity' => !$too_low ? REQUIREMENT_OK : REQUIREMENT_WARNING,
      'description' => !$too_low ? '' : st('Warning: memory_limit should be at least 128M.'),
    );

    $php_post_max_size = ini_get('post_max_size');
    $too_low = (substr($php_post_max_size, 0, -1) < 100) && (substr($php_post_max_size, -1) != 'M' || substr($php_post_max_size, -1) != 'G');
    $requirements['php_post_max'] = array(
      'title' => '<b>' . st('post_max_size:') . '</b>',
      'value' => $php_post_max_size,
      'severity' => !$too_low ? REQUIREMENT_OK : REQUIREMENT_WARNING,
      'description' => !$too_low ? '' : st('Warning: post_max_size should be at least 100M.'),
    );

    // Done.
    return array('errors' => 0, 'requirements' => $requirements);
  }

  /**
   * Check if the directory is exist, else create it.
   *
   * @param $path
   *   Directory to create.
   */
  public static function mountpoint_mkdir($path) {
    // Will add left slash.
    $path = ltrim($path, '/');

    // Expecting mountpoint to be set as variable.
    $mountpoint = variable_get('mediamosa_current_mount_point');

    // Create dir.
    if (!file_exists($mountpoint . '/' .  $path)) {
      drupal_mkdir($mountpoint . '/' . $path, NULL, TRUE);
    }

    // To separate simpletest from our installation, simpletest has its own
    // mount pount.
    if (!file_exists($mountpoint . '/media/simpletest/' . $path)) {
      drupal_mkdir($mountpoint . '/media/simpletest/' . $path, NULL, TRUE);
    }
  }

  /**
   * Create the basic mountpoint setup.
   */
  public static function setup_mountpoint() {
    // Inside the storage location, create a MediaMosa storage structure.
    mediamosa_profile::mountpoint_mkdir('data');

    // We store each file in separate directories based on the first letter of the
    // file. We need to create these directories.
    $prefixes = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    for ($x = 0; $x < strlen($prefixes); $x++) {
      mediamosa_profile::mountpoint_mkdir('data/' . $prefixes{$x});
    }

    // Other.
    mediamosa_profile::mountpoint_mkdir('data/transcode');
    mediamosa_profile::mountpoint_mkdir('data/transition');
    mediamosa_profile::mountpoint_mkdir('ftp');

    // /media is replacing /still_links.
    mediamosa_profile::mountpoint_mkdir('media');
    mediamosa_profile::mountpoint_mkdir('media/ticket');

    // New for generated object code (like .asx files).
    mediamosa_profile::mountpoint_mkdir('media/object');

    // /media/download is replacing download_links.
    mediamosa_profile::mountpoint_mkdir('media/download');

    // Create the htaccess file to protected the media directory.
    mediamosa_storage::file_create_htaccess(variable_get('mediamosa_current_mount_point') . '/media', mediamosa_storage::file_get_media_htaccess_contents());
  }
}
