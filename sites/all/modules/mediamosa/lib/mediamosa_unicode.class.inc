<?php
/**
 * @file
 * Unicode library is our layer for supporting charsets.
 *
 * MediaMosa only supports UTF8.
 */

class mediamosa_unicode {

  /**
   * Translate special chars to simple version
   * Šíleně žluťoučký will translate to Silene zlutoucky
   * While it will leave untranslatable utf8 chars as original.
   *
   * @param string $string
   */
  public static function clear($string) {
    $result = '';

    // Set local to utf8, but make sure we switch back to original, no idea
    // what effect this might have.
    $old_locale = setlocale(LC_ALL, 0);
    setlocale(LC_ALL, array('en_GB.utf8', 'en_US.utf8'));
    $ascii = iconv('UTF-8', 'ASCII//TRANSLIT', (string)$string);
    setlocale(LC_ALL, $old_locale); // Back to orginal.

    for ($pos = 0; $pos < self::strlen($ascii); $pos++) {
      $result .= $ascii[$pos] == '?' ? mb_substr($string, $pos, 1) : $ascii[$pos];
    }

    return $result;
  }

  /**
   * Returns the correct size of an string
   */
  public static function strlen($string) {
    return mb_strlen($string);
  }

  /**
   * UTF8 version of substr
   * Using drupal function
   */
  public static function substr($string, $start, $length = NULL) {
    return is_null($length) ? mb_substr($string, $start) : mb_substr($string, $start, $length);
  }

  /**
   * Reverse the contents of the string.
   * mb_strrev doesn't exist, so need to make one.
   *
   * @param string $string
   */
  public static function strrev($string) {
    $string_rev = '';
    for ($pos = mb_strlen($string); $pos >= 0; $pos--) {
      $string_rev .= mb_substr($string, $pos, 1);
    }

    return $string_rev;
  }

  /**
   * To lower case
   */
  public static function strtolower($string) {
    return mb_strtolower($string);
  }

  /**
   * To upper case
   */
  public static function strtoupper($string) {
    return mb_strtoupper($string);
  }

  /**
   * Returns pos of found needle
   */
  public static function strpos($string, $needle, $offset = 0) {
    return mb_strpos($string, $needle, $offset);
  }

  /**
   * Returns pos of found needle
   */
  public static function stripos($string, $needle, $offset = 0) {
    return mb_strpos(self::strtolower($string), self::strtolower($needle), $offset);
  }

  /**
   * A simple test if the text is printable without control chars like LF or EOL chars.
   *
   * @param string $string
   */
  public static function printable($string) {
    for ($pos = 0; $pos < mb_strlen($string); $pos++) {
      if (in_array(mb_substr($string, $pos, 1), array("\n", "\t", "\0", "\r"))) {
        return FALSE;
      }
    }

    return TRUE;
  }

  /**
   * Formats a date, using a date type or a custom date format string.
   *
   * @param $timestamp
   *   A UNIX timestamp to format.
   * @param $type
   *   (optional) The format to use, one of:
   *   - 'short', 'medium', or 'long' (the corresponding built-in date formats).
   *   - The name of a date type defined by a module in hook_date_format_types(),
   *     if it's been assigned a format.
   *   - The machine name of an administrator-defined date format.
   *   - 'custom', to use $format.
   *   Defaults to 'medium'.
   * @param $format
   *   (optional) If $type is 'custom', a PHP date format string suitable for
   *   input to date(). Use a backslash to escape ordinary text, so it does not
   *   get interpreted as date format characters.
   * @param $timezone
   *   (optional) Time zone identifier, as described at
   *   http://php.net/manual/en/timezones.php Defaults to the time zone used to
   *   display the page.
   * @param $langcode
   *   (optional) Language code to translate to. Defaults to the language used to
   *   display the page.
   *
   * @return
   *   A translated date string in the requested format.
   */
  public static function format_date($timestamp, $type = 'medium', $format = '', $timezone = NULL, $langcode = NULL) {
    return format_date($timestamp, $type, $format, $timezone, $langcode);
  }

  /**
   * Return an array with any multi-byte value that XML can not handle.
   *
   * Good test for this is reading the XML into the SimpleTest class. Will fail
   * if any of these chars are used.
   *
   * @return array
   *   The invalid characters.
   */
  public static function get_invalid_char_str() {
    static $invalid_char = array();

    if (empty($invalid_char)) {
      for ($x = 0; $x < 32; $x++) {
        if (!in_array($x, array(0x09, 0x0A, 0x0D))) {
          $invalid_char[] = chr($x);
        }
      }
    }

    return $invalid_char;
  }

  /**
   * Although XML only requires < > " ' & to be encoded, we have problems with
   * non-asc values generating invalid XML when using SimpleXML class.
   *
   * Even if we encode them to &#00 values, the SimpleXML class will fail
   * loading the XML. Current fix is now replacing the invalid values with '?'.
   *
   * @param string $text
   *  The value to encode.
   */
  public static function xmlentities($text) {
    // Seems multibyte save because only values 00-31 (without tab, LF, CR) are
    // used.
    return str_replace(self::get_invalid_char_str(), '?', htmlspecialchars($text));
  }
}
