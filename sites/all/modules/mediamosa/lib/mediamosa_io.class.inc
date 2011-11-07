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
  static public function log($message, array $variables = array()) {
    mediamosa_debug::log($message, $variables, 'mediamosa_io');
  }

  /**
   * Attempts to create the directory specified by pathname.
   * Fails with exception.
   *
   * @param string $pathname
   *  Path of the directory to create.
   * @param bool $recursive
   *  Enable recursive to create the sub maps required to create directory.
   * @param int $mode
   *  Default mode value for created directory, defaults to mode 0775.
   */
  static public function mkdir($pathname, $recursive = TRUE, $mode = 0775) {

    // Already exists? Then don't try to create.
    if (file_exists($pathname) && is_dir($pathname)) {
      return TRUE; // Return TRUE for unit test.
    }

    if (!drupal_mkdir($pathname, $mode, $recursive)) {
      throw new mediamosa_exception_error(
        mediamosa_error::ERRORCODE_DIR_UNABLE_TO_CREATE,
        array(
          '@location' => $pathname,
          '@reason' => mediamosa::last_php_error_message()
        )
      );
    }
    drupal_chmod($pathname, NULL); // default 0775 to override umask problems.
    
    return TRUE;
  }

  /**
   * Deletes a file.
   *
   * @param string $filename
   *  The file to delete.
   */
  static public function unlink($filename) {
    if (!file_exists($filename)) {
      return TRUE;
    }

    return @unlink($filename);
  }

  /**
   * Clears file status cache.
   */
  static public function clearstatcache() {
    clearstatcache();
  }
  /**
   * Gets file size.
   */
  static public function filesize($filename) {
    // Filesize needs clear cache.
    self::clearstatcache();

    return file_exists($filename) ? filesize($filename) : FALSE;
  }

  /**
   * Return the filename without extension.
   *
   * @param string $filename
   */
  static public function get_base_filename($filename) {
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE ? $filename : mediamosa_unicode::substr($filename, 0, $last_dot);
  }

  /**
   * Return the file extension (excluding the dot, like 'jpg' or 'mp4').
   *
   * @param string $filename
   */
  static public function get_ext($filename) {
    $last_dot = strripos($filename, '.'); // 0-index

    return $last_dot === FALSE && ($last_dot + 1) != mediamosa_unicode::strlen($filename) ? '' : mediamosa_unicode::substr($filename, $last_dot + 1);
  }

  /**
   * Returns directory name component of path.
   *
   * @param string $path
   */
  static public function dirname($path) {
    return dirname($path);
  }

  /**
   * Move one file to other file (rename).
   *
   * @param string $old_name
   * @param string $new_name
   *
   * @return bool
   */
  static public function rename($old_name, $new_name) {
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
  static public function exec($command, array &$output = NULL, &$return_var = NULL) {
    self::log("Executing command '@command'.", array('@command' => $command));
    return exec($command, $output, $return_val);
  }

  /**
   * Test if file is exists.
   *
   * @param string $filename
   */
  static public function file_exists($filename, $do_sleep = FALSE) {
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
  static public function is_link($filename) {
    return is_link($filename);
  }

  /**
   * Test if path is a directory.
   *
   * @param string $path
   */
  static public function is_dir($path) {
    return is_dir($path);
  }

  /**
   * Create a symbolic link.
   *
   * @param string $target
   *  Target file.
   * @param string $link
   *  Link to file to create.
   */
  static public function symlink($target, $link) {
    return symlink($target, $link);
  }

  /**
   * Tells whether a file exists and is readable.
   *
   * @param string $filename
   */
  static public function is_readable($filename) {
    return is_readable($filename);
  }


  /**
   * Tells whether the filename is writable.
   *
   * @param string $filename
   */
  static public function is_writable($filename) {
    return is_writable($filename);
  }

  /**
   * Tells whether the filename is a regular file.
   *
   * @param string $filename
   */
  static public function is_file($filename) {
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
  static public function touch($filename, $time = NULL, $atime = NULL) {
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
  static public function copy($source, $dest) {
    return @copy($source, $dest);
  }

  /**
   * Move the file source to dest.
   *
   * @param string $source
   * @param string $dest
   */
  static public function move($source, $dest) {
    @copy($source, $dest);
    self::unlink($source);
  }

  /**
   * File scan directory.
   *
   * @param dir
   * @param mask
   * @param options
   * @param depth
   */
  static public function file_scan_directory($dir, $mask, $options = array(), $depth = 0) {
    return file_scan_directory($dir, $mask, $options, $depth);
  }

  /**
   * File get content.
   *
   * @param filename
   */
  static public function file_get_contents($filename) {
    return file_get_contents($filename);
  }

  /**
   * Returns available space on filesystem or disk partition.
   *
   * @param string $directory
   */
  static public function disk_free_space($directory) {
    return disk_free_space($directory);
  }

  /**
   * Returns available space on filesystem or disk partition.
   *
   * @param string $directory
   */
  static public function disk_total_space($directory) {
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
  static public function command_installed($command, &$exec_output, $allowed_ret_values = array(0)) {
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
  static public function disk_usage($directory, $recursive = FALSE) {
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
  static public function disk_usage_recursive($directory) {
    return self::disk_usage($directory, TRUE);
  }
}
