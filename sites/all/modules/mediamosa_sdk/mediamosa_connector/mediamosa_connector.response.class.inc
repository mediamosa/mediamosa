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
 * Helper class to support output format of MediaMosa REST XML (or other formats
 * in the future).
 */
class mediamosa_connector_response {

  // ------------------------------------------------------------------- Consts.
  const VERSION_MAJOR = 'major';
  const VERSION_MINOR = 'minor';
  const VERSION_RELEASE = 'release';
  const VERSION_BUILD = 'build';
  const VERSION_INFO = 'info';

  // --------------------------------------------------------------------- Vars.
  // Store the SimpleXML here.
  public $xml = NULL;

  // -------------------------------------------------------------- Constructor.
  /**
   * Basic constructor.
   *
   * Provide the data returned from the REST call and its content-type.
   *
   * @param string $data
   *  The raw data returned by the REST call.
   * @param string $content_type
   *  The content type of the returned raw data.
   */
  public function __construct($data = NULL, $content_type = 'text/xml') {
    // $data is not required.
    if (!empty($data)) {
      self::set_response($data, $content_type);
    }
  }

  // ------------------------------------------------------------ Magic methods.
   /**
   * Magic function to support -> on object.
   *
   * @param $name
   */
  public function __get($name) {
    assert($name != 'xml');
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

  // ---------------------------------------------------------------- Functions.
  /**
   * Return the stored XML (if any).
   */
  public function get() {
    return $this->xml;
  }


  /**
   * Set the response using the data string.
   *
   * @param string $data
   *  The data returned with HTTP.
   * @param string $content_type
   *  The content type, like 'text/xml'.
   */
  public function set_response($data, $content_type) {

    // Some might contain ; parts.
    if (strpos($content_type, ';') !== FALSE) {
      list($content_type, $other) = explode(';', $content_type, 2);
    }

    // Trim it.
    $content_type = trim($content_type);

    switch ($content_type) {
      case 'text/xml':
        // For now XML only.
        self::set_response_xml($data);
        break;

      default:
        assert(0); // So its logged.
        break;
    }
  }

  /**
   * Response is XML, create the SimpleXML object from it.
   *
   * @param string $data
   */
  protected function set_response_xml($data) {
    // Replace the namespaces and translate to 2.x version.
    // We need to replace it like this, because SimpleXML does not seem to
    // handle this correctly, and also tries to retrieve the XSL each time.
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

    // Build the simpleXML object from our data.
    $this->xml = new SimpleXMLElement($data);

    // Map new header items of 3.x to 2.x.
    if (!empty($this->xml->header->request)) {
      $this->xml->header->request_result = $this->xml->header->request->result;
      $this->xml->header->request_result_id = $this->xml->header->request->resultId;
      $this->xml->header->request_result_description = $this->xml->header->request->resultDescription;

      // Ok, because of SimpleXML and its terrible workings with namespaces, we
      // will need to redo the opensearch params.
      $this->xml->header->itemsPerPage = $this->xml->header->item_count;
      $this->xml->header->totalResults = $this->xml->header->item_count_total;
      $this->xml->header->startIndex = $this->xml->header->item_offset;
    }
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
      self::VERSION_MAJOR => (int) $major,
      self::VERSION_MINOR => (int) $minor,
      self::VERSION_RELEASE => (int) $release,
      self::VERSION_BUILD => (int) $build,
      self::VERSION_INFO => $info,
    );
  }
}
