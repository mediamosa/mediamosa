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
  * Default collection include file.
  */

/**
 * Core MediaMosa core library.
 */
class mediamosa_lib {

  // ------------------------------------------------------------------ Consts.
  const MIME_TYPE_APPLICATION_X_EMPTY = 'application/x-empty';
  const MIME_TYPE_AUDIO = 'audio';

  // ------------------------------------------------------------------ Static Functions.
  /**
   * Purge session.
   *
   * @param string $name
   */
  static public function purge_session($name) {
    if (!empty($_SESSION[$name])) {
      unset($_SESSION[$name]);
    }
  }

  /**
   * Converts a string boolean ('TRUE', 'true', 'FALSE', 'false')
   * to a normal boolean value TRUE/FALSE
   *
   * @param string $str
   * @return boolean
   */
  static public function boolstr2bool($str) {
    if (is_bool($str)) {
      return $str;
    }
    $str = trim(mediamosa_unicode::strtolower($str));
    assert($str == 'true' || $str == 'false' || $str == '');

    return $str == 'true';
  }

  /**
   * Converts a boolean to a string variant  ('TRUE', 'FALSE')
   *
   * @param boolean $boolean
   * @return string
   */
  static public function bool2boolstr($boolean) {
    return $boolean ? 'TRUE' : 'FALSE';
  }

  /**
   * Converts order direction to CQL equivalent.
   * @param string order direction
   * @return string CQL order direction
   */
  static public function order_direction2cql($order_direction) {
    return drupal_strtolower($order_direction) == 'desc' ? '/sort.descending' : '';
  }

  /**
   * Create a name pair list from a string.
   *
   * create_named_array('name1:value1;name2:value2;name3:value3', ';', ':')
   * returns;
   * array('name1' => 'value1', 'name2' => 'value2', 'name3' => 'value3')
   *
   * @param $string
   *  String to convert
   * @param $pair_seperator
   *  Main seperator char.
   * @param $name_value_seperator
   *  Name value sperator.
   */
  static public function create_named_array($string, $pair_seperator = ';', $name_value_seperator = ':') {
    $a_result = array();

    if (!mediamosa_unicode::strlen($string)) {
      return array();
    }

    $a_pairs = explode($pair_seperator, $string);
    foreach ($a_pairs as $pair) {
      if (mediamosa_unicode::strpos($pair, $name_value_seperator) !== FALSE) {
        list($name, $value) = explode($name_value_seperator, $pair, 2);
        $a_result[$name] = $value;
      }
    }

    return $a_result;
  }

  /*
   * Convert an iso8601 date string (YYYY-MM-DD HH:MM:SS) to
   * number of seconds since 01-01-1970.
   *
   */
  static public function iso8601date_to_unix($iso8601_date_str) {
    $a_value = preg_split("/[-:\\s]/", $iso8601_date_str);
    $a_value[0] = (int) $a_value[0];
    $a_value[1] = isset($a_value[1]) && $a_value[1] > 0 ? (int) $a_value[1] : 1;// MM
    $a_value[2] = isset($a_value[2]) && $a_value[2] > 0 ? (int) $a_value[2] : 1;// DD
    $a_value[3] = isset($a_value[3]) ? (int) $a_value[3] : 0;// HH
    $a_value[4] = isset($a_value[4]) ? (int) $a_value[4] : 0;// MM
    $a_value[5] = isset($a_value[5]) ? (int) $a_value[5] : 0;// SS

    // Get current timezone
    $tz = date_default_timezone_get();

    // Set to GMT/UTC so we can convert without timezone problems
    date_default_timezone_set('UTC');

    // Convert to integer
    $time = gmmktime($a_value[3], $a_value[4], $a_value[5], $a_value[1], $a_value[2], $a_value[0]);

    // Back to original timezone, so we dont screw with other time functions
    date_default_timezone_set($tz);

    return $time;
  }

  /**
   * Simple function to correctly convert simplexml to array.
   *
   * @param object $xml
   *
   * @deprecated: Stop using this function, as it will be removed in the future.
   *              Instead use the SimpleXML object directly.
   */
  static public function simplexml2array($xml) {
    if (is_object($xml) && get_class($xml) == 'mediamosa_connector_response') {
      return self::simplexml2array($xml->get());
    }

    $xml_copy = $xml;
    if (is_object($xml) && get_class($xml) == 'SimpleXMLElement') {
      foreach ($xml->attributes() as $key => $value) {
        if ($value) {
          $attributes[$key] = (string)$value;
        }
      }

      $xml = get_object_vars($xml);
    }

    if (is_array($xml)) {
      if (empty($xml)) {
        return (string)$xml_copy;
      }

      $result = array();
      foreach ($xml as $key => $value) {
        $result[$key] = self::simplexml2array($value);
      }

      return $result;
    }

    return (string)$xml;
  }

  /**
   * Basicly the same as simplexml2array, with the exception that it will fix
   * the problem with single and multi items where a single item will end up
   * in a non-indexed array and multi items will.
   *
   * @param object $xml
   *
   * @deprecated: Stop using this function, as it will be removed in the future.
   *              Instead use the SimpleXML object directly.
   */
  static public function responsexml2array($xml) {
    if (!$xml) {
      return $xml;
    }

    if (get_class($xml) == 'mediamosa_connector_response') {
      return self::responsexml2array($xml->get());
    }

    // Convert to nice array.
    $xml = self::simplexml2array($xml);

    if (!isset($xml['items']['item'])) {
      return $xml;
    }

    // Fix the problem with 1 or 2+ items in array.
    if (!isset($xml['items']['item'][0])) {
      $tmp = array(0 => $xml['items']['item']);
      unset($xml['items']);
      $xml['items']['item'] = $tmp;
    }

    // Fix problem with 1 or 2+ mediafiles in array.
    foreach ($xml['items']['item'] as &$item) {
      if (!empty($item['mediafiles']) && !isset($item['mediafiles']['mediafile'][0])) {
        $tmp = array(0 => $item['mediafiles']['mediafile']);
        unset($item['mediafiles']['mediafile']);
        $item['mediafiles']['mediafile'] = $tmp;
      }
    }

    return $xml;
  }

  /**
   * Returns a link to show asset in maintenance.
   *
   * @param string $asset_id
   * @param string $text (should be wrapped with t())
   */
  static public function l_asset($asset_id, $text = '') {
    $text = empty($text) ? $asset_id : $text;
    return l($text, mediamosa_settings::get_url_asset($asset_id));
  }

  /**
   * Returns a link to show collection in maintenance.
   *
   * @param string $coll_id
   * @param string $text
   */
  static public function l_collection($coll_id, $text = '') {
    $text = empty($text) ? $coll_id : $text;
    return l($text, mediamosa_settings::get_url_collection($coll_id));
  }

  /**
   * Adds English ordinal suffix to numbers.
   * @param integer $value
   */
  static public function ordinal($value) {

    if ($value % 100 > 10 && $value % 100 < 14) {
      return $value . 'th';
    }

    switch ($value % 10) {
      case 0:
        return $value . 'th';

      case 1:
        return $value . 'st';

      case 2:
        return $value . 'nd';

      case 3:
        return $value . 'rd';
    }

    // Anything else ends with 'th'.
    return $value . 'th';
  }

  /**
   * Make diff between two text.
   *
   * @param $old
   *   Old text.
   * @param $new
   *   New text.
   *
   * @return string
   *   Differences.
   */
  static public function make_diff($old, $new) {
    $old = explode("\n", $old);
    $new = explode("\n", $new);

    $diff = array_diff($old, $new);

    return implode("\n", $diff);
  }

  /**
   * Validate IP address.
   *
   * @param $ip_address
   *
   * @return TRUE in case of valid IP address.
   */
  static public function is_valid_ip_address($ip_address) {
    // Validate the IP address.
    $regexp = '/^(([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]).){3}([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$/';
    return preg_match($regexp, $ip_address);
  }
}
