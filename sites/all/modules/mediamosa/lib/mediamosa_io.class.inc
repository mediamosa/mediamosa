<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
  * In and out file library layer.
  */

class mediamosa_io {

  /**
   * Log for mediamosa_io.
   *
   * @param string $message
   * @param array $variables
   * @param string $severity
   */
  public static function log($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'mediamosa_io');
  }

  /**
   * Attempts to create the directory specified by pathname.
   *
   * Fails with exception. This overrides the recursive php mkdir
   * as it does not correctly sets chmod recursively. It also uses the
   * sannas mount point as a basis, so it never tries to create a directory
   * outside this tree.
   *
   * @param string $pathname
   *  Path of the directory to create.
   * @param bool $recursive
   *  Enable recursive to create the sub maps required to create directory.
   * @param int $mode
   *  Default mode value for created directory, defaults to mode 0775.
   */
  public static function mkdir($pathname, $recursive = TRUE, $mode = 0775) {
    // Log it.
    mediamosa_debug::log('Creating directory :@dir, recursive: @recursive', array('@dir' => $pathname,  '@recursive' => $recursive ? 'Yes' : 'No'));

    // Already exists? Then don't try to create.
    if (file_exists($pathname) && is_dir($pathname)) {
      return TRUE; // Return TRUE for unit test.
    }

    // first determine sannas mount point.
    $sannas = rtrim(mediamosa_configuration_storage::mount_point_get(), DIRECTORY_SEPARATOR); // remove possible trailing slash.
    $pathname = rtrim($pathname, DIRECTORY_SEPARATOR); // remove possible trailing slash.
    $sannas_depth = count(explode(DIRECTORY_SEPARATOR, $sannas));

    $dirs = explode(DIRECTORY_SEPARATOR, $pathname);
    $count = count($dirs);

    $path = $sannas; // we start with sannas mount point.
    for ($i = $sannas_depth; $i < $count; $i++) {
      $path .= DIRECTORY_SEPARATOR . $dirs[$i];
      if (!is_dir($path)) {
        if (!drupal_mkdir($path)) {
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
    return TRUE;
  }

  /**
   * Deletes a file.
   *
   * @param string $filename
   *  The file to delete.
   */
  public static function unlink($filename) {
    if (!file_exists($filename)) {
      return TRUE;
    }

    return @unlink($filename);
  }

  /**
   * Remove complete style directory with content.
   *
   * @param string $dir
   * @param string $style
   */
  public static function delete_style_directory($dir, $style = NULL) {
    if (!is_dir($dir)) {
      return;
    }

    // Security checks before we do anything.

    // Relative path is not allowed.
    if (drupal_substr($dir, 0, 1) != '/') {
      return;
    }

    $dir_array = explode(DIRECTORY_SEPARATOR, $dir);
    $dir_count = count($dir_array);

    // Directory parts.
    // /SANNAS/still_links/style/style_name/
    // /SANNAS/data/still/style/style_name/
    if ($dir_count < 6) {
      return;
    }

    // It should be /.../style/style_name/.
    if ($dir_array[$dir_count - 3] != mediamosa_settings::STILL_LINK_STYLE_LOCATION && $dir_array[$dir_count - 3] != mediamosa_settings::STILL_DATA_STYLE_LOCATION) {
      return;
    }

    // The last part should be the style name.
    if (!$style || $dir_array[$dir_count - 2] != $style) {
      return;
    }

    $command = 'rm -r ' . escapeshellarg($dir);
    $last_line = mediamosa_io::exec($command, $output, $return_var);
  }

  /**
   * Clears file status cache.
   */
  public static function clearstatcache() {
    clearstatcache();
  }
  /**
   * Gets file size.
   */
  public static function filesize($filename) {
    // Filesize needs clear cache.
    self::clearstatcache();

    return file_exists($filename) ? filesize($filename) : FALSE;
  }

  /**
   * Return the filename without extension.
   *
   * @param string $filename
   */
  public static function get_base_filename($filename) {
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE ? $filename : mediamosa_unicode::substr($filename, 0, $last_dot);
  }

  /**
   * Return the file extension (excluding the dot, like 'jpg' or 'mp4').
   *
   * @param $filename
   */
  public static function get_ext($filename) {
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE && ($last_dot + 1) != mediamosa_unicode::strlen($filename) ? '' : mediamosa_unicode::substr($filename, $last_dot + 1);
  }

  /**
   * Returns directory name component of path.
   *
   * @param $path
   */
  public static function dirname($path) {
    return dirname($path);
  }

  /**
   * Returns trailing name component of path.
   *
   * @param $path
   *   A path.
   * @param $suffix
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
   * @param string $old_name
   * @param string $new_name
   *
   * @return bool
   */
  public static function rename($old_name, $new_name) {
    $success = FALSE;
    $message = 'cause unknown';
    try {
      // Make sure dest. map exists.
      $dirname = dirname($new_name);
      self::mkdir($dirname, TRUE);

      $success = rename($old_name, $new_name);
    }
    catch (Exception $e) {
      $message = $e->getMessage();
    }

    if (!$success) {
      self::log("Renaming file '@old_name' to '@new_name' failed, @message.", array('@old_name' => $old_name, '@new_name' => $new_name, '@message' => $message));
    }
    else {
      self::log("Successfully renamed file '@old_name' to '@new_name'.", array('@old_name' => $old_name, '@new_name' => $new_name));
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
   * Test if file is a symbolic link.
   *
   * @param string $filename
   */
  public static function is_link($filename) {
    return is_link($filename);
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
        // Flag if both path have a common start point.
        $relative_count = FALSE;
        for ($x = 0; $x < count($target_parts) && $x < count($link_parts); $x++) {
          if ($target_parts[$x] != $link_parts[$x]) {
            break;
          }

          unset($link_parts[$x]);
          unset($target_parts[$x]);
        }
      }

      // Rebuild target.
      $target = str_repeat('..' . DIRECTORY_SEPARATOR, count($link_parts)) . implode(DIRECTORY_SEPARATOR, $target_parts) . (is_file($target) ? DIRECTORY_SEPARATOR . self::basename($target) : '');
    }

    return $target;
  }

  /**
   * Create a symbolic link.
   *
   * @param string $target
   *  Target file.
   * @param string $link
   *  Link to file to create.
   */
  public static function symlink($target, $link, $make_relative = TRUE) {
    if ($make_relative) {
      // Make symlinks targets relative.
      $target = self::symlink_absolute2relative($target, $link);
    }

    mediamosa_debug::log('Creating symlink; link: @link, target: @target', array('@link' => $link, '@target' => $target));
    return symlink($target, $link);
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
   * @param string $filename
   *  The name of the file being touched.
   * @param integer $time
   *  The touch time. If time  is not supplied, the current system time is used.
   * @param integer $atime
   *  If present, the access time of the given filename is set to the value of atime.
   *  Otherwise, it is set to time.
   */
  public static function touch($filename, $time = NULL, $atime = NULL) {
    if (!isset($time)) {
      $time = time();
    }

    return @touch($filename, $time, $atime);
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
