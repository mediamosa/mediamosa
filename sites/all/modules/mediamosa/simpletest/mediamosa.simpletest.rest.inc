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
  * REST calls specific for use inside the simpletest sandbox.
  */

/**
 * URI: /simpletest/whoami
 *
 * Returns information about current EGA login / session.
 *
 * Is only enabled during simpletest runs.
 */
class mediamosa_rest_call_simpletest_whoami extends mediamosa_rest_call {

  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Return app id.
    $app_id = $this->get_param_value_app();

    // Fill it.
    $item = array(
      'app_id' => $app_id,
      'mediamosa_dbus' => $_SESSION['mediamosa_dbus'],
      'cookie' => $_COOKIE,
      'mediamosa_app' => $_SESSION['mediamosa_app'],
    );

    // Add info to output.
    $mediamosa->add_item($item);
  }
}

/**
 * URI: /simpletest/cleanup
 *
 * Cleans up the simpletest information on this server.
 *
 * Internal Only.
 */
class mediamosa_rest_call_simpletest_cleanup extends mediamosa_rest_call {

  public function get_var_setup() {
    $a_var_setup = array(
      mediamosa_rest_call::VARS => array(),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Copied from simpletest_clean_temporary_directories()
    $count = 0;

    $path = drupal_realpath('public://simpletest');
    if (file_exists($path)) {
      // Clear out the previous verbose files.
      file_unmanaged_delete_recursive('public://simpletest/verbose');

      $files = scandir($path);
      foreach ($files as $file) {
        $path = 'public://simpletest/' . $file;
        if (is_dir($path) && is_numeric($file)) {
          $count++;
        }
      }
    }

    $message = $count > 0 ? format_plural($count, 'Removed 1 temporary directory on @server_name.', 'Removed @count temporary directories on @server_name.') : t('No temporary directories to remove.');

    // Log what we did.
    mediamosa_debug::log($message, array('@server_name' => $_SERVER['SERVER_NAME']));

    // Add to output.
    $mediamosa->add_item(array('temporary_dir' => array('removed' => $count)));
  }
}
