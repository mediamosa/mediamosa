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
 * Mimetype related functions.
 */
class mediamosa_mimetype {
  // ---------------------------------------------------------------- Functions.
  /**
   * Translate the file extension to a mimetype.
   *
   * @param $extension
   *   The extension to map to mimetype.
   *
   * @return
   *   Returns the found mimetype or application/octet-stream.
   */
  static public function extension2mimetype($extension) {
    // Re-use the Drupal mimetype library.
    include_once DRUPAL_ROOT . '/includes/file.mimetypes.inc';
    $mapping = file_mimetype_mapping();

    if (isset($mapping['extensions'][$extension])) {
      return $mapping['mimetypes'][$mapping['extensions'][$extension]];
    }

    return 'application/octet-stream';
  }

  /**
   * Try to match mime-type to ext.
   *
   * @param $mimetype
   */
  static public function mimetype2extension($mimetype) {
    // Re-use the Drupal mimetype library.
    include_once DRUPAL_ROOT . '/includes/file.mimetypes.inc';
    $mapping = file_mimetype_mapping();

    $ext_id = array_search($mimetype, $mapping['mimetypes']);
    if ($ext_id !== FALSE) {
      return array_search($ext_id, $mapping['extensions']);
    }

    return FALSE;
  }
}
