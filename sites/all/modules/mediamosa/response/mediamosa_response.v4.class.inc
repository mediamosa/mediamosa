<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
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
 * Response object for usage of output format of 3.0.
 */

/**
 * Our class will reuse as much from version 2 as we can.
 *
 * Warning; v4 output is still experimental.
 */
class mediamosa_response_v4 extends mediamosa_response {

  /**
   * Generate output.
   */
  protected function generate_output() {
    if (!mediamosa::is_v4_output()) {
      return parent::generate_output();
    }

    switch ($this->response_type) {
      case mediamosa_response::RESPONSE_TYPE_XML:
        // Render the template.
        $this->xml();
    }

    // No handlers, fall back on v2 output.
    return parent::generate_output();
  }

  /**
   * Generate the XML output in v4 format.
   *
   * @return string
   */
  public function xml() {
    if (!$this->item_count && $this->response['header']['request_result_id'] == mediamosa_error::ERRORCODE_OKAY && !$this->response_success) {
      self::set_result(self::SUCCESS, mediamosa_error::ERRORCODE_EMPTY_RESULT);
    }

    // Get default template.
    $xml = self::render_template(drupal_get_path('module', 'mediamosa') . '/response/mediamosa_response.v4.tpl.inc');

    // Output the XML.
    header('Connection: close');
    header('Content-Type: text/xml; charset=utf-8');
    header('Content-Length: ' . strlen($xml));
    header('Date: ' . date('r'));
    exit($xml);
  }
}
