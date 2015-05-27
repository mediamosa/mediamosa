<?php
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
  public static function purge_session($name) {
    if (!empty($_SESSION[$name])) {
      unset($_SESSION[$name]);
    }
  }

  /**
   * Converts a string boolean ('TRUE', 'true', 'FALSE', 'false')
   * to a normal boolean value TRUE/FALSE
   *
   * @param string $str
   * @return bool
   */
  public static function boolstr2bool($str) {
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
   * @param bool $boolean
   * @return string
   */
  public static function bool2boolstr($boolean) {
    return $boolean ? 'TRUE' : 'FALSE';
  }

  /**
   * Converts order direction to CQL equivalent.
   * @param string order direction
   * @return string CQL order direction
   */
  public static function order_direction2cql($order_direction) {
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
  public static function create_named_array($string, $pair_seperator = ';', $name_value_seperator = ':') {
    $result = array();

    // If empty string, then return empty array.
    if ($string === '') {
      return array();
    }

    $pairs = explode($pair_seperator, $string);
    foreach ($pairs as $pair) {
      if (mediamosa_unicode::strpos($pair, $name_value_seperator) !== FALSE) {
        list($name, $value) = explode($name_value_seperator, $pair, 2);
        $result[$name] = $value;
      }
    }

    return $result;
  }

  /*
   * Convert an iso8601 date string (YYYY-MM-DD HH:MM:SS) to
   * number of seconds since 01-01-1970.
   *
   */
  public static function iso8601date_to_unix($iso8601_date_str) {
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
  public static function simplexml2array($xml) {
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
  public static function responsexml2array($xml) {
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
  public static function l_asset($asset_id, $text = '') {
    $text = empty($text) ? $asset_id : $text;
    return l($text, mediamosa_settings::get_url_asset($asset_id));
  }

  /**
   * Returns a link to show collection in maintenance.
   *
   * @param string $coll_id
   * @param string $text
   */
  public static function l_collection($coll_id, $text = '') {
    $text = empty($text) ? $coll_id : $text;
    return l($text, mediamosa_settings::get_url_collection($coll_id));
  }

  /**
   * Adds English ordinal suffix to numbers.
   * @param int $value
   */
  public static function ordinal($value) {

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
  public static function make_diff($old, $new) {
    $old = explode("\n", $old);
    $new = explode("\n", $new);

    $diff = array_diff($old, $new);

    return implode("\n", $diff);
  }

  /**
   * Return the config value in bytes.
   *
   * @param string $val
   *   The string value to convert.
   *
   * @return int
   *   The byte value.
   */
  public static function php_config_value_to_bytes($val) {
    $val = trim($val);

    if (drupal_strlen($val) > 1) {
      $last = strtolower(drupal_substr($val, -1, 1));
      switch ($last) {
        case 'g':
          return drupal_substr($val, 0, -1) * 1073741824;

        case 'm':
          return drupal_substr($val, 0, -1) * 1048576;

        case 'k':
          return drupal_substr($val, 0, -1) * 1024;
      }
    }

    return (int) $val;
  }

  /**
   * Check if the string contains serialized data.
   *
   * @param string $candidate
   *   The string containing the possible serialized data.
   *
   * @return bool
   *   Returns TRUE when serialized, FALSE otherwise.
   */
  public static function is_serialized($candidate) {
    if (!is_string($candidate)) {
      return FALSE;
    }

    $candidate = trim($candidate);
    $unserialized = @unserialize($candidate);

    return ($candidate === 'b:0;' || $unserialized !== FALSE);
  }


}
