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
  * The REST calls of the Application Authentication.
  */

/**
 * URI: /login
 * Login onto MediaMosa.
 */
class mediamosa_rest_call_app_ident extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  const DBUS = 'dbus';

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup[mediamosa_rest_call::VARS] = array(
      self::DBUS => array(
        mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
        mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
        mediamosa_rest_call::VAR_DESCRIPTION => 'The DBUS login value.',
      )
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the dbus line.
    $dbus = $this->get_param_value(mediamosa_rest_call_app_ident::DBUS);

    // Login.
    $result = mediamosa_app_authentication::app_dbus_protocol($dbus);

    // Must return OK.
    if (mediamosa_unicode::substr($result, 0, 5) == 'ERROR' || mediamosa_unicode::substr($result, 0, 8) == 'REJECTED') {
      $mediamosa->set_result(mediamosa_response::ERROR, mediamosa_error::ERRORCODE_DBUS_PROTOCOL_ERROR, 'DBUS ' . $result);
    }
    else {
      $mediamosa->set_result(mediamosa_response::SUCCESS, mediamosa_error::ERRORCODE_OKAY, 'DBUS authentication protocol');
    }

    $mediamosa->add_item(array('dbus' => $result));
  }
}

/**
 * URI: /check_cookie (deprecated)
 *      /login/cookie
 *
 * Retrieve cookie information.
 */
class mediamosa_rest_call_check_cookie extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.

  // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Output cookie.
    $mediamosa->add_item($_COOKIE);

    // Ok.
    $mediamosa->set_result_okay();
  }
}
