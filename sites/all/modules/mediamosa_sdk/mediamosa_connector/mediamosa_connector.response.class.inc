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
 * Helper class to support output format of MediaMosa REST XML.
 */

class mediamosa_connector_response {

  const VERSION_MAJOR = 'major';
  const VERSION_MINOR = 'minor';
  const VERSION_RELEASE = 'release';
  const VERSION_BUILD = 'build';
  const VERSION_INFO = 'info';

  // Store the SimpleXML here.
  protected $xml;

  /**
   * Basic construtor.
   *
   * @param string $data
   */
  public function __construct($data) {

    // Replace the namespaces and translate to 2.x version.
    $data = strtr($data,
      array(
        '<opensearch:itemsPerPage>' => '<item_count>',
        '<opensearch:totalResults>' => '<item_count_total>',
        '<opensearch:startIndex>' => '<item_offset>',
        '</opensearch:itemsPerPage>' => '</item_count>',
        '</opensearch:totalResults>' => '</item_count_total>',
        '</opensearch:startIndex>' => '</item_offset>',
      )
    );

    $this->xml = new SimpleXMLElement($data);

    // Map old header items of 2.x to 3.x.
    if ($this->mediamosa_is_version_3() && isset($this->xml->header->request->result)) {
      $this->xml->header->request_result = $this->xml->header->request->result;
      $this->xml->header->request_result_id = $this->xml->header->request->resultId;
      $this->xml->header->request_result_description = $this->xml->header->request->resultDescription;
    }
  }

  /**
   * Magic function to support -> on object.
   *
   * @param $name
   */
  public function __get($name) {
    if (isset($this->xml->$name)) {
      return $this->xml->$name;
    }

    $trace = debug_backtrace();
    trigger_error('Undefined property via mediamosa_connector_response::__get(): ' . $name . ' in ' . $trace[0]['file'] . ' on line ' . $trace[0]['line'], E_NOTICE);

    return null;
  }

  /**
   * Magic function to support -> on object.
   *
   * @param $name
   */
  public function __isset($name) {
    return isset($this->xml->$name);
  }

  /**
   * Return the SimpleXML object.
   */
  public function get() {
    return $this->xml;
  }

  /**
   * Look if we are version 3 or higher.
   */
  public function mediamosa_is_version_3() {
    $version = $this->mediamosa_get_version();
    return $version[self::VERSION_MAJOR] >= 3;
  }

  /**
   * Use to get version of the MediaMosa server.
   */
  public function mediamosa_get_version() {
    $version = isset($this->xml->header->version) ? $this->xml->header->version : '0.0.0.0';

    list($major, $minor, $release, $build, $info) = preg_split("/[.:-]+/", $version, 5) + array(0 => 1, 1 => 0, 2 => 0, 3 => 1, 4 => '');
    return array(
      self::VERSION_MAJOR => $major,
      self::VERSION_MINOR => $minor,
      self::VERSION_RELEASE => $release,
      self::VERSION_BUILD => $build,
      self::VERSION_INFO => $info,
    );
  }


}
