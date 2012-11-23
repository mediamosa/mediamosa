<?php
/**
 * @file
 * GD library helper functions.
 */

class mediamosa_gd {
  // ---------------------------------------------------------------- Functions.
  /**
   * Calculate the difference in lumosity.
   *
   * @param integer $r1
   *   Red 1.
   * @param integer $g1
   *   Green 1.
   * @param integer $b1
   *   Blue 1.
   * @param integer $r2
   *   Red 2.
   * @param integer $g2
   *   Green 2.
   * @param integer $b2
   *   Blue 2.
   *
   * @return integer
   *   Difference in color.
   */
  public static function lumdiff($r1, $g1, $b1, $r2, $g2, $b2) {
    $l1 = 0.2126 * pow($r1 / 255, 2.2) + 0.7152 * pow($g1 / 255, 2.2) + 0.0722 * pow($b1 / 255, 2.2);
    $l2 = 0.2126 * pow($r2 / 255, 2.2) + 0.7152 * pow($g2 / 255, 2.2) + 0.0722 * pow($b2 / 255, 2.2);

    return (max($l1, $l2) + 0.05) / (min($l1, $l2) + 0.05);
  }

  /**
   * Calc color difference between each color (rgb).
   *
   * @param integer $r1
   *   Red 1.
   * @param integer $g1
   *   Green 1.
   * @param integer $b1
   *   Blue 1.
   * @param integer $r2
   *   Red 2.
   * @param integer $g2
   *   Green 2.
   * @param integer $b2
   *   Blue 2.
   */
  public static function coldiff($r1, $g1, $b1, $r2, $g2, $b2) {
    return max($r1, $r2) - min($r1, $r2) + max($g1, $g2) - min($g1, $g2) + max($b1, $b2) - min($b1, $b2);
  }

  /**
   * Calculate the difference in color distance.
   *
   * @param integer $r1
   *   Red 1.
   * @param integer $g1
   *   Green 1.
   * @param integer $b1
   *   Blue 1.
   * @param integer $r2
   *   Red 2.
   * @param integer $g2
   *   Green 2.
   * @param integer $b2
   *   Blue 2.
   *
   * @return integer
   *   Difference in color.
   */
  public static function pythdiff($r1, $g1, $b1, $r2, $g2, $b2) {
    $rd = $r1 - $r2;
    $gd = $g1 - $g2;
    $bd = $b1 - $b2;

    return sqrt($rd * $rd + $gd * $gd + $bd * $bd);
  }

  /**
   * Calculate the difference in brightness.
   *
   * @param integer $r1
   *   Red 1.
   * @param integer $g1
   *   Green 1.
   * @param integer $b1
   *   Blue 1.
   * @param integer $r2
   *   Red 2.
   * @param integer $g2
   *   Green 2.
   * @param integer $b2
   *   Blue 2.
   *
   * @return integer
   *   The difference in brightness.
   */
  public static function brghtdiff($r1, $g1, $b1, $r2, $g2, $b2) {
    $br1 = (299 * $r1 + 587 * $g1 + 114 * $b1) / 1000;
    $br2 = (299 * $r2 + 587 * $g2 + 114 * $b2) / 1000;

    return abs($br1 - $br2);
  }

  /**
   * Convert Hex to RGB.
   *
   * @param string $hexcolor
   *   The hexidecimal color to convert.
   *
   * @return array
   *   An array per color (red, green, blue).
   */
  public static function hex2rgb($hexcolor = "") {
    $rgb = array();
    $rgb['red'] = hexdec(mediamosa_unicode::substr($hexcolor, 0, 2));
    $rgb['green'] = hexdec(mediamosa_unicode::substr($hexcolor, 2, 2));
    $rgb['blue'] = hexdec(mediamosa_unicode::substr($hexcolor, 4, 2));

    return $rgb;
  }

  /**
   * Convert RGB to Hex.
   *
   * @param array $rgb
   *   A array holding the red, green and blue color values.
   *
   * @return string
   *   RGB value converted to hexidecimal.
   */
  public static function rgb2hex(array $rgb = array()) {
    return mediamosa_unicode::substr('0' . dechex($rgb['red']), -2) . mediamosa_unicode::substr('0' . dechex($rgb['green']), -2) . mediamosa_unicode::substr('0' . dechex($rgb['blue']), -2);
  }

  /**
   * Wrapper for imagecopymerge() function.
   *
   * @param resource $dest
   *   Destination image link resource.
   * @param resource $src
   *   Source image link resource.
   * @param int $dst_x
   *   The x-coordinate of destination point.
   * @param int $dst_y
   *   The y-coordinate of destination point.
   * @param int $src_x
   *   The x-coordinate of source point.
   * @param int $src_y
   *   The y-coordinate of source point.
   * @param int $src_w
   *   Source width.
   * @param int $src_h
   *   Source height.
   * @param int $pct
   *   The two images will be merged according to pct which can range from 0 to
   *   100. When pct = 0, no action is taken, when 100 this function behaves
   *   identically to imagecopy() for pallete images, while it implements alpha
   *   transparency for true colour images.
   *
   * @return boolean
   *   Returns TRUE on success or FALSE on failure.
   *
   * @link http://www.php.net/imagecopymerge
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagecopymerge($dest, $src, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct) {
    if (!function_exists('imagecopymerge')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagecopymerge($dest, $src, $dst_x, $dst_y, $src_x, $src_y, $src_w, $src_h, $pct);
  }

  /**
   * Get the size of an image.
   *
   * @param string $uri
   *   This parameter specifies the file you wish to retrieve information about.
   *   It can reference a local file or (configuration permitting) a remote file
   *   using one of the supported streams.
   *
   * @param array &$imageinfo
   *   This optional parameter allows you to extract some extended information
   *   from the image file. Currently, this will return the different JPG APP
   *   markers as an associative array. Some programs use these APP markers to
   *   embed text information in images. A very common one is to embed IPTC
   *   information in the APP13 marker. You can use the iptcparse() function to
   *   parse the binary APP13 marker into something readable.
   *
   * @return array|boolean
   *   Returns FALSE on failure or array with numeric keys;
   *  - '0':
   *    Pixel width.
   *  - '1':
   *    Pixel height.
   *  - '2':
   *    Is one of the IMAGETYPE_XXX constants indicating the type of the image.
   *  - '3':
   *    Is a text string with the correct height="yyy" width="xxx" string that
   *    can be used directly in an IMG tag.
   *
   * @link http://www.php.net/getimagesize
   *
   * @throws mediamosa_exception_error().
   */
  public static function getimagesize($uri, array &$imageinfo = array()) {
    if (!function_exists('getimagesize')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @getimagesize(mediamosa_io::realpath($uri), $imageinfo);
  }

  /**
   * Create a new image from file or URL.
   *
   * @param string $uri
   *   A stream wrapper URI or a filesystem path to the JPEG image.
   *
   * @return resource
   *   Image resource identifier on success, FALSE on errors.
   *
   * @link http://www.php.net/imagecreatefromjpeg
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagecreatefromjpeg($uri) {
    if (!function_exists('imagecreatefromjpeg')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagecreatefromjpeg(mediamosa_io::realpath($uri));
  }

  /**
   * Create a new image from file or URL.
   *
   * @param string $uri
   *   A stream wrapper URI or a filesystem path to the PNG image.
   *
   * @return resource
   *   Image resource identifier on success, FALSE on errors.
   *
   * @link http://www.php.net/imagecreatefrompng
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagecreatefrompng($uri) {
    if (!function_exists('imagecreatefrompng')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagecreatefrompng(mediamosa_io::realpath($uri));
  }

  /**
   * Get image width.
   *
   * @param resource $image
   *   The resource identifier.
   *
   * @return int
   *   Return the width of the image or false on errors.
   *
   * @link http://www.php.net/imagesx
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagesx($image) {
    if (!function_exists('imagesx')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagesx($image);
  }

  /**
   * Get image height.
   *
   * @param resource $image
   *   The resource identifier.
   *
   * @return integer|boolean
   *   Return the height of the image or FALSE on errors.
   *
   * @link http://www.php.net/imagesy
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagesy($image) {
    if (!function_exists('imagesy')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagesy($image);
  }

  /**
   * Get the index of the color of a pixel.
   *
   * @param resource $image
   *   The resource identifier.
   * @param integer $x
   *   The x-coordinate of the point on surface.
   * @param integer $y
   *   The y-coordinate of the point on surface.
   *
   * @return int
   *   The index of the color.
   *
   * @link http://www.php.net/imagecolorat
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagecolorat($image, $x, $y) {
    if (!function_exists('imagecolorat')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagecolorat($image, $x, $y);
  }

  /**
   * Destroy an image.
   *
   * @param resource $image
   *   The resource identifier.
   *
   * @return boolean
   *   Returns TRUE on success or FALSE on failure.
   *
   * @link http://www.php.net/imagedestroy
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagedestroy($image) {
    if (!function_exists('imagedestroy')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return @imagedestroy($image);
  }

  /**
   * Output image to browser or file.
   *
   * @param resource $image
   *   The resource object.
   * @param string $uri
   *   A stream wrapper URI or a filesystem path to save the file to. If NULL,
   *   the raw image stream will be outputted directly.
   * @param integer $quality
   *   Quality is optional, and ranges from 0 (worst quality, smaller file) to
   *   100 (best quality, biggest file). The default is the default IJG quality
   *   value (about 75).
   *
   * @return boolean
   *   Returns TRUE on success or FALSE on failure.
   *
   * @link http://php.net/imagejpeg
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagejpeg($image, $uri = NULL, $quality = NULL) {
    if (!function_exists('imagejpeg')) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_MISSING_EXTENSION, array('@extension' => 'GD library for PHP'));
    }

    return imagejpeg($image, (empty($uri) ? NULL : mediamosa_io::realpath($uri)), $quality);
  }

  /**
   * Output a PNG image to either the browser or a file.
   *
   * @param resource $image
   *   The resource object.
   * @param string $uri
   *   A stream wrapper URI or a filesystem path to save the file to. If NULL
   *   the raw image stream will be outputted directly. NULL is invalid if the
   *   quality and filters arguments are not used.
   * @param integer $quality
   *   Compression level: from 0 (no compression) to 9.
   * @param integer $filters
   *   Allows reducing the PNG file size. It is a bitmask field which may be set
   *   to any combination of the PNG_FILTER_XXX constants. PNG_NO_FILTER or
   *   PNG_ALL_FILTERS may also be used to respectively disable or activate all
   *   filters.
   *
   * @return boolean
   *   Returns TRUE on success or FALSE on failure.
   *
   * @link http://php.net/imagepng
   *
   * @throws mediamosa_exception_error().
   */
  public static function imagepng($image, $uri = NULL, $quality = NULL, $filters = NULL) {
    imagepng($image, (empty($uri) ? NULL : mediamosa_io::realpath($uri)), $quality, $filters);
  }

  /**
   * Calculate the aspect ratio.
   *
   * Calculates the image size based on target and source sizes.
   *
   * @param integer $source_width
   *   Width of source.
   * @param integer $source_height
   *   Height of source.
   * @param string $target_size
   *   The size of the target in WIDTHxHEIGHT format.
   * @param integer $h_padding
   *   Horizontal padding.
   * @param integer $v_padding
   *   Vertical padding.
   * @param boolean $use_padding
   *   Use padding parameters.
   *
   * @return array|boolean
   *   An associated array with aspect info;
   *   - width
   *   - height
   *   - h_padding
   *   - v_padding
   */
  public static function calcAspectRatio($source_width, $source_height, $target_size, $h_padding = NULL, $v_padding = NULL, $use_padding = TRUE) {
    // Get target width and height, format is 'WIDTHxHEIGHT'.
    $matches = array();
    if (!preg_match('/(\d+)x(\d+)/', $target_size, $matches)) {
      return FALSE;
    }

    // Get the data.
    $target_width = (int) $matches[1];
    $target_height = (int) $matches[2];

    // None of the width / heights can 0 or lower.
    $target_known = $target_width > 0 && $target_height > 0;
    $source_known = $source_width > 0 && $source_height > 0;

    // When source is unknown, but target is not, then just return the target
    // size.
    if ($target_known && !$source_known) {
      return array(
        'width' => $target_width,
        'height' => $target_height,
        'h_padding' => 0,
        'v_padding' => 0,
      );
    }

    // Both must be known.
    if (!$target_known || !$source_known) {
      return FALSE;
    }

    // If v + h padding are both provided, then we just return the target size
    // with padding.
    if (isset($h_padding) && is_numeric($h_padding) && $h_padding >= 0 && isset($v_padding) && is_numeric($v_padding) && $v_padding >= 0) {
      return array(
        'width' => $target_width,
        'height' => $target_height,
        'h_padding' => (int) $h_padding,
        'v_padding' => (int) $v_padding,
      );
    }

    // Calculate ratio.
    $w_ratio = $source_width / $target_width;
    $h_ratio = $source_height / $target_height;

    // Depending on ratio difference we choose the calculation.
    if ($w_ratio > $h_ratio) {
      // Total size of padding.
      $padding = $target_height - ($source_height / $w_ratio);

      // Single size of padding (must be even).
      $padding = ($padding - ($padding % 4)) / 2;

      // Calculate new height.
      $new_height = (int) floor($target_height - (2 * $padding));

      // Make even.
      $new_height -= $new_height & 1;

      return array(
        'width' => $target_width,
        'height' => $new_height,
        'h_padding' => (int) ($use_padding ? floor($padding) : 0),
        'v_padding' => 0,
      );
    }

    // Total size of padding.
    $padding = $target_width - ($source_width / $h_ratio);

    // Single size of padding (must be even).
    $padding = ($padding - ($padding % 4)) / 2;

    // Calculate new width for target.
    $new_width = (int) floor($target_width - (2 * $padding));

    // Make even.
    $new_width -= $new_width & 1;

    return array(
      'width' => (int) $new_width,
      'height' => (int) $target_height,
      'h_padding' => 0,
      'v_padding' => (int) ($use_padding ? floor($padding) : 0),
    );
  }
}
