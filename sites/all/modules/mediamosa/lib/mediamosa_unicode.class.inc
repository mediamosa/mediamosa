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
 * Unicode library is our layer for supporting charsets. MediaMosa only supports UTF8 for now.
 */
class mediamosa_unicode {

  /**
   * Translate special chars to simple version
   * Šíleně žluťoučký will translate to Silene zlutoucky
   * While it will leave untranslatable utf8 chars as original.
   *
   * @param string $string
   */
  static public function clear($string) {
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
  static public function strlen($string) {
    return mb_strlen($string);
  }

  /**
   * UTF8 version of substr
   * Using drupal function
   */
  static public function substr($string, $start, $length = NULL) {
    return is_null($length) ? mb_substr($string, $start) : mb_substr($string, $start, $length);
  }

  /**
   * Reverse the contents of the string.
   * mb_strrev doesn't exist, so need to make one.
   *
   * @param string $string
   */
  static public function strrev($string) {
    $string_rev = '';
    for ($pos = mb_strlen($string); $pos >= 0; $pos--) {
      $string_rev .= mb_substr($string, $pos, 1);
    }

    return $string_rev;
  }

  /**
   * To lower case
   */
  static public function strtolower($string) {
    return mb_strtolower($string);
  }

  /**
   * To upper case
   */
  static public function strtoupper($string) {
    return mb_strtoupper($string);
  }

  /**
   * Returns pos of found needle
   */
  static public function strpos($string, $needle, $offset = 0) {
    return mb_strpos($string, $needle, $offset);
  }

  /**
   * Returns pos of found needle
   */
  static public function stripos($string, $needle, $offset = 0) {
    return mb_strpos(self::strtolower($string), self::strtolower($needle), $offset);
  }

  /**
   * A simple test if the text is printable without control chars like LF or EOL chars.
   *
   * @param string $string
   */
  static public function printable($string) {
    for ($pos = 0; $pos < mb_strlen($string); $pos++) {
      if (in_array(mb_substr($string, $pos, 1), array("\n", "\t", "\0", "\r"))) {
        return FALSE;
      }
    }

    return TRUE;
  }

  /**
   * Format date
   */
  static public function format_date($timestamp, $type = 'medium', $format = '', $timezone = NULL, $langcode = NULL) {
    return format_date($timestamp, $type, $format, $timezone, $langcode);
  }

  /**
   * Return an array with any multi-byte value that XML can not handle. Good
   * test for this is reading the XML into the SimpleTest class. Will fail if
   * any of these chars are used.
   */
  static public function get_invalid_char_str() {
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
  static public function xmlentities($text) {
    // Seems multibyte save because only values 00-31 (without tab, LF, CR) are
    // used.
    return str_replace(self::get_invalid_char_str(), '?', htmlspecialchars($text));
  }
}
