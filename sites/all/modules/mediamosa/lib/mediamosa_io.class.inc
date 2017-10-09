<?php
/**
 * @file
 * In and out file library layer.
 */

class mediamosa_io {
  // ------------------------------------------------------------------- Consts.
  /**
   * PHP:stat() mode values; Mask for all types. Octal value.
   */
  const STAT_IFMT = 0170000;

  /**
   * PHP:stat() mode values; Type: socket. Octal value.
   */
  const STAT_IFSOCK = 0140000;
  /**
   * PHP:stat() mode values; Type: symbolic link. Octal value.
   */
  const STAT_IFLNK = 0120000;

  /**
   * PHP:stat() mode values; Type: regular file. Octal value.
   */
  const STAT_IFREG = 0100000;

  /**
   * PHP:stat() mode values; Type: block device. Octal value.
   */
  const STAT_IFBLK = 0060000;

  /**
   * PHP:stat() mode values; Type: directory. Octal value.
   */
  const STAT_IFDIR = 0040000;

  /**
   * PHP:stat() mode values; Type: character device. Octal value.
   */
  const STAT_IFCHR = 0020000;

  /**
   * PHP:stat() mode values; Type: fifo. Octal value.
   */
  const STAT_IFIFO = 0010000;

  /**
   * PHP:stat() mode values; Set-uid bit. Octal value.
   */
  const STAT_ISUID = 0004000;

  /**
   * PHP:stat() mode values; Set-gid bit. Octal value.
   */
  const STAT_ISGID = 0002000;

  /**
   * PHP:stat() mode values; Sticky bit. Octal value.
   */
  const STAT_ISVTX = 0001000;

  /**
   * PHP:stat() mode values; Mask for owner permissions. Octal value.
   */
  const STAT_IRWXU = 00700;

  /**
   * PHP:stat() mode values; Owner: read permission. Octal value.
   */
  const STAT_IRUSR = 00400;
  /**
   * PHP:stat() mode values; Owner: write permission. Octal value.
   */
  const STAT_IWUSR = 00200;

  /**
   * PHP:stat() mode values; Owner: execute permission. Octal value.
   */
  const STAT_IXUSR = 00100;

  /**
   * PHP:stat() mode values; Mask for group permissions. Octal value.
   */
  const STAT_IRWXG = 00070;

  /**
   * PHP:stat() mode values; Group: read permission. Octal value.
   */
  const STAT_IRGRP = 00040;

  /**
   * PHP:stat() mode values; Group: write permission. Octal value.
   */
  const STAT_IWGRP = 00020;

  /**
   * PHP:stat() mode values; Group: execute permission. Octal value.
   */
  const STAT_IXGRP = 00010;

  /**
   * PHP:stat() mode values; Mask for others permissions. Octal value.
   */
  const STAT_IRWXO = 00007;

  /**
   * PHP:stat() mode values; Others: read permission. Octal value.
   */
  const STAT_IROTH = 00004;

  /**
   * PHP:stat() mode values; Others: write permission. Octal value.
   */
  const STAT_IWOTH = 00002;

  /**
   * PHP:stat() mode values; Others: execute permission. Octal value.
   */
  const STAT_IXOTH = 00001;


  // ---------------------------------------------------------------- Functions.
  /**
   * Log for mediamosa_io.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   */
  public static function log($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'mediamosa_io');
  }

  /**
   * Return the stream wrapper using the file uri.
   *
   * Works fine with all Drupal wrappers.
   *
   * @param string $uri
   *   The file uri.
   *
   * @return mediamosa_io_streamwrapper
   *   The stream wrapper instance.
   *
   * @throws mediamosa_exception_error
   */
  public static function require_stream_wrapper_instance_by_uri($uri) {
    // Find it.
    $instance = self::find_stream_wrapper_instance_by_uri($uri);
    if (!$instance) {
      // Fatal, need it.
      throw new mediamosa_exception_error_streamwrapper_not_found(array('@uri' => $uri));
    }

    return $instance;
  }

  /**
   * Return the stream wrapper using the file uri.
   *
   * Works fine with all Drupal wrappers.
   *
   * @param string $uri
   *   The file uri.
   *
   * @return mediamosa_io_streamwrapper
   *   The stream wrapper instance or FALSE.
   */
  public static function find_stream_wrapper_instance_by_uri($uri) {
    // Get scheme.
    $scheme = self::get_uri_scheme($uri);

    if (!empty($scheme)) {
      $class = file_stream_wrapper_get_class($scheme);
      if (class_exists($class)) {
        $instance = new $class();
        $instance->setUri($uri);
        return $instance;
      }
    }

    return FALSE;
  }

  /**
   * Test if uri is realpath or uri format.
   *
   * @param string $uri
   *   The uri to test.
   *
   * @return bool
   *   Returns TRUE when uri.
   */
  public static function is_uri($uri) {
    return self::get_uri_scheme($uri) != '';
  }

  /**
   * Return the scheme of the uri.
   *
   * @param string $uri
   *   The uri.
   *
   * @return string
   *   The uri scheme, like 'mediamosa-storage' or ''.
   */
  public static function get_uri_scheme($uri) {
    $position = strpos($uri, '://');
    return $position ? substr($uri, 0, $position) : '';
  }

  /**
   * Return the path of the uri.
   *
   * @param string $uri
   *   The uri.
   *
   * @return string
   *   The (trimmed) path.
   */
  public static function get_uri_path($uri) {
    $position = strpos($uri, '://');
    return $position ? trim(substr($uri, $position + 3), '\/') : '';
  }

  /**
   * Returns the absolute local filesystem path of a stream URI.
   *
   * @param string $uri
   *   A stream wrapper URI or a filesystem path.
   *
   * @return string
   *   The absolute local filesystem path, or FALSE on failure.
   *
   * @throws mediamosa_exception_error_file_not_found()
   */
  public static function realpath($uri) {
    // Empty uri is always fatal.
    if (empty($uri)) {
      throw new mediamosa_exception_program_error('URI value is required for realpath.');
    }

    // No scheme, then its already in realpath.
    if (self::get_uri_scheme($uri) == '') {
      // Is already realpath.
      return $uri;
    }

    try {
      // Get wrapper.
      $wrapper = self::require_stream_wrapper_instance_by_uri($uri);
    }
    catch (mediamosa_exception_error_streamwrapper_not_found $e) {
      return $uri;
    }

    // Call realpath().
    $realpath = $wrapper->realpath();

    // Make sure it never returns root.
    if ($realpath == DIRECTORY_SEPARATOR || empty($realpath)) {
      throw new mediamosa_exception_error_file_not_found("The mediamosa_io::realpath() functions was not able to translate uri '@uri', realpath translated this to: '@realpath'.", array('@uri' => $uri, '@realpath' => $realpath));
    }

    // Done.
    return $realpath;
  }

  /**
   * Returns the absolute local filesystem path of a stream URI.
   *
   * Is exception safe and should be used for watchdog calls.
   *
   * @param string $uri
   *   A stream wrapper URI or a filesystem path.
   *
   * @return string
   *   The realpath or $uri when failure.
   *
   * @see realpath()
   */
  public static function realpath_safe($uri) {
    try {
      return self::realpath($uri);
    }
    catch (Exception $e) {
      return $uri;
    }
  }

  /**
   * Returns the absolute URL version of a stream URI.
   *
   * @param string $uri
   *   A stream wrapper URI or a filesystem path, possibly including one or more
   *   symbolic links.
   *
   * @return string
   *   The absolute URL or FALSE on failure.
   */
  public static function realurl($uri) {
    return file_create_url($uri);
  }

  /**
   * Attempts to create the directory specified by pathname.
   *
   * Fails with exception. This overrides the recursive php mkdir
   * as it does not correctly sets chmod recursively. It also uses the
   * sannas mount point as a basis, so it never tries to create a directory
   * outside this tree.
   *
   * @param string $uri
   *   The uri of the directory to create.
   * @param bool $recursive
   *   Enable recursive to create the sub maps required to create directory.
   * @param int $mode
   *   Default mode value for created directory, defaults to mode 0775.
   *
   * @return bool
   *   Return TRUE or throws exception.
   *
   * @throws mediamosa_exception_error()
   *   - ERRORCODE_DIR_UNABLE_TO_CREATE
   */
  public static function mkdir($uri, $recursive = TRUE, $mode = NULL) {
    // Reuse mode from drupal.
    if (!isset($mode)) {
      $mode = variable_get('file_chmod_directory', 0775);
    }

    // Already exists? Then don't try to create.
    if (self::file_exists($uri)) {
      if (!self::is_dir($uri)) {
        throw new mediamosa_exception_error(
          mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
          array(
            '@location' => self::realpath_safe($uri),
            '@reason' => 'can not create dir, target exists and is file type',
          )
        );
      }

      // Exists & is dir.
      return;
    }

    // Not recursive?
    if (!$recursive) {
      if (!drupal_mkdir($uri, $mode)) {
        throw new mediamosa_exception_error(
          mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
          array(
            '@location' => self::realpath_safe($uri),
            '@reason' => mediamosa::last_php_error_message(),
          )
        );
      }
      // Set mode again.
      drupal_chmod($uri, $mode);

      // Directory created.
      return TRUE;
    }

    $separators = '/';
    if (DIRECTORY_SEPARATOR != '/') {
      // For Windows OS add special separator.
      $separators .= DIRECTORY_SEPARATOR;
    }

    // We do our own recursive mkdir here; as it does not correctly sets chmod
    // recursively.
    $uri_scheme = self::get_uri_scheme($uri);

    // Get path without scheme.
    $uri_path = ($uri_scheme == '' ? trim($uri, $separators) : self::get_uri_path($uri));

    // Either scheme:/ or '' (will add '/' because of implode).
    $uri_scheme = ($uri_scheme == '' ? '' : ($uri_scheme . ':/'));


    // Precreate scheme location.
    try {
      if (!empty($uri_scheme) && !self::file_exists($uri_scheme . '/')) {
        if (!drupal_mkdir($uri_scheme . '/', $mode, TRUE)) {
          throw new mediamosa_exception_error(
            mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
            array(
              '@location' => self::realpath_safe($uri_scheme . '/'),
              '@reason' => mediamosa::last_php_error_message(),
            )
          );
        }
        // Set mode again.
        drupal_chmod($uri_scheme . '/', $mode);
      }
    }
    catch (mediamosa_exception $e) {
      // Catch invalid uri errors. Some streamwrapper do not allow using the
      // stream instance with only the scheme.
    }

    // Split up on separator(s).
    $parts = preg_split('/[' . preg_quote($separators, '/') . ']/', $uri_path, -1, PREG_SPLIT_NO_EMPTY);

    // Path to rebuild.
    $path = array($uri_scheme);

    // Now do recursive on the parts.
    for ($x = 0; $x < count($parts); $x++) {
      $part = $parts[$x];
      // No empty parts.
      if ($part === '') {
        continue;
      }

      // The '-' will happen because our stream uri have '/-/' as path <->
      // mediafile_path seperator.
      if ($part == '-') {
        if (!isset($parts[++$x])) {
          continue;
        }
        $path[] = '/' . $part;
        $part = $parts[$x];
      }

      $path[] = $part;
      $uri = implode($path, DIRECTORY_SEPARATOR);

      if (!self::file_exists($uri)) {
        if (!drupal_mkdir($uri, $mode, TRUE)) {
          throw new mediamosa_exception_error(
            mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
            array(
              '@location' => self::realpath_safe($uri),
              '@reason' => mediamosa::last_php_error_message(),
            )
          );
        }
        // Set mode again.
        drupal_chmod($uri, $mode);
      }
    }

    return TRUE;
  }

  /**
   * Deletes a file.
   *
   * @param string $uri
   *   A URI or pathname.
   *
   * @return bool
   *   Return TRUE on success or FALSE with failure.
   */
  public static function unlink($uri) {
    if (!self::file_exists($uri)) {
      return TRUE;
    }

    // Make sure md5 is recalculated next time.
    $md5_file = &drupal_static('mediamosa_io::md5file', array());
    unset($md5_file[$uri]);

    // Remove file.
    return drupal_unlink($uri);
  }

  /**
   * Tells whether the filename is a symbolic link.
   *
   * @param string $uri
   *   The uri of the link to test.
   */
  public static function is_link($uri) {
    return is_link($uri);
  }

  /**
   * Clears file status cache.
   */
  public static function clearstatcache($filename = NULL) {
    self::log('Cleared stat cache');
    if ($filename) {
      clearstatcache(TRUE, $filename);
    }
    else {
      clearstatcache(TRUE);
    }
  }

  /**
   * Return the filename without extension.
   *
   * FIXME: Name of function is wrong:
   * FIXME: The basename of a file path is filename with possible extension.
   *
   * @param string $filename
   *   The filename to process.
   *
   * @return string
   *   The filename without extension.
   */
  public static function get_base_filename($filename) {
    return pathinfo($filename, PATHINFO_FILENAME);
  }

  /**
   * Return the file extension (excluding the dot, like 'jpg' or 'mp4').
   *
   * @param string $filename
   *   The filename to get extension from.
   *
   * @return string
   *   The file extension without the dot or ''.
   */
  public static function get_ext($filename) {
    $first_dot = stripos($filename, '.');

    return $first_dot === FALSE && ($first_dot + 1) != mediamosa_unicode::strlen($filename) ? '' : mediamosa_unicode::substr($filename, $first_dot + 1);
  }

  /**
   * Returns directory name component of path.
   *
   * @param string $path
   *   The path to process.
   *
   * @return string
   *   The directory name component.
   */
  public static function dirname($path) {
    return drupal_dirname($path);
  }

  /**
   * Returns trailing name component of path.
   *
   * @param string $uri
   *   A uri.
   * @param string $suffix
   *   (optional) If the name component ends in suffix this will also be cut
   *   off.
   *
   * @return string
   *   Returns the base name of the given path.
   */
  public static function basename($uri, $suffix = '') {
    return drupal_basename($uri, $suffix);
  }

  /**
   * Move one file to other file (rename).
   *
   * Notes;
   * It will not move local to external locations.
   * It will not move across different external locations.
   * You can only move files externally when source and destination are the
   * same location.
   * You can move files across local locations.
   *
   * @param string $from_uri
   *   Rename from name.
   * @param string $to_uri
   *   Rename to name.
   *
   * @return bool
   *   Returns TRUE when successful.
   */
  public static function rename($from_uri, $to_uri) {
    $success = FALSE;
    $message = 'cause unknown';

    try {
      $mediamosa_io_streamwrapper_from = NULL;
      $mediamosa_io_streamwrapper_to = NULL;

      // Create wrapper when we can. If we can not, its a local path.
      if (self::is_uri($from_uri)) {
        $mediamosa_io_streamwrapper_from = mediamosa_io::require_stream_wrapper_instance_by_uri($from_uri);
      }

      // Create wrapper when we can. If we can not, its a local path.
      if (self::is_uri($to_uri)) {
        $mediamosa_io_streamwrapper_to = mediamosa_io::require_stream_wrapper_instance_by_uri($to_uri);
      }

      // What is local?
      $from_is_local = !$mediamosa_io_streamwrapper_from || ($mediamosa_io_streamwrapper_from && $mediamosa_io_streamwrapper_from->is_local());
      $to_is_local = !$mediamosa_io_streamwrapper_to || ($mediamosa_io_streamwrapper_to && $mediamosa_io_streamwrapper_to->is_local());

      // Is both local?
      if ($from_is_local && $to_is_local) {
        // Both are local.
        //
        // Make sure dest. map exists.
        self::mkdir(self::dirname($to_uri), TRUE);

        // To realpath.
        $from_realpath = mediamosa_io::realpath($from_uri);
        $to_realpath = mediamosa_io::realpath($to_uri);

        // Rename.
        $success = rename($from_realpath, $to_realpath);
      }
      elseif (!$from_is_local && !$to_is_local) {
        // Both external, must be same location. Compare storage profile ID.
        if ($mediamosa_io_streamwrapper_from->get_storage_profile_id() != $mediamosa_io_streamwrapper_to->get_storage_profile_id()) {
          // Both external, but not same location.
          throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t('cannot rename a file across external wrapper types')));
        }

        // Same location external, we can rename.
        //
        // Make sure dest. map exists.
        self::mkdir(self::dirname($to_uri), TRUE);

        // Rename using the streamwrapper.
        $success = $mediamosa_io_streamwrapper_from->rename($from_uri, $to_uri);
      }
      else {
        // One external, one local, not allowed.
        throw new mediamosa_exception_error(mediamosa_sdk::ERRORCODE_STORAGE_IO_ERROR, array('@error' => t('cannot rename a file across wrapper types')));
      }
    }
    catch (Exception $e) {
      $message = $e->getMessage();
    }

    if (!$success || !self::file_exists($to_uri)) {
      self::log("Renaming file '@old_name' to '@new_name' failed, @message.",
        array(
          '@old_name' => $from_uri,
          '@new_name' => $to_uri,
          '@message' => $message,
        )
      );
    }
    else {
      self::log("Successfully renamed file '@old_name' to '@new_name'.", array('@old_name' => mediamosa_io::realpath_safe($from_uri), '@new_name' => mediamosa_io::realpath_safe($to_uri)));
    }

    return $success;
  }

  /**
   * Execute an external program.
   *
   * @param string $command
   *   The command that will be executed.
   * @param int $return_var
   *   If the return_var argument is present along with the output argument,
   *   then the return status of the executed command will be written to this
   *   variable.
   *
   * @return array
   *   Array filled with every line of output from the command. Trailing
   *   whitespace, such as \n, is not included in this array.
   */
  public static function exec($command, &$return_var = NULL) {
    self::log("Executing command '@command'.", array('@command' => $command));
    $output = array();
    exec($command, $output, $return_var);
    return $output;
  }

  /**
   * Converts absolute symlink target->link to relative link.
   */
  public static function symlink_absolute2relative($target, $link) {
    // Check if absolute, link and target needs to absolute for this to work.
    if (mediamosa_unicode::substr($target, 0, 1) == DIRECTORY_SEPARATOR && mediamosa_unicode::substr($link, 0, 1) == DIRECTORY_SEPARATOR) {
      // Remove double slashes.
      $target = str_replace(DIRECTORY_SEPARATOR . DIRECTORY_SEPARATOR, DIRECTORY_SEPARATOR, $target);
      $link = str_replace(DIRECTORY_SEPARATOR . DIRECTORY_SEPARATOR, DIRECTORY_SEPARATOR, $link);

      // Get path.
      $target_path = self::is_file($target) ? self::dirname($target) : $target;

      // Strip /.
      $target_path = trim($target_path, DIRECTORY_SEPARATOR);
      $link = trim(self::dirname($link), DIRECTORY_SEPARATOR);

      // Into parts.
      $target_parts = explode(DIRECTORY_SEPARATOR, $target_path);
      $link_parts = explode(DIRECTORY_SEPARATOR, $link);

      // Must contain something.
      if (empty($link_parts)) {
        return $target;
      }

      // If root is not the same, then related .. comes in front.
      if (!empty($target_parts) && $target_parts[0] == $link_parts[0]) {
        $max_parts = min(count($target_parts), count($link_parts));
        for ($x = 0; $x < $max_parts; $x++) {
          if ($target_parts[$x] != $link_parts[$x]) {
            break;
          }

          unset($link_parts[$x]);
          unset($target_parts[$x]);
        }
      }

      // Rebuild target.
      $target = (count($link_parts) ? str_repeat('..' . DIRECTORY_SEPARATOR, count($link_parts)) : '.') . implode(DIRECTORY_SEPARATOR, $target_parts) . (self::is_file($target) ? DIRECTORY_SEPARATOR . self::basename($target) : '');
    }

    return $target;
  }

  /**
   * Create a symbolic link.
   *
   * @param string $target_uri
   *   Target file.
   * @param string $link_uri
   *   Link to file to create.
   */
  public static function symlink($target_uri, $link_uri, $make_relative = TRUE) {
    $target = self::realpath($target_uri);
    $link = self::realpath($link_uri);
    if (empty($target) || empty($link)) {
      self::log('Unable to create symlink (realpath fails); link: @link, target: @target', array('@link' => $link, '@target' => $target));
      return FALSE;
    }

    self::clearstatcache($link);

    if (self::file_exists($link)) {
      self::log('Symlink already exists: @link, target: @target', array('@link' => $link, '@target' => $target));
      return TRUE;
    }

    if ($make_relative) {
      // Make symlinks targets relative.
      $target = self::symlink_absolute2relative($target, $link);
    }

    self::log('Creating symlink; link: @link, target: @target', array('@link' => $link, '@target' => $target));
    $result = symlink($target, $link);
    if (!$result) {
      self::log('Failed to create symlink; link: @link, target: @target', array('@link' => $link, '@target' => $target));
    }

    // Seems that symlink does not reset the realpath cache.
    self::clearstatcache($link);

    return $result;
  }

  /**
   * Tells whether a file exists and is readable.
   *
   * @param string $uri
   *   The uri to test.
   *
   * @return bool
   *   Returns TRUE if the file or directory specified by filename exists and is
   *   readable, FALSE otherwise.
   */
  public static function is_readable($uri) {
    return is_readable($uri);
  }

  /**
   * Tells whether the filename is writable.
   *
   * @param string $uri
   *   The uri to test.
   *
   * @return bool
   *   Returns TRUE if the filename exists and is writable.
   */
  public static function is_writable($uri) {
    return is_writable($uri);
  }

  /**
   * Tells whether the filename is a regular file.
   *
   * @param string $uri
   *   The file to check.
   *
   * @return bool
   *   Returns TRUE when file, FALSE otherwise.
   */
  public static function is_file($uri) {
    try {
      // Get wrapper.
      $wrapper = self::require_stream_wrapper_instance_by_uri($uri);
    }
    catch (mediamosa_exception_error_streamwrapper_not_found $e) {
      return is_file($uri);
    }

    // Need wrapper method.
    return $wrapper && method_exists($wrapper, 'is_file') ? $wrapper->is_file() : is_file($uri);
  }

  /**
   * Test if path is a directory.
   *
   * @param string $uri
   *   The path to test.
   *
   * @return bool
   *   Returns TRUE when dir, FALSE otherwise.
   */
  public static function is_dir($uri) {
    try {
      // Get wrapper.
      $wrapper = self::require_stream_wrapper_instance_by_uri($uri);
    }
    catch (mediamosa_exception_error_streamwrapper_not_found $e) {
      return is_dir($uri);
    }

    // Need wrapper method.
    return $wrapper && method_exists($wrapper, 'is_dir') ? $wrapper->is_dir() : is_dir($uri);
  }

  /**
   * Test if uri is located on local space.
   *
   * @param string $uri
   *   The uri to file or dir.
   *
   * @return bool
   *   Returns TRUE for local.
   */
  public static function is_local($uri) {
    // When not uri, expect local.
    if (!self::is_uri($uri)) {
      return TRUE;
    }

    try {
      // Get wrapper.
      $wrapper = self::require_stream_wrapper_instance_by_uri($uri);
    }
    catch (mediamosa_exception_error_streamwrapper_not_found $e) {
      return TRUE;
    }

    // Need wrapper method.
    return $wrapper && method_exists($wrapper, 'is_local') ? $wrapper->is_local() : TRUE;

  }

  /**
   * Test if file is exists.
   *
   * @param string $uri
   *   The file/path to test.
   *
   * @return bool
   *   Returns TRUE when file exists.
   */
  public static function file_exists($uri) {
    try {
      // Get wrapper.
      $wrapper = self::require_stream_wrapper_instance_by_uri($uri);
    }
    catch (mediamosa_exception_error_streamwrapper_not_found $e) {
      return file_exists($uri);
    }

    // Need wrapper method.
    return $wrapper && method_exists($wrapper, 'file_exists') ? $wrapper->file_exists() : file_exists($uri);
  }

  /**
   * Gets file size.
   *
   * @param string $uri
   *   The file location.
   *
   * @return integer|boolean
   *   The size of the file or FALSE.
   */
  public static function filesize($uri) {
    // Must exists.
    if (!self::file_exists($uri)) {
      return FALSE;
    }

    try {
      // Get wrapper.
      $wrapper = self::require_stream_wrapper_instance_by_uri($uri);
    }
    catch (mediamosa_exception_error_streamwrapper_not_found $e) {
      return filesize($uri);
    }

    // Need wrapper method.
    return $wrapper && method_exists($wrapper, 'filesize') ? $wrapper->filesize() : filesize($uri);
  }

  /**
   * Create an empty file.
   *
   * If file already exists, touch function is skipped.
   *
   * @param string $uri
   *   The uri of the file being created.
   *
   * @param boolean
   *   Returns TRUE on success, FALSE otherwise.
   */
  public static function touch($uri) {
    // Touch does not realpath.
    return self::file_exists($uri) ? TRUE : touch(self::realpath($uri));
  }

  /**
   * Makes a copy of the file source to dest.
   *
   * Warning: If the destination file already exists, it will be overwritten.
   *
   * @param string $source
   *   Path to the source file.
   * @param string $dest
   *   The destination path. If dest is a URL, the copy operation may fail if
   *   the wrapper does not support overwriting of existing files.
   */
  public static function copy($source, $dest) {
    return @copy($source, $dest);
  }

  /**
   * Move the file source to dest.
   *
   * @param string $source_uri
   *   Source name uri.
   * @param string $dest_uri
   *   Destination name uri.
   *
   * @return bool
   *   Returns TRUE for succes, FALSE otherwise.
   */
  public static function move($source_uri, $dest_uri) {
    return @rename(self::realpath($source_uri), self::realpath($dest_uri));
  }

  /**
   * Finds all files that match a given mask in a given directory.
   *
   * Directories and files beginning with a period are excluded; this
   * prevents hidden files and directories (such as SVN working directories)
   * from being scanned.
   *
   * @param string $dir
   *   The base directory or URI to scan, without trailing slash.
   * @param string $mask
   *   The preg_match() regular expression of the files to find.
   * @param array $options
   *   An associative array of additional options, with the following elements:
   *   - 'nomask': The preg_match() regular expression of the files to ignore.
   *     Defaults to '/(\.\.?|CVS)$/'.
   *   - 'callback': The callback function to call for each match. There is no
   *     default callback.
   *   - 'recurse': When TRUE, the directory scan will recurse the entire tree
   *     starting at the provided directory. Defaults to TRUE.
   *   - 'key': The key to be used for the returned associative array of files.
   *     Possible values are 'uri', for the file's URI; 'filename', for the
   *     basename of the file; and 'name' for the name of the file without the
   *     extension. Defaults to 'uri'.
   *   - 'min_depth': Minimum depth of directories to return files from.
   *     Defaults to 0.
   * @param int $depth
   *   Current depth of recursion. This parameter is only used internally and
   *   should not be passed in.
   *
   * @return array
   *   An associative array (keyed on the chosen key) of objects with 'uri',
   *   'filename', and 'name' members corresponding to the matching files.
   */
  public static function file_scan_directory($dir, $mask, $options = array(), $depth = 0) {
    return file_scan_directory($dir, $mask, $options, $depth);
  }

  /**
   * Returns available space on filesystem or disk partition.
   *
   * Given a file name instead of a directory, the behaviour of the
   * function is unspecified and may differ between operating systems and
   * PHP versions.
   *
   * @param string $uri
   *   A directory of the filesystem or disk partition.
   *
   * @return float
   *   The number of available bytes as a float or FALSE on failure.
   *
   * @link http://php.net/manual/en/function.disk-free-space.php
   */
  public static function disk_free_space($uri) {
    return disk_free_space(mediamosa_io::realpath($uri));
  }

  /**
   * Returns available space on filesystem or disk partition.
   *
   * @param string $uri
   *   The directory to get totals from.
   */
  public static function disk_total_space($uri) {
    return disk_total_space(mediamosa_io::realpath($uri));
  }

  /**
   * Test if given command is install / found.
   *
   * Also returns its default stdio / strerr output in $exec_output
   *
   * @param string $command
   *   Command to test.
   * @param string $exec_output
   *   Gets the output of stdio/stderr
   * @param array $allowed_ret_values
   *   List of return values that indicate command is installed. Most programs
   *   will use exit(0) as indication that it was run. Some use exit(1) or
   *   higher as exit value.
   */
  public static function command_installed($command, &$exec_output, array $allowed_ret_values = array(0)) {
    $exec_output = array();
    $ret_val = 0;
    exec($command . ' 2>/dev/null', $exec_output, $ret_val);

    if (empty($exec_output)) {
      // Maybe stderr gave something back.
      exec($command . ' 2>&1', $exec_output);
    }

    // If ret_val is ok, then check if $exec_output is empty.
    if (in_array($ret_val, $allowed_ret_values)) {
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Calculate the space used by a directory.
   *
   * @param string $directory
   *   The path for scan.
   * @param bool $recursive
   *   Go deeper in subdirs.
   * @param bool $follow_symlink
   *   Also include symlinked directories.
   */
  public static function disk_usage($directory, $recursive = FALSE, $follow_symlink = TRUE) {
    if ((is_link($directory) && !$follow_symlink) || !self::file_exists($directory)) {
      return 0;
    }
    if (self::is_file($directory)) {
      return filesize($directory);
    }

    $disk_usage = 0;
    if (is_dir($directory)) {
      $dir = dir($directory);
      while (($entry = $dir->read()) !== FALSE) {
        $entry_path = $directory . '/' . $entry;
        if ($entry == '.' || $entry == '..' || !self::file_exists($entry_path) || (!$follow_symlink && is_link($entry_path))) {
          continue;
        }

        if (self::is_file($entry_path)) {
          $disk_usage += filesize($entry_path);
        }
        elseif ($recursive && self::is_dir($entry_path)) {
          $disk_usage += self::disk_usage($entry_path, $recursive, $follow_symlink);
        }
      }
      $dir->close();
    }

    return $disk_usage;
  }

  /**
   * Calculate the space used by a directory.
   */
  public static function disk_usage_recursive($directory) {
    return self::disk_usage($directory, TRUE);
  }

  /**
   * Examines a file object.
   *
   * Returns appropriate content headers for download.
   *
   * Based on Drupal's version 'file_get_content_headers()'. We also re-use the
   * file_inline_types variable.
   *
   * @param string $filename
   *   The filename.
   *
   * @return array
   *   An associative array of headers.
   *
   * @see file_get_content_headers()
   */
  public static function file_get_content_headers($filename, $mime_type) {
    $content_length = self::filesize($filename);
    $last_modified = filemtime($filename);
    // Hotfix: mime_header_encode() generates multi-line when utf-8 is used.
    //         But drupal_add_http_header() can not handle multi-line header
    //         items. We do break max line-length of 76 for HTTP, but seems no
    //         browser has a issue with this.
    $filename = rawurlencode(pathinfo($filename, PATHINFO_BASENAME));
    $mime_type = mime_header_encode($mime_type);

    // Serve images, text, and flash content for display rather than download.
    $inline_types = variable_get('file_inline_types', array(
      '^text/',
      '^image/',
      'flash$',
    ));
    $disposition = 'attachment';
    foreach ($inline_types as $inline_type) {
      // Exclamation marks are used as delimiters to avoid escaping slashes.
      if (preg_match('!' . $inline_type . '!', $mime_type)) {
        $disposition = 'inline';
      }
    }

    return array(
      'Content-Type' => $mime_type . '; name="' . $filename . '"',
      'Content-Length' => $content_length,
      'Content-Disposition' => $disposition . '; filename="' . $filename . '"',
      'Cache-Control' => 'private, max-age=' . mediamosa_settings::MEDIA_CACHE_SECOND . ', must-revalidate',
      'Last-Modified' => gmdate(DATE_RFC1123, $last_modified),
      'Expires' => gmdate(DATE_RFC1123, time() + mediamosa_settings::MEDIA_CACHE_SECOND),
    );
  }

  /**
   * Transfer file using HTTP to client.
   *
   * Based on Drupal's file_transfer().
   *
   * @param string $uri
   *   String specifying the file URI to transfer.
   * @param array $headers
   *   An array of HTTP headers to send along with file.
   */
  public static function file_transfer($uri, array $headers) {
    if (ob_get_level()) {
      ob_end_clean();
    }

    foreach ($headers as $name => $value) {
      drupal_add_http_header($name, $value);
    }
    drupal_send_headers();

    // Transfer file in chunks to save memory usage.
    $fd = fopen($uri, 'rb');
    if ($fd) {
      while (!feof($fd)) {
        print fread($fd, mediamosa_settings::DOWNLOAD_CHUNK_SIZE);
      }
      fclose($fd);

      if (!user_is_logged_in()) {
        // Prevent session save if we have no session to save.
        drupal_save_session(FALSE);
      }

      drupal_exit();
    }
  }

  /**
   * Calculate the MD5 of the file.
   *
   * @param string $uri
   *   The uri.
   *
   * @return string
   *   The MD5 string (16 chars).
   */
  public static function md5_file($uri) {
    // Cache.
    $md5_files = &drupal_static('mediamosa_io::md5file', array());

    // Get realpath.
    $realpath = self::realpath($uri);

    // See of cached version.
    if (!empty($md5_files[$realpath])) {
      return $md5_files[$realpath];
    }

    // Keep mem in check.
    if (count($md5_files) == 1000) {
      array_pop($md5_files);
    }

    // Use realpath, because this is always end product.
    $md5_files[$realpath] = file_exists($realpath) ? md5_file($realpath) : FALSE;

    return $md5_files[$realpath];
  }

  /**
   * Reads entire file into a string.
   *
   * @param string $uri
   *   Uri of the file to read.
   * @param bool $use_include_path
   *   As of PHP 5 the FILE_USE_INCLUDE_PATH can be used to trigger include path
   *   search.
   * @param resource $context
   *   A valid context resource created with stream_context_create(). If you
   *   the filters.
   *   don't need to use a custom context, you can skip this parameter by NULL.
   * @param int $offset
   *   The offset where the reading starts on the original stream.
   * @param int $maxlen
   *   Maximum length of data read. The default is to read until end of file is
   *   reached. Note that this parameter is applied to the stream processed by
   *
   * @return string
   *   The function returns the read data or FALSE on failure. Becareful with
   *   memory.
   */
  public static function file_get_contents($uri, $use_include_path = FALSE, $context = NULL, $offset = -1, $maxlen = NULL) {
    return file_get_contents(self::realpath($uri), $use_include_path, $context, $offset, $maxlen);
  }
}
