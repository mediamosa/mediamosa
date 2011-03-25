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
  * The REST calls of the REST interface.
  */

/**
 * URI: /rest
 * Rest calls for/of rest interface.
 */
class mediamosa_rest_call_list extends mediamosa_rest_call {

  public function get_var_setup() {
    $var_setup = array(
      mediamosa_rest_call::VARS => array(),
    );

    // Enrich with default REST vars limit / offset.
    $var_setup = self::get_var_setup_range($var_setup);

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $limit = $this->get_param_value(mediamosa_rest_call::LIMIT);
    $offset = $this->get_param_value(mediamosa_rest_call::OFFSET);

    // Get all rest calls.
    $calls = mediamosa_rest::get_rest_calls($limit, $offset);

    foreach ($calls as $call_method) { // Method
      foreach ($call_method as $call_uri) { // Uri
        $mediamosa->add_item($call_uri);
      }
    }
  }
}

/**
 * URI: /rest/#$uri_id
 * Retrieve specific information about an REST call.
 */
class mediamosa_rest_call_get extends mediamosa_rest_call {

  public function get_var_setup() {
    $var_setup = array();

    $var_setup[mediamosa_rest_call::VARS][mediamosa_rest_call::ID] = array(
      mediamosa_rest_call::VAR_TYPE => mediamosa_sdk::TYPE_SERIAL,
      mediamosa_rest_call::VAR_IS_REQUIRED => mediamosa_rest_call::VAR_IS_REQUIRED_YES,
      mediamosa_rest_call::VAR_DESCRIPTION => mediamosa::t('The numeric ID of the REST call.'),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup, FALSE);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get URI.
    $uri_id = $this->get_param_value(mediamosa_rest_call::ID);

    // Get all rest calls.
    $rest_calls = mediamosa_rest::get_rest_calls(0, 0, mediamosa_rest_call::ID, NULL);

    if (!isset($rest_calls[$uri_id])) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_REST_NOT_FOUND, array('@uri_id' => $uri_id));
    }

    // Create class from it.
    $rest_call = new $rest_calls[$uri_id][mediamosa_rest_call::CLASS_NAME]($rest_calls[$uri_id]);

    // Check if override for class selection is here.
    if (method_exists($rest_call, 'get_object_rest_call')) {
      $rest_call = $rest_call->get_object_rest_call($rest_calls[$uri_id]);
    }

    // Turn off so we can call the get_var_setup without problems.
    // Although get_var_setup should not test variables.
    $rest_call->check_for_unspecified = FALSE;

    // Get the var setup for this call.
    $var_setup = $rest_call->get_var_setup();

    // Now include the variables of this REST call.
    $rest_calls[$uri_id][mediamosa_rest_call::VARS] = $var_setup[mediamosa_rest_call::VARS];

    // Add it to the XML output.
    $mediamosa->add_item($rest_calls[$uri_id]);
  }
}
