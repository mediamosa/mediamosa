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
  * MediaMosa Exception classes.
  */

define('MEDIAMOSA_EXCEPTION_SEVERITY_HIGH', 80);
define('MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL', 50);
define('MEDIAMOSA_EXCEPTION_SEVERITY_LOW', 20);

/**
 * The basic mediamosa_exception class
 */
class mediamosa_exception extends Exception {

  const MEDIAMOSA_EXCEPTION_SEVERITY_HIGH = 80;
  const MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL = 50;
  const MEDIAMOSA_EXCEPTION_SEVERITY_LOW = 20;

  public function __construct($errorcode, $msg = '', $severity = self::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    $msg = empty($msg) ? sprintf("MediaMosa exception was thrown: %d", $errorcode) : $msg;

    if ($severity >= MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL || mediamosa_debug::is_debug()) {
      $log_serverity = $severity >= MEDIAMOSA_EXCEPTION_SEVERITY_HIGH ? WATCHDOG_CRITICAL : WATCHDOG_NOTICE;

      $levels = watchdog_severity_levels();

      // Get backtrace when you can.
      $backtrace = $this->getTraceAsString();

      if ($log_error) {
        mediamosa_watchdog::log(
          '@message (Exception thrown (severity: @severity) on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
          array(
            '@severity' => $levels[$log_serverity],
            '@message' => $msg,
            '@line' => $this->getLine(),
            '@file' => $this->getFile(),
            '@backtrace' => empty($backtrace) ? 'not available' : $backtrace,
          ),
          $log_serverity
        );
      }
    }

    parent::__construct($msg, (int) $errorcode);
  }

  public function mediamosa_exception_rest_response() {
    mediamosa::get()->set_result(mediamosa_response::ERROR, $this->getCode(), $this->getMessage());
  }
}

/**
 * Throw for override of output. Will log error and redirect to URI/URL and
 * will exit() at end. Will not generate output.
 */
class mediamosa_exception_redirect_and_exit extends mediamosa_exception {
  private $redirect_uri;

  public function __construct($redirect_uri, $errorcode, $msg = '', $severity = self::MEDIAMOSA_EXCEPTION_SEVERITY_LOW) {
    $this->redirect_uri = $redirect_uri;

    parent::__construct($errorcode, $msg, $severity);
  }

  /**
   * Return redirect url.
   */
  public function get_redirect_uri() {
    return $this->redirect_uri;
  }
}

/**
 * Wrapper class.
 */
class mediamosa_exception_error extends mediamosa_exception {
  private $a_error;

  public function __construct($mediamosa_errorcode, $tags = array(), $severity = MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    if (!is_numeric($mediamosa_errorcode) && !is_null(constant($mediamosa_errorcode))) {
      $mediamosa_errorcode = constant($mediamosa_errorcode);
    }

    $this->a_error = mediamosa_error::error_code_find($mediamosa_errorcode, $tags);
    parent::__construct($this->a_error['code'], $this->a_error['message'], $severity, $log_error);
  }

  public function mediamosa_exception_error_array_get() {
    return $this->a_error;
  }
}

/**
 * Wrapper class.
 */
class mediamosa_exception_error_debug extends mediamosa_exception_error {
  public function __construct($mediamosa_errorcode, $tags = array(), $severity = MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL) {
    parent::__construct($mediamosa_errorcode, $tags, $severity, FALSE);
  }
 }

/**
 * Exceptions extended on mediamosa_exception.
 */
class mediamosa_exception_error_unexpected_error extends mediamosa_exception_program_error {
  public function __construct() {
    parent::__construct('Unexpected error!');
  }
}

class mediamosa_exception_program_error extends mediamosa_exception {
  public function __construct($message, $tags = array()) {
    $message = strtr($message, $tags);
    parent::__construct(mediamosa_error::HTTP_INTERNAL_SERVER_ERROR, $message, MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
  }
}

/**
 * Thrown when order by parameter is not allowed
 */
class mediamosa_exception_error_sort_field_error extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@field"]));
    parent::__construct(mediamosa_error::ERRORCODE_SORT_FIELD_ERROR, $tags);
  }
}

class mediamosa_exception_error_invalid_timestamp extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@name"]));
    parent::__construct(mediamosa_error::ERRORCODE_INVALID_TIMESTAMP, $tags);
  }
}

class mediamosa_exception_error_missing_timestamp extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@name"]));
    parent::__construct(mediamosa_error::ERRORCODE_MISSING_TIMESTAMP, $tags);
  }
}

// When access has been denied
class mediamosa_exception_error_access_denied extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_NOT_AUTHORIZED, $tags);
  }
}

// When asset was unappropriate.
class mediamosa_exception_error_is_unappropriate extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_IS_UNAPPROPRIATE, $tags);
  }
}

// When ega authentaction failed
class mediamosa_exception_error_authentication_failure extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_ACCESS_DENIED, $tags);
  }
}

class mediamosa_exception_error_empty_result extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_EMPTY_RESULT, $tags);
  }
}

class mediamosa_exception_error_acl_group_already_exists extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@group"]));
//    assert(isset($tags["@group_type"])); not required at this time
    parent::__construct(mediamosa_error::ERRORCODE_ACL_GROUP_ALREADY_EXISTS, $tags);
  }
}

class mediamosa_exception_error_acl_group_not_found extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@group"]));
    parent::__construct(mediamosa_error::ERRORCODE_ACL_GROUP_NOT_FOUND, $tags);
  }
}

class mediamosa_exception_error_acl_name_not_found extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@hostname"]));
    parent::__construct(mediamosa_error::ERRORCODE_ACL_NAME_NOT_FOUND, $tags);
  }
}

class mediamosa_exception_error_group_not_empty extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@group"]));
    parent::__construct(mediamosa_error::ERRORCODE_GROUP_NOT_EMPTY, $tags);
  }
}

class mediamosa_exception_error_invalid_data_parameter extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags["@parameter"]));
    assert(isset($tags["@data"]));
    parent::__construct(mediamosa_error::ERRORCODE_INVALID_DATA_PARAMETER, $tags);
  }
}

class mediamosa_exception_error_job_could_not_be_removed extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags['@job_id']));
    parent::__construct(mediamosa_error::ERRORCODE_JOB_COULD_NOT_BE_REMOVED, $tags, MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
  }
}

class mediamosa_exception_error_job_not_found extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags['@job_id']));
    parent::__construct(mediamosa_error::ERRORCODE_JOB_NOT_FOUND, $tags);
  }
}

class mediamosa_exception_error_asset_not_found extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    assert(isset($tags['@asset_id']));
    parent::__construct(mediamosa_error::ERRORCODE_ASSET_NOT_FOUND, $tags);
  }
}

class mediamosa_exception_error_mediafile_not_found extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    assert(isset($tags['@mediafile_id']));
    parent::__construct(mediamosa_error::ERRORCODE_MEDIAFILE_NOT_FOUND, $tags);
  }
}

class mediamosa_exception_error_invalid_ticket extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    assert(isset($tags['@ticket_id']));
    parent::__construct(mediamosa_error::ERRORCODE_INVALID_TICKET, $tags);
  }
}

class mediamosa_exception_error_userman_invalid_user extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    assert(isset($tags['@user_id']));
    parent::__construct(mediamosa_error::ERRORCODE_USERMAN_INVALID_USER, $tags);
  }
}

class mediamosa_exception_error_cant_retype_group_must_be_empty extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags['@group']));
    parent::__construct(mediamosa_error::ERRORCODE_CANT_RETYPE_GROUP_MUST_BE_EMPTY, $tags);
  }
}

class mediamosa_exception_error_query_error extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_QUERY_ERROR, $tags, MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
  }
}

class mediamosa_exception_error_job_mediafile_not_found extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags['@mediafile_id']));
    parent::__construct(mediamosa_error::ERRORCODE_JOB_MEDIAFILE_NOT_FOUND, $tags);
  }
}

class mediamosa_exception_error_still_is_not_creatable extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags['@mediafile_id']));
    parent::__construct(mediamosa_error::ERRORCODE_STILL_IS_NOT_CREATABLE, $tags);
  }
}

class mediamosa_exception_error_file_not_found extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags['@filename']));
    parent::__construct(mediamosa_error::ERRORCODE_FILE_NOT_FOUND, $tags);
  }
}

class mediamosa_exception_error_webservice_disabled extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    parent::__construct(mediamosa_error::ERRORCODE_WEBSERVICE_DISABLED, $tags);
  }
}

class mediamosa_exception_error_rest_call_is_disabled extends mediamosa_exception_error {
  public function __construct($tags = array()) {
    assert(isset($tags['@uri']));
    parent::__construct(mediamosa_error::ERRORCODE_REST_CALL_IS_DISABLED, $tags);
  }
}
// Throw when CQL is combined with normal search/order by parameters
class mediamosa_exception_error_cql_exclusive extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::ERRORCODE_CQL_EXCLUSIVE);
  }
}

// Throw when CQL is combined with normal search/order by parameters
class mediamosa_exception_error_cql_error extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    assert(isset($tags['@error']));
    parent::__construct(mediamosa_error::ERRORCODE_CQL_ERROR, $tags);
  }
}

// Throw when master/slave access is disabled for this app.
class mediamosa_exception_error_masterslave_disallowed extends mediamosa_exception_error_debug {
  public function __construct($tags = array()) {
    assert(isset($tags['@acl_id']));
    parent::__construct(mediamosa_error::ERRORCODE_MASTERSLAVE_DISALLOWED, $tags);
  }
}

// Bad request.
class mediamosa_exception_error_400 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_BAD_REQUEST);
  }
}

// Forbidden
class mediamosa_exception_error_403 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_FORBIDDEN);
  }
}

// Not found.
class mediamosa_exception_error_404 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_NOT_FOUND);
  }
}

/**
 * Thrown when error is fatal or can not be handled.
 */
class mediamosa_exception_error_500 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_INTERNAL_SERVER_ERROR);
  }
}

// Not implemented.
class mediamosa_exception_error_501 extends mediamosa_exception_error_debug {
  public function __construct() {
    parent::__construct(mediamosa_error::HTTP_NOT_IMPLEMENTED);
  }
}
