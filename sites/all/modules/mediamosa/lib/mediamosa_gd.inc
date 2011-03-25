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
 * GD helper functions.
 * (functions seem unused!)
 */

class mediamosa_gd {
  /**
   * Difference in lumosity.
   *
   * @param integer $r1
   * @param integer $g1
   * @param integer $b1
   * @param integer $r2
   * @param integer $g2
   * @param integer $b2
   * @return integer Difference in color
   */
  static public function lumdiff($r1, $g1, $b1, $r2, $g2, $b2) {
      $l1 = 0.2126 * pow($r1/255, 2.2) +
            0.7152 * pow($g1/255, 2.2) +
            0.0722 * pow($b1/255, 2.2);

      $l2 = 0.2126 * pow($r2/255, 2.2) +
            0.7152 * pow($g2/255, 2.2) +
            0.0722 * pow($b2/255, 2.2);

      if ($l1 > $l2) {
          return ($l1+0.05) / ($l2+0.05);
      }
      else {
          return ($l2+0.05) / ($l1+0.05);
      }
  }

  /**
   * Calc color difference between each color (rgb).
   *
   * @param $r1
   * @param $g1
   * @param $b1
   * @param $r2
   * @param $g2
   * @param $b2
   */
  static public function coldiff($r1, $g1, $b1, $r2, $g2, $b2) {
    return max($r1, $r2) - min($r1, $r2) + max($g1, $g2) - min($g1, $g2) + max($b1, $b2) - min($b1, $b2);
  }

  /**
   * Difference in color distance.
   *
   * @param integer $r1
   * @param integer $g1
   * @param integer $b1
   * @param integer $r2
   * @param integer $g2
   * @param integer $b2
   * @return integer Difference in color
   */
  static public function pythdiff($r1, $g1, $b1, $r2, $g2, $b2) {
      $rd = $r1 - $r2;
      $gd = $g1 - $g2;
      $bd = $b1 - $b2;

      return sqrt($rd * $rd + $gd * $gd + $bd * $bd);
  }

  /**
   * Difference in brightness.
   *
   * @param integer $R1
   * @param integer $G1
   * @param integer $B1
   * @param integer $R2
   * @param integer $G2
   * @param integer $B2
   * @return integer Difference in color
   */
  static public function brghtdiff($r1, $g1, $b1, $r2, $g2, $b2) {
      $br1 = (299 * $r1 + 587 * $g1 + 114 * $b1) / 1000;
      $br2 = (299 * $r2 + 587 * $g2 + 114 * $b2) / 1000;

      return abs($br1 - $br2);
  }

  /**
   * Convert Hex to RGB.
   *
   * @param string $hexcolor Hexidecimal color
   * @return array RGB color
   */
  static public function hex2rgb($hexcolor = "") {
    $rgb = array();
    $rgb['red'] = hexdec(mediamosa_unicode::substr($hexcolor, 0, 2));
    $rgb['green'] = hexdec(mediamosa_unicode::substr($hexcolor, 2, 2));
    $rgb['blue'] = hexdec(mediamosa_unicode::substr($hexcolor, 4, 2));

    return $rgb;
  }

  /**
   * Convert RGB to Hex.
   *
   * @param array $rgb Color values
   * @return string RGB value converted to hexidecimal
   */
  static public function rgb2hex($rgb = array()) {
    $hex = "";
    $hex =
      mediamosa_unicode::substr('0' . dechex($rgb['red']), -2) .
      mediamosa_unicode::substr('0' . dechex($rgb['green']), -2) .
      mediamosa_unicode::substr('0' . dechex($rgb['blue']), -2);
    return $hex;
  }
}
