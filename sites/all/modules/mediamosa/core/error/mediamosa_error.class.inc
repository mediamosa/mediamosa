<?php
/**
 * @file
 * The MediaMosa error class.
 */

class mediamosa_error extends mediamosa_sdk {
  // ---------------------------------------------------------------- Functions.
  /**
   * Get the listing of the error code table.
   *
   * @return array
   *   The error code listing.
   */
  public static function error_code_list($order_by = mediamosa_error_db::CODE, $order_direction = mediamosa_type::ORDER_DIRECTION_ASC, $errorcode = 0) {
    static $codes = NULL;

    if (!isset($codes[$order_by][$order_direction])) {
      $codes[$order_by][$order_direction] = mediamosa_db::db_select(mediamosa_error_db::TABLE_NAME, 'e')
        ->fields('e')
        ->orderBy($order_by, $order_direction)
        ->execute()
        ->fetchAllAssoc($order_by, PDO::FETCH_ASSOC);
    }

    // If errorcode was specified, get that one specific.
    if ($errorcode && $order_by == mediamosa_error_db::CODE) {
      if (isset($codes[$order_by][$order_direction][$errorcode])) {
        return array($errorcode => (array) $codes[$order_by][$order_direction][$errorcode]);
      }

      return array();
    }
    elseif ($errorcode) {
      assert(0);
    }

    return $codes[$order_by][$order_direction];
  }

  /**
   * Find the error code and its textual message.
   *
   * @param integer $errorcode
   *   The mediamosa error code.
   * @param $args
   *   An associative array of replacements to make. Occurrences in $string of
   *   any key in $args are replaced with the corresponding value.
   * @param boolean $return_default
   *   If code can not be found, it either return a default error or boolean
   *   FALSE.
   *
   * @return array|boolean
   *   Either returns an associative array or FALSE depending on argument
   *   $return_default;
   *   - 'message'
   *     The error code message.
   *   - 'code'
   *     The mediamosa error code.
   *
   * @see mediamosa_sdk
   */
  public static function error_code_find($errorcode, $args = array(), $return_default = TRUE) {
    // Get the current listing.
    $code = self::error_code_list(mediamosa_error_db::CODE, mediamosa_type::ORDER_DIRECTION_ASC, $errorcode);

    // Do we find the error?
    if (!isset($code[$errorcode])) {
      if ($return_default) {
        return array(
          mediamosa_error_db::MESSAGE => strtr('Server error; Unable to find error code: @errorcode', array('@errorcode' => $errorcode)),
          mediamosa_error_db::CODE => self::HTTP_INTERNAL_SERVER_ERROR,
        );
      }

      // Return FALSE that we failed to find it.
      return FALSE;
    }

    // Fill message with tags.
    if (!empty($args)) {
      $code[$errorcode][mediamosa_error_db::MESSAGE] = strtr($code[$errorcode][mediamosa_error_db::MESSAGE], $args);
    }

    return $code[$errorcode];
  }

  /**
   * Find the error code and the description.
   *
   * @param integer $errorcode
   *   The mediamosa error code.
   * @param $args
   *   An associative array of replacements to make. Occurrences in $string of
   *   any key in $args are replaced with the corresponding value.
   *
   * @return string
   *   The error code message.
   */
  public static function error_code_find_description($errorcode, $tags = array()) {
    $code = self::error_code_find($errorcode, $tags);

    return ($code ? $code[mediamosa_error_db::MESSAGE] : 'Description unavailable.');
  }

  /**
   * Get errorcode by row id.
   *
   * @param integer $id
   *   The row ID.
   *
   * @return array
   *  The error code row.
   */
  public static function get($id) {
    return mediamosa_db::db_select(mediamosa_error_db::TABLE_NAME, 'e')
      ->fields('e')
      ->condition(mediamosa_error_db::ID, $id)
      ->execute()
      ->fetchAssoc();
  }
}
