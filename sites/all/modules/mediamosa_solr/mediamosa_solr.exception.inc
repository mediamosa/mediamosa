<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Default Exception class.
 */

/**
 * The basic mediamosa_solr_exception class
 */
class mediamosa_solr_exception extends mediamosa_exception {

  const MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_HIGH = 80;
  const MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_NORMAL = 50;
  const MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_LOW = 20;

  public function __construct($msg = '', $severity = self::MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_NORMAL, $log_error = TRUE) {
    $msg = empty($msg) ? sprintf("MediaMosa Solr exception was thrown.") : $msg;

    // Low is not logged by us.
    if ($severity >= self::MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_NORMAL) {
      $log_serverity = $severity >= self::MEDIAMOSA_SOLR_EXCEPTION_SEVERITY_HIGH ? WATCHDOG_CRITICAL : WATCHDOG_NOTICE;

      // Get the trace, if any.
      $backtrace = $this->getTraceAsString();

      if ($log_error) {
        mediamosa_watchdog::log(
          '@message (Exception thrown on line @line, in @file)<br />Partial backtrace: <pre>@backtrace</pre>',
          array(
            '@message' => $msg,
            '@line' => $this->getLine(),
            '@file' => $this->getFile(),
            '@backtrace' => empty($backtrace) ? 'not available' : $backtrace,
          ),
          $log_serverity,
          'MediaMosa Solr'
        );
      }
    }

    parent::__construct($msg);
  }
}
