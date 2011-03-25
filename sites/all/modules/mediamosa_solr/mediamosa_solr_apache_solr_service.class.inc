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
 * Our extended classes for Solr connections.
 */

// We need SolrPhpClient class.
if (!class_exists('Apache_Solr_Service')) {
  // If we haven't install properly, the requirements hook will tell us.
  // So ignore if we can't find it here.
  // Installing the mediamosa_solr module will fail if during install the class
  // isn't found either. So don't worry about the problem we are not loading
  // MediamosaApacheSolrService for registry indexing (drupal autoloader).
  if (!file_exists(dirname(__FILE__) . '/3rdparty/SolrPhpClient/Apache/Solr/Service.php')) {
    return;
  }

  include_once dirname(__FILE__) . '/3rdparty/SolrPhpClient/Apache/Solr/Service.php';
}

assert(class_exists('Apache_Solr_Service'));

/**
 * We will extend on the Apache Solr Service just for future enhancements.
 *
 * All specific MediaMosa functions must start with mediamosa (mediamosaSolr..)
 * to prevent double usage on lower class names in future.
 */
class mediamosa_solr_apache_solr_service extends Apache_Solr_Service {

  // ------------------------------------------------------------------- Consts.
  const MEDIAMOSA_SOLR_DEFAULT_HOST = 'solr.mediamosa.local';
  const MEDIAMOSA_SOLR_DEFAULT_PORT = 8983;
  const MEDIAMOSA_SOLR_DEFAULT_PATH = '/solr/';

  // -------------------------------------------------------------- Constructor.
  public function __construct($url = NULL) {

    if (!isset($url)) {
      $url = self::mediamosaGetUrl();
    }

    // Split up into host, port and path.
    $host = self::mediamosaGetHost($url);
    $port = self::mediamosaGetPort($url);
    $path = self::mediamosaGetPath($url);

    parent::__construct($host, $port, $path);
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * We use ^ and ~ for delimeters, we need to filter them out for content
   * (sometimes).
   */

  static public function filterDelimeters($value) {
    return str_replace(array('_'), '', $value);
  }

  /**
   * Filter and escape our value;
   *
   * @param $value
   */
  static public function escapeForFunction($value) {
    return self::escape($value);
  }

  /**
   * Escape a value for special query characters such as ':', '(', ')', '*', '?', etc.
   *
   * Added space escape.
   *
   * @param string $value
   * @return string
   */
  static public function escapeForSpace($value) {
    $pattern = '/(\+|\s|-|&&|\|\||!|\(|\)|\{|}|\[|]|\^|"|~|\*|\?|:|\\\)/';
    $replace = '\\\$1';

    return preg_replace($pattern, $replace, $value);
  }

  /**
   * Return the url for the solr servlet.
   */
  static public function mediamosaGetUrl() {
    $url = variable_get('mediamosa_solr_url', 'http://' . self::MEDIAMOSA_SOLR_DEFAULT_HOST . '/');

    // Parse the url into parts.
    $parse_url = parse_url($url);

    // Path is mandatory.
    if (isset($parse_url['path']) && $parse_url['path'] == '/') {
      unset($parse_url['path']);
    }

    // Add possible missing stuff.
    $parse_url += array(
      'host' => self::MEDIAMOSA_SOLR_DEFAULT_HOST,
      'port' => self::MEDIAMOSA_SOLR_DEFAULT_PORT,
      'path' => self::MEDIAMOSA_SOLR_DEFAULT_PATH,
    );

    // Solr only uses host, port and path.
    return 'http://' . $parse_url['host'] . ':' . $parse_url['port'] . '/' . trim($parse_url['path'], '/') . '/';
  }

  /**
   * Sets the URL in Drupal variable.
   */
  static public function mediamosaSetUrl($url) {
    // Parse the url into parts.
    $parse_url = parse_url($url);

    // Path is mandatory.
    if (isset($parse_url['path']) && $parse_url['path'] == '/') {
      unset($parse_url['path']);
    }

    // Add possible missing stuff.
    $parse_url += array(
      'host' => self::MEDIAMOSA_SOLR_DEFAULT_HOST,
      'port' => self::MEDIAMOSA_SOLR_DEFAULT_PORT,
      'path' => self::MEDIAMOSA_SOLR_DEFAULT_PATH,
    );

    // Solr only uses host, port and path.
    variable_set('mediamosa_solr_url', 'http://' . $parse_url['host'] . ':' . $parse_url['port'] . '/' . trim($parse_url['path'], '/') . '/');
  }

  /**
   * Return the host for the solr servlet.
   */
  static public function mediamosaGetHost($url = NULL) {
    $url =  isset($url) ? $url : self::mediamosaGetUrl();

    // Parse.
    return parse_url($url, PHP_URL_HOST);
  }

  /**
   * Return the port for the solr servlet.
   */
  static public function mediamosaGetPort($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Get me the port.
    $port = (int) parse_url($url, PHP_URL_PORT);

    // Port is optional in the URL.
    return $port > 0 ? $port : self::MEDIAMOSA_SOLR_DEFAULT_PORT;
  }

  /**
   * Return the path for the solr servlet.
   */
  static public function mediamosaGetPath($url = NULL) {
    $url = isset($url) ? $url : self::mediamosaGetUrl();

    // Get me the path.
    $path = trim(parse_url($url, PHP_URL_PATH), '/');

    // Path must start and end with '/'.
    return !empty($path) ? '/' . $path . '/' : self::MEDIAMOSA_SOLR_DEFAULT_PATH;
  }

  /**
   * Get the Solr object.
   *
   * Will ping the first time if it is created the first time.
   *
   * @param string $url
   *
   * @return mediamosa_solr_apache_solr_service
   *
   * @throws mediamosa_solr_exception
   */
  static public function mediamosaGetSolrObject($url = NULL, $reset = FALSE) {
    $mediamosa_apache_solr_service = &drupal_static(__FUNCTION__);

    $url = !empty($url) ? $url : self::mediamosaGetUrl();

    // Put in static for reuse.
    if (empty($mediamosa_apache_solr_service[$url]) || $reset) {
      $mediamosa_apache_solr_service[$url] = new mediamosa_solr_apache_solr_service($url);

      if (empty($mediamosa_apache_solr_service[$url])) {
        throw new mediamosa_solr_exception('Unable to create Solr object');
      }

      // Ping it, will throw exception when failed.
      $pinged = $mediamosa_apache_solr_service[$url]->ping() !== FALSE;

      if (!$pinged) {
        throw new mediamosa_solr_exception('Unable to reach Lucene Solr database');
      }
    }

    // Now return the object.
    return $mediamosa_apache_solr_service[$url];
  }


  /**
   * Ping Solr with either given params or use default.
   *
   * @param string $url
   *
   * @throws mediamosa_solr_exception (when $fatal == TRUE)
   */
  static public function mediamosaPing($url = NULL, $fatal = FALSE) {
    // Get the solr connection.
    try {
      self::mediamosaGetSolrObject($url);
    }
    catch (Exception $e) {
      if ($fatal) {
        throw $e;
      }

      return FALSE;
    }

    return TRUE;
  }
}
