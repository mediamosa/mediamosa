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
 * Image tool functions.
 */
class mediamosa_tool_image {
  /**
   * Mime type: image/tiff
   */
  const MEDIAMOSA_TOOL_IMAGE_MIME_TYPE_TIFF = 'image/tiff';

  /**
   * Mime type: image/*
   */
  const MEDIAMOSA_TOOL_IMAGE_MIME_TYPE_ALL = 'image';

  /**
   * Name / ID of the module.
   */
  const NAME = 'image';

  /**
   * Key value separator.
   */
  const MEDIAMOSA_TOOL_IMAGE_KEY_VALUE_SEPARATOR = ':';

  /**
   * Warning level for ImageMagic.
   */
  const MEDIAMOSA_MAINTENANCE_STATUS_CONFIGURATION_IMAGEMAGIC_WARNING = '6';

  /**
   * Is this my supported mime type.
   *
   * @param string $mime_type
   */
  static public function is_supported($mime_type) {
    return preg_match('#' . self::MEDIAMOSA_TOOL_IMAGE_MIME_TYPE_ALL . '\/.+#', $mime_type);
  }

  /**
   * Transcode the file.
   *
   * @param array $options
   */
  static public function transcode($options) {
    if ($options['file_extension'] == 'swf') {
      // Build the values.
      $execution_string_options = array(
        '@file_location' => dirname($options['location_dest_file']) . DIRECTORY_SEPARATOR . $options['jobserver_job_id'] . DIRECTORY_SEPARATOR,
        '@mediafile_location' => $options['location_source_file'],
        '@mediafile_id' => $options['mediafile_id'],
        '@params' => $options['parameter_string'],
      );

      // Execution string.
      // Go to the subdirectory.
      // Create jpg from image (eg. from tiff).
      // Copy the result to transcode directory.
      // Remove the temporary directory (and files).
      // Create status file.
      return strtr('mkdir @file_location; cd @file_location; convert @mediafile_location @params @mediafile_id.jpg; cp @mediafile_id.jpg ../@mediafile_id.jpg; cd ..; rm @file_location -r; { echo "Errors: none"; echo "Status: done"; echo "Progress: 1.000"; echo "Warnings: none"; } > @mediafile_id.status;', $options);
    }
  }
}
