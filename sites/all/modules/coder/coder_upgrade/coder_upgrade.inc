<?php
// $Id: coder_upgrade.inc,v 1.7 2010/11/19 18:19:55 solotandem Exp $

/**
 * @file
 * Constants and utility functions for the coder_upgrade module.
 *
 * Copyright 2008-10 by Jim Berry ("solotandem", http://drupal.org/user/240748)
 */

/**
 * The default directory to store modules to be converted.
 * Relative to file_directory_path().
 */
define('DEADWOOD_DIR', 'coder_upgrade');

/**
 * The default directory to store modules to be converted.
 * Relative to file_directory_path().
 */
define('DEADWOOD_OLD', 'coder_upgrade/old');

/**
 * The default directory to store converted modules.
 * Relative to file_directory_path().
 */
define('DEADWOOD_NEW', 'coder_upgrade/new');

/**
 * The default directory to store patch files.
 * Relative to file_directory_path().
 */
define('DEADWOOD_PATCH', 'coder_upgrade/patch');

/**
 * Passes a string through t() and wraps the result in html entity <p>.
 */
function tp($string, $args = array()) {
  return '<p>' . t($string, $args) . '</p>';
}

/**
 * Scans a directory and finds all first-level directories beneath it.
 *
 * TODO Replace this with a call to file_scan_directory in include/files.inc.
 *
 * @param string $path Directory path.
 * @return Array of directory names.
 */
function coder_upgrade_scan_directory($path) {
  static $ignore = array('.', '..', '.svn');
  $dirs = array();

  $path = $path . '/';
  if (!is_dir($path)) {
    return $dirs;
  }
  $files = scandir($path);
  foreach ($files as $file) {
    $file_path = $path . $file;
    if (is_dir($file_path) && !in_array(basename($file_path), $ignore)) {
      $dirs[] = $file;
    }
  }
  return $dirs;
}

/**
 * Removes all files from a directory and optionally removes the directory.
 *
 * @param string $path Directory path.
 */
function coder_upgrade_clean_directory($path, $remove_me = FALSE) {
  $path = $path . '/';
  if (!is_dir($path)) {
    return;
  }
  $files = scandir($path);
  foreach ($files as $file) {
    if ($file != '.' && $file != '..') {
      $file_path = $path . $file;
      if (is_dir($file_path)) {
        coder_upgrade_clean_directory($file_path, TRUE);
      }
      else {
        file_unmanaged_delete($file_path);
      }
    }
  }
  if ($remove_me) {
    rmdir($path);
  }
}

/**
 * Returns full directory path relative to sites directory.
 *
 * @param string $name
 *   Name of the directory.
 * @param boolean $add_slash
 *   Indicates whether to add a trailing slash.
 * @param boolean $stream_format
 *   Indicates whether to use the actual path or a stream protocol.
 * @return string
 *   A string of the directory path.
 */
function coder_upgrade_directory_path($name, $add_slash = TRUE, $stream_format = FALSE) {
  $slash = $add_slash ? '/' : '';
  $prefix_no_slash = $stream_format ? file_default_scheme() . ':/' : file_directory_path();
  $prefix = $prefix_no_slash . '/';

  switch ($name) {
    case 'base':
      return $prefix . variable_get('coder_upgrade_dir', DEADWOOD_DIR) . $slash;
    case 'old':
      return $prefix . variable_get('coder_upgrade_dir_old', DEADWOOD_OLD) . $slash;
    case 'new':
      return $prefix . variable_get('coder_upgrade_dir_new', DEADWOOD_NEW) . $slash;
    case 'patch':
      return $prefix . variable_get('coder_upgrade_dir_patch', DEADWOOD_PATCH) . $slash;
    case '':
      return $prefix_no_slash; // @todo Is this correct with a stream format?
    default:
      return $prefix . $name . $slash;
  }
}

/**
 * Returns the local public directory path.
 *
 * Adapted from function removed from core on 2010-09-01
 * (see http://drupal.org/cvs?commit=415020).
 *
 * @return string
 *   A string containing the directory path of a stream. FALSE is returned if
 *   the scheme is invalid or a wrapper could not be instantiated.
 */
if (!function_exists('file_directory_path')) {
  function file_directory_path($scheme = 'public') {
    global $files_base;

    if (isset($files_base)) {
      // This is being run as a separate process outside of Drupal.
      return $files_base;
    }
    elseif ($wrapper = file_stream_wrapper_get_instance_by_scheme($scheme)) {
      return $wrapper->getDirectoryPath();
    }
    else {
      return FALSE;
    }
  }
}

/*
 * The following functions are declared to allow the code to be executed as a
 * separate process outside of Drupal.
 */

if (!function_exists('drupal_get_path')) {
  function drupal_get_path($type, $name) {
    global $modules_base;
    return $modules_base . "/$name";
  }
}

if (!function_exists('module_load_include')) {
  function module_load_include($type, $module, $name = NULL) {
    if (empty($name)) {
      $name = $module;
    }

    if (function_exists('drupal_get_path')) {
      $file = DRUPAL_ROOT . '/' . drupal_get_path('module', $module) . "/$name.$type";
      if (is_file($file)) {
        require_once $file;
        return $file;
      }
    }
    return FALSE;
  }
}

if (!function_exists('drupal_alter')) {
  function drupal_alter($type, &$data, &$context1 = NULL, &$context2 = NULL) {
    global $_coder_upgrade_upgrade_modules;

    $hook = $type . '_alter';
    foreach ($_coder_upgrade_upgrade_modules as $module) {
      $function = $module . '_' . $hook;
      cdp($function, '$function');
      if (function_exists($function)) {
        $function($data, $context1, $context2);
      }
    }
  }
}

if (!function_exists('file_unmanaged_delete')) {
  function file_unmanaged_delete($path) {
    if (is_dir($path)) {
      return FALSE;
    }
    if (is_file($path)) {
      return unlink($path);
    }
  }
}

if (!function_exists('variable_get')) {
  function variable_get($name, $default = NULL) {
    global $variables;

    return isset($variables[$name]) ? $variables[$name] : $default;
  }
}
