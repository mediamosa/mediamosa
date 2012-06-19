<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
 * Install for statistics.
 */

require_once 'diskspace_used/mediamosa_statistics_diskspace_used.install.inc';
require_once 'file_upload/mediamosa_statistics_file_upload.install.inc';
require_once 'job/mediamosa_statistics_job.install.inc';
require_once 'rest_log/mediamosa_statistics_rest_log.install.inc';
require_once 'search_request/mediamosa_statistics_search_request.install.inc';
require_once 'stream_request/mediamosa_statistics_stream_request.install.inc';
require_once 'streaming_ticket/mediamosa_statistics_streaming_ticket.install.inc';
require_once 'file_helper/mediamosa_statistics_file_helper.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_statistics_install() {
  // Create our table(s).
  _mediamosa_statistics_diskspace_used_install();
  _mediamosa_statistics_file_upload_install();
  _mediamosa_statistics_job_install();
  _mediamosa_statistics_rest_log_install();
  _mediamosa_statistics_search_request_install();
  _mediamosa_statistics_stream_request_install();
  _mediamosa_statistics_streaming_ticket_install();
  _mediamosa_statistics_file_helper_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_statistics_uninstall() {
  // Drop our table(s).
  _mediamosa_statistics_file_helper_uninstall();
  _mediamosa_statistics_streaming_ticket_uninstall();
  _mediamosa_statistics_stream_request_uninstall();
  _mediamosa_statistics_search_request_uninstall();
  _mediamosa_statistics_rest_log_uninstall();
  _mediamosa_statistics_job_uninstall();
  _mediamosa_statistics_file_upload_uninstall();
  _mediamosa_statistics_diskspace_used_uninstall();
}
