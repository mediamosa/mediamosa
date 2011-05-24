<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
    return mediamosa_unicode::substr('0' . dechex($rgb['red']), -2) . mediamosa_unicode::substr('0' . dechex($rgb['green']), -2) . mediamosa_unicode::substr('0' . dechex($rgb['blue']), -2);
  }

  /**
   * Wrapper for imagecopymerge() function.
   *
   * @link http://www.php.net/manual/en/function.imagecopymerge.php
   *
   * @param resource $dest
   *  Destination image link resource.
   * @param resource $src
   *  Source image link resource.
   * @param int $dst_x
   *  x-coordinate of destination point.
   * @param int $dst_y
   *  y-coordinate of destination point.
   * @param int $src_x
   *  x-coordinate of source point.
   * @param int $src_y
   *  y-coordinate of source point.
   * @param int $src_w
   *  Source width.
   * @param int $src_h
   *  Source height.
   * @param int $pct
   *  The two images will be merged according to pct which can range from 0 to
   *  100. When pct = 0, no action is taken, when 100 this function behaves
   *  identically to imagecopy() for pallete images, while it implements alpha
   *  transparency for true colour images.
   */
  static public function imagecopymerge($dest, $src, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct) {
    if (!function_exists('imagecopymerge')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

  	return @imagecopymerge($dest, $src, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct);
  }

  /**
   * Get the size of an image.
   *
   * @link http://www.php.net/manual/en/function.getimagesize.php
   *
   * @param string $filname
   *  This parameter specifies the file you wish to retrieve information about.
   *  It can reference a local file or (configuration permitting) a remote file
   *  using one of the supported streams.
   *
   * @param array &$imageinfo
   *  This optional parameter allows you to extract some extended information
   *  from the image file. Currently, this will return the different JPG APP
   *  markers as an associative array. Some programs use these APP markers to
   *  embed text information in images. A very common one is to embed IPTC
   *  information in the APP13 marker. You can use the iptcparse() function to
   *  parse the binary APP13 marker into something readable.
   */
  static public function getimagesize($filename, array &$imageinfo = array()) {
    if (!function_exists('getimagesize')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @getimagesize($filename, $imageinfo);
  }

  /**
   * Create a new image from file or URL
   *
   * @link http://www.php.net/manual/en/function.imagecreatefromjpeg.php
   *
   * @param filename string
   *  Path to the JPEG image.
   *
   * @return resource
   *  Image resource identifier on success, FALSE on errors.
   */
  static public function imagecreatefromjpeg($filename) {
    if (!function_exists('imagecreatefromjpeg')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagecreatefromjpeg($filename);
  }

  /**
   * Create a new image from file or URL
   *
   * @link http://www.php.net/manual/en/function.imagecreatefrompng.php
   *
   * @param filename string
   *  Path to the PNG image.
   *
   * @return resource
   *  Image resource identifier on success, FALSE on errors.
   */
  static public function imagecreatefrompng($filename) {
    if (!function_exists('imagecreatefrompng')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagecreatefrompng($filename);
  }

  /**
   * Get image width
   *
   * @link http://www.php.net/manual/en/function.imagesx.php
   *
   * @param image resource
   *
   * @return int
   *  Return the width of the image or false on errors.
   */
  static public function imagesx($image) {
    if (!function_exists('imagesx')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagesx($image);
  }

  /**
   * Get image height
   *
   * @link http://www.php.net/manual/en/function.imagesy.php
   *
   * @param image resource
   *
   * @return int
   *  Return the height of the image or false on errors.
   */
  static public function imagesy($image) {
    if (!function_exists('imagesy')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagesy($image);
  }

  /**
   * Get the index of the color of a pixel
   *
   * @link http://www.php.net/manual/en/function.imagecolorat.php
   *
   * @param image resource
   * @param x int
   *  x-coordinate of the point.
   * @param y int
   *  y-coordinate of the point.
   *
   * @return int
   *  The index of the color.
   */
  static public function imagecolorat($image, $x, $y) {
    if (!function_exists('imagecolorat')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagecolorat($image, $x, $y);
  }

  /**
   * Destroy an image
   *
   * @link http://www.php.net/manual/en/function.imagedestroy.php
   *
   * @param image resource
   *
   * @return bool
   *  Returns TRUE on success or FALSE on failure.
   */
  function imagedestroy($image) {
    if (!function_exists('imagedestroy')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagedestroy($image);
  }

  /**
   * Output image to browser or file
   * @link http://php.net/manual/en/function.imagejpeg.php
   * @param image resource
   * @param filename string[optional] <p>
   * The path to save the file to. If not set or &null;, the raw image stream
   * will be outputted directly.
   * </p>
   * <p>
   * To skip this argument in order to provide the
   * quality parameter, use &null;.
   * </p>
   * @param quality int[optional] <p>
   * quality is optional, and ranges from 0 (worst
   * quality, smaller file) to 100 (best quality, biggest file). The
   * default is the default IJG quality value (about 75).
   * </p>
   * @return bool Returns true on success or false on failure.
   */
  static public function imagejpeg($image, $filename = NULL, $quality = NULL) {
    imagejpeg($image, $filename, $quality);
  }

  /**
   * Output a PNG image to either the browser or a file
   * @link http://php.net/manual/en/function.imagepng.php
   * @param image resource
   * @param filename string[optional] <p>
   * The path to save the file to. If not set or &null;, the raw image stream
   * will be outputted directly.
   * </p>
   * <p>
   * &null; is invalid if the quality and
   * filters arguments are not used.
   * </p>
   * @param quality int[optional] <p>
   * Compression level: from 0 (no compression) to 9.
   * </p>
   * @param filters int[optional] <p>
   * Allows reducing the PNG file size. It is a bitmask field which may be
   * set to any combination of the PNG_FILTER_XXX
   * constants. PNG_NO_FILTER or
   * PNG_ALL_FILTERS may also be used to respectively
   * disable or activate all filters.
   * </p>
   * @return bool Returns true on success or false on failure.
   */
  static public function imagepng($image, $filename = NULL, $quality = NULL, $filters = NULL) {
    imagepng($image, $filename, $quality, $filters);
  }
}
