<?php
/**
 * @file
 * In and out file library layer.
 */

class mediamosa_io {

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
   * Returns the absolute local filesystem path of a stream URI. Will return the
   * result with ending slash (/ OR \ windows) if provided uri also ended with
   * slash.
   *
   * Note: Like PHP's realpath(), it will return FALSE when provided URI does
   *       not exists as directory stucture, dirname($uri) must exists.
   *       Also realpath does not return correctly to files that do not exists.
   *       If 'foo.txt' exists and 'foo_link.txt does not, and you do
   *       realpath('foo_link.txt'), it will return 'foo.txt' instead.
   *
   * @param $uri
   *   A stream wrapper URI or a filesystem path, possibly including one or more
   *   symbolic links.
   *
   * @return
   *   The absolute local filesystem path (with no symbolic links), or FALSE on
   *   failure.
   */
  public static function realpath($uri) {
    // Empty uri is always fatal.
    if (empty($uri)) {
      return FALSE;
    }

    // Make sure result will also end with slash when original $uri did also.
    $end = drupal_substr($uri, -1) == '/' ? DIRECTORY_SEPARATOR : '';

    // Keep the filename.
    if (empty($end)) {
      $end = basename($uri);
      if ($end != $uri) {
        $end = DIRECTORY_SEPARATOR . $end;
      }
      $uri = dirname($uri);
    }

    // Get stream wrapper.
    $wrapper = file_stream_wrapper_get_instance_by_uri($uri);
    if ($wrapper) {
      $realpath = rtrim($wrapper->realpath(), '\\/') . $end;
    }
    else {
      $realpath = rtrim(realpath($uri), '\\/') . $end;
    }

    // Make sure it never returns root.
    if ($realpath == DIRECTORY_SEPARATOR || empty($realpath)) {
      throw new mediamosa_exception_program_error("The mediamosa_io::realpath() functions was not able to translate uri '@uri', realpath translated this to: '@realpath'.", array('@uri' => $uri, '@realpath' => $realpath));
    }

    // Done.
    return $realpath;
  }

  /**
   * Returns the absolute URL version of a stream URI.
   *
   * @param $uri
   *   A stream wrapper URI or a filesystem path, possibly including one or more
   *   symbolic links.
   *
   * @return
   *   The absolute URL or FALSE on failure.
   */
  public static function realurl($uri) {
    return file_create_url($uri);
  }

  /**
   * Attempts to create the directory specified by pathname. Pathname must be
   * inside mediamosa-storage:// path.
   *
   * Fails with exception. This overrides the recursive php mkdir
   * as it does not correctly sets chmod recursively. It also uses the
   * sannas mount point as a basis, so it never tries to create a directory
   * outside this tree.
   *
   * @param string $pathname
   *   Path of the directory to create.
   * @param string $recursive
   *   Enable recursive to create the sub maps required to create directory.
   * @param integer $mode
   *   Default mode value for created directory, defaults to mode 0775.
   *
   * @throws mediamosa_exception_error()
   */
  public static function mkdir($uri, $recursive = TRUE, $mode = 0775) {
    // Already exists? Then don't try to create.
    if (file_exists($uri) && is_dir($uri)) {
      return;
    }

    // Not recursive?
    if (!$recursive) {
      if (!drupal_mkdir($uri, $mode)) {
        throw new mediamosa_exception_error(
          mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
          array(
            '@location' => $uri,
            '@reason' => mediamosa::last_php_error_message()
          )
        );
      }

      // Directory created.
      return;
    }

    // Scheme?
    if (strpos($uri, '://') !== FALSE) {
      list($scheme, $path) = explode('://', $uri, 2);

      // Realpath does not return path when parts of the directory do not exist
      // yet.

      // Convert, as realpath can return empty.
      $scheme = self::realpath($scheme . '://');
      if (!$scheme) {
        throw new mediamosa_exception_error(
          mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
          array(
            '@location' => $uri,
            '@reason' => 'Scheme is not known',
          )
        );
      }

      // Rebuild.
      $uri = $scheme . '/' . $path;
    }

    // Check if directory we create is in mediamosa-storage://
    $storage_path = mediamosa_storage::get_realpath();

    // Must start with storage_path.
    if (mediamosa_unicode::substr($uri, 0, mediamosa_unicode::strlen($storage_path)) != $storage_path) {
      throw new mediamosa_exception_error(
        mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
        array(
          '@location' => $uri,
          '@reason' => 'Can not create directory outside the MediaMosa mount pount.',
        )
      );
    }

    // Recursive mkdir.
    $path = '';
    foreach (explode(DIRECTORY_SEPARATOR, trim($uri, DIRECTORY_SEPARATOR)) as $folder) {
      $path .= '/' . $folder;
      if (!file_exists($path)) {
        if (!drupal_mkdir($path, $mode)) {
          throw new mediamosa_exception_error(
            mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
            array(
              '@location' => $path,
              '@reason' => mediamosa::last_php_error_message()
            )
          );
        }
        drupal_chmod($path, 0775);
      }
    }
  }

  /**
   * Deletes a file.
   *
   * @param string uri
   *   A URI or pathname.
   *
   * @return boolean
   *   Return TRUE on success or FALSE with failure.
   */
  public static function unlink($uri) {
    if (!file_exists($uri)) {
      return TRUE;
    }

    return drupal_unlink($uri);
  }

  /**
   * Tells whether the filename is a symbolic link
   *
   * @param string $uri
   *   The uri of the link to test.
   */
  public static function is_link($uri) {
    return is_link(self::realpath($uri));
  }

  /**
   * Clears file status cache.
   */
  public static function clearstatcache() {
    clearstatcache();
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
    // Filesize needs clear cache.
    self::clearstatcache();

    // Convert to filename.
    $filename = self::realpath($uri);
    return file_exists($filename) ? filesize($filename) : FALSE;
  }

  /**
   * Return the filename without extension.
   *
   * @param string $filename
   *   The filename to process.
   *
   * @return string
   *   The filename without extension.
   */
  public static function get_base_filename($filename) {
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE ? $filename : mediamosa_unicode::substr($filename, 0, $last_dot);
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
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE && ($last_dot + 1) != mediamosa_unicode::strlen($filename) ? '' : mediamosa_unicode::substr($filename, $last_dot + 1);
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
    return dirname($path);
  }

  /**
   * Returns trailing name component of path.
   *
   * @param string $path
   *   A path.
   * @param string $suffix
   *  If the name component ends in suffix this will also be cut off (optional).
   *
   * @return
   *   Returns the base name of the given path.
   */
  public static function basename($path, $suffix = '') {
    return basename($path);
  }

  /**
   * Move one file to other file (rename).
   *
   * @param $from_uri
   *   Rename from name.
   * @param $to_uri
   *   Rename to name.
   *
   * @return
   *   Returns TRUE when successful.
   */
  public static function rename($from_uri, $to_uri) {
    $success = FALSE;
    $message = 'cause unknown';

    try {
      // Make sure dest. map exists.
      self::mkdir(dirname($to_uri), TRUE);

      $from_realpath = mediamosa_io::realpath($from_uri);
      $to_realpath = mediamosa_io::realpath($to_uri);

      // Now rename.
      $success = rename($from_realpath, $to_realpath);
    }
    catch (Exception $e) {
      $message = $e->getMessage();
    }

    if (!$success || !file_exists($to_realpath)) {
      self::log("Renaming file '@old_name' to '@new_name' failed, @message.", array('@old_name' => $from_realpath, '@new_name' => $to_realpath, '@message' => $message));
    }
    else {
      self::log("Successfully renamed file '@old_name' to '@new_name'.", array('@old_name' => $from_realpath, '@new_name' => $to_realpath));
    }

    return $success;
  }

  /**
   * Execute an external program
   *
   * @param $str
   */
  public static function exec($command, array &$output = NULL, &$return_var = NULL) {
    self::log("Executing command '@command'.", array('@command' => $command));
    return exec($command, $output, $return_val);
  }

  /**
   * Test if file is exists.
   *
   * @param string $filename
   */
  public static function file_exists($filename, $do_sleep = FALSE) {
    // File exists needs clear cache.
    self::clearstatcache();

    // Sleep, so file_exists may read non-cache.
    if ($do_sleep) {
      sleep(2);
    }

    // Test it.
    return file_exists($filename);
  }

  /**
   * Test if path is a directory.
   *
   * @param string $path
   */
  public static function is_dir($path) {
    return is_dir($path);
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
      $target_path = is_file($target) ? self::dirname($target) : $target;

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
      $target =  (count($link_parts) ? str_repeat('..' . DIRECTORY_SEPARATOR, count($link_parts)) : '.') . implode(DIRECTORY_SEPARATOR, $target_parts) . (is_file($target) ? DIRECTORY_SEPARATOR . self::basename($target) : '');
    }

    return $target;
  }

  /**
   * Create a symbolic link.
   *
   * @param string $target
   *   Target file.
   * @param string $link
   *   Link to file to create.
   */
  public static function symlink($target, $link, $make_relative = TRUE) {
    $target = self::realpath($target);
    $link = self::realpath($link);
    if (empty($target) || empty($link)) {
      self::log('Unable to create symlink (realpath fails); link: @link, target: @target', array('@link' => $link, '@target' => $target));
      return FALSE;
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

    return $result;
  }

  /**
   * Tells whether a file exists and is readable.
   *
   * @param string $filename
   */
  public static function is_readable($filename) {
    return is_readable($filename);
  }


  /**
   * Tells whether the filename is writable.
   *
   * @param string $filename
   */
  public static function is_writable($filename) {
    return is_writable($filename);
  }

  /**
   * Tells whether the filename is a regular file.
   *
   * @param string $filename
   */
  public static function is_file($filename) {
    return is_file($filename);
  }

  /**
   * Sets access and modification time of file.
   *
   * @param string $uri
   *   The uri of the file being touched.
   * @param integer $time
   *   The touch time. If time  is not supplied, the current system time is
   *   used.
   * @param integer $atime
   *   If present, the access time of the given filename is set to the value of
   *   atime. Otherwise, it is set to time.
   */
  public static function touch($uri, $time = NULL, $atime = NULL) {
    if (!isset($time)) {
      $time = time();
    }

    // Touch does not realpath.
    $uri = self::realpath($uri);

    return touch($uri, $time, $atime);
  }

  /**
   * Makes a copy of the file source to dest.
   *
   * @param string $source
   * @param string $dest
   */
  public static function copy($source, $dest) {
    return @copy($source, $dest);
  }

  /**
   * Move the file source to dest.
   *
   * @param string $source
   * @param string $dest
   */
  public static function move($source, $dest) {
    @rename($source, $dest);

    //self::unlink($source);
  }

  /**
   * File scan directory.
   *
   * @param dir
   * @param mask
   * @param options
   * @param depth
   */
  public static function file_scan_directory($dir, $mask, $options = array(), $depth = 0) {
    return file_scan_directory($dir, $mask, $options, $depth);
  }

  /**
   * File get content.
   *
   * @param filename
   */
  public static function file_get_contents($filename) {
    return file_get_contents($filename);
  }

  /**
   * Returns available space on filesystem or disk partition.
   *
   * @param string $directory
   */
  public static function disk_free_space($directory) {
    return disk_free_space($directory);
  }

  /**
   * Returns available space on filesystem or disk partition.
   *
   * @param string $directory
   */
  public static function disk_total_space($directory) {
    return disk_total_space($directory);
  }

  /**
   * Test if given command is install / found.
   *
   * Also returns its default stdio / strerr output in $exec_output
   *
   * @param $command
   *  Command to test.
   * @param $exec_output
   *  Gets the output of stdio/stderr
   * @param $allowed_ret_values
   *  List of return values that indicate command is installed. Most programs
   *  will use exit(0) as indication that it was run. Some use exit(1) or higher
   *  as exit value.
   */
  public static function command_installed($command, &$exec_output, $allowed_ret_values = array(0)) {
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
   */
  public static function disk_usage($directory, $recursive = FALSE) {
    if (is_file($directory)) {
      return filesize($directory);
    }

    $disk_usage = 0;
    if (is_dir($directory)) {
      $dir = dir($directory);
      while (($entry = $dir->read()) !== FALSE) {
        if ($entry == '.' || $entry == '..') {
          continue;
        }

        $entry_path = $directory . '/' . $entry;
        if (is_file($entry_path)) {
          $disk_usage += filesize($entry_path);
        }
        elseif ($recursive && is_dir($entry_path)) {
          $disk_usage += self::disk_usage($entry_path, TRUE);
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
   * Determine Mime type of a file.
   *
   * @param $filename
   *  file to get the mimetype of.
   * @param $default
   *  In case this function cannot determine a mime-type,
   *  $default is returned.
   */
  public static function get_mimetype($filename) {
    $type = NULL;

    // First try the finfo_file() function.
    if (function_exists('finfo_file')) {
      $finfo = finfo_open(FILEINFO_MIME_TYPE);
      $type = finfo_file($finfo, $filename);
      finfo_close($finfo);
    }
    // Step 2: use the "file" command.
    if (!$type || $type == 'application/octet-stream') {
      $file_mime = exec('file -b -L --mime-type ' . escapeshellarg($filename), $output, $return_var);
      if ($return_var == '0' && $file_mime) {
        $type = $file_mime;
      }
    }
    return $type;
  }

  /**
   * Examines a file object and returns appropriate content headers for
   * download.
   *
   * Based on Drupal's version 'file_get_content_headers()'. We also re-use the
   * file_inline_types variable.
   *
   * @param $file
   *   A file object.
   *
   * @return
   *   An associative array of headers.
   *
   * @see file_get_content_headers()
   */
  public static function file_get_content_headers($filename, $mimetype) {
    $content_length = filesize($filename);
    $last_modified = filemtime($filename);
    $filename = mime_header_encode($filename);
    $mimetype = mime_header_encode($mimetype);

    // Serve images, text, and flash content for display rather than download.
    $inline_types = variable_get('file_inline_types', array('^text/', '^image/', 'flash$'));
    $disposition = 'attachment';
    foreach ($inline_types as $inline_type) {
      // Exclamation marks are used as delimiters to avoid escaping slashes.
      if (preg_match('!' . $inline_type . '!', $mimetype)) {
        $disposition = 'inline';
      }
    }

    return array(
      'Content-Type' => $mimetype . '; name="' . $filename . '"',
      'Content-Length' => $content_length,
      'Content-Disposition' => $disposition . '; filename="' . $filename . '"',
      'Cache-Control' => 'private, max-age=' . mediamosa_settings::MEDIA_CACHE_SECOND . ', must-revalidate',
      'Last-Modified' => gmdate(DATE_RFC1123, $last_modified),
      'Expires' => gmdate(DATE_RFC1123, time() + mediamosa_settings::MEDIA_CACHE_SECOND),
    );
  }

  /**
   * Transfer file using HTTP to client. In future we will use scheme wrappers
   * for files.
   *
   * Based on Drupal's file_transfer().
   *
   * @param $file
   *   String specifying the file URI to transfer.
   * @param $headers
   *   An array of HTTP headers to send along with file.
   */
  public static function file_transfer($file, $headers) {
    if (ob_get_level()) {
      ob_end_clean();
    }

    foreach ($headers as $name => $value) {
      drupal_add_http_header($name, $value);
    }
    drupal_send_headers();

    // Transfer file in chunks to save memory usage.
    $fd = fopen($file, 'rb');
    if ($fd) {
      while (!feof($fd)) {
        print fread($fd, mediamosa_settings::DOWNLOAD_CHUNK_SIZE);
      }
      fclose($fd);

      drupal_exit();
    }
  }
}
